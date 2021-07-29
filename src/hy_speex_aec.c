/**
 * 
 * Release under GPLv-3.0.
 * 
 * @file    hy_speex_aec.c
 * @brief   
 * @author  gnsyxiang <gnsyxiang@163.com>
 * @date    05/03 2021 10:46
 * @version v0.0.1
 * 
 * @since    note
 * @note     note
 * 
 *     change log:
 *     NO.     Author              Date            Modified
 *     00      zhenquan.qiu        05/03 2021      create the file
 * 
 *     last modified: 05/03 2021 10:46
 */
#include <stdio.h>

#include "hy_speex_aec.h"

#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

#include "hy_utils/hy_mem.h"
#include "hy_utils/hy_string.h"
#include "hy_utils/hy_assert.h"
#include "hy_utils/hy_log.h"

#define ALONE_DEBUG 1

typedef struct {
   SpeexEchoState       *st;
   SpeexPreprocessState *den;
} _speex_aec_context_t;

hy_s32_t HySpeexAecProcess(void *handle, void *in_frame, void *speaker_frame, void *out_frame)
{
    HY_ASSERT_NULL_RET_VAL(!handle || !in_frame || !speaker_frame || !out_frame, -1);

    _speex_aec_context_t *context = handle;

    speex_echo_cancellation(context->st, in_frame, speaker_frame, out_frame);
    speex_preprocess_run(context->den, out_frame);

    return 0;
}

void HySpeexAecDestroy(void **handle)
{
    HY_ASSERT_NULL_RET(!handle || !*handle);

    _speex_aec_context_t *context = *handle;

    speex_echo_state_destroy(context->st);
    speex_preprocess_state_destroy(context->den);

    HY_FREE(handle);
}

void *HySpeexAecCreate(HySpeexAecConfig_t *aec_config)
{
    _speex_aec_context_t *context = NULL;

    do {
        context = (_speex_aec_context_t *)HY_MALLOC_BREAK(sizeof(*context));

        context->st = speex_echo_state_init(aec_config->frame_size, aec_config->filter_length);
        if (!context->st) {
            LOGE("speex_echo_state_init faild \n");
            break;
        }

        context->den = speex_preprocess_state_init(aec_config->frame_size, aec_config->sample_rate);
        if (!context->den) {
            LOGE("speex_preprocess_state_init faild \n");
            break;
        }

        return context;
    } while (0);

    HySpeexAecDestroy((void **)&context);

    return NULL;
}

