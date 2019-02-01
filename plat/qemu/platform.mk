#
# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ARM_ARCH_MAJOR},7)
# ARMv7 Qemu support in trusted firmware expects the Cortex-A15 model.
# Qemu Cortex-A15 model does not implement the virtualization extension.
# For this reason, we cannot set ARM_CORTEX_A15=yes and must define all
# the ARMv7 build directives.
MARCH32_DIRECTIVE 	:= 	-mcpu=cortex-a15
$(eval $(call add_define,ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING))
$(eval $(call add_define,ARMV7_SUPPORTS_GENERIC_TIMER))
# Qemu expects a BL32 boot stage.
NEED_BL32		:=	yes
endif # ARMv7

ifeq (${SPD},opteed)
add-lib-optee 		:= 	yes
endif
ifeq ($(AARCH32_SP),optee)
add-lib-optee 		:= 	yes
endif

include lib/libfdt/libfdt.mk

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,QEMU_LOAD_BL32))
endif

PLAT_PATH               :=      plat/qemu/
PLAT_INCLUDES		:=	-Iplat/qemu/include

ifeq (${ARM_ARCH_MAJOR},8)
PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/${ARCH}
endif

PLAT_BL_COMMON_SOURCES	:=	plat/qemu/qemu_common.c			  \
				plat/qemu/qemu_console.c		  \
				drivers/arm/pl011/${ARCH}/pl011_console.S \

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

ifneq (${TRUSTED_BOARD_BOOT},0)

    include drivers/auth/mbedtls/mbedtls_crypto.mk
    include drivers/auth/mbedtls/mbedtls_x509.mk

    AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot.c

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
				lib/semihosting/${ARCH}/semihosting_call.S \
				plat/qemu/qemu_io_storage.c		\
				plat/qemu/${ARCH}/plat_helpers.S	\
				plat/qemu/qemu_bl1_setup.c

ifeq (${ARM_ARCH_MAJOR},8)
BL1_SOURCES		+=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S
else
BL1_SOURCES		+=	lib/cpus/${ARCH}/cortex_a15.S
endif

BL2_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/${ARCH}/semihosting_call.S\
				plat/qemu/qemu_io_storage.c		\
				plat/qemu/${ARCH}/plat_helpers.S	\
				plat/qemu/qemu_bl2_setup.c		\
				plat/qemu/dt.c				\
				plat/qemu/qemu_bl2_mem_params_desc.c	\
				plat/qemu/qemu_image_load.c		\
				common/desc_image_load.c

ifeq ($(add-lib-optee),yes)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif


ifeq (${ARM_ARCH_MAJOR},8)
BL31_SOURCES		+=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_gicv2.c		\
				plat/common/plat_psci_common.c		\
				plat/qemu/qemu_pm.c			\
				plat/qemu/topology.c			\
				plat/qemu/aarch64/plat_helpers.S	\
				plat/qemu/qemu_bl31_setup.c
endif

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,bl32_extra1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,bl32_extra2,--tos-fw-extra2))
endif

SEPARATE_CODE_AND_RODATA := 1
ENABLE_STACK_PROTECTOR	 := 0
ifneq ($(ENABLE_STACK_PROTECTOR), 0)
	PLAT_BL_COMMON_SOURCES += plat/qemu/qemu_stack_protector.c
endif

# Use MULTI_CONSOLE_API by default only on AArch64
# as it is not yet supported on AArch32
ifeq ($(ARCH),aarch64)
MULTI_CONSOLE_API	:= 1
endif

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
