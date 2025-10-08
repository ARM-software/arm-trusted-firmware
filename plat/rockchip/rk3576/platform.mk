#
# Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RK_PLAT			:=	plat/rockchip
RK_PLAT_SOC		:=	${RK_PLAT}/${PLAT}
RK_PLAT_COMMON		:=	${RK_PLAT}/common

DISABLE_BIN_GENERATION	:=	1

include drivers/arm/gic/v2/gicv2.mk
include lib/libfdt/libfdt.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		:=	-Idrivers/arm/gic/common/			\
				-Idrivers/arm/gic/v2/				\
				-Idrivers/scmi-msg/				\
				-Iinclude/bl31					\
				-Iinclude/common				\
				-Iinclude/drivers				\
				-Iinclude/drivers/arm				\
				-Iinclude/drivers/io				\
				-Iinclude/drivers/ti/uart			\
				-Iinclude/lib					\
				-Iinclude/lib/cpus/${ARCH}			\
				-Iinclude/lib/el3_runtime			\
				-Iinclude/lib/psci				\
				-Iinclude/plat/common				\
				-Iinclude/services				\
				-I${RK_PLAT_COMMON}/				\
				-I${RK_PLAT_COMMON}/pmusram/			\
				-I${RK_PLAT_COMMON}/include/			\
				-I${RK_PLAT_COMMON}/drivers/pmu/		\
				-I${RK_PLAT_COMMON}/drivers/parameter/		\
				-I${RK_PLAT_COMMON}/scmi/			\
				-I${RK_PLAT_SOC}/				\
				-I${RK_PLAT_SOC}/drivers/dmc/			\
				-I${RK_PLAT_SOC}/drivers/pmu/			\
				-I${RK_PLAT_SOC}/drivers/secure/		\
				-I${RK_PLAT_SOC}/drivers/soc/			\
				-I${RK_PLAT_SOC}/include/			\
				-I${RK_PLAT_SOC}/scmi/

RK_GIC_SOURCES		:=	${GICV2_SOURCES}				\
				plat/common/plat_gicv2.c			\
				${RK_PLAT}/common/rockchip_gicv2.c

PLAT_BL_COMMON_SOURCES	:=	${XLAT_TABLES_LIB_SRCS}				\
				common/desc_image_load.c			\
				lib/bl_aux_params/bl_aux_params.c		\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/common/plat_psci_common.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	${RK_PLAT_COMMON}/rockchip_stack_protector.c
endif

BL31_SOURCES		+=	${RK_GIC_SOURCES}				\
				drivers/arm/cci/cci.c				\
				drivers/ti/uart/aarch64/16550_console.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/scmi-msg/base.c				\
				drivers/scmi-msg/clock.c			\
				drivers/scmi-msg/entry.c			\
				drivers/scmi-msg/reset_domain.c			\
				drivers/scmi-msg/smt.c				\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				$(LIBFDT_SRCS)					\
				${RK_PLAT_COMMON}/aarch64/plat_helpers.S	\
				${RK_PLAT_COMMON}/aarch64/platform_common.c	\
				${RK_PLAT_COMMON}/bl31_plat_setup.c		\
				${RK_PLAT_COMMON}/plat_pm.c			\
				${RK_PLAT_COMMON}/plat_pm_helpers.c		\
				${RK_PLAT_COMMON}/plat_topology.c		\
				${RK_PLAT_COMMON}/rockchip_sip_svc.c		\
				${RK_PLAT_COMMON}/params_setup.c		\
				${RK_PLAT_COMMON}/pmusram/cpus_on_fixed_addr.S	\
				${RK_PLAT_COMMON}/rockchip_sip_svc.c		\
				${RK_PLAT_COMMON}/scmi/rockchip_common_clock.c	\
				${RK_PLAT_COMMON}/scmi/scmi.c			\
				${RK_PLAT_COMMON}/scmi/scmi_clock.c		\
				${RK_PLAT_COMMON}/scmi/scmi_rstd.c		\
				${RK_PLAT_SOC}/scmi/rk3576_clk.c		\
				${RK_PLAT_SOC}/plat_sip_calls.c         	\
				${RK_PLAT_SOC}/drivers/dmc/suspend.c		\
				${RK_PLAT_SOC}/drivers/pmu/pmu.c		\
				${RK_PLAT_SOC}/drivers/pmu/pm_pd_regs.c 	\
				${RK_PLAT_SOC}/drivers/secure/firewall.c	\
				${RK_PLAT_SOC}/drivers/secure/secure.c		\
				${RK_PLAT_SOC}/drivers/soc/soc.c

# Enable workarounds for selected Cortex-A53 errata
ERRATA_A53_835769		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1
ERRATA_A53_1530924		:=	1

ERRATA_A72_1319367		:=	1

ENABLE_PLAT_COMPAT		:=	0
MULTI_CONSOLE_API		:=	1
CTX_INCLUDE_EL2_REGS		:=	0
GICV2_G0_FOR_EL3		:=	1
CTX_INCLUDE_AARCH32_REGS	:=	0

# Do not enable SVE
ENABLE_SVE_FOR_NS		:=	0

WORKAROUND_CVE_2017_5715	:=	0

PLAT_EXTRA_LD_SCRIPT	:=	1
