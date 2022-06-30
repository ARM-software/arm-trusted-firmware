#
# Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/st/common/common.mk

ARM_CORTEX_A7		:=	yes
ARM_WITH_NEON		:=	yes
USE_COHERENT_MEM	:=	0

# Default Device tree
DTB_FILE_NAME		?=	stm32mp157c-ev1.dtb

STM32MP13		?=	0
STM32MP15		?=	0

ifeq ($(STM32MP13),1)
ifeq ($(STM32MP15),1)
$(error Cannot enable both flags STM32MP13 and STM32MP15)
endif
STM32MP13		:=	1
STM32MP15		:=	0
else ifeq ($(STM32MP15),1)
STM32MP13		:=	0
STM32MP15		:=	1
else ifneq ($(findstring stm32mp13,$(DTB_FILE_NAME)),)
STM32MP13		:=	1
STM32MP15		:=	0
else ifneq ($(findstring stm32mp15,$(DTB_FILE_NAME)),)
STM32MP13		:=	0
STM32MP15		:=	1
endif

ifeq ($(STM32MP13),1)
# Will use SRAM2 as mbedtls heap
STM32MP_USE_EXTERNAL_HEAP :=	1

# DDR controller with single AXI port and 16-bit interface
STM32MP_DDR_DUAL_AXI_PORT:=	0
STM32MP_DDR_32BIT_INTERFACE:=	0

ifeq (${TRUSTED_BOARD_BOOT},1)
# PKA algo to include
PKA_USE_NIST_P256	:=	1
PKA_USE_BRAINPOOL_P256T1:=	1
endif

# STM32 image header version v2.0
STM32_HEADER_VERSION_MAJOR:=	2
STM32_HEADER_VERSION_MINOR:=	0
endif

ifeq ($(STM32MP15),1)
# DDR controller with dual AXI port and 32-bit interface
STM32MP_DDR_DUAL_AXI_PORT:=	1
STM32MP_DDR_32BIT_INTERFACE:=	1

# STM32 image header version v1.0
STM32_HEADER_VERSION_MAJOR:=	1
STM32_HEADER_VERSION_MINOR:=	0

# Add OP-TEE reserved shared memory area in mapping
STM32MP15_OPTEE_RSV_SHM	:=	0
$(eval $(call add_defines,STM32MP15_OPTEE_RSV_SHM))

STM32MP_CRYPTO_ROM_LIB :=	1

# Decryption support
ifneq ($(DECRYPTION_SUPPORT),none)
$(error "DECRYPTION_SUPPORT not supported on STM32MP15")
endif
endif

PKA_USE_NIST_P256	?=	0
PKA_USE_BRAINPOOL_P256T1 ?=	0

ifeq ($(AARCH32_SP),sp_min)
# Disable Neon support: sp_min runtime may conflict with non-secure world
TF_CFLAGS		+=	-mfloat-abi=soft
endif

# Not needed for Cortex-A7
WORKAROUND_CVE_2017_5715:=	0
WORKAROUND_CVE_2022_23960:=	0

# Number of TF-A copies in the device
STM32_TF_A_COPIES		:=	2

# PLAT_PARTITION_MAX_ENTRIES must take care of STM32_TF-A_COPIES and other partitions
# such as metadata (2) to find all the FIP partitions (default is 2).
PLAT_PARTITION_MAX_ENTRIES	:=	$(shell echo $$(($(STM32_TF_A_COPIES) + 4)))

ifeq (${PSA_FWU_SUPPORT},1)
# Number of banks of updatable firmware
NR_OF_FW_BANKS			:=	2
NR_OF_IMAGES_IN_FW_BANK		:=	1

FWU_MAX_PART = $(shell echo $$(($(STM32_TF_A_COPIES) + 2 + $(NR_OF_FW_BANKS))))
ifeq ($(shell test $(FWU_MAX_PART) -gt $(PLAT_PARTITION_MAX_ENTRIES); echo $$?),0)
$(error "Required partition number is $(FWU_MAX_PART) where PLAT_PARTITION_MAX_ENTRIES is only \
$(PLAT_PARTITION_MAX_ENTRIES)")
endif
endif

ifeq ($(STM32MP13),1)
STM32_HASH_VER		:=	4
STM32_RNG_VER		:=	4
else # Assuming STM32MP15
STM32_HASH_VER		:=	2
STM32_RNG_VER		:=	2
endif

