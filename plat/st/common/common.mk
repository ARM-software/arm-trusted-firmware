#
# Copyright (c) 2023-2026, STMicroelectronics - All Rights Reserved
# Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RESET_TO_BL2			:=	1

STM32MP_RECONFIGURE_CONSOLE	?=	0
STM32MP_UART_BAUDRATE		?=	115200

TRUSTED_BOARD_BOOT		?=	0
STM32MP_USE_EXTERNAL_HEAP	?=	0

# Use secure library from the ROM code for authentication
STM32MP_CRYPTO_ROM_LIB		?=	0

# Please don't increment this value without good understanding of
# the monotonic counter
STM32_TF_VERSION		?=	0

# Enable dynamic memory mapping
PLAT_XLAT_TABLES_DYNAMIC	:=	1

# STM32 image header binary type for BL2
STM32_HEADER_BL2_BINARY_TYPE	:=	0x10

TF_CFLAGS			+=	-Wsign-compare
ifeq ($(findstring clang,$(notdir $(CC))),)
# Only for GCC
TF_CFLAGS			+=	-Wformat-signedness
endif

# Number of TF-A copies in the device
STM32_TF_A_COPIES		:=	2

# PLAT_PARTITION_MAX_ENTRIES must take care of STM32_TF-A_COPIES and other partitions
PLAT_PARTITION_MAX_ENTRIES	:=	$(shell echo $$(($(STM32_TF_A_COPIES) + $(STM32_EXTRA_PARTS))))

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

# Boot devices
STM32MP_EMMC			?=	0
STM32MP_SDMMC			?=	0
STM32MP_RAW_NAND		?=	0
STM32MP_SPI_NAND		?=	0
STM32MP_SPI_NOR			?=	0

# Put both BL2 and FIP in eMMC boot partition
STM32MP_EMMC_BOOT		?=	0

# Serial boot devices
STM32MP_UART_PROGRAMMER		?=	0
STM32MP_USB_PROGRAMMER		?=	0

$(eval DTC_V = $(shell $($(ARCH)-dtc) -v | awk '{print $$NF}'))
$(eval DTC_VERSION = $(shell printf "%d" $(shell echo ${DTC_V} | cut -d- -f1 | sed "s/\./0/g" | grep -o "[0-9]*")))
DTC_CPPFLAGS			+=	${INCLUDES}
DTC_FLAGS			+=	-Wno-unit_address_vs_reg
ifeq ($(shell test $(DTC_VERSION) -ge 10601; echo $$?),0)
DTC_FLAGS			+=	-Wno-interrupt_provider
endif

# Macros and rules to build TF binary
STM32_TF_LINKERFILE		:=	${BUILD_PLAT}/${PLAT}.ld
STM32_TF_ELF_LDFLAGS		:=	$(call ld_prefix,--hash-style=gnu)
STM32_TF_ELF_LDFLAGS		+=	$(call ld_prefix,--as-needed)
STM32_TF_ELF_LDFLAGS		+=	$(call ld_prefix,--script ${STM32_TF_LINKERFILE})
STM32_TF_ELF_LDFLAGS		+=	-static

ASFLAGS				+=	-DBL2_BIN_PATH=\"${BUILD_PLAT}/bl2.bin\"

# Variables for use with stm32image
STM32IMAGEPATH			?=	tools/stm32image
STM32IMAGE			?=	${BUILD_PLAT}/${STM32IMAGEPATH}/stm32image$(.exe)
STM32IMAGE_SRC			:=	${STM32IMAGEPATH}/stm32image.c
STM32_DEPS			+=	${STM32IMAGE}

FIP_DEPS			+=	dtbs
STM32MP_HW_CONFIG		:=	${BL33_CFG}

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${STM32MP_HW_CONFIG},--hw-config))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1,,$(ENCRYPT_BL32)))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2,,$(ENCRYPT_BL32)))
endif

# Enable flags for C files
$(eval $(call assert_booleans,\
	$(sort \
		PLAT_XLAT_TABLES_DYNAMIC \
		STM32MP_EMMC \
		STM32MP_EMMC_BOOT \
		STM32MP_RAW_NAND \
		STM32MP_RECONFIGURE_CONSOLE \
		STM32MP_SDMMC \
		STM32MP_SPI_NAND \
		STM32MP_SPI_NOR \
		STM32MP_UART_PROGRAMMER \
		STM32MP_USB_PROGRAMMER \
)))

$(eval $(call assert_numerics,\
	$(sort \
		STM32_TF_VERSION \
		STM32MP_UART_BAUDRATE \
)))

$(eval $(call add_defines,\
	$(sort \
		PLAT_XLAT_TABLES_DYNAMIC \
		STM32_TF_VERSION \
		STM32MP_EMMC \
		STM32MP_EMMC_BOOT \
		STM32MP_RAW_NAND \
		STM32MP_RECONFIGURE_CONSOLE \
		STM32MP_SDMMC \
		STM32MP_SPI_NAND \
		STM32MP_SPI_NOR \
		STM32MP_UART_BAUDRATE \
		STM32MP_UART_PROGRAMMER \
		STM32MP_USB_PROGRAMMER \
)))

# Include paths and source files
PLAT_INCLUDES			+=	-Iplat/st/common/include/

include lib/fconf/fconf.mk
include lib/libfdt/libfdt.mk
include lib/zlib/zlib.mk
ifeq (${PSA_FWU_SUPPORT},1)
include drivers/fwu/fwu.mk
endif

