dnl ===============================================================
dnl 
dnl Release under GPLv-3.0.
dnl 
dnl @file    check_speexdsp.m4
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

# CHECK_SPEEXDSP()
# --------------------------------------------------------------
# check speexdsp

AC_DEFUN([CHECK_SPEEXDSP], [

    AC_ARG_ENABLE([speexdsp],
        [AS_HELP_STRING([--disable-speexdsp], [disable support for speexdsp])],
            [], [enable_speexdsp=yes])

    case "$enable_speexdsp" in
        yes)
            have_speexdsp=no

            case "$PKG_CONFIG" in
                '') ;;
                *)
                    SPEEXDSP_LIBS=`$PKG_CONFIG --libs speexdsp 2>/dev/null`

                    case "$SPEEXDSP_LIBS" in
                        '') ;;
                        *)
                            SPEEXDSP_LIBS="$SPEEXDSP_LIBS"
                            have_speexdsp=yes
                        ;;
                    esac

                    SPEEXDSP_INCS=`$PKG_CONFIG --cflags speexdsp 2>/dev/null`
                ;;
            esac

            case "$have_speexdsp" in
                yes) ;;
                *)
                    save_LIBS="$LIBS"
                    LIBS=""
                    SPEEXDSP_LIBS=""

                    # clear cache
                    unset ac_cv_search_speex_echo_state_init
                    AC_SEARCH_LIBS([speex_echo_state_init], [speexdsp],
                            [have_speexdsp=yes
                                SPEEXDSP_LIBS="$LIBS"],
                            [have_speexdsp=no],
                            [])
                    LIBS="$save_LIBS"
                ;;
            esac

            CPPFLAGS_SAVE=$CPPFLAGS
            CPPFLAGS="$CPPFLAGS $SPEEXDSP_INCS"
            AC_CHECK_HEADERS([speex/speex_echo.h], [], [have_speexdsp=no])

            CPPFLAGS=$CPPFLAGS_SAVE
            AC_SUBST(SPEEXDSP_INCS)
            AC_SUBST(SPEEXDSP_LIBS)

            case "$have_speexdsp" in
                yes)
                    AC_DEFINE(HAVE_speexdsp, 1, [Define if the system has speexdsp])
                ;;
                *)
                    AC_MSG_ERROR([speexdsp is a must but can not be found. You should add the \
directory containing `speexdsp.pc' to the `PKG_CONFIG_PATH' environment variable, \
or set `CPPFLAGS' and `LDFLAGS' directly for speexdsp, or use `--disable-speexdsp' \
to disable support for speexdsp encryption])
                ;;
            esac
        ;;
    esac

    # check if we have and should use speexdsp
    AM_CONDITIONAL(COMPILE_SPEEXDSP, [test "$enable_speexdsp" != "no" && test "$have_speexdsp" = "yes"])
])

