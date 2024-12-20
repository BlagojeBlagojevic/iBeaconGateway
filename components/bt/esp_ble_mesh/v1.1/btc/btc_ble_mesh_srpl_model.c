/*
 * SPDX-FileCopyrightText: 2020-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <errno.h>

#include "btc_ble_mesh_model_common.h"
#include "btc_ble_mesh_srpl_model.h"
#include "esp_ble_mesh_srpl_model_api.h"

#if CONFIG_BLE_MESH_SRPL_CLI

extern int bt_mesh_solic_pdu_rpl_items_clear(void *param, void *uar);

/* Solicitation PDU RPL Configuration Client model related functions */

static inline void btc_ble_mesh_srpl_client_cb_to_app(esp_ble_mesh_srpl_client_cb_event_t event,
                                                      esp_ble_mesh_srpl_client_cb_param_t *param)
{
    esp_ble_mesh_srpl_client_cb_t btc_ble_mesh_cb =
        btc_profile_cb_get(BTC_PID_SRPL_CLIENT);
    if (btc_ble_mesh_cb) {
        btc_ble_mesh_cb(event, param);
    }
}

void btc_ble_mesh_srpl_client_arg_deep_copy(btc_msg_t *msg, void *p_dest, void *p_src)
{
    btc_ble_mesh_srpl_client_args_t *dst = p_dest;
    btc_ble_mesh_srpl_client_args_t *src = p_src;

    if (!msg || !dst || !src) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    switch (msg->act) {
    case BTC_BLE_MESH_ACT_SRPL_CLIENT_SEND:
        dst->srpl_send.params = bt_mesh_calloc(sizeof(esp_ble_mesh_client_common_param_t));
        dst->srpl_send.msg = bt_mesh_calloc(sizeof(esp_ble_mesh_srpl_client_msg_t));
        if (dst->srpl_send.params && dst->srpl_send.msg) {
            memcpy(dst->srpl_send.params, src->srpl_send.params,
                   sizeof(esp_ble_mesh_client_common_param_t));
            memcpy(dst->srpl_send.msg, src->srpl_send.msg,
                   sizeof(esp_ble_mesh_srpl_client_msg_t));
        } else {
            BT_ERR("%s, Out of memory, act %d", __func__, msg->act);
        }
        break;
    default:
        BT_DBG("%s, Unknown act %d", __func__, msg->act);
        break;
    }
}

void btc_ble_mesh_srpl_client_arg_deep_free(btc_msg_t *msg)
{
    btc_ble_mesh_srpl_client_args_t *arg = NULL;

    if (!msg) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    arg = (btc_ble_mesh_srpl_client_args_t *)msg->arg;

    switch (msg->act) {
    case BTC_BLE_MESH_ACT_SRPL_CLIENT_SEND:
        if (arg->srpl_send.msg) {
            bt_mesh_free(arg->srpl_send.msg);
        }
        if (arg->srpl_send.params) {
            bt_mesh_free(arg->srpl_send.params);
        }
        break;
    default:
        break;
    }
}

static void btc_ble_mesh_srpl_client_copy_req_data(btc_msg_t *msg, void *p_dest, void *p_src)
{
    esp_ble_mesh_srpl_client_cb_param_t *p_dest_data = p_dest;
    esp_ble_mesh_srpl_client_cb_param_t *p_src_data = p_src;

    if (!msg || !p_src_data || !p_dest_data) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    if (p_src_data->params) {
        p_dest_data->params = bt_mesh_calloc(sizeof(esp_ble_mesh_client_common_param_t));
        if (!p_dest_data->params) {
            BT_ERR("%s, Out of memory, act %d", __func__, msg->act);
            return;
        }

        memcpy(p_dest_data->params, p_src_data->params, sizeof(esp_ble_mesh_client_common_param_t));
    }
}

static void btc_ble_mesh_srpl_client_free_req_data(btc_msg_t *msg)
{
    esp_ble_mesh_srpl_client_cb_param_t *arg = NULL;

    if (!msg) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    arg = (esp_ble_mesh_srpl_client_cb_param_t *)msg->arg;

    switch (msg->act) {
    case ESP_BLE_MESH_SRPL_CLIENT_SEND_COMP_EVT:
    case ESP_BLE_MESH_SRPL_CLIENT_SEND_TIMEOUT_EVT:
    case ESP_BLE_MESH_SRPL_CLIENT_RECV_RSP_EVT:
    case ESP_BLE_MESH_SRPL_CLIENT_RECV_PUB_EVT:
        if (arg->params) {
            bt_mesh_free(arg->params);
        }
        break;
    default:
        break;
    }
}

