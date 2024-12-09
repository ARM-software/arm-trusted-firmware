#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

HOB_LIST_SOURCES	+=	$(addprefix lib/hob/,	\
				hob.c)

INCLUDES	+=	-Iinclude/lib/hob

BL31_SOURCES	+=	$(HOB_LIST_SOURCES)
