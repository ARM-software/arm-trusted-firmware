#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk

# Enable new version of image loading on QEMU platforms
LOAD_IMAGE_V2		:=	1

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,QEMU_LOAD_BL32))
endif

PLAT_PATH               :=      plat/qemu/
PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/		\
				-Iinclude/plat/arm/common/aarch64/	\
				-Iplat/qemu/include			\
				-Iinclude/common/tbbr

# Use translation tables library v2 by default
ARM_XLAT_TABLES_LIB_V1		:=	0
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))


PLAT_BL_COMMON_SOURCES	:=	plat/qemu/qemu_common.c			\
				drivers/arm/pl011/aarch64/pl011_console.S

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c
else
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}
endif

ifneq (${TRUSTED_BOARD_BOOT},0)

    include drivers/auth/mbedtls/mbedtls_crypto.mk
    include drivers/auth/mbedtls/mbedtls_x509.mk

    USE_TBBR_DEFS	:=	1

    AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot.c

    PLAT_INCLUDES	+=	-Iinclude/bl1/tbbr

    BL1_SOURCES		+=	${AUTH_SOURCES}				\
				bl1/tbbr/tbbr_img_desc.c		\
				plat/common/tbbr/plat_tbbr.c		\
				plat/qemu/qemu_trusted_boot.c	     	\
				$(PLAT_PATH)/qemu_rotpk.S

    BL2_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				plat/qemu/qemu_trusted_boot.c	     	\
				$(PLAT_PATH)/qemu_rotpk.S

    ROT_KEY             = $(BUILD_PLAT)/rot_key.pem
    ROTPK_HASH          = $(BUILD_PLAT)/rotpk_sha256.bin

    $(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))

    $(BUILD_PLAT)/bl1/qemu_rotpk.o: $(ROTPK_HASH)
    $(BUILD_PLAT)/bl2/qemu_rotpk.o: $(ROTPK_HASH)

    certificates: $(ROT_KEY)

    $(ROT_KEY):
	@echo "  OPENSSL $@"
	$(Q)openssl genrsa 2048 > $@ 2>/dev/null

    $(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null
endif

BL1_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/aarch64/semihosting_call.S \
				plat/qemu/qemu_io_storage.c		\
				lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				plat/qemu/aarch64/plat_helpers.S	\
				plat/qemu/qemu_bl1_setup.c

BL2_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/aarch64/semihosting_call.S\
				plat/qemu/qemu_io_storage.c		\
				plat/qemu/aarch64/plat_helpers.S	\
				plat/qemu/qemu_bl2_setup.c		\
				plat/qemu/dt.c				\
				$(LIBFDT_SRCS)
ifeq (${LOAD_IMAGE_V2},1)
BL2_SOURCES		+=	plat/qemu/qemu_bl2_mem_params_desc.c	\
				plat/qemu/qemu_image_load.c		\
				common/desc_image_load.c
endif
ifeq (${SPD},opteed)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif


BL31_SOURCES		+=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_psci_common.c		\
				plat/qemu/qemu_pm.c			\
				plat/qemu/topology.c			\
				plat/qemu/aarch64/plat_helpers.S	\
				plat/qemu/qemu_bl31_setup.c		\
				plat/qemu/qemu_gic.c


# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 	0

BL32_RAM_LOCATION	:=	tdram
ifeq (${BL32_RAM_LOCATION}, tsram)
  BL32_RAM_LOCATION_ID = SEC_SRAM_ID
else ifeq (${BL32_RAM_LOCATION}, tdram)
  BL32_RAM_LOCATION_ID = SEC_DRAM_ID
else
  $(error "Unsupported BL32_RAM_LOCATION value")
endif

# Process flags
$(eval $(call add_define,BL32_RAM_LOCATION_ID))

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0
