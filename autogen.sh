#!/bin/sh
[ -e config.cache ] && rm -f config.cache

libtoolize --automake
#gtkdocize --flavour no-tmpl || exit 1
aclocal
autoconf
autoheader
automake -a
exit

