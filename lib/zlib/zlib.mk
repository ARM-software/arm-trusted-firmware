#
# Copyright (c) 2018-2026, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBZLIB_SRCS	:=	contrib/zlib/adler32.c	\
			contrib/zlib/crc32.c	\
			contrib/zlib/inffast.c	\
			contrib/zlib/inflate.c	\
			contrib/zlib/inftrees.c	\
			contrib/zlib/zutil.c

# Implemented for TF
LIBZLIB_SRCS	+=	lib/zlib/tf_gunzip.c

INCLUDES	+=	-Iinclude/lib/zlib -Icontrib/zlib

LIBZLIB_CFLAGS	:=	-DZ_SOLO -DDEF_WBITS=31
$(eval $(call MAKE_LIB,zlib))
