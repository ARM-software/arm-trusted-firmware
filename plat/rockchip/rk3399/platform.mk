#
# Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RK_PLAT		:=	plat/rockchip
RK_PLAT_SOC	:=	${RK_PLAT}/${PLAT}
RK_PLAT_COMMON	:=	${RK_PLAT}/common

DISABLE_BIN_GENERATION	:=	1

PLAT_INCLUDES		:=	-I${RK_PLAT_COMMON}/			\
				-I${RK_PLAT_COMMON}/include/		\
				-I${RK_PLAT_COMMON}/aarch64/		\
				-I${RK_PLAT_COMMON}/drivers/pmu/	\
				-I${RK_PLAT_SOC}/			\
				-I${RK_PLAT_SOC}/drivers/pmu/		\
				-I${RK_PLAT_SOC}/drivers/pwm/		\
				-I${RK_PLAT_SOC}/drivers/secure/	\
				-I${RK_PLAT_SOC}/drivers/soc/		\
				-I${RK_PLAT_SOC}/drivers/dram/		\
				-I${RK_PLAT_SOC}/drivers/dp/		\
				-I${RK_PLAT_SOC}/include/		\
				-I${RK_PLAT_SOC}/include/shared/	\

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

RK_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				${RK_PLAT}/common/rockchip_gicv3.c

PLAT_BL_COMMON_SOURCES	:=	common/desc_image_load.c			\
				lib/bl_aux_params/bl_aux_params.c		\
				lib/xlat_tables/xlat_tables_common.c	\
				lib/xlat_tables/aarch64/xlat_tables.c	\
				plat/common/aarch64/crash_console_helpers.S \
				plat/common/plat_psci_common.c

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	${RK_PLAT_COMMON}/rockchip_stack_protector.c
endif

BL31_SOURCES	+=	${RK_GIC_SOURCES}				\
			drivers/arm/cci/cci.c				\
			drivers/ti/uart/aarch64/16550_console.S		\
			drivers/delay_timer/delay_timer.c		\
			drivers/delay_timer/generic_delay_timer.c	\
			drivers/gpio/gpio.c				\
			lib/cpus/aarch64/cortex_a53.S			\
			lib/cpus/aarch64/cortex_a72.S			\
			${RK_PLAT_COMMON}/aarch64/plat_helpers.S	\
			${RK_PLAT_COMMON}/bl31_plat_setup.c		\
			${RK_PLAT_COMMON}/params_setup.c		\
			${RK_PLAT_COMMON}/aarch64/pmu_sram_cpus_on.S	\
			${RK_PLAT_COMMON}/plat_pm.c			\
			${RK_PLAT_COMMON}/plat_topology.c		\
			${RK_PLAT_COMMON}/aarch64/platform_common.c	\
			${RK_PLAT_COMMON}/rockchip_sip_svc.c		\
			${RK_PLAT_SOC}/plat_sip_calls.c			\
			${RK_PLAT_SOC}/drivers/gpio/rk3399_gpio.c	\
			${RK_PLAT_SOC}/drivers/pmu/pmu.c		\
			${RK_PLAT_SOC}/drivers/pmu/pmu_fw.c		\
			${RK_PLAT_SOC}/drivers/pmu/m0_ctl.c		\
			${RK_PLAT_SOC}/drivers/pwm/pwm.c		\
			${RK_PLAT_SOC}/drivers/secure/secure.c		\
			${RK_PLAT_SOC}/drivers/soc/soc.c		\
			${RK_PLAT_SOC}/drivers/dram/dfs.c		\
			${RK_PLAT_SOC}/drivers/dram/dram.c		\
			${RK_PLAT_SOC}/drivers/dram/dram_spec_timing.c	\
			${RK_PLAT_SOC}/drivers/dram/suspend.c

include lib/coreboot/coreboot.mk
include lib/libfdt/libfdt.mk

$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_855873	:=	1

# M0 source build
PLAT_M0                 :=      ${PLAT}m0
BUILD_M0		:=	${BUILD_PLAT}/m0

RK3399M0FW=${BUILD_M0}/${PLAT_M0}.bin
$(eval $(call add_define_val,RK3399M0FW,\"$(RK3399M0FW)\"))

RK3399M0PMUFW=${BUILD_M0}/${PLAT_M0}pmu.bin
$(eval $(call add_define_val,RK3399M0PMUFW,\"$(RK3399M0PMUFW)\"))

ifdef PLAT_RK_DP_HDCP
BL31_SOURCES	+= ${RK_PLAT_SOC}/drivers/dp/cdn_dp.c

HDCPFW=${RK_PLAT_SOC}/drivers/dp/hdcp.bin
$(eval $(call add_define_val,HDCPFW,\"$(HDCPFW)\"))

${BUILD_PLAT}/bl31/cdn_dp.o: CCACHE_EXTRAFILES=$(HDCPFW)
${RK_PLAT_SOC}/drivers/dp/cdn_dp.c: $(HDCPFW)
endif

# CCACHE_EXTRAFILES is needed because ccache doesn't handle .incbin
export CCACHE_EXTRAFILES
${BUILD_PLAT}/bl31/pmu_fw.o: CCACHE_EXTRAFILES=$(RK3399M0FW):$(RK3399M0PMUFW)
${RK_PLAT_SOC}/drivers/pmu/pmu_fw.c: $(RK3399M0FW)

$(eval $(call MAKE_PREREQ_DIR,${BUILD_M0},${BUILD_PLAT}))
.PHONY: $(RK3399M0FW)
$(RK3399M0FW): | ${BUILD_M0}
	$(MAKE) -C ${RK_PLAT_SOC}/drivers/m0 BUILD=$(abspath ${BUILD_PLAT}/m0)

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0
