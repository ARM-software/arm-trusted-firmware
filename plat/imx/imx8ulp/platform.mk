#
# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

PLAT_INCLUDES		:=	-Iplat/imx/imx8ulp/include		\
				-Iplat/imx/common/include		\
				-Iplat/imx/imx8ulp/upower

IMX_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/lpuart_console.S	\
				plat/imx/common/imx8_helpers.S		\
				plat/imx/imx8ulp/imx8ulp_bl31_setup.c	\
				plat/imx/imx8ulp/imx8ulp_psci.c		\
				plat/imx/imx8ulp/apd_context.c		\
				plat/imx/common/imx8_topology.c		\
				plat/imx/common/imx_sip_svc.c		\
				plat/imx/common/imx_sip_handler.c	\
				plat/imx/common/imx_bl31_common.c	\
				plat/common/plat_psci_common.c		\
				lib/cpus/aarch64/cortex_a35.S		\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				plat/imx/imx8ulp/xrdc/xrdc_core.c		\
				plat/imx/imx8ulp/imx8ulp_caam.c         \
				plat/imx/imx8ulp/dram.c 	        \
				drivers/scmi-msg/base.c			\
				drivers/scmi-msg/entry.c		\
				drivers/scmi-msg/smt.c			\
				drivers/scmi-msg/power_domain.c		\
				drivers/scmi-msg/sensor.c		\
				plat/imx/imx8ulp/scmi/scmi.c		\
				plat/imx/imx8ulp/scmi/scmi_pd.c		\
				plat/imx/imx8ulp/scmi/scmi_sensor.c	\
				plat/imx/imx8ulp/upower/upower_api.c	\
				plat/imx/imx8ulp/upower/upower_hal.c	\
				${XLAT_TABLES_LIB_SRCS}			\
				${IMX_GIC_SOURCES}

ifeq ($(findstring clang,$(notdir $(CC))),)
    TF_CFLAGS_aarch64	+=	-fno-strict-aliasing
endif

USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1
SEPARATE_NOBITS_REGION	:=	1
SEPARATE_RWDATA_REGION	:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1
COLD_BOOT_SINGLE_CPU := 1
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
BL32_BASE		?=	0xa6000000
BL32_SIZE		?=	0x2000000
$(eval $(call add_define,BL32_BASE))
$(eval $(call add_define,BL32_SIZE))

ifeq (${SPD},trusty)
	BL31_CFLAGS    +=      -DPLAT_XLAT_TABLES_DYNAMIC=1
endif
