# Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
# Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
# Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
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
TFA_NO_PM := 0
CPU_PWRDWN_SGI ?= 6
$(eval $(call add_define_val,CPU_PWR_DOWN_REQ_INTR,ARM_IRQ_SEC_SGI_${CPU_PWRDWN_SGI}))

override CTX_INCLUDE_AARCH32_REGS    := 0

# Platform to support Dynamic XLAT Table by default
override PLAT_XLAT_TABLES_DYNAMIC := 1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

ifdef TFA_NO_PM
   $(eval $(call add_define,TFA_NO_PM))
endif

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

CONSOLE  ?=      pl011
ifeq (${CONSOLE}, $(filter ${CONSOLE},pl011 pl011_0 pl011_1 dcc dtb none))
	else
	  $(error "Please define CONSOLE")
  endif

$(eval $(call add_define_val,CONSOLE,CONSOLE_ID_${CONSOLE}))

# Runtime console in default console in DEBUG build
ifeq ($(DEBUG), 1)
CONSOLE_RUNTIME ?= $(CONSOLE)
endif

# Runtime console
ifdef CONSOLE_RUNTIME
ifeq 	(${CONSOLE_RUNTIME}, $(filter ${CONSOLE_RUNTIME},pl011 pl011_0 pl011_1 dcc dtb))
$(eval $(call add_define_val,CONSOLE_RUNTIME,RT_CONSOLE_ID_${CONSOLE_RUNTIME}))
else
	$(error "Please define CONSOLE_RUNTIME")
endif
endif

ifeq (${TRANSFER_LIST},0)
XILINX_OF_BOARD_DTB_ADDR ?= 0x1000000
$(eval $(call add_define,XILINX_OF_BOARD_DTB_ADDR))
endif

ifeq (${SPD},spmd)
SPMC_MANIFEST_DTB_ADDR ?= 0x9800000
$(eval $(call add_define,SPMC_MANIFEST_DTB_ADDR))
endif

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iplat/xilinx/common/include/			\
				-Iplat/amd/common/include/			\
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
				plat/xilinx/common/plat_clkfunc.c		\
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
				plat/common/plat_psci_common.c

ifeq ($(TFA_NO_PM), 0)
BL31_SOURCES		+=	plat/xilinx/common/pm_service/pm_api_sys.c	\
				plat/xilinx/common/pm_service/pm_ipi.c		\
				${PLAT_PATH}/plat_psci_pm.c			\
				${PLAT_PATH}/pm_service/pm_svc_main.c	\
				${PLAT_PATH}/pm_service/pm_client.c
else
BL31_SOURCES		+=	${PLAT_PATH}/plat_psci.c			\
				drivers/scmi-msg/base.c				\
				drivers/scmi-msg/entry.c			\
				drivers/scmi-msg/smt.c				\
				drivers/scmi-msg/clock.c			\
				drivers/scmi-msg/power_domain.c			\
				drivers/scmi-msg/reset_domain.c			\
				${PLAT_PATH}/scmi.c
endif

BL31_SOURCES		+=	common/fdt_wrappers.c                           \
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

ifeq (${SPD},spmd)
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c        \
				common/uuid.c                           \
				${LIBFDT_SRCS}                          \
				${FDT_WRAPPERS_SOURCES}

ARM_SPMC_MANIFEST_DTS	:=	${PLAT_PATH}/spmc_sel1_optee_manifest.dts

FDT_SOURCES		+=	${ARM_SPMC_MANIFEST_DTS}

VERSAL2_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${ARM_SPMC_MANIFEST_DTS})).dtb

$(eval $(call TOOL_ADD_PAYLOAD,${VERSAL2_TOS_FW_CONFIG},--tos-fw-config,${VERSAL2_TOS_FW_CONFIG}))
endif

ifeq ($(DEBUG),1)
BL31_SOURCES            +=      ${PLAT_PATH}/plat_ocm_coherency.c
endif

ifeq (${ERRATA_ABI_SUPPORT}, 1)
# enable the cpu macros for errata abi interface
CORTEX_A78_AE_H_INC     := 1
$(eval $(call add_define, CORTEX_A78_AE_H_INC))
endif

# Enable Handoff protocol using transfer lists
TRANSFER_LIST                   ?= 0

ifeq (${TRANSFER_LIST},1)
include lib/transfer_list/transfer_list.mk
BL31_SOURCES           +=	plat/amd/common/plat_fdt.c
BL31_SOURCES           +=	plat/amd/common/plat_xfer_list.c
else
BL31_SOURCES           +=	plat/xilinx/common/plat_fdt.c
endif

XLNX_DT_CFG	?= 1
ifeq (${TRANSFER_LIST},0)
ifndef XILINX_OF_BOARD_DTB_ADDR
XLNX_DT_CFG	:= 0
endif
endif
$(eval $(call add_define,XLNX_DT_CFG))

ifdef CUSTOM_PKG_PATH
include $(CUSTOM_PKG_PATH)/custom_pkg.mk
else
BL31_SOURCES		+=	plat/xilinx/common/custom_sip_svc.c
endif
