#ifndef  SERVER_H
#define  SERVER_H
#include "defs.h"
#include "async_server_utils.h"

#include "tag.h"
#include "parking.h"
#define tag  "Server"






extern void server_init();
void _get_page_handler(void *req);
void _post_change_stateBle(void *params);
void _post_change_stateCamera(void *params);


ASYNC_HANDLER_REGISTER_FUNCTION(_get_page_handler, GET_PAGE);

ASYNC_HANDLER_REGISTER_FUNCTION(_post_change_stateBle, BLE_POST);

ASYNC_HANDLER_REGISTER_FUNCTION(_post_change_stateCamera, CAMERA_POST);


//ASYNC_HANDLER(_get_page_handler);

#endif
