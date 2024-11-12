#
# Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_QEMU_PATH		:=      plat/qemu/qemu
PLAT_QEMU_COMMON_PATH	:=      plat/qemu/common

SEPARATE_CODE_AND_RODATA := 1
ENABLE_STACK_PROTECTOR	 := 0

include plat/qemu/common/common.mk

# Use the GICv2 driver on QEMU by default
QEMU_USE_GIC_DRIVER	:= QEMU_GICV2

ifeq (${ARM_ARCH_MAJOR},7)
# ARMv7 Qemu support in trusted firmware expects the Cortex-A15 model.
# Qemu Cortex-A15 model does not implement the virtualization extension.
# For this reason, we cannot set ARM_CORTEX_A15=yes and must define all
# the ARMv7 build directives.
MARCH_DIRECTIVE 	:= 	-mcpu=cortex-a15
$(eval $(call add_define,ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING))
$(eval $(call add_define,ARMV7_SUPPORTS_GENERIC_TIMER))
$(eval $(call add_define,ARMV7_SUPPORTS_VFP))
# Qemu expects a BL32 boot stage.
NEED_BL32		:=	yes
endif # ARMv7

ifeq (${SPD},opteed)
add-lib-optee 		:= 	yes
endif
ifeq ($(AARCH32_SP),optee)
add-lib-optee 		:= 	yes
endif
ifeq ($(SPMC_OPTEE),1)
$(eval $(call add_define,SPMC_OPTEE))
add-lib-optee 		:= 	yes
endif

ifeq (${TRANSFER_LIST},1)
include lib/transfer_list/transfer_list.mk
endif

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,QEMU_LOAD_BL32))
endif

ifneq (${TRUSTED_BOARD_BOOT},0)

    AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot_common.c

    BL1_SOURCES		+=	${AUTH_SOURCES}				\
				bl1/tbbr/tbbr_img_desc.c		\
				plat/common/tbbr/plat_tbbr.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_trusted_boot.c	     	\
				$(PLAT_QEMU_COMMON_PATH)/qemu_rotpk.S	\
				drivers/auth/tbbr/tbbr_cot_bl1.c

    BL2_SOURCES		+=	${AUTH_SOURCES}				\
				plat/common/tbbr/plat_tbbr.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_trusted_boot.c	     	\
				$(PLAT_QEMU_COMMON_PATH)/qemu_rotpk.S	\
				drivers/auth/tbbr/tbbr_cot_bl2.c

    include drivers/auth/mbedtls/mbedtls_x509.mk

    ROT_KEY             = $(BUILD_PLAT)/rot_key.pem
    ROTPK_HASH          = $(BUILD_PLAT)/rotpk_sha256.bin

    $(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))

    $(BUILD_PLAT)/bl1/qemu_rotpk.o: $(ROTPK_HASH)
    $(BUILD_PLAT)/bl2/qemu_rotpk.o: $(ROTPK_HASH)

    certificates: $(ROT_KEY)

    $(ROT_KEY): | $$(@D)/
	$(s)echo "  OPENSSL $@"
	$(q)${OPENSSL_BIN_PATH}/openssl genrsa 2048 > $@ 2>/dev/null

    $(ROTPK_HASH): $(ROT_KEY) | $$(@D)/
	$(s)echo "  OPENSSL $@"
	$(q)${OPENSSL_BIN_PATH}/openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	${OPENSSL_BIN_PATH}/openssl dgst -sha256 -binary > $@ 2>/dev/null
endif

# Include Measured Boot makefile before any Crypto library makefile.
# Crypto library makefile may need default definitions of Measured Boot build
# flags present in Measured Boot makefile.
ifeq (${MEASURED_BOOT},1)
    MEASURED_BOOT_MK := drivers/measured_boot/event_log/event_log.mk
    $(info Including ${MEASURED_BOOT_MK})
    include ${MEASURED_BOOT_MK}

    BL2_SOURCES		+=	plat/qemu/qemu/qemu_measured_boot.c	\
				plat/qemu/qemu/qemu_helpers.c		\
				${EVENT_LOG_SOURCES}

     BL1_SOURCES	+=      plat/qemu/qemu/qemu_bl1_measured_boot.c

endif

ifneq ($(filter 1,${MEASURED_BOOT} ${TRUSTED_BOARD_BOOT}),)
    CRYPTO_SOURCES	:=	drivers/auth/crypto_mod.c

    BL1_SOURCES		+=	${CRYPTO_SOURCES}
    BL2_SOURCES		+=	${CRYPTO_SOURCES}

    # We expect to locate the *.mk files under the directories specified below
    #
    include drivers/auth/mbedtls/mbedtls_crypto.mk
endif

BL2_SOURCES		+=	${FDT_WRAPPERS_SOURCES}					\
				common/uuid.c

ifeq ($(add-lib-optee),yes)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

