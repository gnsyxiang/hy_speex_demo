/**
 *
 * Release under GPLv-3.0.
 * 
 * @file    hy_speex_aec.h
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
#ifndef __HY_SPEEX_DEMO_INCLUDE_HY_SPEEX_AEC_H_
#define __HY_SPEEX_DEMO_INCLUDE_HY_SPEEX_AEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hy_utils/hy_type.h"

typedef struct {
    hy_u32_t sample_rate;
    hy_u32_t frame_size;
    hy_u32_t filter_length;
} HySpeexAecConfig_t;

void *HySpeexAecCreate(HySpeexAecConfig_t *aec_config);
void HySpeexAecDestroy(void **handle);

hy_s32_t HySpeexAecProcess(void *handle, void *in_frame, void *speaker_frame, void *out_frame);

#ifdef __cplusplus
}
#endif

#endif

