#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

XLAT_TABLES_LIB_SRCS	:=	$(addprefix lib/xlat_tables_v2/,	\
				${ARCH}/enable_mmu.S			\
				${ARCH}/xlat_tables_arch.c		\
				xlat_tables_internal.c)

INCLUDES		+=	-Ilib/xlat_tables_v2/${ARCH}
