#
# Copyright (c) 2018-2026, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ZLIB_SOURCES	:=	contrib/zlib/adler32.c	\
			contrib/zlib/crc32.c	\
			contrib/zlib/inffast.c	\
			contrib/zlib/inflate.c	\
			contrib/zlib/inftrees.c	\
			contrib/zlib/zutil.c

# Implemented for TF
ZLIB_SOURCES	+=	lib/zlib/tf_gunzip.c

INCLUDES	+=	-Iinclude/lib/zlib -Icontrib/zlib

# REVISIT: the following flags need not be given globally
TF_CFLAGS	+=	-DZ_SOLO -DDEF_WBITS=31