static void btc_ble_mesh_srpl_client_cb(esp_ble_mesh_srpl_client_cb_param_t *cb_params, uint8_t act)
{
    btc_msg_t msg = {0};

    /* If corresponding callback is not registered, event will not be posted. */
    if (!btc_profile_cb_get(BTC_PID_SRPL_CLIENT)) {
        return;
    }

    msg.sig = BTC_SIG_API_CB;
    msg.pid = BTC_PID_SRPL_CLIENT;
    msg.act = act;

    btc_transfer_context(&msg, cb_params, sizeof(esp_ble_mesh_srpl_client_cb_param_t),
                         btc_ble_mesh_srpl_client_copy_req_data,
                         btc_ble_mesh_srpl_client_free_req_data);
}

void bt_mesh_srpl_client_cb_evt_to_btc(uint32_t opcode, uint8_t event,
                                       struct bt_mesh_model *model,
                                       struct bt_mesh_msg_ctx *ctx,
                                       const uint8_t *val, size_t len)
{
    esp_ble_mesh_srpl_client_cb_param_t cb_params = {0};
    esp_ble_mesh_client_common_param_t params = {0};
    uint8_t act = 0;

    if (!model || !ctx || len > sizeof(cb_params.recv)) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    switch (event) {
    case BTC_BLE_MESH_EVT_SRPL_CLIENT_SEND_TIMEOUT:
        act = ESP_BLE_MESH_SRPL_CLIENT_SEND_TIMEOUT_EVT;
        break;
    case BTC_BLE_MESH_EVT_SRPL_CLIENT_RECV_RSP:
        act = ESP_BLE_MESH_SRPL_CLIENT_RECV_RSP_EVT;
        break;
    case BTC_BLE_MESH_EVT_SRPL_CLIENT_RECV_PUB:
        act = ESP_BLE_MESH_SRPL_CLIENT_RECV_PUB_EVT;
        break;
    default:
        BT_ERR("Unknown Solicitation PDU RPL Config client event type %d", event);
        return;
    }

    params.opcode = opcode;
    params.model = (esp_ble_mesh_model_t *)model;
    params.ctx.net_idx = ctx->net_idx;
    params.ctx.app_idx = ctx->app_idx;
    params.ctx.addr = ctx->addr;
    params.ctx.recv_ttl = ctx->recv_ttl;
    params.ctx.recv_op = ctx->recv_op;
    params.ctx.recv_dst = ctx->recv_dst;
    params.ctx.recv_rssi = ctx->recv_rssi;
    params.ctx.send_ttl = ctx->send_ttl;

    cb_params.params = &params;

    if (val && len) {
        memcpy(&cb_params.recv, val, len);
    }

    btc_ble_mesh_srpl_client_cb(&cb_params, act);
}

void btc_ble_mesh_srpl_client_recv_pub_cb(uint32_t opcode,
                                          struct bt_mesh_model *model,
                                          struct bt_mesh_msg_ctx *ctx,
                                          struct net_buf_simple *buf)
{
    if (!model || !ctx || !buf) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    bt_mesh_srpl_client_cb_evt_to_btc(opcode,
        BTC_BLE_MESH_EVT_SRPL_CLIENT_RECV_PUB,
        model, ctx, buf->data, buf->len);
}

static int btc_ble_mesh_srpl_client_send(esp_ble_mesh_client_common_param_t *params,
                                         esp_ble_mesh_srpl_client_msg_t *msg)
{
    bt_mesh_client_common_param_t param = {0};

    if (params == NULL || msg == NULL) {
        BT_ERR("%s, Invalid parameter", __func__);
        return -EINVAL;
    }

    btc_ble_mesh_set_client_common_param(params, &param, false);

    switch (param.opcode) {
    case ESP_BLE_MESH_MODEL_OP_SRPL_ITEMS_CLEAR:
    case ESP_BLE_MESH_MODEL_OP_SRPL_ITEMS_CLEAR_UNACK:
        return bt_mesh_solic_pdu_rpl_items_clear(&param, &msg->srpl_items_clear.addr_range);
    default:
        BT_ERR("Invalid Solicitation PDU RPL Config Set opcode 0x%04x", param.opcode);
        return -EINVAL;
    }
}

