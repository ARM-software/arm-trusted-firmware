#
# Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

override BL2_AT_EL3			:= 1
override COLD_BOOT_SINGLE_CPU		:= 1
override PROGRAMMABLE_RESET_ADDRESS	:= 1
override USE_COHERENT_MEM		:= 1
override ENABLE_SVE_FOR_NS		:= 0

# Disabling ENABLE_PIE saves memory footprint a lot, but you need to adjust
# UNIPHIER_MEM_BASE so that all TF images are loaded at their link addresses.
override ENABLE_PIE			:= 1

ALLOW_RO_XLAT_TABLES			:= 1

ifeq ($(ALLOW_RO_XLAT_TABLES),1)
BL31_CPPFLAGS += -DPLAT_RO_XLAT_TABLES
BL32_CPPFLAGS += -DPLAT_RO_XLAT_TABLES
endif

# The dynamic xlat table is only used in BL2
BL2_CPPFLAGS += -DPLAT_XLAT_TABLES_DYNAMIC

# Cortex-A53 revision r0p4-51rel0
# needed for LD20, unneeded for LD11, PXs3 (no ACE)
ERRATA_A53_855873		:= 1

FIP_ALIGN			:= 512

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,UNIPHIER_LOAD_BL32))
endif

# Libraries
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_PATH		:=	plat/socionext/uniphier
PLAT_INCLUDES		:=	-I$(PLAT_PATH)/include

# common sources for BL2, BL31 (and BL32 if SPD=tspd)
PLAT_BL_COMMON_SOURCES	+=	plat/common/aarch64/crash_console_helpers.S \
				$(PLAT_PATH)/uniphier_console.S		\
				$(PLAT_PATH)/uniphier_console_setup.c	\
				$(PLAT_PATH)/uniphier_helpers.S		\
				$(PLAT_PATH)/uniphier_soc_info.c	\
				$(PLAT_PATH)/uniphier_xlat_setup.c	\
				${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	common/desc_image_load.c		\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				$(PLAT_PATH)/uniphier_bl2_setup.c	\
				$(PLAT_PATH)/uniphier_boot_device.c	\
				$(PLAT_PATH)/uniphier_emmc.c		\
				$(PLAT_PATH)/uniphier_image_desc.c	\
				$(PLAT_PATH)/uniphier_io_storage.c	\
				$(PLAT_PATH)/uniphier_nand.c		\
				$(PLAT_PATH)/uniphier_scp.c		\
				$(PLAT_PATH)/uniphier_usb.c

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

BL31_SOURCES		+=	drivers/arm/cci/cci.c			\
				${GICV3_SOURCES}			\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				$(PLAT_PATH)/uniphier_bl31_setup.c	\
				$(PLAT_PATH)/uniphier_boot_device.c	\
				$(PLAT_PATH)/uniphier_cci.c		\
				$(PLAT_PATH)/uniphier_gicv3.c		\
				$(PLAT_PATH)/uniphier_psci.c		\
				$(PLAT_PATH)/uniphier_scp.c		\
				$(PLAT_PATH)/uniphier_smp.S		\
				$(PLAT_PATH)/uniphier_syscnt.c		\
				$(PLAT_PATH)/uniphier_topology.c

ifeq (${TRUSTED_BOARD_BOOT},1)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

BL2_SOURCES		+=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot_common.c	\
				drivers/auth/tbbr/tbbr_cot_bl2.c	\
				plat/common/tbbr/plat_tbbr.c		\
				$(PLAT_PATH)/uniphier_rotpk.S		\
				$(PLAT_PATH)/uniphier_tbbr.c

ROT_KEY			= $(BUILD_PLAT)/rot_key.pem
ROTPK_HASH		= $(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(BUILD_PLAT)/bl2/uniphier_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)
$(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	$(Q)${OPENSSL_BIN_PATH}/openssl genrsa 2048 > $@ 2>/dev/null

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)${OPENSSL_BIN_PATH}/openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	${OPENSSL_BIN_PATH}/openssl dgst -sha256 -binary > $@ 2>/dev/null

endif

ifeq (${FIP_GZIP},1)

include lib/zlib/zlib.mk

BL2_SOURCES		+=	common/image_decompress.c		\
				$(ZLIB_SOURCES)

$(eval $(call add_define,UNIPHIER_DECOMPRESS_GZIP))

# compress all images loaded by BL2
SCP_BL2_PRE_TOOL_FILTER	:= GZIP
BL31_PRE_TOOL_FILTER	:= GZIP
BL32_PRE_TOOL_FILTER	:= GZIP
BL33_PRE_TOOL_FILTER	:= GZIP

endif

.PHONY: bl2_gzip
bl2_gzip: $(BUILD_PLAT)/bl2.bin.gz
%.gz: %
	@echo "  GZIP    $@"
	$(Q)gzip -n -f -9 $< --stdout > $@
