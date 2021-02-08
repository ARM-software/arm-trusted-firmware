#
#Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
#
#SPDX-License-Identifier: BSD-3-Clause
#

include drivers/arm/gic/v2/gicv2.mk

RK_PLAT			:=	plat/rockchip
RK_PLAT_SOC		:=	${RK_PLAT}/${PLAT}
RK_PLAT_COMMON		:=	${RK_PLAT}/common

DISABLE_BIN_GENERATION	:=	1

PLAT_INCLUDES		:=	-Idrivers/arm/gic/common/			\
				-Idrivers/arm/gic/v2/				\
				-Iinclude/plat/common/				\
				-I${RK_PLAT_COMMON}/				\
				-I${RK_PLAT_COMMON}/include/			\
				-I${RK_PLAT_COMMON}/drivers/parameter/		\
				-I${RK_PLAT_COMMON}/pmusram			\
				-I${RK_PLAT_SOC}/				\
				-I${RK_PLAT_SOC}/drivers/pmu/			\
				-I${RK_PLAT_SOC}/drivers/secure/		\
				-I${RK_PLAT_SOC}/drivers/soc/			\
				-I${RK_PLAT_SOC}/include/

RK_GIC_SOURCES         :=	${GICV2_SOURCES}				\
				plat/common/plat_gicv2.c			\
				plat/common/aarch64/crash_console_helpers.S	\
				${RK_PLAT}/common/rockchip_gicv2.c

PLAT_BL_COMMON_SOURCES	:=	lib/bl_aux_params/bl_aux_params.c		\
				lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				plat/common/plat_psci_common.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	${RK_PLAT_COMMON}/rockchip_stack_protector.c
endif

BL31_SOURCES		+=	${RK_GIC_SOURCES}				\
				common/desc_image_load.c			\
				drivers/arm/cci/cci.c				\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/ti/uart/aarch64/16550_console.S		\
				lib/cpus/aarch64/cortex_a35.S			\
				${RK_PLAT_COMMON}/aarch64/plat_helpers.S	\
				${RK_PLAT_COMMON}/aarch64/platform_common.c	\
				${RK_PLAT_COMMON}/bl31_plat_setup.c		\
				${RK_PLAT_COMMON}/params_setup.c		\
				${RK_PLAT_COMMON}/pmusram/cpus_on_fixed_addr.S	\
				${RK_PLAT_COMMON}/plat_pm.c			\
				${RK_PLAT_COMMON}/plat_topology.c		\
				${RK_PLAT_COMMON}/rockchip_sip_svc.c		\
				${RK_PLAT_SOC}/drivers/pmu/pmu.c		\
				${RK_PLAT_SOC}/drivers/secure/secure.c		\
				${RK_PLAT_SOC}/drivers/soc/soc.c		\
				${RK_PLAT_SOC}/plat_sip_calls.c

ifdef PLAT_RK_SECURE_DDR_MINILOADER
BL31_SOURCES		+=	${RK_PLAT_COMMON}/drivers/parameter/ddr_parameter.c
endif

ENABLE_PLAT_COMPAT	:=	0
MULTI_CONSOLE_API	:=	1

include lib/libfdt/libfdt.mk

$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))
$(eval $(call add_define,PLAT_SKIP_OPTEE_S_EL1_INT_REGISTER))
$(eval $(call add_define,PLAT_WARMBOOT_ADDR_NOT_ALIGN))
