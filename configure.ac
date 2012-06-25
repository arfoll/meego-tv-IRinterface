AC_PREREQ(2.61)

# package version number (not shared library version)
# odd micro numbers indicate in-progress development
# even micro numbers indicate released versions
m4_define([ir_interface_version_major], [0])
m4_define([ir_interface_version_minor], [3])
m4_define([ir_interface_version_micro], [0])

m4_define([ir_interface_version],
          [ir_interface_version_major.ir_interface_version_minor.ir_interface_version_micro])
m4_define([ir_interface_api_version],
          [ir_interface_version_major.ir_interface_version_minor])

AC_INIT(ir_interface, ir_interface_version)
AM_INIT_AUTOMAKE(ir_interface, ir_interface_version)
AC_CONFIG_HEADERS(config.h)

IR_INTERFACE_API_VERSION=ir_interface_api_version
AC_SUBST(IR_INTERFACE_API_VERSION)

# Check for programs
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_CXX
# enable pkg-config
PKG_PROG_PKG_CONFIG

#Check for headers
if test "x$prefix" = "xNONE" ; then
  prefix="/usr"
fi

AC_LANG_PUSH([C++])

CPPFLAGS="$CPPFLAGS -I$prefix/include/CE4100/ -I$prefix/include/CE4100/picuart/"


#AC_CHECK_HEADERS([LR_PICInterface.h],,exit 1,)

# Check for libs

AC_CHECK_LIB(osal, os_thread_create, , exit 1, )
AC_CHECK_LIB(picuart, PicInitIR, , exit 1,)




# Add --enable-debug arg
AC_ARG_ENABLE(debug,
              [  --enable-debug            Turn on debugging], ,
              enable_debug=no)
if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility])
fi

# Output files
AC_OUTPUT([Makefile
main/Makefile
tests/Makefile
scripts/Makefile
])


