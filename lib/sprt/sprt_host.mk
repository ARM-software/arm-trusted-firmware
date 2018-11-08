#
# Copyright (c) 2018, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SPRT_LIB_SOURCES	:=	$(addprefix lib/sprt/,			\
					sprt_host.c			\
					sprt_queue.c)

SPRT_LIB_INCLUDES	:=	-Iinclude/lib/sprt/
