#
# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		:=	-Iinclude/common/tbbr			\
				-Iinclude/plat/arm/common/		\
				-Iinclude/plat/arm/common/aarch64/	\
				-Iplat/rpi3/include

PLAT_BL_COMMON_SOURCES	:=	drivers/console/aarch64/console.S	\
				drivers/ti/uart/aarch64/16550_console.S	\
				plat/rpi3/rpi3_common.c

BL1_SOURCES		+=	drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/rpi3/aarch64/plat_helpers.S	\
				plat/rpi3/rpi3_bl1_setup.c		\
				plat/rpi3/rpi3_io_storage.c

BL2_SOURCES		+=	common/desc_image_load.c		\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/rpi3/aarch64/plat_helpers.S	\
				plat/rpi3/aarch64/rpi3_bl2_mem_params_desc.c \
				plat/rpi3/rpi3_bl2_setup.c		\
				plat/rpi3/rpi3_image_load.c		\
				plat/rpi3/rpi3_io_storage.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				plat/common/aarch64/plat_psci_common.c	\
				plat/rpi3/aarch64/plat_helpers.S	\
				plat/rpi3/rpi3_bl31_setup.c		\
				plat/rpi3/rpi3_pm.c			\
				plat/rpi3/rpi3_topology.c

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

# Tune compiler for Cortex-A53
ifeq ($(notdir $(CC)),armclang)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else ifneq ($(findstring clang,$(notdir $(CC))),)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else
    TF_CFLAGS_aarch64	+=	-mtune=cortex-a53
endif

# Build config flags
# ------------------

# Enable all errata workarounds for Cortex-A53
ERRATA_A53_826319		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_836870		:= 1
ERRATA_A53_843419		:= 1
ERRATA_A53_855873		:= 1

WORKAROUND_CVE_2017_5715	:= 0

# Disable the PSCI platform compatibility layer by default
ENABLE_PLAT_COMPAT		:= 0

# Enable reset to BL31 by default
RESET_TO_BL31			:= 1

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# Use Coherent memory
USE_COHERENT_MEM		:= 1

# Enable new version of image loading
LOAD_IMAGE_V2			:= 1

# Use multi console API
MULTI_CONSOLE_API		:= 1

# Platform build flags
# --------------------

# BL33 images are in AArch64 by default
RPI3_BL33_IN_AARCH32		:= 0

# BL32 location
RPI3_BL32_RAM_LOCATION	:= tdram
ifeq (${RPI3_BL32_RAM_LOCATION}, tsram)
  RPI3_BL32_RAM_LOCATION_ID = SEC_SRAM_ID
else ifeq (${RPI3_BL32_RAM_LOCATION}, tdram)
  RPI3_BL32_RAM_LOCATION_ID = SEC_DRAM_ID
else
  $(error "Unsupported RPI3_BL32_RAM_LOCATION value")
endif

# Process platform flags
# ----------------------

$(eval $(call add_define,RPI3_BL32_RAM_LOCATION_ID))
$(eval $(call add_define,RPI3_BL33_IN_AARCH32))

# Verify build config
# -------------------

ifneq (${LOAD_IMAGE_V2}, 1)
  $(error Error: rpi3 needs LOAD_IMAGE_V2=1)
endif

ifneq (${MULTI_CONSOLE_API}, 1)
  $(error Error: rpi3 needs MULTI_CONSOLE_API=1)
endif

ifeq (${ARCH},aarch32)
  $(error Error: AArch32 not supported on rpi3)
endif

ifeq (${SPD},opteed)
BL2_SOURCES	+=							\
		lib/optee/optee_utils.c
endif

ifeq (${TRUSTED_BOARD_BOOT},1)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

USE_TBBR_DEFS		:=	1

AUTH_SOURCES		:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot.c

PLAT_INCLUDES		+=	-Iinclude/bl1/tbbr

BL1_SOURCES		+=	${AUTH_SOURCES}				\
				bl1/tbbr/tbbr_img_desc.c		\
				plat/common/tbbr/plat_tbbr.c		\
				plat/rpi3/rpi3_tbbr.c	     		\
				plat/rpi3/rpi3_rotpk.S

BL2_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				plat/rpi3/rpi3_tbbr.c	     		\
				plat/rpi3/rpi3_rotpk.S

ROT_KEY		= $(BUILD_PLAT)/rot_key.pem
ROTPK_HASH		= $(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
$(BUILD_PLAT)/bl1/rpi3_rotpk.o: $(ROTPK_HASH)
$(BUILD_PLAT)/bl2/rpi3_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)
$(ROT_KEY): | $(BUILD_PLAT)
	@echo "  OPENSSL $@"
	$(Q)openssl genrsa 2048 > $@ 2>/dev/null

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null

endif

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif
