#
# Copyright (c) 2019-2023, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_QEMU_PATH		:=	plat/qemu/qemu_sbsa
PLAT_QEMU_COMMON_PATH	:=	plat/qemu/common

CRASH_REPORTING		:= 1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 0

SEPARATE_CODE_AND_RODATA := 1
ENABLE_STACK_PROTECTOR	 := 0

ifeq (${SPM_MM},1)
NEED_BL32		:=	yes
EL3_EXCEPTION_HANDLING	:=	1
endif

include plat/qemu/common/common.mk

# Enable new version of image loading on QEMU platforms
LOAD_IMAGE_V2		:=	1

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

ifeq ($(add-lib-optee),yes)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,QEMU_LOAD_BL32))
endif

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

QEMU_GIC_SOURCES	:=	${GICV3_SOURCES}				\
				plat/common/plat_gicv3.c

BL31_SOURCES		+=	${PLAT_QEMU_PATH}/sbsa_gic.c 			\
				${PLAT_QEMU_PATH}/sbsa_platform.c		\
				${PLAT_QEMU_PATH}/sbsa_pm.c			\
				${PLAT_QEMU_PATH}/sbsa_sip_svc.c		\
				${PLAT_QEMU_PATH}/sbsa_topology.c

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

ifeq (${SPM_MM},1)
	BL31_SOURCES		+=	${PLAT_QEMU_COMMON_PATH}/qemu_spm.c
endif

ifeq (${SPD},spmd)
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c	\
				common/uuid.c				\
				${LIBFDT_SRCS}
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

# Use known base for UEFI if not given from command line
# By default BL33 is at FLASH1 base
PRELOADED_BL33_BASE	?= 0x10000000

# Qemu SBSA plafrom only support SEC_SRAM
BL32_RAM_LOCATION_ID	= SEC_SRAM_ID
$(eval $(call add_define,BL32_RAM_LOCATION_ID))

# Don't have the Linux kernel as a BL33 image by default
ARM_LINUX_KERNEL_AS_BL33	:=	0
$(eval $(call assert_boolean,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))

ARM_PRELOADED_DTB_BASE := PLAT_QEMU_DRAM0_BASE
$(eval $(call add_define,ARM_PRELOADED_DTB_BASE))
