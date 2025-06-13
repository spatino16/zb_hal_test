
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

#ifndef ZB_DIAG_TH_BEHAVIOUR_H
#define ZB_DIAG_TH_BEHAVIOUR_H

/** Empty init-function
 *
 * Should be called from @see zb_diag_init()
 */
void zb_diag_th_behaviour_init(void);

zb_bool_t zb_diag_th_disable_support_kn_tlv_in_beacon(void);
zb_bool_t zb_diag_th_disable_update_hub_connectivity(void);

zb_bool_t zb_diag_th_keep_provisional_key_for_16_3(void);

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
typedef struct zb_zdo_secur_start_key_negotiation_rsp_send_param_s zb_zdo_secur_start_key_negotiation_rsp_send_param_t;
void zb_diag_th_key_neg_rsp_not_autho(zb_zdo_secur_start_key_negotiation_rsp_send_param_t *resp);
void zb_diag_th_key_neg_rsp_truncated_tlv(zb_bufid_t bufid, zb_secur_ecdhe_common_ctx_t *key_neg_ctx_ptr);
#endif /* ZB_COORDINATOR_ROLE || ZB_ROUTER_ROLE */

zb_bool_t zb_diag_th_key_neg_rsp_no_tlv(void);
void zb_diag_th_key_neg_rsp_add_extra_tlv(zb_bufid_t bufid, zb_uint8_t allocation_bytes);

void zb_diag_th_key_neg_req_truncated_tlv(zb_bufid_t bufid);
zb_bool_t zb_diag_th_key_neg_req_no_tlv(void);
void zb_diag_th_key_neg_req_add_extra_tlv(zb_bufid_t bufid, zb_uint8_t allocation_bytes);

#endif /* ZB_DIAG_TH_BEHAVIOUR_H */
