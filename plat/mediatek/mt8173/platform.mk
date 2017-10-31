#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT		:=	plat/mediatek
MTK_PLAT_SOC		:=	${MTK_PLAT}/${PLAT}

PLAT_INCLUDES		:=	-I${MTK_PLAT}/common/				\
				-I${MTK_PLAT}/common/drivers/uart/		\
				-Iinclude/plat/arm/common			\
				-Iinclude/plat/arm/common/aarch64		\
				-I${MTK_PLAT_SOC}/drivers/crypt/		\
				-I${MTK_PLAT_SOC}/drivers/mtcmos/		\
				-I${MTK_PLAT_SOC}/drivers/pmic/			\
				-I${MTK_PLAT_SOC}/drivers/rtc/			\
				-I${MTK_PLAT_SOC}/drivers/spm/			\
				-I${MTK_PLAT_SOC}/drivers/timer/		\
				-I${MTK_PLAT_SOC}/include/

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				plat/arm/common/arm_gicv2.c			\
				plat/common/plat_gicv2.c

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				drivers/console/aarch64/console.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				${MTK_PLAT}/common/drivers/uart/8250_console.S	\
				${MTK_PLAT}/common/mtk_plat_common.c		\
				${MTK_PLAT}/common/mtk_sip_svc.c		\
				${MTK_PLAT_SOC}/aarch64/plat_helpers.S		\
				${MTK_PLAT_SOC}/aarch64/platform_common.c	\
				${MTK_PLAT_SOC}/bl31_plat_setup.c		\
				${MTK_PLAT_SOC}/drivers/crypt/crypt.c		\
				${MTK_PLAT_SOC}/drivers/mtcmos/mtcmos.c		\
				${MTK_PLAT_SOC}/drivers/pmic/pmic_wrap_init.c	\
				${MTK_PLAT_SOC}/drivers/rtc/rtc.c		\
				${MTK_PLAT_SOC}/drivers/spm/spm.c		\
				${MTK_PLAT_SOC}/drivers/spm/spm_hotplug.c	\
				${MTK_PLAT_SOC}/drivers/spm/spm_mcdi.c		\
				${MTK_PLAT_SOC}/drivers/spm/spm_suspend.c	\
				${MTK_PLAT_SOC}/drivers/timer/mt_cpuxgpt.c	\
				${MTK_PLAT_SOC}/plat_pm.c			\
				${MTK_PLAT_SOC}/plat_sip_calls.c		\
				${MTK_PLAT_SOC}/plat_topology.c			\
				${MTK_PLAT_SOC}/power_tracer.c			\
				${MTK_PLAT_SOC}/scu.c

# Flag used by the MTK_platform port to determine the version of ARM GIC
# architecture to use for interrupt management in EL3.
ARM_GIC_ARCH		:=	2
$(eval $(call add_define,ARM_GIC_ARCH))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	1
ERRATA_A53_836870	:=	1
ERRATA_A53_855873	:=	1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

$(eval $(call add_define,MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE))

# Do not enable SVE
ENABLE_SVE_FOR_NS		:=	0
