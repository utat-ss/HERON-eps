#include "can_interface.h"


/* CAN Interrupts */

// MOB 0
void status_rx_callback(const uint8_t* data, uint8_t len) {
    print("MOB 0: Status RX Callback\n");
    print("Received Message:\n");
    print_bytes((uint8_t *) data, len);
}

// MOB 1
void status_tx_callback(uint8_t* data, uint8_t* len) {
    print("MOB 1: Status TX Callback\n");
}

// MOB 2
void cmd_tx_callback(uint8_t* data, uint8_t* len) {
    print("MOB 2: CMD TX Callback\n");
}

// MOB 4
// CMD RX - received commands
void cmd_rx_callback(const uint8_t* data, uint8_t len) {
    print("\n\nMOB 4: CMD RX Callback\n");
    print_bytes((uint8_t *) data, len);

    if (len == 0) {
        print("Received empty message\n");
    }

    // If the RX message exists, add it to the queue of received messages to process
    else {
		if (data[1] == CAN_EPS_HK) {
	        enqueue(&can_rx_msgs, (uint8_t *) data);
	        print("Enqueued RX\n");
		}
    }
}

// MOB 5
// DATA TX - transmitting data
void data_tx_callback(uint8_t* data, uint8_t* len) {
    print("\n\nMOB 5: Data TX Callback\n");

    if (queue_empty(&can_tx_msgs)) {
        *len = 0;
        print("No message to transmit\n");
    }

    // If there is a message in the TX queue, transmit it
    else {
        dequeue(&can_tx_msgs, data);
        *len = 8;

        print("Dequeued TX\n");
        print("Transmitting Message:\n");
        print_bytes(data, *len);
    }
}




// CAN MOBs

mob_t status_rx_mob = {
	.mob_num = 0,
	.mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = PAY_STATUS_RX_MOB_ID,
	.id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,

    .rx_cb = status_rx_callback
};

mob_t status_tx_mob = {
    .mob_num = 1,
	.mob_type = TX_MOB,
    .id_tag = PAY_STATUS_TX_MOB_ID,
    .ctrl = default_tx_ctrl,

    .tx_data_cb = status_tx_callback
};

mob_t cmd_tx_mob = {
	.mob_num = 2,
	.mob_type = TX_MOB,
	.id_tag = PAY_CMD_TX_MOB_ID,
	.ctrl = default_tx_ctrl,

	.tx_data_cb = cmd_tx_callback
};

mob_t cmd_rx_mob = {
	.mob_num = 4,
	.mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = EPS_CMD_RX_MOB_ID,
	.id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,

    .rx_cb = cmd_rx_callback
};

mob_t data_tx_mob = {
    .mob_num = 5,
	.mob_type = TX_MOB,
    .id_tag = EPS_DATA_TX_MOB_ID,
    .ctrl = default_tx_ctrl,

    .tx_data_cb = data_tx_callback
};
