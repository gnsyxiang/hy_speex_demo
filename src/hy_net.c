/**
 * 
 * Release under GPLv-3.0.
 * 
 * @file    hy_net.c
 * @brief   
 * @author  gnsyxiang <gnsyxiang@163.com>
 * @date    29/07 2021 11:04
 * @version v0.0.1
 * 
 * @since    note
 * @note     note
 * 
 *     change log:
 *     NO.     Author              Date            Modified
 *     00      zhenquan.qiu        29/07 2021      create the file
 * 
 *     last modified: 29/07 2021 11:04
 */
#include <stdio.h>
#include <signal.h>
#include <pthread.h>

#include "hy_net.h"

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>

#include "hy_utils/hy_mem.h"
#include "hy_utils/hy_string.h"
#include "hy_utils/hy_assert.h"
#include "hy_utils/hy_error.h"
#include "hy_utils/hy_log.h"

#define ALONE_DEBUG 1

#define _TEST_SIGNAL

#define _NET_STATE_CB(context, state)                           \
    do {                                                        \
        if (context && context->config_save.state_cb) {         \
            context->config_save.state_cb(state, context->config_save.args); \
        }                                                       \
    } while (0)

typedef struct {
    HyNetConfigSave_t   config_save;

    struct event_base   *base;
    struct bufferevent  *bev;
    pthread_t           id;
    struct event        **signal_ev;
} _net_context_t;

hy_s32_t HyNetWrite(void *handle, void *buf, hy_u32_t len)
{
    HY_ASSERT_NULL_RET_VAL(!handle || !buf, -1);
    _net_context_t *context = handle;

    return bufferevent_write(context->bev, buf, len);
}

static void _socket_read_cb(struct bufferevent *bev, void *arg)
{
    _net_context_t *context = arg;

    char buf[1024] = {0}; 
    size_t ret = bufferevent_read(bev, buf, sizeof(buf));

    if (context->config_save.data_cb) {
        context->config_save.data_cb(buf, ret, context->config_save.args);
    }
}

static void _socket_write_cb(struct bufferevent *bev, void *arg)
{
}

static void _socket_event_cb(struct bufferevent *bev, short events, void *arg)
{
    _net_context_t *context = arg;

    if (events & BEV_EVENT_EOF) {
        LOGE("connection closed \n");
    } else if(events & BEV_EVENT_ERROR) {
        LOGE("some other error \n");
    } else if(events & BEV_EVENT_CONNECTED) {
        LOGI("connect to the server \n");

        _NET_STATE_CB(context, HY_NET_STATE_CONNECTED);

        return;
    }

    bufferevent_free(context->bev);
    context->bev = NULL;
}

static void *_dispatch_loop(void *args)
{
    _net_context_t *context = args;

    event_base_dispatch(context->base);

    _NET_STATE_CB(context, HY_NET_STATE_DISCONNECT);

    return NULL;
}

static void _libevent_destroy(_net_context_t *context)
{
    if (context->bev) {
        bufferevent_free(context->bev);
    }

    if (context->base) {
        event_base_free(context->base);
    }
}

