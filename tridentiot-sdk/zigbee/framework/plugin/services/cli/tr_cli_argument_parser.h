/// ****************************************************************************
/// @file tr_cli_argument_parser.h
///
/// @brief Functions used for parsing CLI command arguments
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_CLI_ARGUMENT_PARSER_H
#define TR_CLI_ARGUMENT_PARSER_H

#include "tr_af.h"
#include "tr_cli.h"

typedef struct
{
    zb_uint16_t cluster_ids[8];
    zb_uint8_t  num_cluster_ids;
    zb_uint16_t attr_ids[8];
    zb_uint8_t  num_attr_ids;
    zb_uint8_t  data_type;
    zb_char_t   value[34];
    zb_uint8_t  value_entered;
    zb_uint16_t mfg_id;
    zb_uint8_t  cmd_dir;
    zb_uint8_t  aps_encryption;
} tr_standard_args_s;

/// ****************************************************************************
/// @defgroup services_api_cli_args CLI Argument Parser API References
/// @ingroup services_api_references
/// @{
/// ****************************************************************************

/// @brief API for printing a ZBOSS command status
/// @param cmd_string pointer to null terminated string to print
/// @param status ZBOSS status to decode and print
void tr_print_cli_cmd_status(char     *cmd_string,
                             zb_ret_t status);

/// @brief API for parsing standard arguments used by many CLI commands
/// @param parsed_args_ptr pointer to the standard cli command argument structure
/// @param argc number of arguments for the current command
/// @param argv pointer to the argument array
/// @return
zb_uint8_t tr_cli_parse_standard_args(tr_standard_args_s *parsed_args_ptr,
                                      zb_int_t           argc,
                                      zb_char_t          **argv);

/// @brief debug API for printing the argument structure
/// @param args_struct_ptr pointer to the standard cli command argument structure
void tr_print_cli_arg_struct(tr_standard_args_s *args_struct_ptr);

/// @brief API for printing an attribute value based on its ZCL type
/// @param attr_desc pointer to the attribute descriptor
/// @param ext_attr_data_p pointer to the attribute value
void tr_print_attr_value(zb_zcl_attr_t *attr_desc,
                         zb_uint8_t    *ext_attr_data_p);

/// @brief API for printing an EUI64 in hex with the right byte order
/// @param eui64 pointer to the eui64 array
void tr_print_eui64(zb_ieee_addr_t eui64);

/// @brief APU for printing the contents of a transmit buffer
/// @param buf buffer index
/// @param ptr pointer to the start of the command in the buffer
void tr_print_tx_buffer(zb_bufid_t buf,
                        zb_uint8_t *ptr);

/// @} // end of services_api_references

#endif // TR_CLI_ARGUMENT_PARSER_H
