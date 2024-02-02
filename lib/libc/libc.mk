#
# Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

include lib/libc/libc_common.mk

LIBC_SRCS	+=	$(addprefix lib/libc/,		\
			memset.c)
