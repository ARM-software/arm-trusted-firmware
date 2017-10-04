#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PSCI_LIB_SOURCES	:=	lib/el3_runtime/cpu_data_array.c	\
				lib/el3_runtime/${ARCH}/cpu_data.S	\
				lib/el3_runtime/${ARCH}/context_mgmt.c	\
				lib/cpus/${ARCH}/cpu_helpers.S		\
				lib/cpus/errata_report.c		\
				lib/locks/exclusive/${ARCH}/spinlock.S	\
				lib/psci/psci_off.c			\
				lib/psci/psci_on.c			\
				lib/psci/psci_suspend.c			\
				lib/psci/psci_common.c			\
				lib/psci/psci_main.c			\
				lib/psci/psci_setup.c			\
				lib/psci/psci_system_off.c		\
				lib/psci/psci_mem_protect.c		\
				lib/psci/${ARCH}/psci_helpers.S

ifeq (${ARCH}, aarch64)
PSCI_LIB_SOURCES	+=	lib/el3_runtime/aarch64/context.S
endif

ifeq (${USE_COHERENT_MEM}, 1)
PSCI_LIB_SOURCES		+=	lib/locks/bakery/bakery_lock_coherent.c
else
PSCI_LIB_SOURCES		+=	lib/locks/bakery/bakery_lock_normal.c
endif

ifeq (${ENABLE_PSCI_STAT}, 1)
PSCI_LIB_SOURCES		+=	lib/psci/psci_stat.c
endif
