# Copyright (c) 2018-2021, ARM Limited and Contributors. All rights reserved.
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

# A72 Erratum for SoC
ERRATA_A72_859971 := 1
ERRATA_A72_1319367 := 1

ifdef VERSAL_ATF_MEM_BASE
    $(eval $(call add_define,VERSAL_ATF_MEM_BASE))

    ifndef VERSAL_ATF_MEM_SIZE
        $(error "VERSAL_ATF_BASE defined without VERSAL_ATF_SIZE")
    endif
    $(eval $(call add_define,VERSAL_ATF_MEM_SIZE))

    ifdef VERSAL_ATF_MEM_PROGBITS_SIZE
        $(eval $(call add_define,VERSAL_ATF_MEM_PROGBITS_SIZE))
    endif
endif

ifdef VERSAL_BL32_MEM_BASE
    $(eval $(call add_define,VERSAL_BL32_MEM_BASE))

    ifndef VERSAL_BL32_MEM_SIZE
        $(error "VERSAL_BL32_BASE defined without VERSAL_BL32_SIZE")
    endif
    $(eval $(call add_define,VERSAL_BL32_MEM_SIZE))
endif

ifdef IPI_CRC_CHECK
    $(eval $(call add_define,IPI_CRC_CHECK))
endif

VERSAL_PLATFORM ?= silicon
$(eval $(call add_define_val,VERSAL_PLATFORM,VERSAL_PLATFORM_ID_${VERSAL_PLATFORM}))

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iplat/xilinx/common/include/			\
				-Iplat/xilinx/common/ipi_mailbox_service/	\
				-Iplat/xilinx/versal/include/			\
				-Iplat/xilinx/versal/pm_service/

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				drivers/arm/dcc/dcc_console.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${GICV3_SOURCES}				\
				drivers/arm/pl011/aarch64/pl011_console.S	\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/arm/common/arm_cci.c			\
				plat/arm/common/arm_common.c			\
				plat/common/plat_gicv3.c			\
				plat/xilinx/versal/aarch64/versal_helpers.S	\
				plat/xilinx/versal/aarch64/versal_common.c

VERSAL_CONSOLE	?=	pl011
ifeq (${VERSAL_CONSOLE}, $(filter ${VERSAL_CONSOLE},pl011 pl011_0 pl011_1 dcc))
else
  $(error "Please define VERSAL_CONSOLE")
endif

$(eval $(call add_define_val,VERSAL_CONSOLE,VERSAL_CONSOLE_ID_${VERSAL_CONSOLE}))

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				lib/cpus/aarch64/cortex_a72.S			\
				plat/common/plat_psci_common.c			\
				plat/xilinx/common/ipi.c			\
				plat/xilinx/common/plat_startup.c		\
				plat/xilinx/common/ipi_mailbox_service/ipi_mailbox_svc.c \
				plat/xilinx/common/pm_service/pm_ipi.c		\
				plat/xilinx/versal/bl31_versal_setup.c		\
				plat/xilinx/versal/plat_psci.c			\
				plat/xilinx/versal/plat_versal.c		\
				plat/xilinx/versal/plat_topology.c		\
				plat/xilinx/versal/sip_svc_setup.c		\
				plat/xilinx/versal/versal_gicv3.c		\
				plat/xilinx/versal/versal_ipi.c			\
				plat/xilinx/versal/pm_service/pm_svc_main.c	\
				plat/xilinx/versal/pm_service/pm_api_sys.c	\
				plat/xilinx/versal/pm_service/pm_client.c

ifeq ($(HARDEN_SLS_ALL), 1)
TF_CFLAGS_aarch64      +=      -mharden-sls=all
endif
