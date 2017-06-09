#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RK_PLAT			:=	plat/rockchip
RK_PLAT_SOC		:=	${RK_PLAT}/${PLAT}
RK_PLAT_COMMON		:=	${RK_PLAT}/common

PLAT_INCLUDES		:=	-Idrivers/arm/gic/common/			\
				-Idrivers/arm/gic/v2/			\
				-Iinclude/plat/common/				\
				-I${RK_PLAT_COMMON}/                            \
				-I${RK_PLAT_COMMON}/include/                    \
				-I${RK_PLAT_COMMON}/pmusram                     \
				-I${RK_PLAT_COMMON}/drivers/pmu/                \
				-I${RK_PLAT_COMMON}/drivers/parameter/		\
				-I${RK_PLAT_SOC}/				\
				-I${RK_PLAT_SOC}/drivers/pmu/			\
				-I${RK_PLAT_SOC}/drivers/soc/			\
				-I${RK_PLAT_SOC}/include/

RK_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				plat/common/plat_gicv2.c			\
				${RK_PLAT}/common/rockchip_gicv2.c

PLAT_BL_COMMON_SOURCES	:=	lib/aarch64/xlat_tables.c			\
				plat/common/aarch64/plat_psci_common.c

BL31_SOURCES		+=	${RK_GIC_SOURCES}				\
				drivers/arm/cci/cci.c				\
				drivers/console/console.S			\
				drivers/ti/uart/16550_console.S			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				${RK_PLAT_COMMON}/drivers/parameter/ddr_parameter.c	\
				${RK_PLAT_COMMON}/aarch64/plat_helpers.S	\
				${RK_PLAT_COMMON}/bl31_plat_setup.c		\
				${RK_PLAT_COMMON}/pmusram/pmu_sram_cpus_on.S	\
				${RK_PLAT_COMMON}/plat_pm.c			\
				${RK_PLAT_COMMON}/plat_topology.c		\
				${RK_PLAT_COMMON}/aarch64/platform_common.c	\
				${RK_PLAT_SOC}/drivers/pmu/pmu.c		\
				${RK_PLAT_SOC}/drivers/soc/soc.c

ENABLE_PLAT_COMPAT 	:=      0

$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))
$(eval $(call add_define,PLAT_SKIP_OPTEE_S_EL1_INT_REGISTER))
