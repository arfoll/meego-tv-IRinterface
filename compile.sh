#!/bin/bash

# Compilaton script to compile meego-tv-IRinterface using Intel SDK
# Brendan Le Foll <brendan@fridu.net

#set the location of your Intel SDK
export INTEL_SDK_DIR="/home/cce-dev/IntelCE-21.2.11233.283228"

CFLAGS_OPT="-O2"
export TOOLCHAIN_DIR="$INTEL_SDK_DIR/build_i686/staging_dir"
export STAGING_DIR="$INTEL_SDK_DIR/build_i686/staging_dir"
export PATH="$TOOLCHAIN_DIR/bin:$PATH"
export AR="i686-cm-linux-ar"
export AS="i686-cm-linux-as"
export DEFAULT_ASSEMBLER="i686-cm-linux-as"
export CC="i686-cm-linux-gcc --sysroot=$STAGING_DIR"
export GCC="i686-cm-linux-gcc --sysroot=$STAGING_DIR"
export CPP="i686-cm-linux-cpp --sysroot=$STAGING_DIR"
export CXX="i686-cm-linux-g++ --sysroot=$STAGING_DIR"
export FC="i686-cm-linux-gfortran --sysroot=$STAGING_DIR"
export LD="i686-cm-linux-ld --sysroot=$STAGING_DIR"
export DEFAULT_LINKER="i686-cm-linux-ld --sysroot=$STAGING_DIR"
export OBJCOPY="i686-cm-linux-objcopy"
export NM="i686-cm-linux-nm"
export RANLIB="i686-cm-linux-ranlib"
export STRIP="i686-cm-linux-strip"
export CFLAGS="-I$STAGING_DIR/usr/include -I$STAGING_DIR/include/pic24 -I$STAGING_DIR/include -I$STAGING_DIR/include/linux_user -I$STAGING_DIR/idts_common/  $CFLAGS_OPT"
export CXXFLAGS="$CFLAGS -I$STAGING_DIR/usr/include/c++/4.5.1 -I$STAGING_DIR/usr/include/c++/4.5.1/i686-cm-linux $CFLAGS_OPT"
export LDFLAGS="-L$STAGING_DIR/usr/lib -L$STAGING_DIR/lib"
export PKG_CONFIG_LIBDIR="$STAGING_DIR/usr/lib/pkgconfig"
export PKG_CONFIG_PATH="$STAGING_DIR/usr/lib/pkgconfig"

#run configure
./configure --build=i686-redhat-linux --host=i686-cm-linux --prefix=${STAGING_DIR}/usr --disable-static

#compile
make

