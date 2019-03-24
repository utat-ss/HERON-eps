/*
A test for all EPS functionality by simulating sending and receiving CAN messages.
RX and TX are defined from EPS's perspective.
*/

#include <can/can.h>
#include <can/data_protocol.h>
#include <can/ids.h>
#include <uart/uart.h>

#include "../../src/general.h"

// Set to true to simulate OBC's CAN messages
bool sim_obc = false;
// Set to true to print EPS's TX and RX CAN messages
bool print_can_msgs = false;


// Callback function signature to run a command
typedef void(*uart_cmd_fn_t)(void);

// UART-activated command
typedef struct {
    char* description;
    uart_cmd_fn_t fn;
} uart_cmd_t;


void req_eps_hk_fn(void);
void heater_1_0c_fn(void);
void heater_1_100c_fn(void);
void heater_2_0c_fn(void);
void heater_2_100c_fn(void);

// All possible commands
uart_cmd_t all_cmds[] = {
    {
        .description = "Request EPS HK data",
        .fn = req_eps_hk_fn
    },
    {
        .description = "Set heater 1 setpoint = 0C",
        .fn = heater_1_0c_fn
    },
    {
        .description = "Set heater 1 setpoint = 100C",
        .fn = heater_1_100c_fn
    },
    {
        .description = "Set heater 2 setpoint = 0C",
        .fn = heater_2_0c_fn
    },
    {
        .description = "Set heater 2 setpoint = 100C",
        .fn = heater_2_100c_fn
    }
};
// Length of array
const uint8_t all_cmds_len = sizeof(all_cmds) / sizeof(all_cmds[0]);




// Enqueues a message for EPS to receive
void enqueue_rx_msg(uint8_t msg_type, uint8_t field_number, uint32_t raw_data) {
    uint8_t rx_msg[8] = { 0x00 };
    rx_msg[0] = 0;    // TODO
    rx_msg[1] = msg_type;
    rx_msg[2] = field_number;
    rx_msg[3] = (raw_data >> 16) & 0xFF;
    rx_msg[4] = (raw_data >> 8) & 0xFF;
    rx_msg[5] = raw_data & 0xFF;
    enqueue(&can_rx_msg_queue, rx_msg);
}


void print_voltage(uint16_t raw_data) {
    print(" 0x%.3X = %.3f V\n", raw_data, adc_raw_data_to_eps_vol(raw_data));
}

void print_current(uint16_t raw_data) {
    print(" 0x%.3X = %.3f A\n", raw_data, adc_raw_data_to_eps_cur(raw_data));
}

void print_therm_temp(uint16_t raw_data) {
    print(" 0x%.3X = %.2f C\n", raw_data, adc_raw_data_to_therm_temp(raw_data));
}

void print_imu_data(uint16_t raw_data) {
    print(" 0x%.4X\n", raw_data);
}