ifneq (${DECRYPTION_SUPPORT},none)
BL1_SOURCES		+=	drivers/io/io_encrypted.c
BL2_SOURCES		+=	drivers/io/io_encrypted.c
endif

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk
QEMU_GICV2_SOURCES	:=	${GICV2_SOURCES}			\
				plat/common/plat_gicv2.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_gicv2.c

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

QEMU_GICV3_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_gicv3.c

ifeq (${QEMU_USE_GIC_DRIVER}, QEMU_GICV2)
QEMU_GIC_SOURCES	:=	${QEMU_GICV2_SOURCES}
else ifeq (${QEMU_USE_GIC_DRIVER}, QEMU_GICV3)
QEMU_GIC_SOURCES	:=	${QEMU_GICV3_SOURCES}
else
$(error "Incorrect GIC driver chosen for QEMU platform")
endif

ifeq (${ARCH},aarch64)
BL31_SOURCES		+=	drivers/arm/pl061/pl061_gpio.c		\
				drivers/gpio/gpio.c			\
				${PLAT_QEMU_COMMON_PATH}/qemu_pm.c	\
				${PLAT_QEMU_COMMON_PATH}/topology.c

ifeq (${SDEI_SUPPORT}, 1)
BL31_SOURCES		+=	plat/qemu/common/qemu_sdei.c
endif

ifeq (${SPD},spmd)
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c	\
				common/uuid.c				\
				${LIBFDT_SRCS} 				\
				${FDT_WRAPPERS_SOURCES}
endif
endif

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
ifneq (${DECRYPTION_SUPPORT},none)
$(eval $(call TOOL_ADD_IMG,bl32_extra1,--tos-fw-extra1,,$(ENCRYPT_BL32)))
else
$(eval $(call TOOL_ADD_IMG,bl32_extra1,--tos-fw-extra1))
endif
endif
ifneq ($(BL32_EXTRA2),)
ifneq (${DECRYPTION_SUPPORT},none)
$(eval $(call TOOL_ADD_IMG,bl32_extra2,--tos-fw-extra2,,$(ENCRYPT_BL32)))
else
$(eval $(call TOOL_ADD_IMG,bl32_extra2,--tos-fw-extra2))
endif
endif

ifneq ($(QEMU_TB_FW_CONFIG_DTS),)
FDT_SOURCES		+=	${QEMU_TB_FW_CONFIG_DTS}
QEMU_TB_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${QEMU_TB_FW_CONFIG_DTS})).dtb
# Add the TB_FW_CONFIG to FIP
$(eval $(call TOOL_ADD_PAYLOAD,${QEMU_TB_FW_CONFIG},--tb-fw-config,${QEMU_TB_FW_CONFIG}))
endif

ifneq ($(QEMU_TOS_FW_CONFIG_DTS),)
FDT_SOURCES		+=	${QEMU_TOS_FW_CONFIG_DTS}
QEMU_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${QEMU_TOS_FW_CONFIG_DTS})).dtb
# Add the TOS_FW_CONFIG to FIP
$(eval $(call TOOL_ADD_PAYLOAD,${QEMU_TOS_FW_CONFIG},--tos-fw-config,${QEMU_TOS_FW_CONFIG}))
endif

BL32_RAM_LOCATION	:=	tdram
ifeq (${BL32_RAM_LOCATION}, tsram)
  BL32_RAM_LOCATION_ID = SEC_SRAM_ID
  ifeq (${ENABLE_RME},1)
	# Avoid overlap between BL2 and BL32 to ease GPT partition
	$(error "With RME, BL32 must use secure DRAM")
  endif
else ifeq (${BL32_RAM_LOCATION}, tdram)
  BL32_RAM_LOCATION_ID = SEC_DRAM_ID
else
  $(error "Unsupported BL32_RAM_LOCATION value")
endif

# Process flags
$(eval $(call add_define,BL32_RAM_LOCATION_ID))

# Don't have the Linux kernel as a BL33 image by default
ARM_LINUX_KERNEL_AS_BL33	:=	0
$(eval $(call assert_boolean,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))

ARM_PRELOADED_DTB_BASE := PLAT_QEMU_DT_BASE
$(eval $(call add_define,ARM_PRELOADED_DTB_BASE))

qemu_fw.bios: bl1 fip
	$(s)echo "  DD      $@"
	$(q)cp ${BUILD_PLAT}/bl1.bin ${BUILD_PLAT}/$@
	$(q)dd if=${BUILD_PLAT}/fip.bin of=${BUILD_PLAT}/$@ bs=64k seek=4 status=none

qemu_fw.rom: qemu_fw.bios
	$(s)echo "  DD      $@"
	$(q)cp ${BUILD_PLAT}/$^ ${BUILD_PLAT}/$@
	$(q)dd if=/dev/zero of=${BUILD_PLAT}/$@ bs=1M seek=64 count=0 status=none

ifneq (${BL33},)
all: qemu_fw.bios qemu_fw.rom
endif

ifeq (${EL3_EXCEPTION_HANDLING},1)
BL31_SOURCES		+=	plat/common/aarch64/plat_ehf.c
endif