# Download load address for serial boot devices
DWL_BUFFER_BASE 	?=	0xC7000000

# Device tree
ifeq ($(STM32MP13),1)
BL2_DTSI		:=	stm32mp13-bl2.dtsi
FDT_SOURCES		:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl2.dts,$(DTB_FILE_NAME)))
else
BL2_DTSI		:=	stm32mp15-bl2.dtsi
FDT_SOURCES		:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl2.dts,$(DTB_FILE_NAME)))
ifeq ($(AARCH32_SP),sp_min)
BL32_DTSI		:=	stm32mp15-bl32.dtsi
FDT_SOURCES		+=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl32.dts,$(DTB_FILE_NAME)))
endif
endif

# Macros and rules to build TF binary
STM32_TF_STM32		:=	$(addprefix ${BUILD_PLAT}/tf-a-, $(patsubst %.dtb,%.stm32,$(DTB_FILE_NAME)))
STM32_LD_FILE		:=	plat/st/stm32mp1/stm32mp1.ld.S
STM32_BINARY_MAPPING	:=	plat/st/stm32mp1/stm32mp1.S

ifeq ($(AARCH32_SP),sp_min)
# BL32 is built only if using SP_MIN
BL32_DEP		:= bl32
ASFLAGS			+= -DBL32_BIN_PATH=\"${BUILD_PLAT}/bl32.bin\"
endif

STM32MP_FW_CONFIG_NAME	:=	$(patsubst %.dtb,%-fw-config.dtb,$(DTB_FILE_NAME))
STM32MP_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(STM32MP_FW_CONFIG_NAME)
ifneq (${AARCH32_SP},none)
FDT_SOURCES		+=	$(addprefix fdts/, $(patsubst %.dtb,%.dts,$(STM32MP_FW_CONFIG_NAME)))
endif
# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${STM32MP_FW_CONFIG},--fw-config))
ifeq ($(GENERATE_COT),1)
STM32MP_CFG_CERT	:=	$(BUILD_PLAT)/stm32mp_cfg_cert.crt
# Add the STM32MP_CFG_CERT to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${STM32MP_CFG_CERT},--stm32mp-cfg-cert))
endif
ifeq ($(AARCH32_SP),sp_min)
STM32MP_TOS_FW_CONFIG	:= $(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl32.dtb,$(DTB_FILE_NAME)))
$(eval $(call TOOL_ADD_PAYLOAD,${STM32MP_TOS_FW_CONFIG},--tos-fw-config))
endif

# Enable flags for C files
$(eval $(call assert_booleans,\
	$(sort \
		PKA_USE_BRAINPOOL_P256T1 \
		PKA_USE_NIST_P256 \
		STM32MP_CRYPTO_ROM_LIB \
		STM32MP_DDR_32BIT_INTERFACE \
		STM32MP_DDR_DUAL_AXI_PORT \
		STM32MP_USE_EXTERNAL_HEAP \
		STM32MP13 \
		STM32MP15 \
)))

$(eval $(call assert_numerics,\
	$(sort \
		PLAT_PARTITION_MAX_ENTRIES \
		STM32_HASH_VER \
		STM32_HEADER_VERSION_MAJOR \
		STM32_RNG_VER \
		STM32_TF_A_COPIES \
)))

$(eval $(call add_defines,\
	$(sort \
		DWL_BUFFER_BASE \
		PKA_USE_BRAINPOOL_P256T1 \
		PKA_USE_NIST_P256 \
		PLAT_PARTITION_MAX_ENTRIES \
		PLAT_TBBR_IMG_DEF \
		STM32_HASH_VER \
		STM32_HEADER_VERSION_MAJOR \
		STM32_RNG_VER \
		STM32_TF_A_COPIES \
		STM32MP_CRYPTO_ROM_LIB \
		STM32MP_DDR_32BIT_INTERFACE \
		STM32MP_DDR_DUAL_AXI_PORT \
		STM32MP_USE_EXTERNAL_HEAP \
		STM32MP13 \
		STM32MP15 \
)))

# Include paths and source files
PLAT_INCLUDES		+=	-Iplat/st/stm32mp1/include/

