/// ****************************************************************************
/// @file xmodem.c
///
/// @brief
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bootloader.h"
#include "fota_define.h"

// Define XMODEM constants
#define SOH         0x01 // Start of Header (128 bytes block)
#define EOT         0x04 // End of Transmission
#define ACK         0x06 // Acknowledge
#define NAK         0x15 // Negative Acknowledge
#define CAN         0x18 // Cancel
#define MAX_RETRIES 10
#define PACKET_SIZE 128

extern void tr_hal_put_char(char ch);

// forward declarations
uint32_t process_packet(int     block_num,
                        uint8_t *data,
                        int     length);
void create_fota_header(int block_num);

// xmodem rx buffer and index
uint8_t buffer[PACKET_SIZE + 4];  // SOH + Block Number + Block Number Complement + Data + Checksum
uint8_t buf_index = 0;

// flash programming buffer
uint8_t flash_buffer[FLASH_PAGE_PROGRAMMING_SIZE];

// FOTA information
fota_information_t fota_info;

// Function to calculate checksum (sum of 128 data bytes)
uint8_t calculate_checksum(const uint8_t *data,
                           int           length)
{
    uint8_t checksum = 0;

    for (int i = 0 ; i < length ; i++)
    {
        checksum += data[i];
    }
    return checksum;
}

// XMODEM receiver function
void xmodem_receive()
{
    int retries   = 0;
    int block_num = 1;

    // Send initial NAK to start the transmission
    buf_index = 0;
    tr_hal_put_char(NAK);

    while (1)
    {
        // Try to read a packet (SOH + Block Number + Block Number Complement + Data + Checksum)
        int read_bytes = serial_read(PACKET_SIZE + 4, 1000);

        if (read_bytes == 1 && buffer[0] == EOT)
        {
            // end of Transmission (EOT received)
            tr_hal_put_char(ACK);
            PRINT("[XMODEM] File received.\n");
            create_fota_header(block_num - 1);
            break;
        }

        if (read_bytes == PACKET_SIZE + 4)
        {
            // check if the packet starts with SOH (128 bytes block)
            if (buffer[0] != SOH)
            {
                tr_hal_put_char(NAK);
                continue;
            }

            // validate block number and its complement
            if (buffer[1] != (uint8_t)block_num || buffer[2] != (uint8_t)(~block_num))
            {
                tr_hal_put_char(NAK);
                continue;
            }

            // validate checksum
            uint8_t received_checksum   = buffer[PACKET_SIZE + 3];
            uint8_t calculated_checksum = calculate_checksum(&buffer[3], PACKET_SIZE);

            if (received_checksum != calculated_checksum)
            {
                tr_hal_put_char(NAK);
                retries++;

                if (retries >= MAX_RETRIES)
                {
                    PRINT("[XMODEM] Maximum retries exceeded. Transmission failed.\n");
                    tr_hal_put_char(CAN);
                    break;
                }
                continue;
            }

            // process the received data
            process_packet(block_num, &buffer[3], PACKET_SIZE);

            // send ACK to acknowledge the received block
            buf_index = 0;
            tr_hal_put_char(ACK);

            // move to the next block
            block_num++;
            retries = 0;
        }
        else
        {
            retries++;

            if (retries >= MAX_RETRIES)
            {
                PRINT("[XMODEM] Maximum retries exceeded. Transmission failed.\n");
                tr_hal_put_char(CAN);
                break;
            }
            tr_hal_put_char(NAK);
        }
    }
}

// callback from UART rx interrupt
void xmodem_byte_rx(uint8_t data)
{
    // save the data in the xmodem buffer
    buffer[buf_index++] = data;
}

// dwell until we timeout or receive the requested number of bytes
int serial_read(int length,
                int timeout_ms)
{
    uint32_t delay = timeout_ms;

    // loop until the number of bytes expected have come in or we timeout
    while ((buf_index != length) && (delay != 0))
    {
        Delay_us(1000);
        delay--;
    }
    return buf_index;
}

// we have to flash in 256 byte pages, so flash every other xmodem packet
uint32_t process_packet(int     block_num,
                        uint8_t *data,
                        int     length)
{
    uint32_t status = 0;

#if (LOOK_FOR_START_CHAR != 1)

    if (block_num == 1)
    {
        // this is the first block, erase the upgrade partition
        set_flash_erase(FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB, SIZE_OF_FOTA_BANK_2MB);
    }
#endif

    if ((block_num & 1) == 1)
    {
        // this is an odd numbered block, just save the data
        memcpy(&flash_buffer[0], data, PACKET_SIZE);
    }
    else
    {
        // this is an even numbered block, save the data and flash a page
        memcpy(&flash_buffer[PACKET_SIZE], data, PACKET_SIZE);
        status = flash_write_page((uint32_t)flash_buffer, ((block_num - 2) * PACKET_SIZE) + FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB);
    }
    return status;
}

// the transfer is complete, setup the FOTA header so that the bootloader
// will know what to do with the image
void create_fota_header(int block_num)
{
    // was there a single block dangling at the end
    if ((block_num & 1) == 1)
    {
        // the last block wasn't written to flash, take care of it
        flash_write_page((uint32_t)flash_buffer, ((block_num - 2) * PACKET_SIZE) + FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB);
    }

    fota_info.fotabank_startaddr = FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB; // this is where the new image starts
    fota_info.fotabank_datalen   = (block_num - 2) * PACKET_SIZE;     // this is the size of the new image
    fota_info.target_startaddr   = APP_START_ADDRESS;                 // this is where to copy it
    fota_info.fotabank_ready     = FOTA_IMAGE_READY;                  // this is an indicator that the image is good
    fota_info.fotabank_crc       = crc32((uint32_t)fota_info.fotabank_startaddr, fota_info.fotabank_datalen);
    fota_info.fota_result        = 0xFF;                              // this must be 0xFF in order for the bootloader to copy the image

    // erase the fota space
    flash_erase(FLASH_ERASE_SECTOR, FOTA_UPDATE_BANK_INFO_ADDRESS);

    // write the fota info to flash
    flash_write_page((uint32_t)&fota_info, FOTA_UPDATE_BANK_INFO_ADDRESS);

    // reboot to invoke the bootloader, it will transfer the image over the application
    NVIC_SystemReset();
}
