#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

override COLD_BOOT_SINGLE_CPU	:= 1
override ENABLE_PLAT_COMPAT	:= 0
override ERROR_DEPRECATED	:= 1
override LOAD_IMAGE_V2		:= 1
override USE_COHERENT_MEM	:= 1
override USE_TBBR_DEFS		:= 1
override ENABLE_SVE_FOR_NS	:= 0

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

# IO sources for BL1, BL2
IO_SOURCES		:=	drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				$(PLAT_PATH)/uniphier_boot_device.c	\
				$(PLAT_PATH)/uniphier_emmc.c		\
				$(PLAT_PATH)/uniphier_io_storage.c	\
				$(PLAT_PATH)/uniphier_nand.c		\
				$(PLAT_PATH)/uniphier_usb.c

# common sources for BL1, BL2, BL31
PLAT_BL_COMMON_SOURCES	+=	drivers/console/aarch64/console.S	\
				$(PLAT_PATH)/uniphier_console.S		\
				$(PLAT_PATH)/uniphier_helpers.S		\
				$(PLAT_PATH)/uniphier_soc_info.c	\
				$(PLAT_PATH)/uniphier_xlat_setup.c	\
				${XLAT_TABLES_LIB_SRCS}

BL1_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				$(PLAT_PATH)/uniphier_bl1_helpers.S	\
				$(PLAT_PATH)/uniphier_bl1_setup.c	\
				$(IO_SOURCES)

BL2_SOURCES		+=	common/desc_image_load.c		\
				$(PLAT_PATH)/uniphier_bl2_setup.c	\
				$(PLAT_PATH)/uniphier_image_desc.c	\
				$(PLAT_PATH)/uniphier_scp.c		\
				$(IO_SOURCES)

BL31_SOURCES		+=	drivers/arm/cci/cci.c			\
				drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				$(PLAT_PATH)/uniphier_bl31_setup.c	\
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

PLAT_INCLUDES		+=	-Iinclude/common/tbbr

TBB_SOURCES		:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot.c		\
				plat/common/tbbr/plat_tbbr.c		\
				$(PLAT_PATH)/uniphier_rotpk.S		\
				$(PLAT_PATH)/uniphier_tbbr.c

BL1_SOURCES		+=	$(TBB_SOURCES)
BL2_SOURCES		+=	$(TBB_SOURCES)

ROT_KEY			= $(BUILD_PLAT)/rot_key.pem
ROTPK_HASH		= $(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(BUILD_PLAT)/bl1/uniphier_rotpk.o: $(ROTPK_HASH)
$(BUILD_PLAT)/bl2/uniphier_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)
$(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	$(Q)openssl genrsa 2048 > $@ 2>/dev/null

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null

endif

.PHONY: bl1_gzip
bl1_gzip: $(BUILD_PLAT)/bl1.bin.gzip
%.gzip: %
	@echo "  GZIP    $@"
	$(Q)gzip -n -f -9 $< --stdout > $@
