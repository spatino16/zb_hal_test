/// ****************************************************************************
/// @file tr_main.c
///
/// @brief main function for zigbee stack
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
// NOTE: tr_hal_config.h MUST NOT be included elsewhere in the framework files
#include "tr_hal_config.h"

extern void tr_set_user_keepalive_mode(nwk_keepalive_supported_method_t mode);

MAIN()
{
    ARGV_UNUSED;

    ZB_SET_TRACE_OFF();
    ZB_SET_TRAF_DUMP_OFF();

    tr_af_init();
    tr_hal_init();
    tr_zcl_endpoint_config_init();

    zb_set_bdb_primary_channel_set(TR_PRIMARY_CHANNEL_MASK);
    zb_set_bdb_secondary_channel_set(TR_SECONDARY_CHANNEL_MASK);

#ifdef ZB_ED_FUNC
    // configure end-device aging
    zb_set_ed_timeout(TR_ED_AGING_TIMEOUT);
    zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(TR_KEEPALIVE_INTERVAL_MS));
    tr_set_user_keepalive_mode(TR_KEEPALIVE_METHOD);
#endif

    /* Initiate the stack start without starting the commissioning */
    if (zboss_start_no_autostart() != RET_OK)
    {
        tr_app_printf("zboss_start failed\n");
    }
    else
    {
        vTaskStartScheduler();
    }

    MAIN_RETURN(0);
}
