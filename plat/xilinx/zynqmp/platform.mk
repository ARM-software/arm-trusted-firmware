#
# Copyright (c) 2013-2021, Arm Limited and Contributors. All rights reserved.
# Portions copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
# Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
# Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

override ERRATA_A53_855873 := 1
ERRATA_A53_1530924 := 1
override PROGRAMMABLE_RESET_ADDRESS := 1
PSCI_EXTENDED_STATE_ID := 1
A53_DISABLE_NON_TEMPORAL_HINT := 0
SEPARATE_CODE_AND_RODATA := 1
ZYNQMP_WDT_RESTART := 0
IPI_CRC_CHECK := 0
override RESET_TO_BL31 := 1
override WARMBOOT_ENABLE_DCACHE_EARLY := 1
ENABLE_LTO := 1

EL3_EXCEPTION_HANDLING := $(SDEI_SUPPORT)

# pncd SPD requires secure SGI to be handled at EL1
ifeq (${SPD}, $(filter ${SPD},pncd tspd opteed))
ifeq (${ZYNQMP_WDT_RESTART},1)
$(error "Error: ZYNQMP_WDT_RESTART and SPD=pncd are incompatible")
endif
override GICV2_G0_FOR_EL3 := 0
else
override GICV2_G0_FOR_EL3 := 1
endif

# Do not enable SVE
ENABLE_SVE_FOR_NS	:= 0

WORKAROUND_CVE_2017_5715	:=	0

ifdef ZYNQMP_ATF_MEM_BASE
    $(eval $(call add_define,ZYNQMP_ATF_MEM_BASE))

    ifndef ZYNQMP_ATF_MEM_SIZE
        $(error "ZYNQMP_ATF_MEM_BASE defined without ZYNQMP_ATF_MEM_SIZE")
    endif
    $(eval $(call add_define,ZYNQMP_ATF_MEM_SIZE))

    ifdef ZYNQMP_ATF_MEM_PROGBITS_SIZE
        $(eval $(call add_define,ZYNQMP_ATF_MEM_PROGBITS_SIZE))
    endif

    # enable assert() when TF-A runs from DDR memory.
    ENABLE_ASSERTIONS := 1

endif

ifdef ZYNQMP_BL32_MEM_BASE
    $(eval $(call add_define,ZYNQMP_BL32_MEM_BASE))

    ifndef ZYNQMP_BL32_MEM_SIZE
        $(error "ZYNQMP_BL32_MEM_BASE defined without ZYNQMP_BL32_MEM_SIZE")
    endif
    $(eval $(call add_define,ZYNQMP_BL32_MEM_SIZE))
endif


ifdef ZYNQMP_WDT_RESTART
    $(eval $(call add_define,ZYNQMP_WDT_RESTART))
endif

ifdef ZYNQMP_IPI_CRC_CHECK
    $(warning "ZYNQMP_IPI_CRC_CHECK macro is deprecated...instead please use IPI_CRC_CHECK.")
endif

ifdef IPI_CRC_CHECK
    $(eval $(call add_define,IPI_CRC_CHECK))
endif

ifdef ZYNQMP_SECURE_EFUSES
    $(eval $(call add_define,ZYNQMP_SECURE_EFUSES))
endif

ifdef XILINX_OF_BOARD_DTB_ADDR
$(eval $(call add_define,XILINX_OF_BOARD_DTB_ADDR))
endif

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iinclude/plat/arm/common/aarch64/		\
				-Iplat/xilinx/common/include/			\
				-Iplat/xilinx/common/ipi_mailbox_service/	\
				-Iplat/xilinx/zynqmp/include/			\
				-Iplat/xilinx/zynqmp/pm_service/		\

include lib/libfdt/libfdt.mk
# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/dcc/dcc_console.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${GICV2_SOURCES}				\
				drivers/cadence/uart/aarch64/cdns_console.S	\
				plat/arm/common/arm_cci.c			\
				plat/arm/common/arm_common.c			\
				plat/arm/common/arm_gicv2.c			\
				plat/common/plat_gicv2.c			\
				plat/xilinx/common/ipi.c			\
				plat/xilinx/zynqmp/zynqmp_ipi.c			\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/xilinx/zynqmp/aarch64/zynqmp_helpers.S	\
				plat/xilinx/zynqmp/aarch64/zynqmp_common.c	\
				${XLAT_TABLES_LIB_SRCS}

ZYNQMP_CONSOLE	?=	cadence
ifeq (${ZYNQMP_CONSOLE}, $(filter ${ZYNQMP_CONSOLE},cadence cadence0 cadence1 dcc dtb none))
else
  $(error "Please define ZYNQMP_CONSOLE")
endif
$(eval $(call add_define_val,ZYNQMP_CONSOLE,ZYNQMP_CONSOLE_ID_${ZYNQMP_CONSOLE}))

# Runtime console in default console in DEBUG build
ifeq ($(DEBUG), 1)
CONSOLE_RUNTIME ?= cadence
endif

# Runtime console
ifdef CONSOLE_RUNTIME
ifeq (${CONSOLE_RUNTIME}, $(filter ${CONSOLE_RUNTIME},cadence cadence0 cadence1 dcc dtb))
$(eval $(call add_define_val,CONSOLE_RUNTIME,RT_CONSOLE_ID_${CONSOLE_RUNTIME}))
else
$(error "Please define CONSOLE_RUNTIME")
endif
endif

# Build PM code as a Library
include plat/xilinx/zynqmp/libpm.mk

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				plat/common/plat_psci_common.c			\
				common/fdt_fixup.c				\
				common/fdt_wrappers.c				\
				${LIBFDT_SRCS}					\
				plat/xilinx/common/ipi_mailbox_service/ipi_mailbox_svc.c \
				plat/xilinx/common/plat_startup.c		\
				plat/xilinx/common/plat_console.c		\
				plat/xilinx/common/plat_fdt.c			\
				plat/xilinx/zynqmp/bl31_zynqmp_setup.c		\
				plat/xilinx/zynqmp/plat_psci.c			\
				plat/xilinx/zynqmp/plat_zynqmp.c		\
				plat/xilinx/zynqmp/plat_topology.c		\
				plat/xilinx/zynqmp/sip_svc_setup.c

ifeq (${SDEI_SUPPORT},1)
BL31_SOURCES		+=	plat/xilinx/zynqmp/zynqmp_ehf.c			\
				plat/xilinx/zynqmp/zynqmp_sdei.c
endif

BL31_CPPFLAGS		+=	-fno-jump-tables
TF_CFLAGS_aarch64	+=	-mbranch-protection=none

ifdef CUSTOM_PKG_PATH
include $(CUSTOM_PKG_PATH)/custom_pkg.mk
else
BL31_SOURCES		+=	plat/xilinx/zynqmp/custom_sip_svc.c
endif

ifneq (${RESET_TO_BL31},1)
  $(error "Using BL31 as the reset vector is only one option supported on ZynqMP. Please set RESET_TO_BL31 to 1.")
endif
