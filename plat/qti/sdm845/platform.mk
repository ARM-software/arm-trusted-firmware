#####################################################################
# Copyright (c) 2018 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
######################################################################

#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Make for sdm845 QTI platform.

QTI_PLAT_PATH		:=	plat/qti
CHIPSET			:=	${PLAT}
SOC			:=	$(patsubst sdm%,%,${PLAT})

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA	:=	1
USE_COHERENT_MEM		:=	1
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT		:=	0

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID	:=	1
ARM_RECOM_STATE_ID_ENC  :=  1

COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1

RESET_TO_BL31			:=	0

MULTI_CONSOLE_API		:=	1

# Enable stack protector.
ENABLE_STACK_PROTECTOR := strong


QTI_EXTERNAL_INCLUDES	:=	-I${QTI_PLAT_PATH}/${CHIPSET}/inc			\
				-I${QTI_PLAT_PATH}/common/inc				\
				-I${QTI_PLAT_PATH}/common/inc/$(ARCH)			\
				-I${QTI_PLAT_PATH}/qtiseclib/inc			\

QTI_BL31_SOURCES	:=	$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_helpers.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_kryo3_silver.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_kryo3_gold.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(QTI_PLAT_PATH)/common/src/qti_stack_protector.c	\
				$(QTI_PLAT_PATH)/common/src/qti_common.c		\
				$(QTI_PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(QTI_PLAT_PATH)/common/src/qti_gic_v3.c		\
				$(QTI_PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(QTI_PLAT_PATH)/common/src/qti_syscall.c		\
				$(QTI_PLAT_PATH)/common/src/qti_topology.c		\
				$(QTI_PLAT_PATH)/common/src/qti_pm.c			\
				$(QTI_PLAT_PATH)/qtiseclib/src/qtiseclib_cb_interface.c	\


PLAT_INCLUDES		:=	-Idrivers/arm/gic/common/				\
				-Idrivers/arm/gic/v3/					\
				-Iinclude/plat/common/					\

PLAT_INCLUDES		+=	${QTI_EXTERNAL_INCLUDES}

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

include lib/coreboot/coreboot.mk

#PSCI Sources.
PSCI_SOURCES		:=	plat/common/plat_psci_common.c				\

# GIC sources.
GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c			\
				drivers/arm/gic/v3/arm_gicv3_common.c			\
				drivers/arm/gic/v3/gic600.c				\
				drivers/arm/gic/v3/gicv3_main.c				\
				drivers/arm/gic/v3/gicv3_helpers.c			\
				plat/common/plat_gicv3.c				\


CONSOLE_SOURCES		:=	drivers/console/aarch64/console.S			\


BL31_SOURCES		+=	${QTI_BL31_SOURCES}					\
				${PSCI_SOURCES}						\
				${GIC_SOURCES}						\
				${CONSOLE_SOURCES}					\


LIB_QTI_PATH	:=	${QTI_PLAT_PATH}/qtiseclib/lib/${CHIPSET}

# By default libqtisec_dbg.a used by debug variant. When this library doesn't exist,
# debug variant will use release version (libqtisec.a) of the library.
QTISECLIB = qtisec
ifneq (${DEBUG}, 0)
ifneq ("$(wildcard $(LIB_QTI_PATH)/libqtisec_dbg.a)","")
QTISECLIB = qtisec_dbg
else
$(warning Release version of qtisec library used in Debug build!!..)
endif
endif

LDFLAGS += -L ${LIB_QTI_PATH}

LDLIBS += -l$(QTISECLIB)

