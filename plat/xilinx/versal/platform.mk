# Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
# Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

override PROGRAMMABLE_RESET_ADDRESS := 1
PSCI_EXTENDED_STATE_ID := 1
A53_DISABLE_NON_TEMPORAL_HINT := 0
SEPARATE_CODE_AND_RODATA := 1
override RESET_TO_BL31 := 1
PL011_GENERIC_UART := 1
IPI_CRC_CHECK := 0
HARDEN_SLS_ALL := 0
CPU_PWRDWN_SGI ?= 6
$(eval $(call add_define_val,CPU_PWR_DOWN_REQ_INTR,ARM_IRQ_SEC_SGI_${CPU_PWRDWN_SGI}))

# A72 Erratum for SoC
ERRATA_A72_859971 := 1
ERRATA_A72_1319367 := 1

ifdef VERSAL_ATF_MEM_BASE
    $(eval $(call add_define,VERSAL_ATF_MEM_BASE))

    ifndef VERSAL_ATF_MEM_SIZE
        $(error "VERSAL_ATF_MEM_BASE defined without VERSAL_ATF_MEM_SIZE")
    endif
    $(eval $(call add_define,VERSAL_ATF_MEM_SIZE))

    ifdef VERSAL_ATF_MEM_PROGBITS_SIZE
        $(eval $(call add_define,VERSAL_ATF_MEM_PROGBITS_SIZE))
    endif
endif

ifdef VERSAL_BL32_MEM_BASE
    $(eval $(call add_define,VERSAL_BL32_MEM_BASE))

    ifndef VERSAL_BL32_MEM_SIZE
        $(error "VERSAL_BL32_MEM_BASE defined without VERSAL_BL32_MEM_SIZE")
    endif
    $(eval $(call add_define,VERSAL_BL32_MEM_SIZE))
endif

ifdef IPI_CRC_CHECK
    $(eval $(call add_define,IPI_CRC_CHECK))
endif

ifdef VERSAL_PLATFORM
    $(warning "VERSAL_PLATFORM has been deprecated")
endif

ifdef XILINX_OF_BOARD_DTB_ADDR
$(eval $(call add_define,XILINX_OF_BOARD_DTB_ADDR))
endif

PLAT_XLAT_TABLES_DYNAMIC := 0
ifeq (${PLAT_XLAT_TABLES_DYNAMIC},1)
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))
endif

# enable assert() for release/debug builds
ENABLE_ASSERTIONS := 1

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iplat/xilinx/common/include/			\
				-Iplat/xilinx/common/ipi_mailbox_service/	\
				-Iplat/xilinx/versal/include/			\
				-Iplat/xilinx/versal/pm_service/

include lib/libfdt/libfdt.mk
# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	:= 	drivers/arm/dcc/dcc_console.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${GICV3_SOURCES}				\
				drivers/arm/pl011/aarch64/pl011_console.S	\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/arm/common/arm_cci.c			\
				plat/arm/common/arm_common.c			\
				plat/common/plat_gicv3.c			\
				plat/xilinx/versal/aarch64/versal_helpers.S	\
				plat/xilinx/versal/aarch64/versal_common.c	\
				${XLAT_TABLES_LIB_SRCS}

VERSAL_CONSOLE	?=	pl011
ifeq (${VERSAL_CONSOLE}, $(filter ${VERSAL_CONSOLE},pl011 pl011_0 pl011_1 dcc dtb none))
else
  $(error "Please define VERSAL_CONSOLE")
endif

$(eval $(call add_define_val,VERSAL_CONSOLE,VERSAL_CONSOLE_ID_${VERSAL_CONSOLE}))

# Runtime console in default console in DEBUG build
ifeq ($(DEBUG), 1)
CONSOLE_RUNTIME ?= pl011
endif

# Runtime console
ifdef CONSOLE_RUNTIME
ifeq (${CONSOLE_RUNTIME}, $(filter ${CONSOLE_RUNTIME},pl011 pl011_0 pl011_1 dcc dtb))
$(eval $(call add_define_val,CONSOLE_RUNTIME,RT_CONSOLE_ID_${CONSOLE_RUNTIME}))
else
$(error "Please define CONSOLE_RUNTIME")
endif
endif

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				lib/cpus/aarch64/cortex_a72.S			\
				common/fdt_wrappers.c                           \
				plat/common/plat_psci_common.c			\
				plat/xilinx/common/ipi.c			\
				plat/xilinx/common/plat_fdt.c			\
				plat/xilinx/common/plat_console.c               \
				plat/xilinx/common/plat_clkfunc.c               \
				plat/xilinx/common/plat_startup.c		\
				plat/xilinx/common/ipi_mailbox_service/ipi_mailbox_svc.c \
				plat/xilinx/common/pm_service/pm_ipi.c		\
				plat/xilinx/common/pm_service/pm_api_sys.c	\
				plat/xilinx/common/pm_service/pm_svc_main.c	\
				plat/xilinx/common/versal.c			\
				plat/xilinx/versal/bl31_versal_setup.c		\
				plat/xilinx/versal/plat_psci.c			\
				plat/xilinx/versal/plat_versal.c		\
				plat/xilinx/versal/plat_topology.c		\
				plat/xilinx/versal/sip_svc_setup.c		\
				plat/xilinx/versal/versal_gicv3.c		\
				plat/xilinx/versal/versal_ipi.c			\
				plat/xilinx/versal/pm_service/pm_client.c	\
				common/fdt_fixup.c				\
				${LIBFDT_SRCS}

ifeq ($(HARDEN_SLS_ALL), 1)
TF_CFLAGS_aarch64      +=      -mharden-sls=all
endif

ifeq (${ERRATA_ABI_SUPPORT}, 1)
# enable the cpu macros for errata abi interface
CORTEX_A72_H_INC	:= 1
$(eval $(call add_define, CORTEX_A72_H_INC))
endif
