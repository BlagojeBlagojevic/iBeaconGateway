/*
 * SPDX-FileCopyrightText: 2020-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include "btc/btc_manage.h"

#include "btc_ble_mesh_odp_model.h"
#include "esp_ble_mesh_odp_model_api.h"

#if CONFIG_BLE_MESH_ODP_CLI
esp_err_t esp_ble_mesh_register_odp_client_callback(esp_ble_mesh_odp_client_cb_t callback)
{
    ESP_BLE_HOST_STATUS_CHECK(ESP_BLE_HOST_STATUS_ENABLED);

    return (btc_profile_cb_set(BTC_PID_ODP_CLIENT, callback) == 0 ? ESP_OK : ESP_FAIL);
}

esp_err_t esp_ble_mesh_odp_client_send(esp_ble_mesh_client_common_param_t *params,
                                       esp_ble_mesh_odp_client_msg_t *msg)
{
    btc_ble_mesh_odp_client_args_t btc_arg = {0};
    btc_msg_t btc_msg = {0};

    if (params == NULL || params->model == NULL ||
        params->ctx.net_idx == ESP_BLE_MESH_KEY_UNUSED ||
        !ESP_BLE_MESH_ADDR_IS_UNICAST(params->ctx.addr) ||
        (params->opcode == ESP_BLE_MESH_MODEL_OP_OD_PRIV_PROXY_SET && msg == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_BLE_HOST_STATUS_CHECK(ESP_BLE_HOST_STATUS_ENABLED);

    btc_msg.sig = BTC_SIG_API_CALL;
    btc_msg.pid = BTC_PID_ODP_CLIENT;
    btc_msg.act = BTC_BLE_MESH_ACT_ODP_CLIENT_SEND;

    btc_arg.odp_send.params = params;
    btc_arg.odp_send.msg = msg;

    return (btc_transfer_context(&btc_msg, &btc_arg, sizeof(btc_ble_mesh_odp_client_args_t),
            btc_ble_mesh_odp_client_arg_deep_copy,
            btc_ble_mesh_odp_client_arg_deep_free) == BT_STATUS_SUCCESS ? ESP_OK : ESP_FAIL);
}
#endif /* CONFIG_BLE_MESH_ODP_CLI */

#if CONFIG_BLE_MESH_ODP_SRV
esp_err_t esp_ble_mesh_register_odp_server_callback(esp_ble_mesh_odp_server_cb_t callback)
{
    ESP_BLE_HOST_STATUS_CHECK(ESP_BLE_HOST_STATUS_ENABLED);

    return (btc_profile_cb_set(BTC_PID_ODP_SERVER, callback) == 0 ? ESP_OK : ESP_FAIL);
}
#endif /* CONFIG_BLE_MESH_ODP_SRV */
