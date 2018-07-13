#
# Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ARM_CORTEX_A7		:=	yes
ARM_WITH_NEON		:=	yes
LOAD_IMAGE_V2		:=	1
BL2_AT_EL3		:=	1
ENABLE_PLAT_COMPAT	:=	0
USE_COHERENT_MEM	:=	0

STM32_TF_VERSION	?=	0

# Not needed for Cortex-A7
WORKAROUND_CVE_2017_5715:=	0

PLAT_INCLUDES		:=	-Iplat/st/stm32mp1/include/
PLAT_INCLUDES		+=	-Iinclude/common/tbbr
PLAT_INCLUDES		+=	-Iinclude/drivers/st

include lib/libfdt/libfdt.mk

PLAT_BL_COMMON_SOURCES	:=	plat/st/stm32mp1/stm32mp1_common.c

PLAT_BL_COMMON_SOURCES	+=	drivers/console/aarch32/console.S			\
				drivers/st/uart/aarch32/stm32_console.S

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/st/stm32mp1/stm32mp1_stack_protector.c
endif

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch32/cortex_a7.S

PLAT_BL_COMMON_SOURCES	+=	${LIBFDT_SRCS}						\
				drivers/delay_timer/delay_timer.c			\
				drivers/delay_timer/generic_delay_timer.c		\
				drivers/st/clk/stm32mp1_clk.c				\
				drivers/st/clk/stm32mp1_clkfunc.c			\
				drivers/st/gpio/stm32_gpio.c				\
				drivers/st/reset/stm32mp1_reset.c			\
				plat/st/stm32mp1/stm32mp1_dt.c				\
				plat/st/stm32mp1/stm32mp1_helper.S

BL2_SOURCES		+=	drivers/io/io_dummy.c					\
				drivers/io/io_storage.c					\
				plat/st/stm32mp1/bl2_io_storage.c			\
				plat/st/stm32mp1/bl2_plat_setup.c

BL2_SOURCES		+=	common/desc_image_load.c				\
				plat/st/stm32mp1/plat_bl2_mem_params_desc.c		\
				plat/st/stm32mp1/plat_image_load.c

# For memory footprint optimization, build with thumb and interwork support
ASFLAGS			+=	-mthumb -mthumb-interwork
TF_CFLAGS		+=	-mthumb -mthumb-interwork
