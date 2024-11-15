#
# Copyright (c) 2023-2024, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Extra partitions used to find FIP, contains:
# metadata (2) and fsbl-m (2) and the FIP partitions (default is 2).
STM32_EXTRA_PARTS		:=	6

include plat/st/common/common.mk

CRASH_REPORTING			:=	1
ENABLE_PIE			:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1
BL2_IN_XIP_MEM			:=	1

STM32MP_BL33_EL1		?=	1
ifeq ($(STM32MP_BL33_EL1),1)
INIT_UNUSED_NS_EL2		:=	1
endif

# Disable features unsupported in ARMv8.0
ENABLE_SPE_FOR_NS		:=	0
ENABLE_SVE_FOR_NS		:=	0

# Default Device tree
DTB_FILE_NAME			?=	stm32mp257f-ev1.dtb

STM32MP25			:=	1

# STM32 image header version v2.2
STM32_HEADER_VERSION_MAJOR	:=	2
STM32_HEADER_VERSION_MINOR	:=	2

# Set load address for serial boot devices
DWL_BUFFER_BASE 		?=	0x87000000

# DDR types
STM32MP_DDR3_TYPE		?=	0
STM32MP_DDR4_TYPE		?=	0
STM32MP_LPDDR4_TYPE		?=	0
ifeq (${STM32MP_DDR3_TYPE},1)
DDR_TYPE			:=	ddr3
endif
ifeq (${STM32MP_DDR4_TYPE},1)
DDR_TYPE			:=	ddr4
endif
ifeq (${STM32MP_LPDDR4_TYPE},1)
DDR_TYPE			:=	lpddr4
endif

# DDR features
STM32MP_DDR_DUAL_AXI_PORT	:=	1
STM32MP_DDR_FIP_IO_STORAGE	:=	1

# Device tree
BL2_DTSI			:=	stm32mp25-bl2.dtsi
FDT_SOURCES			:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl2.dts,$(DTB_FILE_NAME)))
BL31_DTSI			:=	stm32mp25-bl31.dtsi
FDT_SOURCES			+=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl31.dts,$(DTB_FILE_NAME)))

# Macros and rules to build TF binary
STM32_TF_STM32			:=	$(addprefix ${BUILD_PLAT}/tf-a-, $(patsubst %.dtb,%.stm32,$(DTB_FILE_NAME)))
STM32_LD_FILE			:=	plat/st/stm32mp2/${ARCH}/stm32mp2.ld.S
STM32_BINARY_MAPPING		:=	plat/st/stm32mp2/${ARCH}/stm32mp2.S

STM32MP_FW_CONFIG_NAME		:=	$(patsubst %.dtb,%-fw-config.dtb,$(DTB_FILE_NAME))
STM32MP_FW_CONFIG		:=	${BUILD_PLAT}/fdts/$(STM32MP_FW_CONFIG_NAME)
STM32MP_SOC_FW_CONFIG		:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl31.dtb,$(DTB_FILE_NAME)))
ifeq (${STM32MP_DDR_FIP_IO_STORAGE},1)
STM32MP_DDR_FW_PATH		?=	drivers/st/ddr/phy/firmware/bin/stm32mp2
STM32MP_DDR_FW_NAME		:=	${DDR_TYPE}_pmu_train.bin
STM32MP_DDR_FW			:=	${STM32MP_DDR_FW_PATH}/${STM32MP_DDR_FW_NAME}
endif
FDT_SOURCES			+=	$(addprefix fdts/, $(patsubst %.dtb,%.dts,$(STM32MP_FW_CONFIG_NAME)))

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${STM32MP_FW_CONFIG},--fw-config))

# Add the SOC_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_IMG_PAYLOAD,STM32MP_SOC_FW_CONFIG,$(STM32MP_SOC_FW_CONFIG),--soc-fw-config,$(patsubst %.dtb,%.dts,$(STM32MP_SOC_FW_CONFIG))))

ifeq (${STM32MP_DDR_FIP_IO_STORAGE},1)
# Add the FW_DDR to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_IMG,STM32MP_DDR_FW,--ddr-fw))
endif

# Enable flags for C files
$(eval $(call assert_booleans,\
	$(sort \
		STM32MP_DDR_DUAL_AXI_PORT \
		STM32MP_DDR_FIP_IO_STORAGE \
		STM32MP_DDR3_TYPE \
		STM32MP_DDR4_TYPE \
		STM32MP_LPDDR4_TYPE \
		STM32MP25 \
		STM32MP_BL33_EL1 \
)))

$(eval $(call assert_numerics,\
	$(sort \
		PLAT_PARTITION_MAX_ENTRIES \
		STM32_HEADER_VERSION_MAJOR \
		STM32_TF_A_COPIES \
)))

$(eval $(call add_defines,\
	$(sort \
		DWL_BUFFER_BASE \
		PLAT_DEF_FIP_UUID \
		PLAT_PARTITION_MAX_ENTRIES \
		PLAT_TBBR_IMG_DEF \
		STM32_TF_A_COPIES \
		STM32MP_DDR_DUAL_AXI_PORT \
		STM32MP_DDR_FIP_IO_STORAGE \
		STM32MP_DDR3_TYPE \
		STM32MP_DDR4_TYPE \
		STM32MP_LPDDR4_TYPE \
		STM32MP25 \
		STM32MP_BL33_EL1 \
)))