void btc_ble_mesh_srpl_client_call_handler(btc_msg_t *msg)
{
    esp_ble_mesh_srpl_client_cb_param_t cb = {0};
    btc_ble_mesh_srpl_client_args_t *arg = NULL;

    if (!msg) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    arg = (btc_ble_mesh_srpl_client_args_t *)msg->arg;

    switch (msg->act) {
    case BTC_BLE_MESH_ACT_SRPL_CLIENT_SEND:
        cb.params = arg->srpl_send.params;
        cb.send.err_code = btc_ble_mesh_srpl_client_send(arg->srpl_send.params,
                                                         arg->srpl_send.msg);
        btc_ble_mesh_srpl_client_cb(&cb,
            ESP_BLE_MESH_SRPL_CLIENT_SEND_COMP_EVT);
        break;
    default:
        break;
    }

    btc_ble_mesh_srpl_client_arg_deep_free(msg);
}

void btc_ble_mesh_srpl_client_cb_handler(btc_msg_t *msg)
{
    esp_ble_mesh_srpl_client_cb_param_t *arg = NULL;

    if (!msg) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    arg = (esp_ble_mesh_srpl_client_cb_param_t *)msg->arg;

    if (msg->act < ESP_BLE_MESH_SRPL_CLIENT_EVT_MAX) {
        btc_ble_mesh_srpl_client_cb_to_app(msg->act, arg);
    } else {
        BT_ERR("%s, Unknown act %d", __func__, msg->act);
    }

    btc_ble_mesh_srpl_client_free_req_data(msg);
}

#endif /* CONFIG_BLE_MESH_SRPL_CLI */

#if CONFIG_BLE_MESH_SRPL_SRV

/* Solicitation PDU RPL Config Server model related functions */

static inline void btc_ble_mesh_srpl_server_cb_to_app(esp_ble_mesh_srpl_server_cb_event_t event,
                                                      esp_ble_mesh_srpl_server_cb_param_t *param)
{
    esp_ble_mesh_srpl_server_cb_t btc_ble_mesh_cb =
        btc_profile_cb_get(BTC_PID_SRPL_SERVER);
    if (btc_ble_mesh_cb) {
        btc_ble_mesh_cb(event, param);
    }
}

static void btc_ble_mesh_srpl_server_cb(esp_ble_mesh_srpl_server_cb_param_t *cb_params, uint8_t act)
{
    btc_msg_t msg = {0};

    /* If corresponding callback is not registered, event will not be posted. */
    if (!btc_profile_cb_get(BTC_PID_SRPL_SERVER)) {
        return;
    }

    msg.sig = BTC_SIG_API_CB;
    msg.pid = BTC_PID_SRPL_SERVER;
    msg.act = act;

    btc_transfer_context(&msg, cb_params, sizeof(esp_ble_mesh_srpl_server_cb_param_t), NULL, NULL);
}

void bt_mesh_srpl_server_cb_evt_to_btc(uint8_t event,
                                       struct bt_mesh_model *model,
                                       struct bt_mesh_msg_ctx *ctx,
                                       const uint8_t *val, size_t len)
{
    esp_ble_mesh_srpl_server_cb_param_t cb_params = {0};
    uint8_t act = 0;

    if (!model || !ctx || len > sizeof(cb_params.value)) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    switch (event) {
    case BTC_BLE_MESH_EVT_SRPL_SERVER_STATE_CHANGE:
        act = ESP_BLE_MESH_SRPL_SERVER_STATE_CHANGE_EVT;
        break;
    default:
        BT_ERR("Unknown Solicitation PDU RPL Config server event type %d", event);
        return;
    }

    cb_params.model = (esp_ble_mesh_model_t *)model;
    cb_params.ctx.net_idx = ctx->net_idx;
    cb_params.ctx.app_idx = ctx->app_idx;
    cb_params.ctx.addr = ctx->addr;
    cb_params.ctx.recv_ttl = ctx->recv_ttl;
    cb_params.ctx.recv_op = ctx->recv_op;
    cb_params.ctx.recv_dst = ctx->recv_dst;
    cb_params.ctx.recv_rssi = ctx->recv_rssi;
    cb_params.ctx.send_ttl = ctx->send_ttl;

    if (val && len) {
        memcpy(&cb_params.value, val, len);
    }

    btc_ble_mesh_srpl_server_cb(&cb_params, act);
}

void btc_ble_mesh_srpl_server_cb_handler(btc_msg_t *msg)
{
    esp_ble_mesh_srpl_server_cb_param_t *arg = NULL;

    if (!msg) {
        BT_ERR("%s, Invalid parameter", __func__);
        return;
    }

    arg = (esp_ble_mesh_srpl_server_cb_param_t *)msg->arg;

    if (msg->act < ESP_BLE_MESH_SRPL_SERVER_EVT_MAX) {
        btc_ble_mesh_srpl_server_cb_to_app(msg->act, arg);
    } else {
        BT_ERR("%s, Unknown act %d", __func__, msg->act);
    }
}

#endif /* CONFIG_BLE_MESH_SRPL_SRV */
