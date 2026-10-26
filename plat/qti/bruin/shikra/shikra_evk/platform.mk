#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Makefile for Shikra based QTI Bruin platform.

PLAT_PATH				:=	plat/qti
CHIPSET					:=	shikra

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA		:=	1
USE_COHERENT_MEM			:=	0
WARMBOOT_ENABLE_DCACHE_EARLY		:=	1
HW_ASSISTED_COHERENCY			:=	1

#Enable errata configs for cortex_a55
ERRATA_A55_1221012			:=	1
ERRATA_A55_1530923			:=	1

# Cortex-A78 errata
ERRATA_A78_1941498                    :=      1
ERRATA_A78_1951500                    :=      1
ERRATA_A78_2132060                    :=      1
ERRATA_A78_2242635                    :=      1
ERRATA_A78_2376745                    :=      1
ERRATA_A78_2395406                    :=      1
ERRATA_A78_2742426                    :=      1
ERRATA_A78_2772019                    :=      1
ERRATA_A78_2779479                    :=      1
ERRATA_A78_3888017                    :=      1
ERRATA_A78_4302972                    :=      1
# CVE
WORKAROUND_CVE_2025_10263             :=      1

# CVE-2025-0647 (cpp rctx) requires ARMv8.5-PREDRES which GCC 6 does not support
WORKAROUND_CVE_2025_0647		:=	0

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID			:=	1
ARM_RECOM_STATE_ID_ENC 			:=	1
PSCI_OS_INIT_MODE			:=	1

# GIC-600 configuration
GICV3_SUPPORT_GIC600			:=	1

COLD_BOOT_SINGLE_CPU			:=	1
PROGRAMMABLE_RESET_ADDRESS		:=	1

# Enable the dynamic translation tables library
PLAT_XLAT_TABLES_DYNAMIC		:=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

#disable CTX_INCLUDE_AARCH32_REGS to support shikra silver cores
override CTX_INCLUDE_AARCH32_REGS	:=	0
# Cortex-A55 is not susceptible to Spectre v2 (CVE-2017-5715).
WORKAROUND_CVE_2017_5715		:=      0
DYNAMIC_WORKAROUND_CVE_2018_3639	:=      1
# Enable stack protector.
ENABLE_STACK_PROTECTOR := strong

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\
				-I${PLAT_PATH}/bruin/${CHIPSET}/inc			\
				-I${PLAT_PATH}/bruin/${CHIPSET}/${PLAT}/inc		\
				-I${PLAT_PATH}/bruin/common/inc			\
				-I${PLAT_PATH}/common/inc				\
				-I${PLAT_PATH}/common/inc/$(ARCH)

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c				\
				drivers/qti/crypto/rng.c				\
				lib/cpus/aarch64/cortex_a55.S				\
				lib/cpus/aarch64/cortex_a78.S				\
				lib/bl_aux_params/bl_aux_params.c			\
				plat/common/aarch64/crash_console_helpers.S		\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_PATH)/common/src/qti_stack_protector.c		\
				$(PLAT_PATH)/common/src/qti_common.c			\
				${XLAT_TABLES_LIB_SRCS}

include drivers/arm/gic/v3/gicv3.mk
BL31_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c		\
				drivers/delay_timer/delay_timer.c			\
				plat/common/plat_gicv3.c				\
				${GICV3_SOURCES}					\
				plat/common/plat_psci_common.c				\
				$(PLAT_PATH)/common/src/pm_ps_hold.c			\
				$(PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(PLAT_PATH)/common/src/qti_gic_v3.c			\
				$(PLAT_PATH)/common/src/qti_gicv3_platform.c		\
				$(PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(PLAT_PATH)/common/src/qti_interrupt_svc_el3.c		\
				$(PLAT_PATH)/common/src/qti_syscall.c			\
				$(PLAT_PATH)/common/src/qti_pm.c			\
				$(PLAT_PATH)/common/src/spmi_arb.c			\
				$(PLAT_PATH)/bruin/common/$(ARCH)/bruin_helpers.S	\
				$(PLAT_PATH)/bruin/common/bruin_bl31_setup.c		\
				$(PLAT_PATH)/bruin/common/bruin_topology.c		\
				$(PLAT_PATH)/bruin/common/bruin_pm.c			\
				$(PLAT_PATH)/bruin/shikra/shikra_pm.c

include drivers/qti/smem/smem.mk
include drivers/qti/chipinfo/chipinfo.mk

PLAT_INCLUDES   +=	-Iinclude/drivers/qti/sec_core/${CHIPSET}		\
			-Iinclude/drivers/qti/accesscontrol			\
			-Iinclude/drivers/qti/qtimer/bruin			\
			-Iinclude/drivers/qti/watchdog/bruin

BL31_SOURCES	+=	drivers/qti/sec_core/sec_core.c				\
			drivers/qti/sec_core/${CHIPSET}/sec_core_cfg.c		\
			drivers/qti/accesscontrol/access_control_stub.c		\
			drivers/qti/qtimer/qtimer.c				\
			drivers/qti/watchdog/watchdog.c
