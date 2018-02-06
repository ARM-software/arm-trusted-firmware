#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ZLIB_PATH	:=	lib/zlib

# Imported from zlib 1.2.11 (do not modify them)
ZLIB_SOURCES	:=	$(addprefix $(ZLIB_PATH)/,	\
					adler32.c	\
					crc32.c		\
					inffast.c	\
					inflate.c	\
					inftrees.c	\
					zutil.c)

# Implemented for TF
ZLIB_SOURCES	+=	$(addprefix $(ZLIB_PATH)/,	\
					tf_gunzip.c)

INCLUDES	+=	-Iinclude/lib/zlib

# REVISIT: the following flags need not be given globally
TF_CFLAGS	+=	-DZ_SOLO -DDEF_WBITS=31
