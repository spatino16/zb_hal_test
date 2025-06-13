
/* ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2024 DSR Corporation, Denver CO, USA.
 * www.dsr-zboss.com
 * www.dsr-corporation.com
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

#ifndef ZB_LITE_H
#define ZB_LITE_H

#if defined(ZB_DIAGNOSTIC_TH_MODIFIERS_ENABLED)

#define ZB_DIAG_TH_BEHAVIOUR_INIT() zb_diag_th_behaviour_init()

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
#define ZB_DIAG_TH_KEY_NEG_RSP_NOT_AUTHO(resp) zb_diag_th_key_neg_rsp_not_autho(resp)
#define ZB_DIAG_TH_KEY_NEG_RSP_TRUNCATED_TLV(bufid, key_neg_ctx_ptr) zb_diag_th_key_neg_rsp_truncated_tlv((bufid), (key_neg_ctx_ptr))
#endif /* ZB_COORDINATOR_ROLE || ZB_ROUTER_ROLE */

#define ZB_DIAG_TH_KEY_NEG_REQ_TRUNCATED_TLV(bufid) zb_diag_th_key_neg_req_truncated_tlv(bufid)
#define ZB_DIAG_TH_KEY_NEG_RSP_ADD_EXTRA_TLV(bufid, allocation_bytes) zb_diag_th_key_neg_rsp_add_extra_tlv((bufid), (allocation_bytes))
#define ZB_DIAG_TH_KEY_NEG_REQ_ADD_EXTRA_TLV(bufid, allocation_bytes) zb_diag_th_key_neg_req_add_extra_tlv((bufid), (allocation_bytes))

#define ZB_DIAG_TH_DISABLE_SUPPORT_KN_TLV_IN_BEACON() zb_diag_th_disable_support_kn_tlv_in_beacon()
#define ZB_DIAG_TH_DISABLE_UPDATE_HUB_CONNECTIVITY() zb_diag_th_disable_update_hub_connectivity()
#define ZB_DIAG_TH_KEEP_PROVISIONAL_KEY_FOR_16_3() zb_diag_th_keep_provisional_key_for_16_3()
#define ZB_DIAG_TH_KEY_NEG_RSP_NO_TLV() zb_diag_th_key_neg_rsp_no_tlv()
#define ZB_DIAG_TH_KEY_NEG_REQ_NO_TLV() zb_diag_th_key_neg_req_no_tlv()

#else /* ZB_DIAGNOSTIC_TH_MODIFIERS_ENABLED */

#define ZB_DIAG_TH_BEHAVIOUR_INIT()
#define ZB_DIAG_TH_DISABLE_SUPPORT_KN_TLV_IN_BEACON() zb_return_false_func()
#define ZB_DIAG_TH_DISABLE_UPDATE_HUB_CONNECTIVITY() zb_return_false_func()
#define ZB_DIAG_TH_KEEP_PROVISIONAL_KEY_FOR_16_3() zb_return_false_func()
#define ZB_DIAG_TH_KEY_NEG_RSP_NOT_AUTHO(resp)
#define ZB_DIAG_TH_KEY_NEG_RSP_TRUNCATED_TLV(bufid, key_neg_ctx_ptr)
#define ZB_DIAG_TH_KEY_NEG_RSP_NO_TLV() zb_return_false_func()
#define ZB_DIAG_TH_KEY_NEG_RSP_ADD_EXTRA_TLV(bufid, allocation_bytes)
#define ZB_DIAG_TH_KEY_NEG_REQ_TRUNCATED_TLV(bufid)
#define ZB_DIAG_TH_KEY_NEG_REQ_NO_TLV() zb_return_false_func()
#define ZB_DIAG_TH_KEY_NEG_REQ_ADD_EXTRA_TLV(bufid, allocation_bytes)

#endif /* ZB_DIAGNOSTIC_TH_MODIFIERS_ENABLED */
/* ---------------------------------------------------------------- */

#if defined(ZB_DIAGNOSTIC_DUT_MODIFIERS_ENABLED)

#define ZB_DIAG_DUT_BEHAVIOUR_INIT() zb_diag_dut_behaviour_init()

#else /* ZB_DIAGNOSTIC_DUT_MODIFIERS_ENABLED */

#define ZB_DIAG_DUT_BEHAVIOUR_INIT()

#endif /* ZB_DIAGNOSTIC_DUT_MODIFIERS_ENABLED */
/* ---------------------------------------------------------------- */

#endif /* ZB_LITE_H */
