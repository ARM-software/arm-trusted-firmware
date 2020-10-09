#
# Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ARM_CORTEX_A7		:=	yes
ARM_WITH_NEON		:=	yes
BL2_AT_EL3		:=	1
USE_COHERENT_MEM	:=	0

STM32_TF_VERSION	?=	0

# Enable dynamic memory mapping
PLAT_XLAT_TABLES_DYNAMIC :=	1

ifeq ($(AARCH32_SP),sp_min)
# Disable Neon support: sp_min runtime may conflict with non-secure world
TF_CFLAGS		+=	-mfloat-abi=soft
endif

# Not needed for Cortex-A7
WORKAROUND_CVE_2017_5715:=	0

# Number of TF-A copies in the device
STM32_TF_A_COPIES		:=	2
STM32_BL33_PARTS_NUM		:=	1
ifeq ($(AARCH32_SP),optee)
STM32_RUNTIME_PARTS_NUM		:=	3
else
STM32_RUNTIME_PARTS_NUM		:=	0
endif
PLAT_PARTITION_MAX_ENTRIES	:=	$(shell echo $$(($(STM32_TF_A_COPIES) + \
							 $(STM32_BL33_PARTS_NUM) + \
							 $(STM32_RUNTIME_PARTS_NUM))))

# Boot devices
STM32MP_EMMC		?=	0
STM32MP_SDMMC		?=	0
STM32MP_RAW_NAND	?=	0
STM32MP_SPI_NAND	?=	0
STM32MP_SPI_NOR		?=	0

ifeq ($(filter 1,${STM32MP_EMMC} ${STM32MP_SDMMC} ${STM32MP_RAW_NAND} \
	${STM32MP_SPI_NAND} ${STM32MP_SPI_NOR}),)
$(error "No boot device driver is enabled")
endif

# Device tree
DTB_FILE_NAME		?=	stm32mp157c-ev1.dtb
FDT_SOURCES		:=	$(addprefix fdts/, $(patsubst %.dtb,%.dts,$(DTB_FILE_NAME)))
DTC_FLAGS		+=	-Wno-unit_address_vs_reg

# Macros and rules to build TF binary
STM32_TF_ELF_LDFLAGS	:=	--hash-style=gnu --as-needed
STM32_TF_STM32		:=	$(addprefix ${BUILD_PLAT}/tf-a-, $(patsubst %.dtb,%.stm32,$(DTB_FILE_NAME)))
STM32_TF_LINKERFILE	:=	${BUILD_PLAT}/stm32mp1.ld

ASFLAGS			+= -DBL2_BIN_PATH=\"${BUILD_PLAT}/bl2.bin\"
ifeq ($(AARCH32_SP),sp_min)
# BL32 is built only if using SP_MIN
BL32_DEP		:= bl32
ASFLAGS			+= -DBL32_BIN_PATH=\"${BUILD_PLAT}/bl32.bin\"
endif

# Variables for use with stm32image
STM32IMAGEPATH		?= tools/stm32image
STM32IMAGE		?= ${STM32IMAGEPATH}/stm32image${BIN_EXT}

# Enable flags for C files
$(eval $(call assert_booleans,\
	$(sort \
		STM32MP_EMMC \
		STM32MP_SDMMC \
		STM32MP_RAW_NAND \
		STM32MP_SPI_NAND \
		STM32MP_SPI_NOR \
		PLAT_XLAT_TABLES_DYNAMIC \
)))

$(eval $(call assert_numerics,\
	$(sort \
		STM32_TF_A_COPIES \
		PLAT_PARTITION_MAX_ENTRIES \
)))

$(eval $(call add_defines,\
	$(sort \
		STM32MP_EMMC \
		STM32MP_SDMMC \
		STM32MP_RAW_NAND \
		STM32MP_SPI_NAND \
		STM32MP_SPI_NOR \
		PLAT_XLAT_TABLES_DYNAMIC \
		STM32_TF_A_COPIES \
		PLAT_PARTITION_MAX_ENTRIES \
)))

# Include paths and source files
PLAT_INCLUDES		:=	-Iplat/st/common/include/
PLAT_INCLUDES		+=	-Iplat/st/stm32mp1/include/

include lib/libfdt/libfdt.mk

PLAT_BL_COMMON_SOURCES	:=	common/fdt_wrappers.c					\
				plat/st/common/stm32mp_common.c				\
				plat/st/stm32mp1/stm32mp1_private.c

PLAT_BL_COMMON_SOURCES	+=	drivers/st/uart/aarch32/stm32_console.S

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/st/stm32mp1/stm32mp1_stack_protector.c
endif

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch32/cortex_a7.S

PLAT_BL_COMMON_SOURCES	+=	drivers/arm/tzc/tzc400.c				\
				drivers/delay_timer/delay_timer.c			\
				drivers/delay_timer/generic_delay_timer.c		\
				drivers/st/bsec/bsec.c					\
				drivers/st/clk/stm32mp_clkfunc.c			\
				drivers/st/clk/stm32mp1_clk.c				\
				drivers/st/ddr/stm32mp1_ddr_helpers.c			\
				drivers/st/gpio/stm32_gpio.c				\
				drivers/st/i2c/stm32_i2c.c				\
				drivers/st/iwdg/stm32_iwdg.c				\
				drivers/st/pmic/stm32mp_pmic.c				\
				drivers/st/pmic/stpmic1.c				\
				drivers/st/reset/stm32mp1_reset.c			\
				plat/st/common/stm32mp_dt.c				\
				plat/st/stm32mp1/stm32mp1_context.c			\
				plat/st/stm32mp1/stm32mp1_dbgmcu.c			\
				plat/st/stm32mp1/stm32mp1_helper.S			\
				plat/st/stm32mp1/stm32mp1_security.c			\
				plat/st/stm32mp1/stm32mp1_syscfg.c

