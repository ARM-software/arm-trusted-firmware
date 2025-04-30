#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT := plat/mediatek
MTK_PLAT_SOC := ${MTK_PLAT}/${PLAT}
MTK_SOC := ${PLAT}

include plat/mediatek/build_helpers/mtk_build_helpers.mk
include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES := -I${MTK_PLAT}/common \
		 -I${MTK_PLAT}/common/include \
		 -I${MTK_PLAT}/drivers/gpio/ \
		 -I${MTK_PLAT}/include \
		 -I${MTK_PLAT}/include/${ARCH_VERSION} \
		 -I${MTK_PLAT} \
		 -I${MTK_PLAT_SOC}/drivers/gpio/ \
		 -I${MTK_PLAT_SOC}/include \
		 -Idrivers/arm/gic \

MODULES-y += $(MTK_PLAT)/common
MODULES-y += $(MTK_PLAT)/helpers
MODULES-y += $(MTK_PLAT)/lib/mtk_init
MODULES-y += $(MTK_PLAT)/lib/pm
MODULES-y += $(MTK_PLAT)/topology
MODULES-y += $(MTK_PLAT)/drivers/cirq
MODULES-y += $(MTK_PLAT)/drivers/gic600
MODULES-y += $(MTK_PLAT)/drivers/mcusys
MODULES-y += $(MTK_PLAT)/drivers/timer

PLAT_BL_COMMON_SOURCES := common/desc_image_load.c \
			  drivers/ti/uart/aarch64/16550_console.S \
			  lib/bl_aux_params/bl_aux_params.c

BL31_SOURCES += drivers/delay_timer/delay_timer.c \
		drivers/delay_timer/generic_delay_timer.c \
		drivers/gpio/gpio.c \
		lib/cpus/aarch64/cortex_a55.S \
		lib/cpus/aarch64/cortex_a78.S \
		${GICV3_SOURCES} \
		${XLAT_TABLES_LIB_SRCS} \
		plat/common/plat_gicv3.c \
		plat/common/plat_psci_common.c \
		plat/common/aarch64/crash_console_helpers.S \
		${MTK_PLAT}/common/mtk_plat_common.c \
		${MTK_PLAT}/common/params_setup.c \
		${MTK_PLAT}/drivers/gpio/mtgpio_common.c \
		$(MTK_PLAT)/$(MTK_SOC)/drivers/gpio/mtgpio.c \
		$(MTK_PLAT)/$(MTK_SOC)/plat_mmap.c

ifeq (${CONFIG_MTK_DISABLE_CACHE_AS_RAM}, 1)
BL31_SOURCES += ${MTK_PLAT}/common/cache_ops.c
endif

include plat/mediatek/build_helpers/mtk_build_helpers_epilogue.mk

include lib/coreboot/coreboot.mk
