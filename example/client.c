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

#include "hy_utils/hy_module.h"
#include "hy_utils/hy_mem.h"
#include "hy_utils/hy_log.h"

#define ALONE_DEBUG 1

#define FRAME_SIZE 320

typedef struct {
    void *log_handle;
    void *aec_handle;
} _main_context_t;

static void _module_destroy(_main_context_t **context_pp)
{
    _main_context_t *context = *context_pp;

    // note: 增加或删除要同步到module_create_t中
    module_destroy_t module[] = {
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

    // note: 增加或删除要同步到module_destroy_t中
    module_create_t module[] = {
        {"log",  &context->log_handle,   &log_config,    (create_t)HyLogCreate,         HyLogDestroy},
        {"aec",  &context->aec_handle,   &aec_config,    (create_t)HySpeexAecCreate,    HySpeexAecDestroy},
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
    }

    _module_destroy(&context);

    return 0;
}

