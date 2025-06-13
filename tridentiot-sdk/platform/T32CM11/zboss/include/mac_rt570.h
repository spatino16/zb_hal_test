/* ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2020 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
 * All rights reserved.
 *
 * This is unpublished proprietary source code of DSR Corporation
 * The copyright notice does not evidence any actual or intended
 * publication of such source code.
 *
 * ZBOSS is a registered trademark of Data Storage Research LLC d/b/a DSR
 * Corporation
 *
 * Commercial Usage
 * Licensees holding valid DSR Commercial licenses may use
 * this file in accordance with the DSR Commercial License
 * Agreement provided with the Software or, alternatively, in accordance
 * with the terms contained in a written agreement between you and
 * DSR.
 */
/* PURPOSE:
 */
#ifndef MAC_RT570_H
#define MAC_RT570_H

#define ZB_TRANSCEIVER_START_CHANNEL_NUMBER 11
#define ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER   26

#include "radio.h"

extern rfb_zb_ctrl_t *rfb_ctrl;

void RT570_15_4_Prepare(void);
void RT570_15_4_set_auto_ack(uint8_t ack_enable);
void mac_rt570_hw_init(void);

#define ZB_TRANSCEIVER_INIT_RADIO() // JJ mac_rt570_hw_init()


#define ZB_MAC_RX_FLUSH()

#define ZB_MAC_SET_TRANS_INT()
#define ZB_MAC_CLEAR_TRANS_INT()
#define ZB_MAC_GET_TRANS_INT_FLAG() 0

#define ZB_MAC_READ_INT_STATUS_REG()


#define ZB_MAC_GET_RX_INT_STATUS_BIT() zb_mac_get_int_status(0)

#define ZB_MAC_GET_TX_INT_STATUS_BIT() zb_mac_get_int_status(1)

#define ZB_MAC_CLEAR_RX_INT_STATUS_BIT()  \
        {                                 \
            TRANS_CTX().int_status_0 = 0; \
        }

#define ZB_MAC_CLEAR_TX_INT_STATUS_BIT()  \
        {                                 \
            TRANS_CTX().int_status_1 = 0; \
            TRANS_CTX().tx_status    = 0; \
        }

#define ZB_MAC_SET_RX_INT_STATUS_BIT() (TRANS_CTX().int_status_0 = 1)
#define ZB_MAC_SET_TX_INT_STATUS_BIT() (TRANS_CTX().int_status_1 = 1)

// MAC errors from Rafael
#define TX_SEND_SUCCESS              (0x00)     /**< response by HW. */
#define TX_SEND_CHANNEL_BUSY         (0x10)     /**< response by HW. */
#define TX_SEND_NO_ACK_RECEIVED      (0x20)     /**< response by HW. */
#define TX_FRAME_PENDING             (0x40)     /**< response by HW.*/
#define TX_SEND_SUCCESS_ACK_RECEIVED (0x80)     /**< response by HW.*/

// MAC errors sent to ZBOSS
#define MAC_TX_SUCCESS            (0)
#define MAC_TX_UNKNOWN_ERROR      (1)
#define MAC_TX_CHANNEL_BUSY_ERROR (2)
#define MAC_TX_NO_ACK_ERROR       (3)

// TODO: add in more status details other than busy or not busy - Arch 02/21/2024
// TODO: added more status detail. These numbers need defines - LCD 5/2/2024
#define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() (TRANS_CTX().tx_status == MAC_TX_CHANNEL_BUSY_ERROR)
// #define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() TRANS_CTX().tx_status
// #define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() (TRANS_CTX().tx_status  == ZB_TRANS_CHANNEL_BUSY_ERROR)
#define ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR() 0

// TODO: add in more status details other than busy or not busy - Arch 02/21/2024
// TODO: added more status detail. These numbers need defines - LCD 5/2/2024
#define ZB_TRANS_CHECK_NO_ACK() (TRANS_CTX().tx_status == MAC_TX_NO_ACK_ERROR)
// #define ZB_TRANS_CHECK_NO_ACK() 0
// #define ZB_TRANS_CHECK_NO_ACK() (TRANS_CTX().tx_status == ZB_TRANS_NO_ACK)