static hy_s32_t _libevent_create(_net_context_t *context, HyNetConfig_t *net_config)
{
    do {
#ifdef _WIN32
        evthread_use_windows_threads();
#elif __GNUC__
        evthread_use_pthreads();
#else
#error "use threads error"
#endif

#if 0
        struct event_config* conf = event_config_new();

        //set特征,设置了EV_FEATURE_FDS后其他特征就无法设置
        //event_config_require_features(conf, EV_FEATURE_ET | EV_FEATURE_FDS);
        //event_config_require_features(conf, EV_FEATURE_FDS);

        /*通过‘避免’来取消epoll和poll的支持*/
        event_config_avoid_method(conf, "epoll");
        event_config_avoid_method(conf, "poll");

        context->base = event_base_new_with_config(conf);
        event_config_free(conf);
#else
        context->base = event_base_new();
#endif
        if (!context->base) {
            LOGE("event_base_new faild \n");
            break;
        }

#if 1
        LOGI("supported_methods: \n");
        const char** methods = event_get_supported_methods();
        for(int i = 0; methods[i] != NULL; i++) {
            LOGI("\t%s \n", methods[i]);
        }
        LOGI("current method is: %s \n", event_base_get_method(context->base));

        int f =	event_base_get_features(context->base);
        if(f & EV_FEATURE_ET) {
            LOGI("EV_FEATURE_ET events are supported. \n");
        } else {
            LOGI("EV_FEATURE_ET events are not supported. \n");
        }
#endif

        context->bev = bufferevent_socket_new(context->base,
                -1, BEV_OPT_CLOSE_ON_FREE);
        if (!context->bev) {
            LOGE("bufferevent_socket_new faild \n");
            break;
        }

        struct sockaddr_in serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_port = htons(net_config->port);
        evutil_inet_pton(AF_INET, net_config->ip, &serv.sin_addr.s_addr);

        if (0 != bufferevent_socket_connect(context->bev,
                    (struct sockaddr*)&serv, sizeof(serv))) {
            LOGE("bufferevent_socket_connect faild \n");
            break;
        }

        bufferevent_setcb(context->bev, _socket_read_cb,
                _socket_write_cb, _socket_event_cb, context);
        bufferevent_enable(context->bev, EV_READ | EV_PERSIST);

        _NET_STATE_CB(context, HY_NET_STATE_CONNECTING);

        return 0;
    } while(0);

    _libevent_destroy(context);

    return -1;
}

static void _ctrl_c(int sock, short which, void* args)
{
    _net_context_t *context = (_net_context_t *)args;	

#ifndef _TEST_SIGNAL
    //如果处于非待决状态，则再次进入
    if (!evsignal_pending(*context->signal_ev, NULL)) {
        event_del(*context->signal_ev);
        event_add(*context->signal_ev, NULL);
    }
#endif

    if (context && context->config_save.signal_cb) {
        context->config_save.signal_cb(sock, context->config_save.args);
    }
}

void HyNetDestroy(void **handle)
{
    LOGT("%s:%d \n", __func__, __LINE__);
    HY_ASSERT_NULL_RET(!handle || !*handle);

    _net_context_t *context = *handle;

    if (context->id) {
        event_base_loopexit(context->base, NULL);
        pthread_join(context->id, NULL);
    }

    _libevent_destroy(context);

    HY_FREE(handle);
}

void *HyNetCreate(HyNetConfig_t *net_config)
{
    LOGT("%s:%d \n", __func__, __LINE__);
    HY_ASSERT_NULL_RET_VAL(!net_config, NULL);

    _net_context_t *context = NULL;

    do {
        context = (_net_context_t *)HY_MALLOC_RET_VAL(sizeof(*context), NULL);

        HY_MEMCPY(&context->config_save, &net_config->config_save);

        if (0 != _libevent_create(context, net_config)) {
            LOGE("_libevent_create faild \n");
            break;
        }

        struct event *signal = NULL;
        context->signal_ev = &signal;

#ifdef _TEST_SIGNAL
        //evsignal_new隐藏的状态 EV_SIGNAL|EV_PERSIST，处于no pending
        signal = evsignal_new(context->base, SIGINT, _ctrl_c, context);
#else
        //非持久信号，只进入一次, event_self_cbarg()传递当前的event
        // signal = event_new(context->base, SIGINT, EV_SIGNAL, _ctrl_c, event_self_cbarg());
        signal = event_new(context->base, SIGINT, EV_SIGNAL, _ctrl_c, context);
#endif
        if (!signal) {
            LOGE("SIGINT evsignale_new failed! \n");
            break;
        }

        //使事件处于pending状态
        if (event_add(signal, 0) != 0) {
            LOGE("SIGINT event_add failed! \n");
            break;
        }

        if (0 != pthread_create(&context->id, NULL, _dispatch_loop, context)) {
            LOGE("pthread_create faild \n");
            break;
        }

        return context;
    } while (0);

    HyNetDestroy((void **)&context);

    return NULL;
}

