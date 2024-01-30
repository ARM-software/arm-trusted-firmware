#
# Copyright (c) 2020-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libc/libc_common.mk

ifeq (${ARCH},aarch64)
LIBC_SRCS	+=	$(addprefix lib/libc/aarch64/,	\
			memset.S)
else
LIBC_SRCS	+=	$(addprefix lib/libc/aarch32/,	\
			memset.S)
endif
