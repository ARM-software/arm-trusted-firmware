# Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
# Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
# Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

PLAT_PATH := plat/amd/versal2

override NEED_BL1 := no
override NEED_BL2 := no

# A78 Erratum for SoC
ERRATA_A78_AE_1941500 := 1
ERRATA_A78_AE_1951502 := 1
ERRATA_A78_AE_2376748 := 1
ERRATA_A78_AE_2395408 := 1
ERRATA_ABI_SUPPORT    := 1

# Platform Supports Armv8.2 extensions
ARM_ARCH_MAJOR := 8
ARM_ARCH_MINOR := 2

override PROGRAMMABLE_RESET_ADDRESS := 1
PSCI_EXTENDED_STATE_ID := 1
SEPARATE_CODE_AND_RODATA := 1
override RESET_TO_BL31 := 1
PL011_GENERIC_UART := 1
IPI_CRC_CHECK := 0
GIC_ENABLE_V4_EXTN :=  0
GICV3_SUPPORT_GIC600 := 1

override CTX_INCLUDE_AARCH32_REGS    := 0

# Platform to support Dynamic XLAT Table by default
override PLAT_XLAT_TABLES_DYNAMIC := 1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

ifdef MEM_BASE
    $(eval $(call add_define,MEM_BASE))

    ifndef MEM_SIZE
        $(error "MEM_BASE defined without MEM_SIZE")
    endif
    $(eval $(call add_define,MEM_SIZE))

    ifdef MEM_PROGBITS_SIZE
        $(eval $(call add_define,MEM_PROGBITS_SIZE))
    endif
endif

ifdef BL32_MEM_BASE
    $(eval $(call add_define,BL32_MEM_BASE))

    ifndef BL32_MEM_SIZE
        $(error "BL32_MEM_BASE defined without BL32_MEM_SIZE")
    endif
    $(eval $(call add_define,BL32_MEM_SIZE))
endif

ifdef IPI_CRC_CHECK
    $(eval $(call add_define,IPI_CRC_CHECK))
endif

USE_COHERENT_MEM := 0
HW_ASSISTED_COHERENCY := 1

VERSAL2_CONSOLE  ?=      pl011
ifeq (${VERSAL2_CONSOLE}, $(filter ${VERSAL2_CONSOLE},pl011 pl011_0 pl011_1 dcc dtb none))
	else
	  $(error "Please define VERSAL2_CONSOLE")
  endif

$(eval $(call add_define_val,VERSAL2_CONSOLE,VERSAL2_CONSOLE_ID_${VERSAL2_CONSOLE}))

# Runtime console in default console in DEBUG build
ifeq ($(DEBUG), 1)
CONSOLE_RUNTIME ?= pl011
endif

# Runtime console
ifdef CONSOLE_RUNTIME
ifeq 	(${CONSOLE_RUNTIME}, $(filter ${CONSOLE_RUNTIME},pl011 pl011_0 pl011_1 dcc dtb))
$(eval $(call add_define_val,CONSOLE_RUNTIME,RT_CONSOLE_ID_${CONSOLE_RUNTIME}))
else
	$(error "Please define CONSOLE_RUNTIME")
endif
endif


ifdef XILINX_OF_BOARD_DTB_ADDR
$(eval $(call add_define,XILINX_OF_BOARD_DTB_ADDR))
endif

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iplat/xilinx/common/include/			\
				-Iplat/xilinx/common/ipi_mailbox_service/	\
				-I${PLAT_PATH}/include/				\
				-Iplat/xilinx/versal/pm_service/

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk
include lib/libfdt/libfdt.mk

PLAT_BL_COMMON_SOURCES	:=	\
				drivers/arm/dcc/dcc_console.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${GICV3_SOURCES}				\
				drivers/arm/pl011/aarch64/pl011_console.S	\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/arm/common/arm_common.c			\
				plat/common/plat_gicv3.c			\
				${PLAT_PATH}/aarch64/helpers.S			\
				${PLAT_PATH}/aarch64/common.c			\
				${PLAT_PATH}/plat_topology.c                    \
				${XLAT_TABLES_LIB_SRCS}

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				lib/cpus/aarch64/cortex_a78_ae.S		\
				lib/cpus/aarch64/cortex_a78.S			\
				plat/common/plat_psci_common.c			\
				drivers/scmi-msg/base.c				\
				drivers/scmi-msg/entry.c			\
				drivers/scmi-msg/smt.c				\
				drivers/scmi-msg/clock.c			\
				drivers/scmi-msg/power_domain.c			\
				drivers/scmi-msg/reset_domain.c			\
				${PLAT_PATH}/scmi.c

BL31_SOURCES		+=	${PLAT_PATH}/plat_psci.c

BL31_SOURCES		+=	plat/xilinx/common/plat_fdt.c			\
				common/fdt_wrappers.c                           \
				plat/xilinx/common/plat_fdt.c                   \
				plat/xilinx/common/plat_console.c               \
				plat/xilinx/common/plat_startup.c		\
				plat/xilinx/common/ipi.c			\
				plat/xilinx/common/ipi_mailbox_service/ipi_mailbox_svc.c	\
				${PLAT_PATH}/soc_ipi.c				\
				plat/xilinx/common/versal.c			\
				${PLAT_PATH}/bl31_setup.c			\
				common/fdt_fixup.c				\
				common/fdt_wrappers.c				\
				${LIBFDT_SRCS}					\
				${PLAT_PATH}/sip_svc_setup.c			\
				${PLAT_PATH}/gicv3.c

ifeq (${ERRATA_ABI_SUPPORT}, 1)
# enable the cpu macros for errata abi interface
CORTEX_A78_AE_H_INC     := 1
$(eval $(call add_define, CORTEX_A78_AE_H_INC))
endif

# Enable Handoff protocol using transfer lists
TRANSFER_LIST                   := 1

include lib/transfer_list/transfer_list.mk
BL31_SOURCES           +=      plat/xilinx/common/plat_xfer_list.c
