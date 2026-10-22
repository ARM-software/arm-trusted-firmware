#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_PATH				:=	plat/qti
CHIPSET					:=	agatti

RESET_TO_BL2				:=	1
COLD_BOOT_SINGLE_CPU			:=	1
PROGRAMMABLE_RESET_ADDRESS		:=	1
SEPARATE_CODE_AND_RODATA		:=	1
USE_COHERENT_MEM			:=	0
WARMBOOT_ENABLE_DCACHE_EARLY		:=	1
ENABLE_SPE_FOR_NS			:=	0
ENABLE_SVE_FOR_NS			:=	0
WORKAROUND_CVE_2017_5715		:=	0
WORKAROUND_CVE_2022_23960		:=	0

ERRATA_A53_855873			:=	1
ERRATA_A53_1530924			:=	1

PSCI_EXTENDED_STATE_ID			:=	1
ARM_RECOM_STATE_ID_ENC			:=	1

PLAT_XLAT_TABLES_DYNAMIC		:=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\
				-I${PLAT_PATH}/bruin/${CHIPSET}/inc			\
				-I${PLAT_PATH}/bruin/${CHIPSET}/${PLAT}/inc		\
				-I${PLAT_PATH}/bruin/common/inc			\
				-I${PLAT_PATH}/common/inc				\
				-I${PLAT_PATH}/common/inc/$(ARCH)			\
				-Iinclude/drivers/qti/qtimer/bruin			\
				-Iinclude/drivers/qti/sec_core/${CHIPSET}

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c				\
				lib/bl_aux_params/bl_aux_params.c			\
				plat/common/aarch64/crash_console_helpers.S		\
				$(PLAT_PATH)/bruin/common/$(ARCH)/bruin_helpers.S	\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_kryo2_silver.S	\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_PATH)/common/src/qti_common.c			\
				${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	drivers/io/io_fip.c					\
				drivers/io/io_memmap.c					\
				drivers/io/io_storage.c					\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_bl2_helpers.S	\
				$(PLAT_PATH)/common/src/qti_bl2_setup.c			\
				$(PLAT_PATH)/common/src/qti_image_desc.c		\
				$(PLAT_PATH)/common/src/qti_io_storage.c

include drivers/arm/gic/v3/gicv3.mk

BL31_SOURCES		+=	drivers/delay_timer/delay_timer.c			\
				drivers/delay_timer/generic_delay_timer.c		\
				drivers/qti/accesscontrol/access_control_stub.c		\
				drivers/qti/qtimer/qtimer.c				\
				drivers/qti/sec_core/sec_core.c				\
				drivers/qti/sec_core/${CHIPSET}/sec_core_cfg.c		\
				drivers/qti/watchdog/watchdog_stub.c			\
				plat/common/plat_gicv3.c				\
				${GICV3_SOURCES}					\
				plat/common/plat_psci_common.c				\
				$(PLAT_PATH)/common/src/pm_ps_hold.c			\
				$(PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(PLAT_PATH)/common/src/qti_gic_v3.c			\
				$(PLAT_PATH)/common/src/qti_gicv3_platform.c		\
				$(PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(PLAT_PATH)/common/src/qti_interrupt_svc_el3.c		\
				$(PLAT_PATH)/common/src/qti_pm.c			\
				$(PLAT_PATH)/common/src/qti_syscall.c			\
				$(PLAT_PATH)/common/src/spmi_arb.c			\
				$(PLAT_PATH)/bruin/common/bruin_bl31_setup.c		\
				$(PLAT_PATH)/bruin/common/bruin_topology.c		\
				$(PLAT_PATH)/bruin/common/bruin_pm.c			\
				$(PLAT_PATH)/bruin/${CHIPSET}/${CHIPSET}_pm.c

include drivers/qti/smem/smem.mk
include drivers/qti/chipinfo/chipinfo.mk