/* possibly, need further research for TI */
#define ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(addr)
#define ZB_TRANSCEIVER_SET_COORD_EXT_ADDR(addr)
#define ZB_TRANSCEIVER_SET_PAN_COORDINATOR(pan_coord)

zb_uint32_t zb_mac_get_int_status(zb_uint8_t int_num);

void mac_rt570_clear_pending_bit(void);
void mac_rt570_set_pending_bit(void);
zb_bool_t mac_rt570_pending_bit(void);

#ifndef ZB_MAC_SOFTWARE_PB_MATCHING

#define ZB_MAC_TRANS_CLEAR_PENDING_BIT()
#define ZB_MAC_TRANS_SET_PENDING_BIT()

#else /* !defined ZB_MAC_SOFTWARE_PB_MATCHING */

#define ZB_MAC_TRANS_CLEAR_PENDING_BIT() mac_rt570_clear_pending_bit()
#define ZB_MAC_TRANS_SET_PENDING_BIT()   mac_rt570_set_pending_bit()

#endif /* ZB_MAC_SOFTWARE_PB_MATCHING */

#define ZB_MAC_TRANS_PENDING_BIT() mac_rt570_pending_bit()


void zb_transceiver_set_short_addr(zb_uint16_t addr);

#define ZB_TRANSCEIVER_UPDATE_SHORT_ADDR() zb_transceiver_set_short_addr(MAC_PIB().mac_short_address)

void zb_transceiver_set_pan_id(zb_uint16_t pan_id);

#define ZB_TRANSCEIVER_SET_PAN_ID(pan_id) zb_transceiver_set_pan_id(pan_id)

#define ZB_TRANSCEIVER_UPDATE_PAN_ID()    ZB_TRANSCEIVER_SET_PAN_ID(MAC_PIB().mac_pan_id)

/* TODO: reimplement */
#define ZB_RADIO_INT_DISABLE() ZB_OSIF_GLOBAL_LOCK()
#define ZB_RADIO_INT_ENABLE()  ZB_OSIF_GLOBAL_UNLOCK()

void zb_transceiver_update_long_mac(void);

#define ZB_TRANSCEIVER_UPDATE_LONGMAC() \
        zb_transceiver_update_long_mac()

void zb_transceiver_get_rssi(zb_uint8_t *rssi_value);

#define ZB_TRANSCEIVER_GET_RSSI         zb_transceiver_get_rssi
#define ZB_TRANSCEIVER_GET_ENERGY_LEVEL ZB_TRANSCEIVER_GET_RSSI


void zb_transceiver_start_get_rssi(void);

#define ZB_TRANSCEIVER_START_GET_RSSI(_scan_duration_bi) zb_transceiver_start_get_rssi();

zb_ret_t zb_transceiver_set_channel(zb_uint8_t channel_number);

#define ZB_TRANSCEIVER_SET_CHANNEL(page, channel_number) zb_transceiver_set_channel(channel_number)

void mac_rt570_send_packet(zb_bufid_t buf,
                           zb_uint8_t wait_type);

#define ZB_TRANS_SEND_FRAME(header_length, buf, wait_type) \
        (ZB_DUMP_OUTGOING_DATA(buf),                       \
         (void)header_length, mac_rt570_send_packet((buf), (wait_type)))


// #define ZB_TRANS_REPEAT_SEND_FRAME(header_length, buf, wait_type)
// TODO: based on efr32 implementation, test - Arch 02/21/2024
#define ZB_TRANS_REPEAT_SEND_FRAME ZB_TRANS_SEND_FRAME


zb_uint8_t mac_rt570_read_packet(zb_bufid_t buf,
                                 uint16_t   packet_length,
                                 uint8_t    *rx_data_address,
                                 uint8_t    crc_status,
                                 uint8_t    rssi,
                                 uint8_t    snr);


#define ZB_TRANSCEIVER_ENABLE_AUTO_ACK() // TODO: add functionality for this - Arch 02/08/2024

