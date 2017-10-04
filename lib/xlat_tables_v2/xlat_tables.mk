#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

XLAT_TABLES_LIB_SRCS	:=	$(addprefix lib/xlat_tables_v2/,	\
				${ARCH}/xlat_tables_arch.c		\
				xlat_tables_internal.c)

INCLUDES		+=	-Ilib/xlat_tables_v2/${ARCH}
