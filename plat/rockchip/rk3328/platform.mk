#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RK_PLAT			:=	plat/rockchip
RK_PLAT_SOC		:=	${RK_PLAT}/${PLAT}
RK_PLAT_COMMON		:=	${RK_PLAT}/common

DISABLE_BIN_GENERATION	:=	1

PLAT_INCLUDES		:=	-Idrivers/arm/gic/common/			\
				-Idrivers/arm/gic/v2/			\
				-I${RK_PLAT_COMMON}/                            \
				-I${RK_PLAT_COMMON}/include/                    \
				-I${RK_PLAT_COMMON}/aarch64/			\
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

PLAT_BL_COMMON_SOURCES	:=	common/desc_image_load.c			\
				lib/bl_aux_params/bl_aux_params.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				lib/xlat_tables/xlat_tables_common.c		\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/common/plat_psci_common.c

BL31_SOURCES		+=	${RK_GIC_SOURCES}				\
				drivers/arm/cci/cci.c				\
				drivers/ti/uart/aarch64/16550_console.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				${RK_PLAT_COMMON}/aarch64/plat_helpers.S	\
				${RK_PLAT_COMMON}/params_setup.c		\
				${RK_PLAT_COMMON}/bl31_plat_setup.c		\
				${RK_PLAT_COMMON}/aarch64/pmu_sram_cpus_on.S	\
				${RK_PLAT_COMMON}/plat_pm.c			\
				${RK_PLAT_COMMON}/plat_topology.c		\
				${RK_PLAT_COMMON}/aarch64/platform_common.c	\
				${RK_PLAT_SOC}/drivers/pmu/pmu.c		\
				${RK_PLAT_SOC}/drivers/soc/soc.c

ifdef PLAT_RK_SECURE_DDR_MINILOADER
BL31_SOURCES		+=	${RK_PLAT_COMMON}/drivers/parameter/ddr_parameter.c
endif

include lib/coreboot/coreboot.mk
include lib/libfdt/libfdt.mk

# Enable workarounds for selected Cortex-A53 errata
ERRATA_A53_855873	:=	1

$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))
$(eval $(call add_define,PLAT_SKIP_OPTEE_S_EL1_INT_REGISTER))

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0

WORKAROUND_CVE_2017_5715	:=	0
