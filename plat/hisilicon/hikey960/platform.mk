#
# Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Non-TF Boot ROM
BL2_AT_EL3	:=	1

# On Hikey960, the TSP can execute from TZC secure area in DRAM.
HIKEY960_TSP_RAM_LOCATION	?=	dram
ifeq (${HIKEY960_TSP_RAM_LOCATION}, dram)
  HIKEY960_TSP_RAM_LOCATION_ID = HIKEY960_DRAM_ID
else ifeq (${HIKEY960_TSP_RAM_LOCATION}, sram)
  HIKEY960_TSP_RAM_LOCATION_ID = HIKEY960_SRAM_ID
else
  $(error "Currently unsupported HIKEY960_TSP_RAM_LOCATION value")
endif

CRASH_CONSOLE_BASE		:=	PL011_UART6_BASE
COLD_BOOT_SINGLE_CPU		:=	1
PLAT_PL061_MAX_GPIOS		:=	232
PROGRAMMABLE_RESET_ADDRESS	:=	1
ENABLE_SVE_FOR_NS		:=	0
PLAT_PARTITION_BLOCK_SIZE	:=	4096

# Process flags
$(eval $(call add_define,HIKEY960_TSP_RAM_LOCATION_ID))
$(eval $(call add_define,CRASH_CONSOLE_BASE))
$(eval $(call add_define,PLAT_PL061_MAX_GPIOS))
$(eval $(call add_define,PLAT_PARTITION_BLOCK_SIZE))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,bl32_extra1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,bl32_extra2,--tos-fw-extra2))
endif

USE_COHERENT_MEM	:=	1

PLAT_INCLUDES		:=	-Iplat/hisilicon/hikey960/include

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/aarch64/pl011_console.S \
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				lib/xlat_tables/aarch64/xlat_tables.c	\
				lib/xlat_tables/xlat_tables_common.c	\
				plat/hisilicon/hikey960/aarch64/hikey960_common.c \
				plat/hisilicon/hikey960/hikey960_boardid.c

HIKEY960_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

BL1_SOURCES		+=	bl1/tbbr/tbbr_img_desc.c		\
				drivers/arm/pl061/pl061_gpio.c		\
				drivers/gpio/gpio.c			\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_storage.c			\
				drivers/synopsys/ufs/dw_ufs.c		\
				drivers/ufs/ufs.c 			\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/hisilicon/hikey960/aarch64/hikey960_helpers.S \
				plat/hisilicon/hikey960/hikey960_bl1_setup.c \
				plat/hisilicon/hikey960/hikey960_bl_common.c \
				plat/hisilicon/hikey960/hikey960_io_storage.c \
				${HIKEY960_GIC_SOURCES}

BL2_SOURCES		+=	common/desc_image_load.c		\
				drivers/arm/pl061/pl061_gpio.c		\
				drivers/gpio/gpio.c			\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_storage.c			\
				drivers/partition/gpt.c			\
				drivers/partition/partition.c		\
				drivers/synopsys/ufs/dw_ufs.c		\
				drivers/ufs/ufs.c			\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/hisilicon/hikey960/aarch64/hikey960_helpers.S \
				plat/hisilicon/hikey960/hikey960_bl2_mem_params_desc.c \
				plat/hisilicon/hikey960/hikey960_bl2_setup.c \
				plat/hisilicon/hikey960/hikey960_bl_common.c \
				plat/hisilicon/hikey960/hikey960_image_load.c \
				plat/hisilicon/hikey960/hikey960_io_storage.c \
				plat/hisilicon/hikey960/hikey960_mcu_load.c

ifeq (${SPD},opteed)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

include lib/zlib/zlib.mk
PLAT_INCLUDES		+=	-Ilib/zlib
BL2_SOURCES		+=	$(ZLIB_SOURCES)

BL31_SOURCES		+=	drivers/arm/cci/cci.c			\
				drivers/arm/pl061/pl061_gpio.c		\
				drivers/gpio/gpio.c			\
				lib/cpus/aarch64/cortex_a53.S           \
				lib/cpus/aarch64/cortex_a72.S		\
				lib/cpus/aarch64/cortex_a73.S		\
				plat/common/plat_psci_common.c  \
				plat/hisilicon/hikey960/aarch64/hikey960_helpers.S \
				plat/hisilicon/hikey960/hikey960_bl31_setup.c \
				plat/hisilicon/hikey960/hikey960_bl_common.c \
				plat/hisilicon/hikey960/hikey960_pm.c	\
				plat/hisilicon/hikey960/hikey960_topology.c \
				plat/hisilicon/hikey960/drivers/pwrc/hisi_pwrc.c \
				plat/hisilicon/hikey960/drivers/ipc/hisi_ipc.c \
				${HIKEY960_GIC_SOURCES}

ifneq (${TRUSTED_BOARD_BOOT},0)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

AUTH_SOURCES		:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot_common.c

BL1_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				plat/hisilicon/hikey960/hikey960_tbbr.c	\
				plat/hisilicon/hikey960/hikey960_rotpk.S \
				drivers/auth/tbbr/tbbr_cot_bl1.c

BL2_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				plat/hisilicon/hikey960/hikey960_tbbr.c	\
				plat/hisilicon/hikey960/hikey960_rotpk.S \
				drivers/auth/tbbr/tbbr_cot_bl2.c

ROT_KEY		=	$(BUILD_PLAT)/rot_key.pem
ROTPK_HASH		=	$(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(BUILD_PLAT)/bl1/hikey960_rotpk.o: $(ROTPK_HASH)
$(BUILD_PLAT)/bl2/hikey960_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)
$(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	$(Q)openssl genrsa 2048 > $@ 2>/dev/null

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null
endif

# Enable workarounds for selected Cortex-A53 errata.
ERRATA_A53_836870		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1

FIP_ALIGN			:=	512
