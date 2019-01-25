#
# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

override ERRATA_A53_855873 := 1
override PROGRAMMABLE_RESET_ADDRESS := 1
PSCI_EXTENDED_STATE_ID := 1
A53_DISABLE_NON_TEMPORAL_HINT := 0
SEPARATE_CODE_AND_RODATA := 1
ZYNQMP_WDT_RESTART := 0
override RESET_TO_BL31 := 1

# Do not enable SVE
ENABLE_SVE_FOR_NS	:= 0

WORKAROUND_CVE_2017_5715	:=	0

ifdef ZYNQMP_ATF_MEM_BASE
    $(eval $(call add_define,ZYNQMP_ATF_MEM_BASE))

    ifndef ZYNQMP_ATF_MEM_SIZE
        $(error "ZYNQMP_ATF_BASE defined without ZYNQMP_ATF_SIZE")
    endif
    $(eval $(call add_define,ZYNQMP_ATF_MEM_SIZE))

    ifdef ZYNQMP_ATF_MEM_PROGBITS_SIZE
        $(eval $(call add_define,ZYNQMP_ATF_MEM_PROGBITS_SIZE))
    endif
endif

ifdef ZYNQMP_BL32_MEM_BASE
    $(eval $(call add_define,ZYNQMP_BL32_MEM_BASE))

    ifndef ZYNQMP_BL32_MEM_SIZE
        $(error "ZYNQMP_BL32_BASE defined without ZYNQMP_BL32_SIZE")
    endif
    $(eval $(call add_define,ZYNQMP_BL32_MEM_SIZE))
endif

ZYNQMP_CONSOLE	?=	cadence
$(eval $(call add_define_val,ZYNQMP_CONSOLE,ZYNQMP_CONSOLE_ID_${ZYNQMP_CONSOLE}))

ifdef ZYNQMP_WDT_RESTART
$(eval $(call add_define,ZYNQMP_WDT_RESTART))
endif

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/aarch64/		\
				-Iplat/xilinx/common/include/			\
				-Iplat/xilinx/zynqmp/include/			\
				-Iplat/xilinx/zynqmp/pm_service/		\
				-Iplat/xilinx/zynqmp/ipi_mailbox_service/

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				drivers/cadence/uart/aarch64/cdns_console.S	\
				drivers/console/aarch64/console.S		\
				plat/arm/common/arm_cci.c			\
				plat/arm/common/arm_common.c			\
				plat/arm/common/arm_gicv2.c			\
				plat/common/plat_gicv2.c			\
				plat/xilinx/common/ipi.c			\
				plat/xilinx/zynqmp/aarch64/zynqmp_helpers.S	\
				plat/xilinx/zynqmp/aarch64/zynqmp_common.c

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				plat/common/plat_psci_common.c			\
				plat/xilinx/common/pm_service/pm_ipi.c		\
				plat/xilinx/zynqmp/bl31_zynqmp_setup.c		\
				plat/xilinx/zynqmp/plat_psci.c			\
				plat/xilinx/zynqmp/plat_zynqmp.c		\
				plat/xilinx/zynqmp/plat_startup.c		\
				plat/xilinx/zynqmp/plat_topology.c		\
				plat/xilinx/zynqmp/sip_svc_setup.c		\
				plat/xilinx/zynqmp/zynqmp_ipi.c		\
				plat/xilinx/zynqmp/pm_service/pm_svc_main.c	\
				plat/xilinx/zynqmp/pm_service/pm_api_sys.c	\
				plat/xilinx/zynqmp/pm_service/pm_api_pinctrl.c	\
				plat/xilinx/zynqmp/pm_service/pm_api_ioctl.c	\
				plat/xilinx/zynqmp/pm_service/pm_api_clock.c	\
				plat/xilinx/zynqmp/pm_service/pm_client.c	\
				plat/xilinx/zynqmp/ipi_mailbox_service/ipi_mailbox_svc.c
