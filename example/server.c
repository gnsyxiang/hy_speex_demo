/**
 * 
 * Release under GPLv-3.0.
 * 
 * @file    server.c
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
#include <pthread.h>
#include <ctype.h>

#include "hy_speex_aec.h"

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <event2/listener.h>
#include <event2/buffer.h>

#include "hy_utils/hy_module.h"
#include "hy_utils/hy_mem.h"
#include "hy_utils/hy_string.h"
#include "hy_utils/hy_assert.h"
#include "hy_utils/hy_log.h"

#define ALONE_DEBUG 1

#define FRAME_SIZE 320

typedef struct {
    void *log_handle;
    void *aec_handle;
    void *net_handle;
} _main_context_t;

typedef struct {
    void (*state_cb)(hy_s32_t state, void *args);
    void (*data_cb)(void *buf, size_t len, void *args);
    void *args;
} _server_save_config_t;

typedef struct {
    char        *ip;
    hy_u16_t    port;

    _server_save_config_t save_config;
} _server_config_t;

typedef struct {
    _server_save_config_t save_config;

    struct event_base   *base;
    struct evconnlistener* ev;

    pthread_t           id;

} _server_context_t;

#define _READ_FRAME (1024)

static enum bufferevent_filter_result _filter_in_cb(
        struct evbuffer *src, struct evbuffer *dst,
        ev_ssize_t dst_limit, enum bufferevent_flush_mode mode, void *ctx)
{
    char data[_READ_FRAME] = {0};
    hy_s32_t len = evbuffer_remove(src, data, sizeof(data));

    // 解密处理或者解缩处理
    hy_s32_t i;
    for (i = 0; i < _READ_FRAME; ++i) {
        data[i] = toupper(data[i]);
    }

    evbuffer_add(dst, data, len);

    return BEV_OK;
}

static enum bufferevent_filter_result _filter_out_cb(
        struct evbuffer *src, struct evbuffer *dst,
        ev_ssize_t dst_limit, enum bufferevent_flush_mode mode, void *ctx)
{
    char data[_READ_FRAME] = {0};
    hy_s32_t len = evbuffer_remove(src, data, sizeof(data));

    // 加密处理或者压缩处理
    hy_s32_t i;
    for (i = 0; i < _READ_FRAME; ++i) {
        data[i] = toupper(data[i]);
    }

    evbuffer_add(dst, data, len);

    return BEV_OK;
}

static void _read_cb(struct bufferevent *bev, void *arg)
{
    char buf[_READ_FRAME] = {0}; 
    size_t ret = bufferevent_read(bev, buf, sizeof(buf));

    LOGE("---------------haha: %s \n", buf);
}

static void _write_cb(struct bufferevent *bev, void *arg)
{
}

static void _event_cb(struct bufferevent *bev, hy_s16_t events, void *arg)
{
    if (events & BEV_EVENT_EOF) {
        LOGE("connection closed \n");
    } else if(events & BEV_EVENT_ERROR) {
        LOGE("some other error \n");
    } else if(events & BEV_EVENT_CONNECTED) {
    } else {
        LOGE("others \n");
    }
}

static void listen_cb(struct evconnlistener* e, evutil_socket_t s, struct sockaddr* a, int socklen, void* arg)
{
    _server_context_t *context = (_server_context_t *)arg;

    struct bufferevent* bev = bufferevent_socket_new(context->base, s, BEV_OPT_CLOSE_ON_FREE);

    struct bufferevent* bev_filter = bufferevent_filter_new(bev, 
            _filter_in_cb,
            _filter_out_cb,
            BEV_OPT_CLOSE_ON_FREE,
            0,
            0);

    bufferevent_setcb(bev_filter, _read_cb, _write_cb, _event_cb, NULL);

    bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}

static void *_dispatch_loop(void *args)
{
    _server_context_t *context = args;

    event_base_dispatch(context->base);

    return NULL;
}

static void _server_destroy(void **handle)
{

}

static _server_context_t *_server_create(_server_config_t *server_config)
{
    HY_ASSERT_NULL_RET_VAL(!server_config, NULL);
    _server_context_t *context = NULL;

    do {
        context = (_server_context_t *)HY_MALLOC_BREAK(sizeof(*context));

        HY_MEMCPY(&context->save_config, &server_config->save_config);

        context->base = event_base_new();
        if (!context->base) {
            LOGE("event_base_new faild \n");
            break;
        }

        struct sockaddr_in serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_port   = htons(server_config->port);

        context->ev = evconnlistener_new_bind(context->base,
                listen_cb,
                context,
                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                10,
                (struct sockaddr*)&serv,
                sizeof(serv));

        if (0 != pthread_create(&context->id, NULL, _dispatch_loop, context)) {
            LOGE("pthread_create faild \n");
            break;
        }

        return context;
    } while (0);

    return NULL;
}

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

    _server_config_t server_config;
    server_config.ip    = "";
    server_config.port  = 7809;

    // note: 增加或删除要同步到module_destroy_t中
    module_create_t module[] = {
        {"log",  &context->log_handle,   &log_config,    (create_t)HyLogCreate,         HyLogDestroy},
        {"aec",  &context->aec_handle,   &aec_config,    (create_t)HySpeexAecCreate,    HySpeexAecDestroy},
        {"net",  &context->net_handle,   &server_config, (create_t)_server_create,      _server_destroy},
    };

    RUN_CREATE(module);

    return context;
}

int main(int argc, char *argv[])
{
    _main_context_t *context = _module_create();
    if (!context) {
        LOGE("_module_create faild \n");
        return -1;
    }


    while (1) {

    }

    _module_destroy(&context);

    return 0;
}

#if 0
int main(int argc, char const* argv[])
{
    _main_context_t *context = _module_create();
    if (!context) {
        LOGE("_module_create faild \n");
        return -1;
    }

    if (argc != 4) {
        LOGE("./client mic_signal.sw speaker_signal.sw output.sw\n");
        return -1;
    }

    FILE * echo_fd = fopen(argv[2], "rb");
    FILE * ref_fd  = fopen(argv[1],  "rb");
    FILE * e_fd    = fopen(argv[3], "wb");
    // FILE * echo_frame_fd = fopen("echo_frame.pcm", "wb");

    short in_frame[FRAME_SIZE] = {0};
    short speaker_frame[FRAME_SIZE] = {0};
    short out_frame[FRAME_SIZE] = {0};

    while (!feof(ref_fd) && !feof(echo_fd)) {
        fread(in_frame, sizeof(short), FRAME_SIZE, ref_fd);
        fread(speaker_frame, sizeof(short), FRAME_SIZE, echo_fd);

        HySpeexAecProcess(context->aec_handle, in_frame, speaker_frame, out_frame);

        fwrite(out_frame, sizeof(short), FRAME_SIZE, e_fd);
    }

    fclose(e_fd);
    fclose(echo_fd);
    fclose(ref_fd);

    _module_destroy(&context);

    return 0;
}
#endif

