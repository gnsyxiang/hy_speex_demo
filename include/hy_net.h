/**
 *
 * Release under GPLv-3.0.
 * 
 * @file    hy_net.h
 * @brief   
 * @author  gnsyxiang <gnsyxiang@163.com>
 * @date    29/07 2021 10:54
 * @version v0.0.1
 * 
 * @since    note
 * @note     note
 * 
 *     change log:
 *     NO.     Author              Date            Modified
 *     00      zhenquan.qiu        29/07 2021      create the file
 * 
 *     last modified: 29/07 2021 10:54
 */
#ifndef __HY_SPEEX_DEMO_INCLUDE_HY_NET_H_
#define __HY_SPEEX_DEMO_INCLUDE_HY_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "hy_utils/hy_type.h"

typedef enum {
    HY_NET_TYPE_CLIENT,
    HY_NET_TYPE_SERVER,

    HY_NET_TYPE_MAX,
} HyNetType_t;

typedef enum {
    HY_NET_STATE_DISCONNECT,
    HY_NET_STATE_CONNECTING,
    HY_NET_STATE_CONNECTED,

    HY_NET_STATE_MAX,
} HyNetState_t;

typedef struct {
    void (*state_cb)(HyNetState_t state, void *args);
    void (*data_cb)(void *buf, size_t len, void *args);
    void (*signal_cb)(hy_s32_t fd, void *args);
    void *args;
} HyNetConfigSave_t;

typedef struct {
    char        *ip;
    hy_u16_t    port;
    HyNetType_t type;

    HyNetConfigSave_t config_save;
} HyNetConfig_t;

void *HyNetCreate(HyNetConfig_t *net_config);
void HyNetDestroy(void **handle);

hy_s32_t HyNetWrite(void *handle, void *buf, hy_u32_t len);

#ifdef __cplusplus
}
#endif

#endif