void process_eps_hk_tx_msg(uint8_t* tx_msg) {
    uint8_t field_num = tx_msg[2];
    uint32_t raw_data =
        (((uint32_t) tx_msg[3]) << 16) |
        (((uint32_t) tx_msg[4]) << 8) |
        ((uint32_t) tx_msg[5]);

    switch (field_num) {
        case CAN_EPS_HK_BB_VOL:
            print("BB Vol:");
            print_voltage(raw_data);
            break;
        case CAN_EPS_HK_BB_CUR:
            print("BB Cur:");
            print_current(raw_data);
            break;
        case CAN_EPS_HK_NY_CUR:
            print("-Y Cur:");
            print_current(raw_data);
            break;
        case CAN_EPS_HK_PX_CUR:
            print("+X Cur:");
            print_current(raw_data);
            break;
        case CAN_EPS_HK_PY_CUR:
            print("+Y Cur:");
            print_current(raw_data);
            break;
        case CAN_EPS_HK_NX_CUR:
            print("-X Cur:");
            print_current(raw_data);
            break;
        case CAN_EPS_HK_BAT_TEMP1:
            print("Bat Temp 1:");
            print_therm_temp(raw_data);
            break;
        case CAN_EPS_HK_BAT_TEMP2:
            print("Bat Temp 2:");
            print_therm_temp(raw_data);
            break;
        case CAN_EPS_HK_BAT_VOL:
            print("Bat Vol:");
            print_voltage(raw_data);
            break;
        case CAN_EPS_HK_BAT_CUR:
            print("Bat Cur:");
            print_current(raw_data);
            break;
        case CAN_EPS_HK_BT_CUR:
            print("BT Cur:");
            print_current(raw_data);
            break;
        case CAN_EPS_HK_BT_VOL:
            print("BT Vol:");
            print_voltage(raw_data);
            break;
        case CAN_EPS_HK_HEAT_SP1:
            print("Heater Setpoint 1:");
            print_therm_temp(raw_data);
            break;
        case CAN_EPS_HK_HEAT_SP2:
            print("Heater Setpoint 2:");
            print_therm_temp(raw_data);
            break;
        case CAN_EPS_HK_IMU_ACC_X:
            print("Acc X:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_ACC_Y:
            print("Acc Y:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_ACC_Z:
            print("Acc Z:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_GYR_X:
            print("Gyr X:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_GYR_Y:
            print("Gyr Y:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_GYR_Z:
            print("Gyr Z:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_MAG_X:
            print("Mag X:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_MAG_Y:
            print("Mag Y:");
            print_imu_data(raw_data);
            break;
        case CAN_EPS_HK_IMU_MAG_Z:
            print("Mag Z:");
            print_imu_data(raw_data);
            break;
        default:
            return;
    }

    uint8_t next_field_num = tx_msg[2] + 1;
    if (next_field_num < CAN_EPS_HK_FIELD_COUNT) {
        enqueue_rx_msg(CAN_EPS_HK, next_field_num, 0);
    }
}


void process_eps_ctrl_tx_msg(uint8_t* tx_msg) {
    uint8_t field_num = tx_msg[2];
    // uint32_t raw_data =
    //     (((uint32_t) tx_msg[3]) << 16) |
    //     (((uint32_t) tx_msg[4]) << 8) |
    //     ((uint32_t) tx_msg[5]);

    switch (field_num) {
        case CAN_EPS_CTRL_HEAT_SP1:
            print("Set heater setpoint 1\n");
            break;
        case CAN_EPS_CTRL_HEAT_SP2:
            print("Set heater setpoint 2\n");
            break;
        default:
            break;
    }
}


// Displays the response that EPS sends back
void sim_send_next_tx_msg(void) {
    uint8_t tx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&can_tx_msg_queue)) {
            return;
        }
        dequeue(&can_tx_msg_queue, tx_msg);
    }

    switch (tx_msg[1]) {
        case CAN_EPS_HK:
            process_eps_hk_tx_msg(tx_msg);
            break;
        case CAN_EPS_CTRL:
            process_eps_ctrl_tx_msg(tx_msg);
        default:
            return;
    }
}

void print_next_tx_msg(void) {
    if (!print_can_msgs) {
        return;
    }

    uint8_t tx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&can_tx_msg_queue)) {
            return;
        }
        peek_queue(&can_tx_msg_queue, tx_msg);
    }

    print("CAN TX: ");
    print_bytes(tx_msg, 8);
}

void print_next_rx_msg(void) {
    if (!print_can_msgs) {
        return;
    }

    uint8_t rx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&can_rx_msg_queue)) {
            return;
        }
        peek_queue(&can_rx_msg_queue, rx_msg);
    }

    print("CAN RX: ");
    print_bytes(rx_msg, 8);
}




void req_eps_hk_fn(void) {
    enqueue_rx_msg(CAN_EPS_HK, 0, 0);
}

void heater_1_0c_fn(void) {
    double res = therm_temp_to_res(0);
    double vol = therm_res_to_vol(res);
    uint16_t raw_data = dac_vol_to_raw_data(vol);
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_HEAT_SP1, raw_data);
}

void heater_1_100c_fn(void) {
    double res = therm_temp_to_res(100);
    double vol = therm_res_to_vol(res);
    uint16_t raw_data = dac_vol_to_raw_data(vol);
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_HEAT_SP1, raw_data);
}

void heater_2_0c_fn(void) {
    double res = therm_temp_to_res(0);
    double vol = therm_res_to_vol(res);
    uint16_t raw_data = dac_vol_to_raw_data(vol);
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_HEAT_SP2, raw_data);
}

void heater_2_100c_fn(void) {
    double res = therm_temp_to_res(100);
    double vol = therm_res_to_vol(res);
    uint16_t raw_data = dac_vol_to_raw_data(vol);
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_HEAT_SP2, raw_data);
}




void print_cmds(void) {
    for (uint8_t i = 0; i < all_cmds_len; i++) {
        print("%u: %s\n", i, all_cmds[i].description);
    }
}

uint8_t uart_cb(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        return 0;
    }

    // Print the typed character
    print("%c\n", data[0]);

    // Check for printing the help menu
    if (data[0] == 'h') {
        print_cmds();
    }

    // Check for a valid command number
    else if ('0' <= data[0] && data[0] < '0' + all_cmds_len) {
        // Enqueue the selected command
        uint8_t i = data[0] - '0';
        all_cmds[i].fn();
    }

    else {
        print("Invalid command\n");
    }

    // Processed 1 character
    return 1;
}

int main(void) {
    init_eps();

    print("\n\n\nStarting commands test\n\n");

    // Change these as necessary for testing
    sim_local_actions = false;
    sim_obc = true;
    print_can_msgs = true;

    print("sim_local_actions = %u\n", sim_local_actions);
    print("sim_obc = %u\n", sim_obc);
    print("print_can_msgs = %u\n", print_can_msgs);

    print("At any time, press h to show the command menu\n");
    print_cmds();
    set_uart_rx_cb(uart_cb);

    while(1) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_tx_msg();
            if (sim_obc) {
                sim_send_next_tx_msg();
            } else {
                send_next_tx_msg();
            }
        }

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_rx_msg();
            process_next_rx_msg();
        }
    }

    return 0;
}