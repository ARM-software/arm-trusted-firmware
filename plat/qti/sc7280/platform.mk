#
# Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
# Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Make for SC7280 QTI platform.

QTI_PLAT_PATH		:=	plat/qti
CHIPSET			:=	${PLAT}

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA	:=	1
USE_COHERENT_MEM		:=	1
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT		:=	0

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID	:=  1
ARM_RECOM_STATE_ID_ENC  :=  1

COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1

RESET_TO_BL31			:=	0

MULTI_CONSOLE_API		:=	1

QTI_SDI_BUILD := 0
$(eval $(call assert_boolean,QTI_SDI_BUILD))
$(eval $(call add_define,QTI_SDI_BUILD))

#disable CTX_INCLUDE_AARCH32_REGS to support sc7280 gold cores
override CTX_INCLUDE_AARCH32_REGS	:=	0
WORKAROUND_CVE_2017_5715		:=      0
DYNAMIC_WORKAROUND_CVE_2018_3639	:=      1
# Enable stack protector.
ENABLE_STACK_PROTECTOR := strong


QTI_EXTERNAL_INCLUDES	:=	-I${QTI_PLAT_PATH}/${CHIPSET}/inc			\
				-I${QTI_PLAT_PATH}/common/inc				\
				-I${QTI_PLAT_PATH}/common/inc/$(ARCH)			\
				-I${QTI_PLAT_PATH}/qtiseclib/inc			\
				-I${QTI_PLAT_PATH}/qtiseclib/inc/${CHIPSET}			\

QTI_BL31_SOURCES	:=	$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_helpers.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_kryo6_silver.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_kryo6_gold.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(QTI_PLAT_PATH)/common/src/pm_ps_hold.c			\
				$(QTI_PLAT_PATH)/common/src/qti_stack_protector.c	\
				$(QTI_PLAT_PATH)/common/src/qti_common.c		\
				$(QTI_PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(QTI_PLAT_PATH)/common/src/qti_gic_v3.c		\
				$(QTI_PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(QTI_PLAT_PATH)/common/src/qti_syscall.c		\
				$(QTI_PLAT_PATH)/common/src/qti_topology.c		\
				$(QTI_PLAT_PATH)/common/src/qti_pm.c			\
				$(QTI_PLAT_PATH)/common/src/qti_rng.c			\
				$(QTI_PLAT_PATH)/common/src/spmi_arb.c			\
				$(QTI_PLAT_PATH)/qtiseclib/src/qtiseclib_cb_interface.c	\


PLAT_INCLUDES		:=	-Iinclude/plat/common/					\

PLAT_INCLUDES		+=	${QTI_EXTERNAL_INCLUDES}

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}					\
				plat/common/aarch64/crash_console_helpers.S    \
				common/desc_image_load.c			\
				lib/bl_aux_params/bl_aux_params.c		\

include lib/coreboot/coreboot.mk

#PSCI Sources.
PSCI_SOURCES		:=	plat/common/plat_psci_common.c				\

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1
# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

#Timer sources
TIMER_SOURCES		:=	drivers/delay_timer/generic_delay_timer.c	\
				drivers/delay_timer/delay_timer.c		\

#GIC sources.
GIC_SOURCES		:=	plat/common/plat_gicv3.c			\
				${GICV3_SOURCES}				\

BL31_SOURCES		+=	${QTI_BL31_SOURCES}					\
				${PSCI_SOURCES}						\
				${GIC_SOURCES}						\
				${TIMER_SOURCES}					\

LIB_QTI_PATH	:=	${QTI_PLAT_PATH}/qtiseclib/lib/${CHIPSET}


# Override this on the command line to point to the qtiseclib library which
# will be available in coreboot.org
QTISECLIB_PATH ?=

ifeq ($(QTISECLIB_PATH),)
# if No lib then use stub implementation for qtiseclib interface
$(warning QTISECLIB_PATH is not provided while building, using stub implementation. \
		Please refer docs/plat/qti.rst for more details \
		THIS FIRMWARE WILL NOT BOOT!)
BL31_SOURCES	+=	plat/qti/qtiseclib/src/qtiseclib_interface_stub.c
else
# use library provided by QTISECLIB_PATH
LDFLAGS += -L $(dir $(QTISECLIB_PATH))
LDLIBS += -l$(patsubst lib%.a,%,$(notdir $(QTISECLIB_PATH)))
endif

