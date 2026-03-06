#
# Copyright (c) 2026, Texas Instruments Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_PATH	:=	plat/ti/k3low
TARGET_BOARD	?=	am62lx

# modify BUILD_PLAT to point to board specific build directory
BUILD_PLAT := $(abspath ${BUILD_BASE})/${PLAT}/${TARGET_BOARD}/${BUILD_TYPE}

include plat/ti/common/plat_common.mk
include ${PLAT_PATH}/board/${TARGET_BOARD}/board.mk

USE_COHERENT_MEM	:=	0

PSCI_OS_INIT_MODE	:=  1

ifeq (${IMAGE_BL1}, 1)
override ENABLE_PIE := 0
endif

# We need to enable this for robust clocking
CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION	:=	1
$(eval $(call add_define,CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION))

# Add support for platform supplied linker script for BL31 build
PLAT_EXTRA_LD_SCRIPT	:=	1

K3_SPL_IMG_OFFSET       :=      0x80000
$(eval $(call add_define,K3_SPL_IMG_OFFSET))

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
	./${PLAT_PATH}/common/am62l-bl1-dtb.sh ${BUILD_PLAT}/bl1/bl1.elf ${BUILD_PLAT}/fdts/$(DTB_FILE_NAME) ${BUILD_PLAT}/bl1.bin

all: am62l_bl1
endef

$(eval $(call add_tfcflag))
$(eval $(call add_asflag))
$(eval $(call add_dtb))

FDT_SOURCES	:= fdts/$(patsubst %.dtb,%.dts,$(DTB_FILE_NAME)) \

PLAT_INCLUDES		+=	\
				-I${PLAT_PATH}/common/drivers/k3-ddrss	\
				-I${PLAT_PATH}/common/drivers/k3-ddrss/common	\
				-I${PLAT_PATH}/common/drivers/k3-ddrss/16bit	\
				-I${PLAT_PATH}/common/drivers/lpm		\
				-Ilib/libfdt	\

K3_LPDDR4_SOURCES	+= 	\
				${PLAT_PATH}/common/drivers/k3-ddrss/am62l-ddrss.c \
				${PLAT_PATH}/common/drivers/k3-ddrss/lpddr4_obj_if.c \
				${PLAT_PATH}/common/drivers/k3-ddrss/lpddr4.c \
				${PLAT_PATH}/common/drivers/k3-ddrss/lpddr4_16bit_ctl_regs_rw_masks.c \
				${PLAT_PATH}/common/drivers/k3-ddrss/lpddr4_16bit.c \
				${PLAT_PATH}/common/am62l-ddr-dtb.S \


BL1_SOURCES		+=	\
				${PLAT_PATH}/common/am62l_bl1_setup.c	\
				plat/ti/common/k3_helpers.S	\
				${PLAT_PATH}/common/am62l_topology.c	\
				drivers/io/io_storage.c \
				${K3_LPDDR4_SOURCES}			\
				${K3_TI_SCI_SOURCES}		\


BL32_BASE ?= 0x80200000
$(eval $(call add_define,BL32_BASE))

PRELOADED_BL33_BASE ?= 0x82000000
$(eval $(call add_define,PRELOADED_BL33_BASE))

TIFS_LPM_SAVE_CTX ?= 0x81A00000
$(eval $(call add_define,TIFS_LPM_SAVE_CTX))

K3_HW_CONFIG_BASE ?= 0x88000000
$(eval $(call add_define,K3_HW_CONFIG_BASE))
PLAT_INCLUDES +=	\
			-I${PLAT_PATH}/board/${TARGET_BOARD}/include	\
			-I${PLAT_PATH}					\
			-I${PLAT_PATH}/board/${TARGET_BOARD}/pm		\
			-I${PLAT_PATH}/board/${TARGET_BOARD}/scmi	\
			-Idrivers/scmi-msg				\
			-Iplat/ti/common/include			\
			-Iplat/ti/common/scmi				\
			-I${PLAT_PATH}/common/drivers/firewall		\

K3_PSCI_SOURCES		+=	\
				${PLAT_PATH}/common/am62l_psci.c	\

K3_TI_SCI_TRANSPORT	:=	\
				drivers/ti/ipc/mailbox.c		\

K3_LPM_SOURCES		:=	\
				${PLAT_PATH}/common/drivers/lpm/call_sram.S	\
				${PLAT_PATH}/common/drivers/lpm/ddr.c		\
				${PLAT_PATH}/common/drivers/lpm/gtc.c		\
				${PLAT_PATH}/common/drivers/lpm/k3_lpm_timeout.c	\
				${PLAT_PATH}/common/drivers/lpm/lpm_stub.c		\
				${PLAT_PATH}/common/drivers/lpm/lpm_trace.c	\
				${PLAT_PATH}/common/drivers/lpm/pll_16fft_raw.c	\
				${PLAT_PATH}/common/drivers/lpm/psc_raw.c		\
				${PLAT_PATH}/common/drivers/lpm/rtc.c

BL31_SOURCES		+=	\
				drivers/clk/clk.c				\
				${K3_PSCI_SOURCES}				\
				${K3_TI_SCI_TRANSPORT}				\
				${K3_LPM_SOURCES}				\
				${PLAT_PATH}/common/am62l_bl31_setup.c		\
				${PLAT_PATH}/common/am62l_topology.c		\
				${PLAT_PATH}/common/drivers/firewall/firewall_config.c	\

BL1_SOURCES		+=	\
				${K3_TI_SCI_TRANSPORT}				\