PLAT_BL_COMMON_SOURCES		+=	common/uuid.c					\
					plat/st/common/stm32mp_common.c


include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES		+=	${XLAT_TABLES_LIB_SRCS}

PLAT_BL_COMMON_SOURCES		+=	drivers/clk/clk.c				\
					drivers/delay_timer/delay_timer.c		\
					drivers/delay_timer/generic_delay_timer.c	\
					drivers/st/clk/stm32mp_clkfunc.c		\
					drivers/st/ddr/stm32mp_ddr.c			\
					drivers/st/gpio/stm32_gpio.c			\
					drivers/st/regulator/regulator_core.c		\
					drivers/st/regulator/regulator_fixed.c		\
					plat/st/common/stm32mp_dt.c

BL2_SOURCES			+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}
BL2_SOURCES			+=	$(ZLIB_SOURCES)

BL2_SOURCES			+=	drivers/io/io_fip.c				\
					plat/st/common/bl2_io_storage.c			\
					plat/st/common/plat_image_load.c		\
					plat/st/common/stm32mp_fconf_io.c

BL2_SOURCES			+=	drivers/io/io_block.c				\
					drivers/io/io_mtd.c				\
					drivers/io/io_storage.c

ifneq (${DECRYPTION_SUPPORT},none)
BL2_SOURCES			+=	drivers/io/io_encrypted.c
endif

ifeq (${TRUSTED_BOARD_BOOT},1)
AUTH_MK := drivers/auth/auth.mk
$(info Including ${AUTH_MK})
include ${AUTH_MK}

ifeq (${GENERATE_COT},1)
TFW_NVCTR_VAL			:=	0
NTFW_NVCTR_VAL			:=	0
KEY_ALG				:=	ecdsa
HASH_ALG			:=	sha256

ifeq (${SAVE_KEYS},1)
TRUSTED_WORLD_KEY		?=	${BUILD_PLAT}/trusted.pem
NON_TRUSTED_WORLD_KEY		?=	${BUILD_PLAT}/non-trusted.pem
BL32_KEY			?=	${BUILD_PLAT}/trusted_os.pem
BL33_KEY			?=	${BUILD_PLAT}/non-trusted_os.pem
endif

endif
TF_MBEDTLS_KEY_ALG		:=	ecdsa
KEY_SIZE			:=	256

PLAT_INCLUDES			+=	-Iinclude/drivers/auth/mbedtls

MBEDTLS_CONFIG_FILE		?=	"<stm32mp_mbedtls_config.h>"

include drivers/auth/mbedtls/mbedtls_x509.mk

COT_DESC_IN_DTB			:=	1
AUTH_SOURCES			+=	lib/fconf/fconf_cot_getter.c			\
					lib/fconf/fconf_tbbr_getter.c			\
					plat/st/common/stm32mp_crypto_lib.c

BL2_SOURCES			+=	$(AUTH_SOURCES)					\
					plat/st/common/stm32mp_trusted_boot.c
endif

ifneq ($(filter 1,${STM32MP_EMMC} ${STM32MP_SDMMC}),)
BL2_SOURCES			+=	drivers/mmc/mmc.c				\
					drivers/partition/gpt.c				\
					drivers/partition/partition.c
endif

ifneq ($(filter 1,${STM32MP_SPI_NAND} ${STM32MP_SPI_NOR}),)
BL2_SOURCES			+=	drivers/mtd/spi-mem/spi_mem.c
endif

ifeq (${STM32MP_RAW_NAND},1)
$(eval $(call add_define_val,NAND_ONFI_DETECT,1))
BL2_SOURCES			+=	drivers/mtd/nand/raw_nand.c
endif

ifeq (${STM32MP_SPI_NAND},1)
BL2_SOURCES			+=	drivers/mtd/nand/spi_nand.c
endif

ifeq (${STM32MP_SPI_NOR},1)
ifneq (${STM32MP_FORCE_MTD_START_OFFSET},)
$(eval $(call add_define_val,STM32MP_NOR_FIP_OFFSET,${STM32MP_FORCE_MTD_START_OFFSET}))
endif
BL2_SOURCES			+=	drivers/mtd/nor/spi_nor.c
endif

ifneq ($(filter 1,${STM32MP_RAW_NAND} ${STM32MP_SPI_NAND}),)
ifneq (${STM32MP_FORCE_MTD_START_OFFSET},)
$(eval $(call add_define_val,STM32MP_NAND_FIP_OFFSET,${STM32MP_FORCE_MTD_START_OFFSET}))
endif
BL2_SOURCES			+=	drivers/mtd/nand/core.c
endif

ifneq ($(filter 1,${STM32MP_UART_PROGRAMMER} ${STM32MP_USB_PROGRAMMER}),)
BL2_SOURCES			+=	drivers/io/io_memmap.c
endif

ifeq (${STM32MP_UART_PROGRAMMER},1)
BL2_SOURCES			+=	plat/st/common/stm32cubeprogrammer_uart.c
endif

ifeq (${STM32MP_USB_PROGRAMMER},1)
BL2_SOURCES			+=	drivers/usb/usb_device.c			\
					plat/st/common/stm32cubeprogrammer_usb.c	\
					plat/st/common/usb_dfu.c
endif

BL2_SOURCES			+=	drivers/st/ddr/stm32mp_ddr_test.c		\
					drivers/st/ddr/stm32mp_ram.c

BL2_SOURCES			+=	common/desc_image_load.c

BL2_SOURCES			+=	lib/optee/optee_utils.c