PLAT_BL_COMMON_SOURCES	+=	plat/st/stm32mp1/stm32mp1_private.c

PLAT_BL_COMMON_SOURCES	+=	drivers/st/uart/aarch32/stm32_console.S

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/st/stm32mp1/stm32mp1_stack_protector.c
endif

PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch32/cortex_a7.S

PLAT_BL_COMMON_SOURCES	+=	drivers/arm/tzc/tzc400.c				\
				drivers/st/bsec/bsec2.c					\
				drivers/st/ddr/stm32mp1_ddr_helpers.c			\
				drivers/st/i2c/stm32_i2c.c				\
				drivers/st/iwdg/stm32_iwdg.c				\
				drivers/st/pmic/stm32mp_pmic.c				\
				drivers/st/pmic/stpmic1.c				\
				drivers/st/reset/stm32mp1_reset.c			\
				plat/st/stm32mp1/stm32mp1_dbgmcu.c			\
				plat/st/stm32mp1/stm32mp1_helper.S			\
				plat/st/stm32mp1/stm32mp1_syscfg.c

ifeq ($(STM32MP13),1)
PLAT_BL_COMMON_SOURCES	+=	drivers/st/clk/clk-stm32-core.c				\
				drivers/st/clk/clk-stm32mp13.c				\
				drivers/st/crypto/stm32_rng.c
else
PLAT_BL_COMMON_SOURCES	+=	drivers/st/clk/stm32mp1_clk.c
endif

BL2_SOURCES		+=	plat/st/stm32mp1/plat_bl2_mem_params_desc.c		\
				plat/st/stm32mp1/stm32mp1_fconf_firewall.c

ifeq (${PSA_FWU_SUPPORT},1)
include drivers/fwu/fwu.mk
endif

BL2_SOURCES		+=	drivers/st/crypto/stm32_hash.c				\
				plat/st/stm32mp1/bl2_plat_setup.c

ifeq (${TRUSTED_BOARD_BOOT},1)
ifeq ($(STM32MP13),1)
BL2_SOURCES		+=	drivers/st/crypto/stm32_pka.c
BL2_SOURCES		+=	drivers/st/crypto/stm32_saes.c
endif
endif

ifneq ($(filter 1,${STM32MP_EMMC} ${STM32MP_SDMMC}),)
BL2_SOURCES		+=	drivers/st/mmc/stm32_sdmmc2.c
endif

ifeq (${STM32MP_RAW_NAND},1)
BL2_SOURCES		+=	drivers/st/fmc/stm32_fmc2_nand.c
endif

ifneq ($(filter 1,${STM32MP_SPI_NAND} ${STM32MP_SPI_NOR}),)
BL2_SOURCES		+=	drivers/st/spi/stm32_qspi.c
endif

ifneq ($(filter 1,${STM32MP_RAW_NAND} ${STM32MP_SPI_NAND} ${STM32MP_SPI_NOR}),)
BL2_SOURCES		+=	plat/st/stm32mp1/stm32mp1_boot_device.c
endif

ifeq (${STM32MP_UART_PROGRAMMER},1)
BL2_SOURCES		+=	drivers/st/uart/stm32_uart.c
endif

ifeq (${STM32MP_USB_PROGRAMMER},1)
#The DFU stack uses only one end point, reduce the USB stack footprint
$(eval $(call add_define_val,CONFIG_USBD_EP_NB,1U))
BL2_SOURCES		+=	drivers/st/usb/stm32mp1_usb.c				\
				plat/st/stm32mp1/stm32mp1_usb_dfu.c
endif

BL2_SOURCES		+=	drivers/st/ddr/stm32mp1_ddr.c				\
				drivers/st/ddr/stm32mp1_ram.c

ifeq ($(AARCH32_SP),sp_min)
# Create DTB file for BL32
${BUILD_PLAT}/fdts/%-bl32.dts: fdts/%.dts fdts/${BL32_DTSI} | ${BUILD_PLAT} fdt_dirs
	@echo '#include "$(patsubst fdts/%,%,$<)"' > $@
	@echo '#include "${BL32_DTSI}"' >> $@

${BUILD_PLAT}/fdts/%-bl32.dtb: ${BUILD_PLAT}/fdts/%-bl32.dts
endif

include plat/st/common/common_rules.mk
