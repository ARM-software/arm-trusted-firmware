#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ARM_CORTEX_A12		:=	yes
ARM_ARCH_MAJOR		:=	7

RK_PLAT			:=	plat/rockchip
RK_PLAT_SOC		:=	${RK_PLAT}/${PLAT}
RK_PLAT_COMMON		:=	${RK_PLAT}/common

DISABLE_BIN_GENERATION	:=	1

PLAT_INCLUDES		:=	-I${RK_PLAT_COMMON}/				\
				-I${RK_PLAT_COMMON}/include/			\
				-I${RK_PLAT_COMMON}/aarch32/			\
				-I${RK_PLAT_COMMON}/drivers/pmu/		\
				-I${RK_PLAT_SOC}/				\
				-I${RK_PLAT_SOC}/drivers/pmu/			\
				-I${RK_PLAT_SOC}/drivers/secure/		\
				-I${RK_PLAT_SOC}/drivers/soc/			\
				-I${RK_PLAT_SOC}/include/			\
				-I${RK_PLAT_SOC}/include/shared/		\

RK_GIC_SOURCES         :=	drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				plat/common/plat_gicv2.c			\
				${RK_PLAT}/common/rockchip_gicv2.c

PLAT_BL_COMMON_SOURCES	:=	plat/common/aarch32/crash_console_helpers.S	\
				plat/common/plat_psci_common.c

PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch32/xlat_tables.c

BL32_SOURCES		+=	${RK_GIC_SOURCES}				\
				drivers/arm/cci/cci.c				\
				drivers/ti/uart/aarch32/16550_console.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch32/cortex_a12.S			\
				${RK_PLAT_COMMON}/aarch32/plat_helpers.S	\
				${RK_PLAT_COMMON}/params_setup.c		\
				${RK_PLAT_COMMON}/aarch32/pmu_sram_cpus_on.S	\
				${RK_PLAT_COMMON}/plat_pm.c			\
				${RK_PLAT_COMMON}/plat_topology.c		\
				${RK_PLAT_COMMON}/aarch32/platform_common.c	\
				${RK_PLAT_COMMON}/rockchip_sip_svc.c		\
				${RK_PLAT_SOC}/plat_sip_calls.c			\
				${RK_PLAT_SOC}/drivers/pmu/pmu.c		\
				${RK_PLAT_SOC}/drivers/secure/secure.c		\
				${RK_PLAT_SOC}/drivers/soc/soc.c		\

MULTI_CONSOLE_API	:=	1

include lib/coreboot/coreboot.mk
include lib/libfdt/libfdt.mk

$(eval $(call add_define,PLAT_SP_MIN_EXTRA_LD_SCRIPT))

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0

WORKAROUND_CVE_2017_5715	:=	0