# STM32MP2x is based on Cortex-A35, which is Armv8.0, and does not support BTI
# Disable mbranch-protection to avoid adding useless code
TF_CFLAGS			+=	-mbranch-protection=none

# Include paths and source files
PLAT_INCLUDES			+=	-Iplat/st/stm32mp2/include/
PLAT_INCLUDES			+=	-Idrivers/st/ddr/phy/phyinit/include/
PLAT_INCLUDES			+=	-Idrivers/st/ddr/phy/firmware/include/

PLAT_BL_COMMON_SOURCES		+=	lib/cpus/${ARCH}/cortex_a35.S
PLAT_BL_COMMON_SOURCES		+=	drivers/st/uart/${ARCH}/stm32_console.S
PLAT_BL_COMMON_SOURCES		+=	plat/st/stm32mp2/${ARCH}/stm32mp2_helper.S

PLAT_BL_COMMON_SOURCES		+=	drivers/st/pmic/stm32mp_pmic2.c				\
					drivers/st/pmic/stpmic2.c				\

PLAT_BL_COMMON_SOURCES		+=	drivers/st/i2c/stm32_i2c.c

PLAT_BL_COMMON_SOURCES		+=	plat/st/stm32mp2/stm32mp2_private.c

PLAT_BL_COMMON_SOURCES		+=	drivers/st/bsec/bsec3.c					\
					drivers/st/reset/stm32mp2_reset.c			\
					plat/st/stm32mp2/stm32mp2_syscfg.c

PLAT_BL_COMMON_SOURCES		+=	drivers/st/clk/clk-stm32-core.c				\
					drivers/st/clk/clk-stm32mp2.c

BL2_SOURCES			+=	plat/st/stm32mp2/plat_bl2_mem_params_desc.c

BL2_SOURCES			+=	plat/st/stm32mp2/bl2_plat_setup.c			\
					plat/st/stm32mp2/plat_ddr.c

ifneq ($(filter 1,${STM32MP_EMMC} ${STM32MP_SDMMC}),)
BL2_SOURCES			+=	drivers/st/mmc/stm32_sdmmc2.c
endif

ifeq (${STM32MP_USB_PROGRAMMER},1)
BL2_SOURCES			+=	plat/st/stm32mp2/stm32mp2_usb_dfu.c
endif

BL2_SOURCES			+=	drivers/st/ddr/stm32mp2_ddr.c				\
					drivers/st/ddr/stm32mp2_ddr_helpers.c			\
					drivers/st/ddr/stm32mp2_ram.c

BL2_SOURCES			+=	drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_c_initphyconfig.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_calcmb.c					\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_i_loadpieimage.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_initstruct.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_isdbytedisabled.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_loadpieprodcode.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_mapdrvstren.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_progcsrskiptrain.c			\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_reginterface.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_restore_sequence.c			\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_sequence.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_softsetmb.c				\
					drivers/st/ddr/phy/phyinit/usercustom/ddrphy_phyinit_usercustom_custompretrain.c	\
					drivers/st/ddr/phy/phyinit/usercustom/ddrphy_phyinit_usercustom_saveretregs.c

BL2_SOURCES			+=	drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_d_loadimem.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_f_loaddmem.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_g_execfw.c				\
					drivers/st/ddr/phy/phyinit/src/ddrphy_phyinit_writeoutmem.c				\
					drivers/st/ddr/phy/phyinit/usercustom/ddrphy_phyinit_usercustom_g_waitfwdone.c

# BL31 sources
BL31_SOURCES			+=	${FDT_WRAPPERS_SOURCES}

BL31_SOURCES			+=	plat/st/stm32mp2/bl31_plat_setup.c			\
					plat/st/stm32mp2/stm32mp2_pm.c				\
					plat/st/stm32mp2/stm32mp2_topology.c
# Generic GIC v2
include drivers/arm/gic/v2/gicv2.mk

BL31_SOURCES			+=	${GICV2_SOURCES}					\
					plat/common/plat_gicv2.c				\
					plat/st/common/stm32mp_gic.c

# Generic PSCI
BL31_SOURCES			+=	plat/common/plat_psci_common.c

# Compilation rules
.PHONY: check_ddr_type
.SUFFIXES:

bl2: check_ddr_type

check_ddr_type:
	$(eval DDR_TYPE = $(shell echo $$(($(STM32MP_DDR3_TYPE) + \
					   $(STM32MP_DDR4_TYPE) + \
					   $(STM32MP_LPDDR4_TYPE)))))
	@if [ ${DDR_TYPE} != 1 ]; then \
		echo "One and only one DDR type must be defined"; \
		false; \
	fi

# Create DTB file for BL31
${BUILD_PLAT}/fdts/%-bl31.dts: fdts/%.dts fdts/${BL31_DTSI} | $$(@D)/
	@echo '#include "$(patsubst fdts/%,%,$<)"' > $@
	@echo '#include "${BL31_DTSI}"' >> $@

include plat/st/common/common_rules.mk
