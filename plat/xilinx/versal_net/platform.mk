# Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
# Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
# Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

PLAT_PATH := plat/xilinx/versal_net

# A78 Erratum for SoC
ERRATA_A78_AE_1941500 := 1
ERRATA_A78_AE_1951502 := 1
ERRATA_A78_AE_2376748 := 1
ERRATA_A78_AE_2395408 := 1

override PROGRAMMABLE_RESET_ADDRESS := 1
PSCI_EXTENDED_STATE_ID := 1
SEPARATE_CODE_AND_RODATA := 1
override RESET_TO_BL31 := 1
PL011_GENERIC_UART := 1
IPI_CRC_CHECK := 0
USE_GIC_DRIVER :=	3
GIC_ENABLE_V4_EXTN :=  0
GICV3_SUPPORT_GIC600 := 1
TFA_NO_PM := 0
CPU_PWRDWN_SGI ?= 6
$(eval $(call add_define_val,CPU_PWR_DOWN_REQ_INTR,ARM_IRQ_SEC_SGI_${CPU_PWRDWN_SGI}))

override CTX_INCLUDE_AARCH32_REGS    := 0

ifdef TFA_NO_PM
   $(eval $(call add_define,TFA_NO_PM))
endif

ifdef VERSAL_NET_ATF_MEM_BASE
    $(eval $(call add_define,VERSAL_NET_ATF_MEM_BASE))

    ifndef VERSAL_NET_ATF_MEM_SIZE
        $(error "VERSAL_NET_ATF_MEM_BASE defined without VERSAL_NET_ATF_MEM_SIZE")
    endif
    $(eval $(call add_define,VERSAL_NET_ATF_MEM_SIZE))

    ifdef VERSAL_NET_ATF_MEM_PROGBITS_SIZE
        $(eval $(call add_define,VERSAL_NET_ATF_MEM_PROGBITS_SIZE))
    endif
endif

ifdef VERSAL_NET_BL32_MEM_BASE
    $(eval $(call add_define,VERSAL_NET_BL32_MEM_BASE))

    ifndef VERSAL_NET_BL32_MEM_SIZE
        $(error "VERSAL_NET_BL32_MEM_BASE defined without VERSAL_NET_BL32_MEM_SIZE")
    endif
    $(eval $(call add_define,VERSAL_NET_BL32_MEM_SIZE))
endif

ifdef IPI_CRC_CHECK
    $(eval $(call add_define,IPI_CRC_CHECK))
endif

USE_COHERENT_MEM := 0
HW_ASSISTED_COHERENCY := 1

VERSAL_NET_CONSOLE	?=	pl011
ifeq (${VERSAL_NET_CONSOLE}, $(filter ${VERSAL_NET_CONSOLE},pl011 pl011_0 pl011_1 dcc dtb none))
else
  $(error Please define VERSAL_NET_CONSOLE)
endif

$(eval $(call add_define_val,VERSAL_NET_CONSOLE,VERSAL_NET_CONSOLE_ID_${VERSAL_NET_CONSOLE}))

ifdef XILINX_OF_BOARD_DTB_ADDR
XLNX_DT_CFG     := 1
$(eval $(call add_define,XILINX_OF_BOARD_DTB_ADDR))
else
XLNX_DT_CFG     := 0
endif
$(eval $(call add_define,XLNX_DT_CFG))

# Runtime console in default console in DEBUG build
ifeq ($(DEBUG), 1)
CONSOLE_RUNTIME ?= $(VERSAL_NET_CONSOLE)
endif

# Runtime console
ifdef CONSOLE_RUNTIME
ifeq (${CONSOLE_RUNTIME}, $(filter ${CONSOLE_RUNTIME},pl011 pl011_0 pl011_1 dcc dtb))
$(eval $(call add_define_val,CONSOLE_RUNTIME,RT_CONSOLE_ID_${CONSOLE_RUNTIME}))
else
$(error "Please define CONSOLE_RUNTIME")
endif
endif

# enable assert() for release/debug builds
ENABLE_ASSERTIONS := 1

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iplat/xilinx/common/include/			\
				-Iplat/xilinx/common/ipi_mailbox_service/	\
				-I${PLAT_PATH}/include/				\
				-Iplat/xilinx/versal/pm_service/

include lib/xlat_tables_v2/xlat_tables.mk
include lib/libfdt/libfdt.mk

PLAT_BL_COMMON_SOURCES	:=	\
				drivers/arm/dcc/dcc_console.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/pl011/aarch64/pl011_console.S	\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/arm/common/arm_common.c			\
				${PLAT_PATH}/aarch64/versal_net_helpers.S	\
				${PLAT_PATH}/aarch64/versal_net_common.c	\
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
				plat/xilinx/common/pm_service/pm_svc_main.c	\
				${PLAT_PATH}/pm_service/pm_client.c		\
				${PLAT_PATH}/versal_net_ipi.c
else
BL31_SOURCES		+=	${PLAT_PATH}/plat_psci.c
endif
BL31_SOURCES		+=	plat/xilinx/common/plat_fdt.c			\
				plat/xilinx/common/plat_startup.c		\
				plat/xilinx/common/plat_console.c		\
				plat/xilinx/common/plat_clkfunc.c		\
				plat/xilinx/common/ipi.c			\
				plat/xilinx/common/ipi_mailbox_service/ipi_mailbox_svc.c \
				plat/xilinx/common/versal.c			\
				${PLAT_PATH}/bl31_versal_net_setup.c		\
				common/fdt_fixup.c				\
				common/fdt_wrappers.c				\
				${LIBFDT_SRCS}					\
				${PLAT_PATH}/sip_svc_setup.c			\
				${XLAT_TABLES_LIB_SRCS}

SDEI_SUPPORT := 0
EL3_EXCEPTION_HANDLING := $(SDEI_SUPPORT)
ifeq (${SDEI_SUPPORT},1)
BL31_SOURCES		+=	plat/common/aarch64/plat_ehf.c          \
				plat/xilinx/versal_net/versal_net_sdei.c
endif
