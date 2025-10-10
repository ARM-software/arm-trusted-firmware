#
# Copyright 2024-2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		:=	-Iplat/imx/common/include		\
				-Iplat/imx/imx9/common/include		\
				-Iplat/imx/imx9/imx94/include		\
				-Idrivers/arm/css/scmi			\
				-Idrivers/nxp/scmi/vendor

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

USE_GIC_DRIVER=3
GICV3_SUPPORT_GIC600  :=      1

BL31_SOURCES		+=	drivers/arm/css/scmi/scmi_common.c		\
				drivers/arm/css/scmi/scmi_base_proto.c		\
				drivers/arm/css/scmi/scmi_pwr_dmn_proto.c	\
				drivers/arm/css/scmi/scmi_sys_pwr_proto.c	\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/nxp/scmi/vendor/scmi_imx9.c		\
				plat/common/aarch64/crash_console_helpers.S     \
				plat/common/plat_psci_common.c			\
				plat/imx/common/imx_bl31_common.c		\
				plat/imx/common/imx_sip_handler.c		\
				plat/imx/common/imx_sip_svc.c			\
				plat/imx/common/imx9_sm_sema.c                  \
				plat/imx/common/lpuart_console.S		\
				plat/imx/common/plat_imx8_gic.c			\
				plat/imx/imx9/common/imx9_bl31_setup.c		\
				plat/imx/imx9/common/imx9_psci_common.c		\
				plat/imx/imx9/common/imx9_sys_sleep.c		\
				plat/imx/imx9/common/ele_api.c			\
				plat/imx/imx9/common/scmi/scmi_client.c		\
				plat/imx/imx9/common/aarch64/plat_helpers.S	\
				plat/imx/imx9/common/plat_topology.c		\
				plat/imx/imx9/imx94/imx94_bl31_setup.c		\
				plat/imx/imx9/imx94/imx94_psci.c		\
				lib/cpus/aarch64/cortex_a55.S			\
				${IMX_GIC_SOURCES}				\
				${XLAT_TABLES_LIB_SRCS}

RESET_TO_BL31		:=	1
HW_ASSISTED_COHERENCY	:= 	1
USE_COHERENT_MEM	:=	0
PROGRAMMABLE_RESET_ADDRESS := 1
COLD_BOOT_SINGLE_CPU := 1
ERRATA_A55_1530923 := 1

SYS_PWR_FULL_CTRL ?= 0
$(eval $(call add_define,SYS_PWR_FULL_CTRL))

HAS_XSPI_SUPPORT := 1
$(eval $(call assert_boolean,HAS_XSPI_SUPPORT))
$(eval $(call add_define,HAS_XSPI_SUPPORT))
