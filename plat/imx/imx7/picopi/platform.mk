#
# Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Include imx7 common
include plat/imx/imx7/common/imx7.mk

# Platform
PLAT_INCLUDES		+=	-Iplat/imx/imx7/picopi/include		    \

BL2_SOURCES		+=	drivers/imx/usdhc/imx_usdhc.c		    \
				plat/imx/imx7/picopi/picopi_bl2_el3_setup.c \

# Build config flags
# ------------------

ARM_CORTEX_A7			:= yes
WORKAROUND_CVE_2017_5715	:= 0

RESET_TO_BL31			:= 0

# Non-TF Boot ROM
BL2_AT_EL3			:= 1

# Indicate single-core
COLD_BOOT_SINGLE_CPU		:= 1

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# Use Coherent memory
USE_COHERENT_MEM		:= 1

# Use multi console API
MULTI_CONSOLE_API		:= 1

PLAT_PICOPI_UART		:=5
$(eval $(call add_define,PLAT_PICOPI_UART))