BL2_SOURCES		+=	drivers/io/io_block.c					\
				drivers/io/io_dummy.c					\
				drivers/io/io_mtd.c					\
				drivers/io/io_storage.c					\
				drivers/st/crypto/stm32_hash.c				\
				drivers/st/io/io_stm32image.c				\
				plat/st/common/stm32mp_auth.c				\
				plat/st/common/bl2_io_storage.c				\
				plat/st/stm32mp1/bl2_plat_setup.c

ifneq ($(filter 1,${STM32MP_EMMC} ${STM32MP_SDMMC}),)
BL2_SOURCES		+=	drivers/mmc/mmc.c					\
				drivers/partition/gpt.c					\
				drivers/partition/partition.c				\
				drivers/st/io/io_mmc.c					\
				drivers/st/mmc/stm32_sdmmc2.c
endif

ifeq (${STM32MP_RAW_NAND},1)
$(eval $(call add_define_val,NAND_ONFI_DETECT,1))
BL2_SOURCES		+=	drivers/mtd/nand/raw_nand.c				\
				drivers/st/fmc/stm32_fmc2_nand.c
endif

ifeq (${STM32MP_SPI_NAND},1)
BL2_SOURCES		+=	drivers/mtd/nand/spi_nand.c
endif

ifeq (${STM32MP_SPI_NOR},1)
BL2_SOURCES		+=	drivers/mtd/nor/spi_nor.c
endif

ifneq ($(filter 1,${STM32MP_SPI_NAND} ${STM32MP_SPI_NOR}),)
BL2_SOURCES		+=	drivers/mtd/spi-mem/spi_mem.c				\
				drivers/st/spi/stm32_qspi.c
endif

ifneq ($(filter 1,${STM32MP_RAW_NAND} ${STM32MP_SPI_NAND}),)
BL2_SOURCES		+=	drivers/mtd/nand/core.c
endif

ifneq ($(filter 1,${STM32MP_RAW_NAND} ${STM32MP_SPI_NAND} ${STM32MP_SPI_NOR}),)
BL2_SOURCES		+=	plat/st/stm32mp1/stm32mp1_boot_device.c
endif

BL2_SOURCES		+=	drivers/st/ddr/stm32mp1_ddr.c				\
				drivers/st/ddr/stm32mp1_ram.c

BL2_SOURCES		+=	common/desc_image_load.c				\
				plat/st/stm32mp1/plat_bl2_mem_params_desc.c		\
				plat/st/stm32mp1/plat_image_load.c

ifeq ($(AARCH32_SP),optee)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

# Compilation rules
.PHONY: check_dtc_version stm32image clean_stm32image
.SUFFIXES:

all: check_dtc_version stm32image ${STM32_TF_STM32}

distclean realclean clean: clean_stm32image

stm32image: ${STM32IMAGE}

${STM32IMAGE}: ${STM32IMAGE_SRC}
	${Q}${MAKE} CPPFLAGS="" --no-print-directory -C ${STM32IMAGEPATH}

clean_stm32image:
	${Q}${MAKE} --no-print-directory -C ${STM32IMAGEPATH} clean

check_dtc_version:
	$(eval DTC_V = $(shell $(DTC) -v | awk '{print $$NF}'))
	$(eval DTC_VERSION = $(shell printf "%d" $(shell echo ${DTC_V} | cut -d- -f1 | sed "s/\./0/g")))
	@if [ ${DTC_VERSION} -lt 10404 ]; then \
		echo "dtc version too old (${DTC_V}), you need at least version 1.4.4"; \
		false; \
	fi


${BUILD_PLAT}/stm32mp1-%.o: ${BUILD_PLAT}/fdts/%.dtb plat/st/stm32mp1/stm32mp1.S bl2 ${BL32_DEP}
	@echo "  AS      stm32mp1.S"
	${Q}${AS} ${ASFLAGS} ${TF_CFLAGS} \
		-DDTB_BIN_PATH=\"$<\" \
		-c plat/st/stm32mp1/stm32mp1.S -o $@

$(eval $(call MAKE_LD,${STM32_TF_LINKERFILE},plat/st/stm32mp1/stm32mp1.ld.S,2))

tf-a-%.elf: stm32mp1-%.o ${STM32_TF_LINKERFILE}
	@echo "  LDS     $<"
	${Q}${LD} -o $@ ${STM32_TF_ELF_LDFLAGS} -Map=$(@:.elf=.map) --script ${STM32_TF_LINKERFILE} $<

tf-a-%.bin: tf-a-%.elf
	${Q}${OC} -O binary $< $@
	@echo
	@echo "Built $@ successfully"
	@echo

tf-a-%.stm32: ${STM32IMAGE} tf-a-%.bin
	@echo
	@echo "Generate $@"
	$(eval LOADADDR = $(shell cat $(@:.stm32=.map) | grep RAM | awk '{print $$2}'))
	$(eval ENTRY = $(shell cat $(@:.stm32=.map) | grep "__BL2_IMAGE_START" | awk '{print $$1}'))
	${Q}${STM32IMAGE} -s $(word 2,$^) -d $@ \
		-l $(LOADADDR) -e ${ENTRY} \
		-v ${STM32_TF_VERSION}
	@echo
