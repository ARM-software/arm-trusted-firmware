#
# Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

XLAT_TABLES_LIB_SRCS	:=	$(addprefix lib/xlat_tables_v2/,	\
				${ARCH}/enable_mmu.S			\
				${ARCH}/xlat_tables_arch.c		\
				xlat_tables_context.c			\
				xlat_tables_core.c			\
				xlat_tables_utils.c)

XLAT_TABLES_LIB_V2	:=	1
$(eval $(call add_define,XLAT_TABLES_LIB_V2))

ifeq (${ALLOW_RO_XLAT_TABLES}, 1)
    include lib/xlat_tables_v2/ro_xlat_tables.mk
endif
