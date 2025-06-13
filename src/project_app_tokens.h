/// ****************************************************************************
/// @file project_app_tokens.h
///
/// @brief sample APP token definition file
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef PROJECT_APP_TOKENS_H
#define PROJECT_APP_TOKENS_H
#include "tr_app_tokens.h"

// create a special structure for an application token
typedef struct __attribute__((packed))
{
    uint8_t  first[2];
    uint16_t second;
}
my_special_token_struct_t;

typedef uint8_t app_tok_type_product_upc_code[12];

// create the application tokens
// usage: TR_CREATE_APP_TOKEN("NAME_OF_MY_APP_TOKEN", data_type)
// NOTE: MUST USE STD C DATA TYPES (i.e. uint8_t NOT zb_uint8_t)
TR_START_APP_TOKEN_DEFS
{
    TR_CREATE_APP_TOKEN(MY_APP_TOK_U8, uint8_t)
    TR_CREATE_APP_TOKEN(MY_APP_TOK_U32, uint32_t)
    TR_CREATE_APP_TOKEN(MY_APP_TOK_SPECIAL, my_special_token_struct_t)
    TR_CREATE_APP_TOKEN(MY_APP_TOK_U16, uint16_t)
    TR_CREATE_APP_TOKEN(APP_TOK_UPC, app_tok_type_product_upc_code)
}
TR_END_APP_TOKEN_DEFS
#endif /* ifdef PROJECT_APP_TOKENS_H */
