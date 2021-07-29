dnl ===============================================================
dnl 
dnl Release under GPLv-3.0.
dnl 
dnl @file    check_libevent.m4
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

# CHECK_LIBEVENT()
# --------------------------------------------------------------
# check libevent

AC_DEFUN([CHECK_LIBEVENT], [

    AC_ARG_ENABLE([event],
        [AS_HELP_STRING([--disable-event], [disable support for event])],
            [], [enable_event=yes])

    case "$enable_event" in
        yes)
            have_event=no

            case "$PKG_CONFIG" in
                '') ;;
                *)
                    EVENT_LIBS=`$PKG_CONFIG --libs libevent 2>/dev/null`

                    case "$EVENT_LIBS" in
                        '') ;;
                        *)
                            EVENT_LIBS="$EVENT_LIBS"
                            have_event=yes
                        ;;
                    esac

                    EVENT_INCS=`$PKG_CONFIG --cflags libevent 2>/dev/null`
                ;;
            esac

            case "$have_event" in
                yes) ;;
                *)
                    save_LIBS="$LIBS"
                    LIBS=""
                    EVENT_LIBS=""

                    # clear cache
                    unset ac_cv_search_HyMalloc
                    AC_SEARCH_LIBS([event_base_new], [event],
                            [have_event=yes
                                EVENT_LIBS="$LIBS"],
                            [have_event=no],
                            [])
                    LIBS="$save_LIBS"
                ;;
            esac

            CPPFLAGS_SAVE=$CPPFLAGS
            CPPFLAGS="$CPPFLAGS $EVENT_INCS"
            AC_CHECK_HEADERS([event2/event.h], [], [have_event=no])

            CPPFLAGS=$CPPFLAGS_SAVE
            AC_SUBST(EVENT_INCS)
            AC_SUBST(EVENT_LIBS)

            case "$have_event" in
                yes)
                    AC_DEFINE(HAVE_EVENT, 1, [Define if the system has event])
                ;;
                *)
                    AC_MSG_ERROR([event is a must but can not be found. You should add the \
directory containing `event.pc' to the `PKG_CONFIG_PATH' environment variable, \
or set `CPPFLAGS' and `LDFLAGS' directly for event, or use `--disable-event' \
to disable support for event encryption])
                ;;
            esac
        ;;
    esac

    # check if we have and should use event
    AM_CONDITIONAL(COMPILE_LIBEVENT, [test "$enable_event" != "no" && test "$have_event" = "yes"])
])

