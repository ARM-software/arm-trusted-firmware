#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT := plat/mediatek
MTK_PLAT_SOC := ${MTK_PLAT}/${PLAT}
MTK_SOC := ${PLAT}

include plat/mediatek/build_helpers/mtk_build_helpers.mk
include lib/xlat_tables_v2/xlat_tables.mk

GIC_ENABLE_V4_EXTN	:=      1

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1
PLAT_INCLUDES := -I${MTK_PLAT}/common \
		 -I${MTK_PLAT}/include \
		 -I${MTK_PLAT}/include/${ARCH_VERSION} \
		 -I${MTK_PLAT} \
		 -I${MTK_PLAT_SOC}/include \
		 -Idrivers/arm/gic \

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

MODULES-y += $(MTK_PLAT)/common
MODULES-y += $(MTK_PLAT)/lib/mtk_init
MODULES-y += $(MTK_PLAT)/lib/pm
MODULES-y += $(MTK_PLAT)/drivers/gic600
MODULES-y += $(MTK_PLAT)/helpers
MODULES-y += $(MTK_PLAT)/topology

PLAT_BL_COMMON_SOURCES := common/desc_image_load.c \
			  drivers/arm/pl011/aarch64/pl011_console.S \
			  lib/bl_aux_params/bl_aux_params.c

BL31_SOURCES += drivers/delay_timer/delay_timer.c \
		drivers/delay_timer/generic_delay_timer.c \
		lib/cpus/aarch64/cortex_a520.S \
		lib/cpus/aarch64/cortex_a720.S \
		lib/cpus/aarch64/cortex_x4.S \
		${GICV3_SOURCES} \
		${XLAT_TABLES_LIB_SRCS} \
		plat/common/plat_gicv3.c \
		plat/common/plat_psci_common.c \
		plat/common/aarch64/crash_console_helpers.S \
		${MTK_PLAT}/common/mtk_plat_common.c \
		${MTK_PLAT}/common/params_setup.c \
		$(MTK_PLAT)/$(MTK_SOC)/plat_mmap.c

include plat/mediatek/build_helpers/mtk_build_helpers_epilogue.mk

include lib/coreboot/coreboot.mk

VDK := 1
$(eval $(call add_define, VDK))

ENABLE_FEAT_AMU := 1
