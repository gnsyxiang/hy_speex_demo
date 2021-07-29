/**
 * 
 * Release under GPLv-3.0.
 * 
 * @file    client.c
 * @brief   
 * @author  gnsyxiang <gnsyxiang@163.com>
 * @date    18/03 2021 20:22
 * @version v0.0.1
 * 
 * @since    note
 * @note     note
 * 
 *     change log:
 *     NO.     Author              Date            Modified
 *     00      zhenquan.qiu        18/03 2021      create the file
 * 
 *     last modified: 18/03 2021 20:22
 */
#include <stdio.h>
#include <unistd.h>

#include "hy_speex_aec.h"
#include "hy_net.h"

#include "hy_utils/hy_module.h"
#include "hy_utils/hy_mem.h"
#include "hy_utils/hy_log.h"

#define ALONE_DEBUG 1

#define FRAME_SIZE 320

typedef struct {
    void *log_handle;
    void *aec_handle;
    void *net_handle;

    HyNetState_t net_state;
} _main_context_t;

static void _net_data_cb(void *buf, size_t len, void *args)
{
    LOGD("len: %d, data: %s \n", len, buf);
}

static void _net_state_cb(HyNetState_t state, void *args)
{
    LOGD("state: %d \n", state);

    _main_context_t *context = args;
    context->net_state = state;
}

static void _net_signal_cb(hy_s32_t fd, void *args)
{
    LOGD("fd: %d \n", fd);

    exit(1);
}

static void _module_destroy(_main_context_t **context_pp)
{
    _main_context_t *context = *context_pp;

    // note: 增加或删除要同步到module_create_t中
    module_destroy_t module[] = {
        {"net",     &context->net_handle,   HyNetDestroy},
        {"aec",     &context->aec_handle,   HySpeexAecDestroy},
        {"log",     &context->log_handle,   HyLogDestroy},
    };

    RUN_DESTROY(module);

    HY_FREE(context_pp);
}

static _main_context_t *_module_create(void)
{
    _main_context_t *context = (_main_context_t *)HY_MALLOC_RET_VAL(sizeof(*context), NULL);

    HyLogConfig_t log_config;
    log_config.buf_len          = 512;
    log_config.level            = HY_LOG_LEVEL_INFO;
    log_config.config_file      = "./res/config/log4cplus.rc";

    HySpeexAecConfig_t aec_config;
    aec_config.frame_size       = FRAME_SIZE;
    aec_config.filter_length    = 2048;
    aec_config.sample_rate      = 16000;

    HyNetConfig_t net_config;
    net_config.ip               = "192.168.1.57";
    net_config.port             = 7809;
    net_config.type             = HY_NET_TYPE_CLIENT;
    net_config.config_save.data_cb      = _net_data_cb;
    net_config.config_save.state_cb     = _net_state_cb;
    net_config.config_save.signal_cb    = _net_signal_cb;
    net_config.config_save.args         = context;

    // note: 增加或删除要同步到module_destroy_t中
    module_create_t module[] = {
        {"log",  &context->log_handle,   &log_config,    (create_t)HyLogCreate,         HyLogDestroy},
        {"aec",  &context->aec_handle,   &aec_config,    (create_t)HySpeexAecCreate,    HySpeexAecDestroy},
        {"net",  &context->net_handle,   &net_config,    (create_t)HyNetCreate,         HyNetDestroy},
    };

    RUN_CREATE(module);

    return context;
}

int main(int argc, char const* argv[])
{
    _main_context_t *context = _module_create();
    if (!context) {
        LOGE("_module_create faild \n");
        return -1;
    }

    while (1) {
        sleep(1);
        if (context->net_state == HY_NET_STATE_DISCONNECT) {
            LOGD("net disdconnect \n");
            break;
        }
    }

    _module_destroy(&context);

    return 0;
}