#define ZB_TRANS_CUT_SPECIFIC_HEADER(zb_buffer)

#define ZB_RX_FLUSH_TIMEOUT (60 * ZB_TIME_ONE_SECOND)

typedef struct zb_transceiver_ctx_s
{
    zb_time_t   timer_ov_period;
    zb_time_t   rx_timestamp;
    zb_uint16_t int_status_0;
    zb_uint16_t int_status_1;
    zb_uint16_t tx_status;
    zb_uint8_t  err;
    zb_uint8_t  interrupt_flag;
    zb_uint8_t  recv_buf_full;
    zb_uint8_t  csma_backoffs;
    zb_uint8_t  csma_be;
    zb_uint8_t  csma_enable;
    zb_uint8_t  len;

    zb_uint_t in_recv;
    zb_uint_t rx_fifo_total;
    zb_uint_t rx_error;
    zb_uint_t tx_autoack_inprogress;
    zb_uint_t auto_ack_enabled;

    zb_uint32_t t1, t2;

    zb_time_t high_precision_timer;

    zb_uint16_t total_tx_retries;
    zb_uint16_t failed_tx;
    zb_time_t   rx_flush_wd_timer;
}

zb_transceiver_ctx_t;

#define ZB_TRANS_GET_TX_TIMESTAMP() (0)

void zb_transceiver_set_auto_state(zb_uint8_t rx_on);

#define ZB_TRANSCEIVER_SET_RX_ON_OFF(_rx_on) zb_transceiver_set_auto_state(_rx_on)
#define ZB_TRANSCEIVER_GET_RX_ON_OFF()       ZB_PIB_RX_ON_WHEN_IDLE() // (1)
// TODO: should we get the actual RX state rather than relying on the RX_ON_WHEN_IDLE flag? - Arch 02/21/2024

#define ZB_TRANSCEIVER_SET_TX_POWER(power) tr_set_tx_power((int)power)
#define ZB_TRANSCEIVER_GET_TX_POWER(power) *power = tr_get_tx_power()

#define ZB_IS_TRANSPORT_BUSY()             0
#define ZB_TRANSCEIVER_DEINIT_RADIO()

void mac_rt570_set_promiscuous_mode(zb_uint8_t mode);

#define ZB_TRANSCEIVER_SET_PROMISCUOUS(mode) mac_rt570_set_promiscuous_mode(mode)

#define TR_CCA_MAX_THRESHOLD_MAGNITUDE     120
#define TR_CCA_DEFAULT_THRESHOLD_MAGNITUDE 75

/// @brief set the radio tx power within the limits supported by the mac
/// @param power tx power in dB
/// @return this ultimately returns RFB_EVENT_STATUS, 0 is success, anything else is a failure
int tr_set_tx_power(int power);

/// @brief get the current transmit power
/// @return signed transmit power
int tr_get_tx_power(void);

/// @brief this API is used to set the CCA threshold global variable before the radio is initialized
/// @param threshold unsigned integer representing CCA threshold in dB (75 = -75dB)
void tr_set_cca_threshold_value(uint8_t threshold);

/// @brief this API is used to set the CCA threshold AFTER the radio is initialized
/// @param threshold unsigned integer representing CCA threshold in dB (75 = -75dB)
void tr_set_cca_threshold(uint8_t threshold);

/// @brief get the current CCA threshold setting
/// @return unsigned integer representing CCA threshold in dB (75 = -75dB)
uint8_t tr_get_cca_threshold(void);

/// @brief starts or stops continuous TX tone on a give channel.
/// @note MUST NOT TRANSMIT ANYTHING ELSE WHEN USING THIS FUNCTION!
/// @param chan_or_freq channel or frequency to transmit on. 11-26 for channel, 0
/// to stop, anything else to attempt to use as a frequency
/// @return 0 for success, -1 for failure
int tr_tx_tone(uint32_t chan_or_freq);

/// @brief stop TX tone transmission
/// @return 0 for success, -1 for failure
int tr_tx_tone_stop(void);

#endif /* MAC_RT570_H */
