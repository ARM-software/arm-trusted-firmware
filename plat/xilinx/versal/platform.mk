# Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

override PROGRAMMABLE_RESET_ADDRESS := 1
PSCI_EXTENDED_STATE_ID := 1
A53_DISABLE_NON_TEMPORAL_HINT := 0
SEPARATE_CODE_AND_RODATA := 1
override RESET_TO_BL31 := 1
PL011_GENERIC_UART := 1
MULTI_CONSOLE_API := 1

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

VERSAL_PLATFORM ?= versal_virt
$(eval $(call add_define_val,VERSAL_PLATFORM,VERSAL_PLATFORM_ID_${VERSAL_PLATFORM}))

VERSAL_CONSOLE	?=	pl011
$(eval $(call add_define_val,VERSAL_CONSOLE,VERSAL_CONSOLE_ID_${VERSAL_CONSOLE}))

PLAT_INCLUDES		:=	-Iplat/xilinx/versal/include/

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v3/gicv3_main.c			\
				drivers/arm/gic/v3/gicv3_helpers.c		\
				drivers/arm/pl011/aarch64/pl011_console.S	\
				plat/common/aarch64/crash_console_helpers.S	\
				plat/common/plat_gicv3.c			\
				plat/xilinx/versal/aarch64/versal_helpers.S	\
				plat/xilinx/versal/aarch64/versal_common.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				plat/common/plat_psci_common.c			\
				plat/xilinx/versal/bl31_versal_setup.c		\
				plat/xilinx/versal/plat_psci.c			\
				plat/xilinx/versal/plat_versal.c		\
				plat/xilinx/versal/plat_topology.c		\
				plat/xilinx/versal/sip_svc_setup.c		\
				plat/xilinx/versal/versal_gicv3.c
