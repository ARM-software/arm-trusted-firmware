#
# Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		:=	-Iplat/rpi/common/include		\
				-Iplat/rpi/rpi3/include

PLAT_BL_COMMON_SOURCES	:=	drivers/ti/uart/aarch64/16550_console.S	\
				drivers/arm/pl011/aarch64/pl011_console.S \
				drivers/gpio/gpio.c			\
				drivers/delay_timer/delay_timer.c	\
				drivers/rpi3/gpio/rpi3_gpio.c		\
				plat/rpi/common/aarch64/plat_helpers.S	\
				plat/rpi/common/rpi3_common.c		\
				${XLAT_TABLES_LIB_SRCS}

BL1_SOURCES		+=	drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/rpi/rpi3/rpi3_bl1_setup.c		\
				plat/rpi/common/rpi3_io_storage.c	\
				drivers/rpi3/mailbox/rpi3_mbox.c	\
				plat/rpi/rpi3/rpi_mbox_board.c

BL2_SOURCES		+=	common/desc_image_load.c		\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				drivers/delay_timer/generic_delay_timer.c \
				drivers/io/io_block.c			\
				drivers/mmc/mmc.c			\
				drivers/rpi3/sdhost/rpi3_sdhost.c	\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/rpi/rpi3/aarch64/rpi3_bl2_mem_params_desc.c \
				plat/rpi/rpi3/rpi3_bl2_setup.c		\
				plat/rpi/common/rpi3_image_load.c	\
				plat/rpi/common/rpi3_io_storage.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				plat/common/plat_psci_common.c		\
				plat/rpi/rpi3/rpi3_bl31_setup.c		\
				plat/rpi/common/rpi3_pm.c		\
				plat/rpi/common/rpi3_topology.c		\
				${LIBFDT_SRCS}

# Tune compiler for Cortex-A53
ifeq ($(notdir $(CC)),armclang)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else ifneq ($(findstring clang,$(notdir $(CC))),)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a53
else
    TF_CFLAGS_aarch64	+=	-mtune=cortex-a53
endif

# Platform Makefile target
# ------------------------

RPI3_BL1_PAD_BIN	:=	${BUILD_PLAT}/bl1_pad.bin
RPI3_ARMSTUB8_BIN	:=	${BUILD_PLAT}/armstub8.bin

# Add new default target when compiling this platform
all: armstub

# This target concatenates BL1 and the FIP so that the base addresses match the
# ones defined in the memory map
armstub: bl1 fip
	@echo "  CAT     $@"
	${Q}cp ${BUILD_PLAT}/bl1.bin ${RPI3_BL1_PAD_BIN}
	${Q}truncate --size=131072 ${RPI3_BL1_PAD_BIN}
	${Q}cat ${RPI3_BL1_PAD_BIN} ${BUILD_PLAT}/fip.bin > ${RPI3_ARMSTUB8_BIN}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

# Build config flags
# ------------------

# Enable all errata workarounds for Cortex-A53
ERRATA_A53_826319		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_836870		:= 1
ERRATA_A53_843419		:= 1
ERRATA_A53_855873		:= 1

WORKAROUND_CVE_2017_5715	:= 0

# Disable stack protector by default
ENABLE_STACK_PROTECTOR	 	:= 0

# Reset to BL31 isn't supported
RESET_TO_BL31			:= 0

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# Use Coherent memory
USE_COHERENT_MEM		:= 1

# Platform build flags
# --------------------

# BL33 images are in AArch64 by default
RPI3_BL33_IN_AARCH32		:= 0

# Assume that BL33 isn't the Linux kernel by default
RPI3_DIRECT_LINUX_BOOT		:= 0

# UART to use at runtime. -1 means the runtime UART is disabled.
# Any other value means the default UART will be used.
RPI3_RUNTIME_UART		:= -1

# Use normal memory mapping for ROM, FIP, SRAM and DRAM
RPI3_USE_UEFI_MAP		:= 0

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
$(eval $(call add_define,RPI3_DIRECT_LINUX_BOOT))
ifdef RPI3_PRELOADED_DTB_BASE
$(eval $(call add_define,RPI3_PRELOADED_DTB_BASE))
endif
$(eval $(call add_define,RPI3_RUNTIME_UART))
$(eval $(call add_define,RPI3_USE_UEFI_MAP))

# Verify build config
# -------------------
#
ifneq (${RPI3_DIRECT_LINUX_BOOT}, 0)
  ifndef RPI3_PRELOADED_DTB_BASE
    $(error Error: RPI3_PRELOADED_DTB_BASE needed if RPI3_DIRECT_LINUX_BOOT=1)
  endif
endif

ifneq (${RESET_TO_BL31}, 0)
  $(error Error: rpi3 needs RESET_TO_BL31=0)
endif

ifeq (${ARCH},aarch32)
  $(error Error: AArch32 not supported on rpi3)
endif

ifneq ($(ENABLE_STACK_PROTECTOR), 0)
PLAT_BL_COMMON_SOURCES	+=	drivers/rpi3/rng/rpi3_rng.c		\
				plat/rpi/common/rpi3_stack_protector.c
endif

ifeq (${SPD},opteed)
BL2_SOURCES	+=							\
		lib/optee/optee_utils.c
endif

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

ifneq (${TRUSTED_BOARD_BOOT},0)

    include drivers/auth/mbedtls/mbedtls_crypto.mk
    include drivers/auth/mbedtls/mbedtls_x509.mk

    AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot_common.c

    BL1_SOURCES		+=	${AUTH_SOURCES}				\
				bl1/tbbr/tbbr_img_desc.c		\
				plat/common/tbbr/plat_tbbr.c		\
				plat/rpi/common/rpi3_trusted_boot.c    	\
				plat/rpi/common/rpi3_rotpk.S		\
				drivers/auth/tbbr/tbbr_cot_bl1.c

    BL2_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				plat/rpi/common/rpi3_trusted_boot.c    	\
				plat/rpi/common/rpi3_rotpk.S		\
				drivers/auth/tbbr/tbbr_cot_bl2.c

    ROT_KEY             = $(BUILD_PLAT)/rot_key.pem
    ROTPK_HASH          = $(BUILD_PLAT)/rotpk_sha256.bin

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
