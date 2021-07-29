dnl ===============================================================
dnl 
dnl Release under GPLv-3.0.
dnl 
dnl @file    check_libevent_pthread.m4
dnl @brief   
dnl @author  gnsyxiang <gnsyxiang@163.com>
dnl @date    08/05 2021 10:29
dnl @version v0.0.1
dnl 
dnl @since    note
dnl @note     note
dnl 
dnl     change log:
dnl     NO.     Author              Date            Modified
dnl     00      zhenquan.qiu        08/05 2021      create the file
dnl 
dnl     last modified: 08/05 2021 10:29
dnl ===============================================================

# CHECK_LIBEVENT_PTHREAD()
# --------------------------------------------------------------
# check libevent_pthread

AC_DEFUN([CHECK_LIBEVENT_PTHREAD], [

    AC_ARG_ENABLE([event_pthread],
        [AS_HELP_STRING([--disable-event_pthread], [disable support for event_pthread])],
            [], [enable_event_pthread=yes])

    case "$enable_event_pthread" in
        yes)
            have_event_pthread=no

            case "$PKG_CONFIG" in
                '') ;;
                *)
                    EVENT_PTHREAD_LIBS=`$PKG_CONFIG --libs libevent_pthreads 2>/dev/null`

                    case "$EVENT_PTHREAD_LIBS" in
                        '') ;;
                        *)
                            EVENT_PTHREAD_LIBS="$EVENT_PTHREAD_LIBS"
                            have_event_pthread=yes
                        ;;
                    esac

                    EVENT_PTHREAD_INCS=`$PKG_CONFIG --cflags libevent_pthreads 2>/dev/null`
                ;;
            esac

            case "$have_event_pthread" in
                yes) ;;
                *)
                    save_LIBS="$LIBS"
                    LIBS=""
                    EVENT_PTHREAD_LIBS=""

                    # clear cache
                    unset ac_cv_search_HyMalloc
                    AC_SEARCH_LIBS([evthread_use_pthreads], [event_pthreads event],
                            [have_event_pthread=yes
                                EVENT_PTHREAD_LIBS="$LIBS"],
                            [have_event_pthread=no],
                            [])
                    LIBS="$save_LIBS"
                ;;
            esac

            CPPFLAGS_SAVE=$CPPFLAGS
            CPPFLAGS="$CPPFLAGS $EVENT_PTHREAD_INCS"
            AC_CHECK_HEADERS([event2/thread.h], [], [evthread_use_pthreads=no])

            CPPFLAGS=$CPPFLAGS_SAVE
            AC_SUBST(EVENT_PTHREAD_INCS)
            AC_SUBST(EVENT_PTHREAD_LIBS)

            case "$have_event_pthread" in
                yes)
                    AC_DEFINE(HAVE_EVENT_PTHREAD, 1, [Define if the system has event_pthread])
                ;;
                *)
                    AC_MSG_ERROR([event_pthread is a must but can not be found. You should add the \
directory containing `event_pthread.pc' to the `PKG_CONFIG_PATH' environment variable, \
or set `CPPFLAGS' and `LDFLAGS' directly for event_pthread, or use `--disable-event_pthread' \
to disable support for event_pthread encryption])
                ;;
            esac
        ;;
    esac

    # check if we have and should use event_pthread
    AM_CONDITIONAL(COMPILE_LIBEVENT_PTHREAD, [test "$enable_event_pthread" != "no" && test "$have_event_pthread" = "yes"])
])

