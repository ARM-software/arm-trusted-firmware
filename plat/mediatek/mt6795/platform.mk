#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT		:=	plat/mediatek
MTK_PLAT_SOC		:=	${MTK_PLAT}/${PLAT}

# Add OEM customized codes
OEMS				:= true
MTK_SIP_KERNEL_BOOT_ENABLE := 1


ifneq (${OEMS},none)
  OEMS_INCLUDES		:= -I${MTK_PLAT}/common/custom/
  OEMS_SOURCES		:=	${MTK_PLAT}/common/custom/oem_svc.c
endif

PLAT_INCLUDES		:=	-I${MTK_PLAT}/common/				\
				-I${MTK_PLAT}/common/drivers/uart			\
				-I${MTK_PLAT_SOC}/				\
				-I${MTK_PLAT_SOC}/drivers/timer/			\
				-I${MTK_PLAT_SOC}/include/					\
				-Iinclude/plat/arm/common/					\
				-Iinclude/common/tbbr/					\
				${OEMS_INCLUDES}

PLAT_BL_COMMON_SOURCES	:=	lib/aarch64/xlat_tables.c			\
				plat/common/plat_gic.c

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				plat/common/plat_gicv2.c			\
				drivers/console/console.S			\
				drivers/delay_timer/delay_timer.c		\
				lib/cpus/aarch64/cortex_a53.S			\
				${MTK_PLAT_SOC}/bl31_plat_setup.c		\
				${MTK_PLAT_SOC}/plat_mt_gic.c			\
				${MTK_PLAT}/common/mtk_sip_svc.c		\
				${MTK_PLAT}/common/mtk_plat_common.c		\
				${MTK_PLAT}/common/drivers/uart/8250_console.S		\
				${MTK_PLAT_SOC}/aarch64/plat_helpers.S		\
				${MTK_PLAT_SOC}/drivers/timer/mt_cpuxgpt.c	\
				${MTK_PLAT_SOC}/plat_delay_timer.c		\
				${MTK_PLAT_SOC}/plat_pm.c			\
				${MTK_PLAT_SOC}/plat_topology.c			\
				${MTK_PLAT_SOC}/power_tracer.c			\
				${MTK_PLAT_SOC}/scu.c		\
				${OEMS_SOURCES}

# Flag used by the MTK_platform port to determine the version of ARM GIC
# architecture to use for interrupt management in EL3.
ARM_GIC_ARCH		:=	2
$(eval $(call add_define,ARM_GIC_ARCH))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	1
ERRATA_A53_836870	:=	1

WORKAROUND_CVE_2017_5715	:=	0

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

$(eval $(call add_define,MTK_SIP_KERNEL_BOOT_ENABLE))

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0
