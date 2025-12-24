#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT := plat/mediatek
MTK_PLAT_SOC := ${MTK_PLAT}/${PLAT}
MTK_SOC := ${PLAT}
ARM_ARCH_MAJOR := 9

ERRATA_A720_2792132	:=	1
ERRATA_A720_2844092	:=	1
ERRATA_A720_2926083	:=	1
ERRATA_A720_2940794	:=	1

ERRATA_X4_2726228	:=	1
ERRATA_X4_2740089	:=	1
ERRATA_X4_2763018	:=	1
ERRATA_X4_2816013	:=	1
ERRATA_X4_2897503	:=	1
ERRATA_X4_2923985	:=	1
ERRATA_X4_3076789	:=	1

include plat/mediatek/build_helpers/mtk_build_helpers.mk
include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES := -I${MTK_PLAT}/common \
		 -I${MTK_PLAT}/common/include \
		 -I${MTK_PLAT}/drivers/cpu_pm/topology/inc \
                 -I${MTK_PLAT}/drivers/gpio/ \
		 -I${MTK_PLAT}/include \
		 -I${MTK_PLAT}/include/${ARCH_VERSION} \
		 -I${MTK_PLAT} \
		 -I${MTK_PLAT_SOC}/drivers/dcm/ \
		 -I${MTK_PLAT_SOC}/drivers/gpio/ \
		 -I${MTK_PLAT_SOC}/include \
		 -Idrivers/arm/gic \

MODULES-y += $(MTK_PLAT)/common
MODULES-y += $(MTK_PLAT)/common/lpm_v2
MODULES-y += $(MTK_PLAT)/lib/mtk_init
MODULES-y += $(MTK_PLAT)/lib/pm
MODULES-y += $(MTK_PLAT)/lib/system_reset
MODULES-y += $(MTK_PLAT)/drivers/apusys
MODULES-y += $(MTK_PLAT)/drivers/audio
MODULES-y += $(MTK_PLAT)/drivers/cirq
MODULES-y += $(MTK_PLAT)/drivers/cpu_qos
MODULES-y += $(MTK_PLAT)/drivers/dp
MODULES-y += $(MTK_PLAT)/drivers/emi
MODULES-y += $(MTK_PLAT)/drivers/gicv3
MODULES-y += $(MTK_PLAT)/drivers/mcusys
MODULES-y += $(MTK_PLAT)/drivers/mminfra
MODULES-y += $(MTK_PLAT)/drivers/slbc
MODULES-y += $(MTK_PLAT)/drivers/smmu
MODULES-y += $(MTK_PLAT)/drivers/spm
MODULES-y += $(MTK_PLAT)/drivers/timer
MODULES-y += $(MTK_PLAT)/drivers/ufs
MODULES-y += $(MTK_PLAT)/drivers/vcp
MODULES-y += $(MTK_PLAT)/helpers
MODULES-y += $(MTK_PLAT)/topology
MODULES-$(CONFIG_MTK_BOOKER) += $(MTK_PLAT)/drivers/booker
MODULES-$(CONFIG_MTK_CPU_PM_SUPPORT) += $(MTK_PLAT)/drivers/cpu_pm
MODULES-$(CONFIG_MTK_PMIC) += $(MTK_PLAT)/drivers/pmic
MODULES-$(CONFIG_MTK_SMMU_SID) += $(MTK_PLAT)/drivers/smmu_sid
MODULES-$(CONFIG_MTK_SPMI) += $(MTK_PLAT)/drivers/spmi

MODULES-$(CONFIG_MTK_MTCMOS) += $(MTK_PLAT)/drivers/mtcmos

ifneq ($(MTKLIB_PATH),)
LDLIBS += -Wl,--whole-archive $(MTKLIB_PATH) -Wl,--no-whole-archive
endif

PLAT_BL_COMMON_SOURCES := common/desc_image_load.c \
			  drivers/ti/uart/aarch64/16550_console.S \
			  lib/bl_aux_params/bl_aux_params.c

BL31_SOURCES += drivers/delay_timer/delay_timer.c \
		drivers/delay_timer/generic_delay_timer.c \
		drivers/gpio/gpio.c \
		lib/cpus/aarch64/cortex_a720.S	\
		lib/cpus/aarch64/cortex_x4.S	\
		lib/cpus/aarch64/cortex_x925.S \
		${GICV3_SOURCES} \
		${XLAT_TABLES_LIB_SRCS} \
		plat/common/plat_gicv3.c \
		plat/common/plat_psci_common.c \
		plat/common/aarch64/crash_console_helpers.S \
		${MTK_PLAT}/common/mtk_plat_common.c \
		${MTK_PLAT}/common/params_setup.c \
                ${MTK_PLAT}/drivers/gpio/mtgpio_common.c \
		$(MTK_PLAT)/$(MTK_SOC)/drivers/dcm/mtk_dcm.c \
		$(MTK_PLAT)/$(MTK_SOC)/drivers/dcm/mtk_dcm_utils.c \
                $(MTK_PLAT)/$(MTK_SOC)/drivers/gpio/mtgpio.c \
		$(MTK_PLAT)/$(MTK_SOC)/plat_mmap.c

include plat/mediatek/build_helpers/mtk_build_helpers_epilogue.mk

include lib/coreboot/coreboot.mk
