#
# Copyright (c) 2024, Rockchip, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RK_PLAT			:=	plat/rockchip
RK_PLAT_SOC		:=	${RK_PLAT}/${PLAT}
RK_PLAT_COMMON		:=	${RK_PLAT}/common

DISABLE_BIN_GENERATION	:=	1
include lib/libfdt/libfdt.mk
include lib/xlat_tables_v2/xlat_tables.mk

# GIC-600 configuration
GICV3_IMPL		:=	GIC600
GICV3_SUPPORT_GIC600   	:=      1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

PLAT_INCLUDES		:=	-Iinclude/plat/common				\
				-Idrivers/arm/gic/v3/				\
				-Idrivers/scmi-msg/				\
				-I${RK_PLAT_COMMON}/				\
				-I${RK_PLAT_COMMON}/drivers/pmu/		\
				-I${RK_PLAT_COMMON}/drivers/parameter/		\
				-I${RK_PLAT_COMMON}/include/			\
				-I${RK_PLAT_COMMON}/pmusram/			\
				-I${RK_PLAT_COMMON}/scmi/			\
				-I${RK_PLAT_SOC}/				\
				-I${RK_PLAT_SOC}/drivers/pmu/			\
				-I${RK_PLAT_SOC}/drivers/scmi/			\
				-I${RK_PLAT_SOC}/drivers/secure/		\
				-I${RK_PLAT_SOC}/drivers/soc/			\
				-I${RK_PLAT_SOC}/include/

RK_GIC_SOURCES		:=	${GICV3_SOURCES}				\
				plat/common/plat_gicv3.c			\
				${RK_PLAT}/common/rockchip_gicv3.c

PLAT_BL_COMMON_SOURCES	:=	${XLAT_TABLES_LIB_SRCS}				\
				common/desc_image_load.c			\
				plat/common/aarch64/crash_console_helpers.S	\
				lib/bl_aux_params/bl_aux_params.c		\
				plat/common/plat_psci_common.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	${RK_PLAT_COMMON}/rockchip_stack_protector.c
endif

BL31_SOURCES		+=	${RK_GIC_SOURCES}				\
				drivers/ti/uart/aarch64/16550_console.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/scmi-msg/base.c				\
				drivers/scmi-msg/clock.c			\
				drivers/scmi-msg/entry.c			\
				drivers/scmi-msg/reset_domain.c			\
				drivers/scmi-msg/smt.c				\
				lib/cpus/aarch64/cortex_a55.S			\
				lib/cpus/aarch64/cortex_a76.S			\
				${RK_PLAT_COMMON}/aarch64/plat_helpers.S	\
				${RK_PLAT_COMMON}/aarch64/platform_common.c	\
				${RK_PLAT_COMMON}/bl31_plat_setup.c		\
				${RK_PLAT_COMMON}/plat_pm.c			\
				${RK_PLAT_COMMON}/plat_pm_helpers.c		\
				${RK_PLAT_COMMON}/plat_topology.c		\
				${RK_PLAT_COMMON}/params_setup.c                \
				${RK_PLAT_COMMON}/pmusram/cpus_on_fixed_addr.S	\
				${RK_PLAT_COMMON}/rockchip_sip_svc.c		\
				${RK_PLAT_COMMON}/scmi/scmi.c			\
				${RK_PLAT_COMMON}/scmi/scmi_clock.c		\
				${RK_PLAT_COMMON}/scmi/scmi_rstd.c		\
				${RK_PLAT_SOC}/plat_sip_calls.c         	\
				${RK_PLAT_SOC}/drivers/secure/secure.c		\
				${RK_PLAT_SOC}/drivers/soc/soc.c		\
				${RK_PLAT_SOC}/drivers/pmu/pmu.c		\
				${RK_PLAT_SOC}/drivers/pmu/pm_pd_regs.c		\
				${RK_PLAT_SOC}/drivers/scmi/rk3588_clk.c	\
				${RK_PLAT_SOC}/drivers/scmi/rk3588_rstd.c

CTX_INCLUDE_AARCH32_REGS :=     0
ENABLE_PLAT_COMPAT	:=	0
MULTI_CONSOLE_API	:=	1
ERRATA_A55_1530923	:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM	:=	0

ENABLE_SPE_FOR_LOWER_ELS	:= 0

$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))
$(eval $(call add_define,PLAT_RK_CPU_RESET_EARLY))
