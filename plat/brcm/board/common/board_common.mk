#
# Copyright (c) 2015 - 2020, Broadcom
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_BL_COMMON_SOURCES	+=	plat/brcm/board/common/board_common.c

# If no board config makefile, do not include it
ifneq (${BOARD_CFG},)
BOARD_CFG_MAKE := $(shell find plat/brcm/board/${PLAT} -name '${BOARD_CFG}.mk')
$(eval $(call add_define,BOARD_CFG))
ifneq (${BOARD_CFG_MAKE},)
$(info Including ${BOARD_CFG_MAKE})
include ${BOARD_CFG_MAKE}
else
$(error Error: File ${BOARD_CFG}.mk not found in plat/brcm/board/${PLAT})
endif
endif

# To compile with highest log level (VERBOSE) set value to 50
LOG_LEVEL := 40

# Use custom generic timer clock
ifneq (${GENTIMER_ACTUAL_CLOCK},)
$(info Using GENTIMER_ACTUAL_CLOCK=$(GENTIMER_ACTUAL_CLOCK))
SYSCNT_FREQ := $(GENTIMER_ACTUAL_CLOCK)
$(eval $(call add_define,SYSCNT_FREQ))
endif

ifeq (${STANDALONE_BL2},yes)
$(eval $(call add_define,MMU_DISABLED))
endif

# BL2 XIP from QSPI
RUN_BL2_FROM_QSPI := 0
ifeq (${RUN_BL2_FROM_QSPI},1)
$(eval $(call add_define,RUN_BL2_FROM_QSPI))
endif

# Use CRMU SRAM from iHOST
ifneq (${USE_CRMU_SRAM},)
$(eval $(call add_define,USE_CRMU_SRAM))
endif

# On BRCM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

# Use generic OID definition (tbbr_oid.h)
USE_TBBR_DEFS			:=	1

PLAT_INCLUDES		+=	-Iplat/brcm/board/common

PLAT_BL_COMMON_SOURCES	+=	plat/brcm/common/brcm_common.c \
				plat/brcm/board/common/cmn_sec.c \
				plat/brcm/board/common/bcm_console.c \
				plat/brcm/board/common/plat_setup.c \
				plat/brcm/board/common/platform_common.c \
				drivers/arm/sp804/sp804_delay_timer.c \
				drivers/delay_timer/delay_timer.c \
				drivers/io/io_fip.c \
				drivers/io/io_memmap.c \
				drivers/io/io_storage.c \
				plat/brcm/common/brcm_io_storage.c \
				plat/brcm/board/common/err.c \
				drivers/arm/sp805/sp805.c

BL2_SOURCES		+=	plat/brcm/common/brcm_bl2_mem_params_desc.c \
				plat/brcm/common/brcm_image_load.c \
				common/desc_image_load.c

BL2_SOURCES		+= 	plat/brcm/common/brcm_bl2_setup.c

# Use translation tables library v1 by default
ARM_XLAT_TABLES_LIB_V1		:=	1
ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))
PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/aarch64/xlat_tables.c \
				lib/xlat_tables/xlat_tables_common.c
endif
