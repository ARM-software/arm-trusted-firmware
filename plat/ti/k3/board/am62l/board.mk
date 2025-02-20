#
# Copyright (c) 2024, Texas Instruments Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# We dont have system level coherency capability
USE_COHERENT_MEM	:=	0
K3_TI_SCI_MAILBOX	:=	1
$(eval $(call add_define,K3_TI_SCI_MAILBOX))
DDR_16BIT	:=	1
$(eval $(call add_define,DDR_16BIT))
TI_AM62L_LPM_TRACE	:=	1
$(eval $(call add_define,TI_AM62L_LPM_TRACE))

# Add support for platform supplied linker script for BL31 build
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

K3_SPL_IMG_OFFSET	:=	0x80000
$(eval $(call add_define,K3_SPL_IMG_OFFSET))

ifeq (${IMAGE_BL1}, 1)
override ENABLE_PIE := 0
endif

DTB_FILE_NAME ?= k3-am62l-ddr.dtb

$(eval $(call MAKE_LIB_DIRS))

include lib/libfdt/libfdt.mk

define add_tfcflag
	TF_CFLAGS_aarch64	+= -Wno-address-of-packed-member
endef

define add_asflag
	ASFLAGS 		+= -DBL1_DTB_PATH=\"${BUILD_PLAT}/fdts/$(DTB_FILE_NAME)\"
	ASFLAGS 		+= -DDTB_ARRAY_SIZE=9400
endef

define add_dtb
am62l_bl1: bl1 dtbs
	./${PLAT_PATH}/common/drivers/lpddr4/am62l-bl1-dtb.sh ${BUILD_PLAT}/bl1/bl1.elf ${BUILD_PLAT}/fdts/$(DTB_FILE_NAME) ${BUILD_PLAT}/bl1.bin

all: am62l_bl1
endef

$(eval $(call add_tfcflag))
$(eval $(call add_asflag))
$(eval $(call add_dtb))

FDT_SOURCES	:= fdts/$(patsubst %.dtb,%.dts,$(DTB_FILE_NAME)) \

PLAT_INCLUDES		+=	\
				-I${PLAT_PATH}/common/drivers/lpddr4	\
				-I${PLAT_PATH}/common/drivers/lpddr4/common	\
				-I${PLAT_PATH}/common/drivers/lpddr4/16bit	\
				-Iinclude/lib/libfdt	\

K3_LPDDR4_SOURCES	+= 	\
				${PLAT_PATH}/common/drivers/lpddr4/k3-ddrss.c \
				${PLAT_PATH}/common/drivers/lpddr4/lpddr4_obj_if.c \
				${PLAT_PATH}/common/drivers/lpddr4/lpddr4.c \
				${PLAT_PATH}/common/drivers/lpddr4/lpddr4_16bit_ctl_regs_rw_masks.c \
				${PLAT_PATH}/common/drivers/lpddr4/lpddr4_16bit.c \
				${PLAT_PATH}/common/drivers/lpddr4/k3-ddr-dtb.S \


BL1_SOURCES		+=	\
				${PLAT_PATH}/common/k3_bl1_setup.c	\
				${PLAT_PATH}/common/k3_helpers.S	\
				${PLAT_PATH}/common/k3_topology.c	\
				drivers/io/io_storage.c \
				${K3_LPDDR4_SOURCES}			\
				${K3_TI_SCI_TRANSPORT}	\


K3_TI_SCI_TRANSPORT    =      ${PLAT_PATH}/common/drivers/mailbox/mailbox.c
K3_PSCI_SOURCES		+=	\
				${PLAT_PATH}/common/am62l_psci.c	\

BL32_BASE ?= 0x80200000
$(eval $(call add_define,BL32_BASE))

PRELOADED_BL33_BASE ?= 0x82000000
$(eval $(call add_define,PRELOADED_BL33_BASE))

K3_HW_CONFIG_BASE ?= 0x88000000
$(eval $(call add_define,K3_HW_CONFIG_BASE))

include ${PLAT_PATH}/common/drivers/scmi/ti_scmi.mk
include ${PLAT_PATH}/common/drivers/pm/pm.mk
include ${PLAT_PATH}/board/am62l/pm/soc_pm.mk
include ${PLAT_PATH}/board/am62l/lpm/lpm.mk

PLAT_INCLUDES += -Iplat/ti/k3/board/${TARGET_BOARD}/pm			\
		 -I${PLAT_PATH}/board/am62l/scmi			\

BL31_SOURCES		+=	\
				${PLAT_PATH}/common/k3_svc.c		\
				drivers/scmi-msg/base.c			\
				drivers/scmi-msg/entry.c			\
				drivers/scmi-msg/smt.c			\
				drivers/scmi-msg/clock.c			\
				drivers/scmi-msg/power_domain.c		\
