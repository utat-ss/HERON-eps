/*
Test heaters by typing in commands over UART to manually turn on and off heaters.
Also display thermistor measurements.
*/

#include <adc/adc.h>
#include <uart/uart.h>

#include "../../src/devices.h"
#include "../../src/heaters.h"
#include "../../src/measurements.h"


// Callback function signature to run a command
typedef void(*uart_cmd_fn_t)(void);

// UART-activated command
typedef struct {
    char* description;
    uart_cmd_fn_t fn;
} uart_cmd_t;

void read_thermistor_data_fn(void);
void turn_heater_1_on_fn(void);
void turn_heater_1_off_fn(void);
void set_heater_1_mid_fn(void);
void turn_heater_2_on_fn(void);
void turn_heater_2_off_fn(void);
void set_heater_2_mid_fn(void);


// All possible commands
uart_cmd_t all_cmds[] = {
    {
        .description = "Read thermistor data",
        .fn = read_thermistor_data_fn
    },
    {
        .description = "Turn heater 1 on",
        .fn = turn_heater_1_on_fn
    },
    {
        .description = "Turn heater 1 off",
        .fn = turn_heater_1_off_fn
    },
    {
        .description = "Set heater 1 middle setpoint",
        .fn = set_heater_1_mid_fn
    },
    {
        .description = "Turn heater 2 on",
        .fn = turn_heater_2_on_fn
    },
    {
        .description = "Turn heater 2 off",
        .fn = turn_heater_2_off_fn
    },
    {
        .description = "Set heater 2 middle setpoint",
        .fn = set_heater_2_mid_fn
    }
};
// Length of array
const uint8_t all_cmds_len = sizeof(all_cmds) / sizeof(all_cmds[0]);

// Modify this array to contain the ADC channels you want to monitor
// (channels 10 and 11 are something else - motor positioning sensors)
uint8_t adc_channels[] = { MEAS_THERM_1, MEAS_THERM_2 };
// uint8_t adc_channels[] = {0, 1, 2};
// automatically calculate the length of the array
uint8_t adc_channels_len = sizeof(adc_channels) / sizeof(adc_channels[0]);




void read_thermistor_data_fn(void) {
    print("\n");
    print("Channel, Raw (12 bits), Voltage (V), Resistance (kohms), Temperature (C)\n");

    //Find resistance for each channel
    //only calculate it for the thermistors specified in adc_channels
    for (uint8_t i = 0; i < adc_channels_len; i++) {
        // Read ADC channel data
        uint8_t channel = adc_channels[i];
        fetch_adc_channel(&adc, channel);
        uint16_t raw_data = read_adc_channel(&adc, channel);

        double voltage = adc_raw_data_to_raw_vol(raw_data);
        //Convert adc voltage to resistance of thermistor
        double resistance = therm_vol_to_res(voltage);
        //Convert resistance to temperature of thermistor
        double temperature = adc_raw_data_to_therm_temp(raw_data);

        print("%u: 0x%.3X, %.3f, %.3f, %.3f\n", channel, raw_data, voltage, resistance, temperature);
    }
}

void turn_heater_1_on_fn(void) {
    set_heater_1_temp_setpoint(100);
    print("Set heater 1 setpoint (DAC A) = 100 C\n");
    print("Heater 1 should be ON\n");
}

void turn_heater_1_off_fn(void) {
    set_heater_1_temp_setpoint(0);
    print("Set heater 1 setpoint (DAC A) = 0 C\n");
    print("Heater 1 should be OFF\n");
}

void set_heater_1_mid_fn(void) {
    set_heater_1_temp_setpoint(28);
    print("Set heater 1 setpoint (DAC A) = 28 C\n");
}

void turn_heater_2_on_fn(void) {
    set_heater_2_temp_setpoint(100);
    print("Set heater 2 setpoint (DAC B) = 100 C\n");
    print("Heater 2 should be ON\n");
}

void turn_heater_2_off_fn(void) {
    set_heater_2_temp_setpoint(0);
    print("Set heater 2 setpoint (DAC B) = 0 C\n");
    print("Heater 2 should be OFF\n");
}

void set_heater_2_mid_fn(void) {
    set_heater_2_temp_setpoint(28);
    print("Set heater 2 setpoint (DAC B) = 28 C\n");
}



void print_cmds(void) {
    print("\n");
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
        print_cmds();
    }

    else {
        print("Invalid command\n");
    }

    // Processed 1 character
    return 1;
}

int main(void) {
    init_uart();

    // Set the IMU CSn (PD0) high (because it doesn't have a pullup resistor)
    // so it doesn't interfere with MISO line
    init_cs(PD0, &DDRD);
    set_cs_high(PD0, &PORTD);

    init_spi();
    init_dac(&dac);
    init_adc(&adc);

    print("\n\n\nStarting test\n\n");

    turn_heater_1_off_fn();
    turn_heater_2_off_fn();

    print("\nAt any time, press h to show the command menu\n");
    print_cmds();
    set_uart_rx_cb(uart_cb);

    while (1) {}

    return 0;
}
