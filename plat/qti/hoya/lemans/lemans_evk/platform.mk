#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Makefile for Lemans based QCS9075/EVK QTI platform.

PLAT_PATH				:=	plat/qti
CHIPSET					:=	lemans

RESET_TO_BL2				:=	1

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA		:=	1
USE_COHERENT_MEM			:=	0
WARMBOOT_ENABLE_DCACHE_EARLY		:=	1
HW_ASSISTED_COHERENCY			:=	1

#Enable errata configs for cortex_a78c
ERRATA_A78C_2242638			:=	1
ERRATA_A78C_2376749			:=	1
ERRATA_A78C_2395411			:=	1
ERRATA_A78C_2683027			:=	1
ERRATA_A78C_2712575			:=	1
ERRATA_A78C_2743232			:=	1
ERRATA_A78C_2772121			:=	1
ERRATA_A78C_2779484			:=	1
WORKAROUND_CVE_2025_10263		:=	1

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

#disable CTX_INCLUDE_AARCH32_REGS to support lemans gold cores
override CTX_INCLUDE_AARCH32_REGS	:=	0
WORKAROUND_CVE_2017_5715		:=      0
DYNAMIC_WORKAROUND_CVE_2018_3639	:=      1
# Enable stack protector.
ENABLE_STACK_PROTECTOR := strong

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\
				-I${PLAT_PATH}/hoya/${CHIPSET}/inc				\
				-I${PLAT_PATH}/hoya/${CHIPSET}/${PLAT}/inc			\
				-I${PLAT_PATH}/common/inc				\
				-I${PLAT_PATH}/common/inc/$(ARCH)			\
				-I${PLAT_PATH}/hoya/qtiseclib/inc				\
				-I${PLAT_PATH}/hoya/qtiseclib/inc/${CHIPSET}

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c				\
				drivers/qti/crypto/rng.c				\
				lib/cpus/aarch64/cortex_a78c.S				\
				lib/bl_aux_params/bl_aux_params.c			\
				plat/common/aarch64/crash_console_helpers.S		\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_PATH)/common/src/qti_stack_protector.c		\
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
BL31_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c		\
				drivers/delay_timer/delay_timer.c			\
				plat/common/plat_gicv3.c				\
				${GICV3_SOURCES}					\
				plat/common/plat_psci_common.c				\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_helpers.S		\
				$(PLAT_PATH)/common/src/pm_ps_hold.c			\
				$(PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(PLAT_PATH)/common/src/qti_gic_v3.c			\
				$(PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(PLAT_PATH)/common/src/qti_syscall.c			\
				$(PLAT_PATH)/common/src/qti_topology.c			\
				$(PLAT_PATH)/common/src/qti_pm.c			\
				$(PLAT_PATH)/common/src/spmi_arb.c			\
				$(PLAT_PATH)/hoya/qtiseclib/src/qtiseclib_cb_interface.c

BL31_SOURCES	+=		drivers/qti/sec_core/sec_core_stub.c \
				drivers/qti/accesscontrol/access_control_stub.c

include drivers/qti/smem/smem.mk

# Override this on the command line to point to the qtiseclib library
QTISECLIB_PATH ?=

ifeq ($(QTISECLIB_PATH),)
# if No lib then use stub implementation for qtiseclib interface
$(warning QTISECLIB_PATH is not provided while building, using stub implementation. \
		Please refer to documentation for more details \
		THIS FIRMWARE WILL NOT BOOT!)

include drivers/qti/smmu/smmu.mk
include drivers/qti/pdc/pdc.mk

PLAT_INCLUDES   +=      -Iinclude/drivers/qti/qtimer/${CHIPSET} \
			-Iinclude/drivers/qti/watchdog/${CHIPSET}

BL31_SOURCES	+=	plat/qti/hoya/qtiseclib/src/qtiseclib_interface_stub.c \
			drivers/qti/qtimer/qtimer.c \
			drivers/qti/watchdog/watchdog.c

else
$(eval $(call add_define,QTISECLIB_PATH))
# use library provided by QTISECLIB_PATH
BL31_SOURCES	+=	drivers/qti/qtimer/qtimer_stub.c \
			drivers/qti/watchdog/watchdog_stub.c

LDFLAGS += -L $(dir $(QTISECLIB_PATH))
LDLIBS += -l$(patsubst lib%.a,%,$(notdir $(QTISECLIB_PATH)))
endif
