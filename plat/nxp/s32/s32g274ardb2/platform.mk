#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_DRIVERS_PATH := drivers/nxp
PLAT_COMMON_PATH  := plat/nxp/common
PLAT_S32G274ARDB2 := plat/nxp/s32/s32g274ardb2

CONSOLE           := LINFLEX

include ${PLAT_COMMON_PATH}/plat_make_helper/plat_build_macros.mk

PLAT_INCLUDES = \
	-I${PLAT_S32G274ARDB2}/include

PROGRAMMABLE_RESET_ADDRESS := 1

COLD_BOOT_SINGLE_CPU := 0

ENABLE_SVE_FOR_NS := 0

RESET_TO_BL2 := 1

INIT_UNUSED_NS_EL2 := 1

ERRATA_A53_855873 := 1
ERRATA_A53_836870 := 1
ERRATA_A53_1530924 := 1
ERRATA_SPECULATIVE_AT := 1

# Selecting Drivers for SoC
$(eval $(call SET_NXP_MAKE_FLAG,CONSOLE_NEEDED,BL_COMM))

include ${PLAT_DRIVERS_PATH}/drivers.mk


BL_COMMON_SOURCES += \
	${PLAT_S32G274ARDB2}/plat_console.c \
	${PLAT_S32G274ARDB2}/plat_helpers.S \

BL2_SOURCES += \
	${BL_COMMON_SOURCES} \
	${PLAT_S32G274ARDB2}/plat_bl2_el3_setup.c \
	${PLAT_S32G274ARDB2}/plat_bl2_image_desc.c \
	${PLAT_S32G274ARDB2}/plat_io_storage.c \
	common/desc_image_load.c \
	drivers/io/io_fip.c \
	drivers/io/io_memmap.c \
	drivers/io/io_storage.c \
	lib/cpus/aarch64/cortex_a53.S \

BL31_SOURCES += \
	${GICV3_SOURCES} \
	${PLAT_S32G274ARDB2}/plat_bl31_setup.c \
	${PLAT_S32G274ARDB2}/s32g2_psci.c \
	${PLAT_S32G274ARDB2}/s32g2_soc.c \
	${XLAT_TABLES_LIB_SRCS} \
	lib/cpus/aarch64/cortex_a53.S \
	plat/common/plat_gicv3.c \
	plat/common/plat_psci_common.c \
