#
# Copyright 2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SoC-specific build parameters
SOC		:=	ls1088a
PLAT_PATH	:=	plat/nxp
PLAT_COMMON_PATH:=	plat/nxp/common
PLAT_DRIVERS_PATH:=	drivers/nxp
PLAT_SOC_PATH	:=	${PLAT_PATH}/soc-${SOC}
BOARD_PATH	:=	${PLAT_SOC_PATH}/${BOARD}

# Separate BL2 NOLOAD region (.bss, stack, page tables). need to
# define BL2_NOLOAD_START and BL2_NOLOAD_LIMIT
SEPARATE_BL2_NOLOAD_REGION	:= 1

# get SoC-specific defnitions
include ${PLAT_SOC_PATH}/soc.def
include ${PLAT_COMMON_PATH}/plat_make_helper/soc_common_def.mk
include ${PLAT_COMMON_PATH}/plat_make_helper/plat_build_macros.mk

# For Security Features
DISABLE_FUSE_WRITE	:= 1
ifeq (${TRUSTED_BOARD_BOOT}, 1)
ifeq (${GENERATE_COT},1)
# Save Keys to be used by DDR FIP image
SAVE_KEYS=1
endif
$(eval $(call SET_NXP_MAKE_FLAG,SMMU_NEEDED,BL2))
$(eval $(call SET_NXP_MAKE_FLAG,SFP_NEEDED,BL2))
$(eval $(call SET_NXP_MAKE_FLAG,SNVS_NEEDED,BL2))
# Used by create_pbl tool to
# create bl2_<boot_mode>_sec.pbl image
SECURE_BOOT	:= yes
endif
$(eval $(call SET_NXP_MAKE_FLAG,CRYPTO_NEEDED,BL_COMM))

# Selecting Drivers for SoC
$(eval $(call SET_NXP_MAKE_FLAG,DCFG_NEEDED,BL_COMM))
$(eval $(call SET_NXP_MAKE_FLAG,TIMER_NEEDED,BL_COMM))
$(eval $(call SET_NXP_MAKE_FLAG,INTERCONNECT_NEEDED,BL_COMM))
$(eval $(call SET_NXP_MAKE_FLAG,GIC_NEEDED,BL31))
$(eval $(call SET_NXP_MAKE_FLAG,CONSOLE_NEEDED,BL_COMM))
$(eval $(call SET_NXP_MAKE_FLAG,PMU_NEEDED,BL_COMM))
$(eval $(call SET_NXP_MAKE_FLAG,DDR_DRIVER_NEEDED,BL2))
$(eval $(call SET_NXP_MAKE_FLAG,TZASC_NEEDED,BL2))
$(eval $(call SET_NXP_MAKE_FLAG,I2C_NEEDED,BL2))
$(eval $(call SET_NXP_MAKE_FLAG,IMG_LOADR_NEEDED,BL2))

# Selecting PSCI & SIP_SVC support
$(eval $(call SET_NXP_MAKE_FLAG,PSCI_NEEDED,BL31))
$(eval $(call SET_NXP_MAKE_FLAG,SIPSVC_NEEDED,BL31))


# Adding SoC specific files
include ${PLAT_COMMON_PATH}/soc_errata/errata.mk

PLAT_INCLUDES	+=	-I${PLAT_COMMON_PATH}/include/default\
			-I${BOARD_PATH}\
			-I${PLAT_COMMON_PATH}/include/default/ch_${CHASSIS}\
			-I${PLAT_COMMON_PATH}/soc_errata\
			-I${PLAT_COMMON_PATH}/include\
			-I${PLAT_SOC_PATH}/include

ifeq (${SECURE_BOOT},yes)
include ${PLAT_COMMON_PATH}/tbbr/tbbr.mk
endif

ifeq (${PSCI_NEEDED}, yes)
include ${PLAT_COMMON_PATH}/psci/psci.mk
endif

ifeq (${SIPSVC_NEEDED}, yes)
include ${PLAT_COMMON_PATH}/sip_svc/sipsvc.mk
endif

# for fuse-fip & fuse-programming
ifeq (${FUSE_PROG}, 1)
include ${PLAT_COMMON_PATH}/fip_handler/fuse_fip/fuse.mk
endif

ifeq (${IMG_LOADR_NEEDED},yes)
include $(PLAT_COMMON_PATH)/img_loadr/img_loadr.mk
endif

# Adding source files for the above selected drivers.
include ${PLAT_DRIVERS_PATH}/drivers.mk

PLAT_BL_COMMON_SOURCES	+=	${PLAT_COMMON_PATH}/$(ARCH)/ls_helpers.S\
				${PLAT_SOC_PATH}/${ARCH}/${SOC}_helpers.S\
				${PLAT_SOC_PATH}/soc.c

BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/${SOC}.S\
			${PSCI_SOURCES}\
			${SIPSVC_SOURCES}\
			${PLAT_COMMON_PATH}/$(ARCH)/bl31_data.S

ifeq (${TEST_BL31}, 1)
BL31_SOURCES	+=	${PLAT_SOC_PATH}/$(ARCH)/bootmain64.S  \
			${PLAT_SOC_PATH}/$(ARCH)/nonboot64.S
endif

BL2_SOURCES		+=	${DDR_CNTLR_SOURCES}\
				${TBBR_SOURCES}\
				${FUSE_SOURCES}

# Adding TFA setup files
include ${PLAT_PATH}/common/setup/common.mk
