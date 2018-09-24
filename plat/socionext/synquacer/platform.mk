#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

override RESET_TO_BL31			:= 1
override MULTI_CONSOLE_API		:= 1
override PROGRAMMABLE_RESET_ADDRESS	:= 1
override USE_COHERENT_MEM		:= 1
override SEPARATE_CODE_AND_RODATA	:= 1
override ENABLE_SVE_FOR_NS		:= 0

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_855873		:= 1

# Libraries
include lib/xlat_tables_v2/xlat_tables.mk

ifeq (${SPD},opteed)
TF_CFLAGS_aarch64	+=	-DBL32_BASE=0xfc000000
endif

PLAT_PATH		:=	plat/socionext/synquacer
PLAT_INCLUDES		:=	-I$(PLAT_PATH)/include		\
				-I$(PLAT_PATH)/drivers/scpi	\
				-I$(PLAT_PATH)/drivers/mhu

PLAT_BL_COMMON_SOURCES	+=	$(PLAT_PATH)/sq_helpers.S		\
				drivers/arm/pl011/aarch64/pl011_console.S \
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				${XLAT_TABLES_LIB_SRCS}

BL31_SOURCES		+=	drivers/arm/ccn/ccn.c			\
				drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				$(PLAT_PATH)/sq_bl31_setup.c		\
				$(PLAT_PATH)/sq_ccn.c			\
				$(PLAT_PATH)/sq_topology.c		\
				$(PLAT_PATH)/sq_psci.c			\
				$(PLAT_PATH)/sq_gicv3.c			\
				$(PLAT_PATH)/sq_xlat_setup.c		\
				$(PLAT_PATH)/drivers/scpi/sq_scpi.c	\
				$(PLAT_PATH)/drivers/mhu/sq_mhu.c
