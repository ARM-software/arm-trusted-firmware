# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

# DPU with SCMI may not necessarily work, so allow its independence
BASE_DPU_USE_SCMI_CLK		:=	1
# SCMI power domain control enable
BASE_SCMI_PD_CTRL_EN		:=	1

# System setup
CSS_USE_SCMI_SDS_DRIVER		:=	1
HW_ASSISTED_COHERENCY		:=	1
USE_COHERENT_MEM		:=	0
USE_GIC_DRIVER			:=	3
USE_DSU_DRIVER			:=	1
GIC_ENABLE_V4_EXTN		:=	1
GICV3_SUPPORT_GIC600		:=	1
NEED_BL2U			:=	no
ARM_PLAT_MT			:=	1

# CPU setup
ARM_ARCH_MAJOR			:=	8
ARM_ARCH_MINOR			:=	1
ENABLE_FEAT_RNG_TRAP		:=	2

# Set all supported features with runtime detection
include make_helpers/all_features.mk

CTX_INCLUDE_AARCH32_REGS	:=	0

ifeq (${SPD},spmd)
	SPMD_SPM_AT_SEL2	:=	1
	CTX_INCLUDE_PAUTH_REGS	:=	1
endif

TRNG_SUPPORT			:=	1

# Base RESOLUTION - LIST OF VALID OPTIONS (this impacts only FVP)
BASE_RESOLUTION_OPTIONS		:= 	640x480p60 \
					1920x1080p60
# Set default to the 640x480p60 resolution mode
BASE_RESOLUTION ?= $(firstword $(BASE_RESOLUTION_OPTIONS))

ifeq ($(filter ${BASE_RESOLUTION}, ${BASE_RESOLUTION_OPTIONS}),)
        $(error BASE_RESOLUTION is ${BASE_RESOLUTION}, it must be: ${BASE_RESOLUTION_OPTIONS})
endif


# Use simple panel instead of vencoder with DPU
BASE_DPU_USE_SIMPLE_PANEL := 0

$(call add_defines, \
	BASE_TARGET_DISTRO_$(call uppercase,${BASE_TARGET_DISTRO}) \
	BASE_RESOLUTION_$(call uppercase,${BASE_RESOLUTION}) \
	BASE_DPU_USE_SCMI_CLK \
	BASE_SCMI_PD_CTRL_EN \
	BASE_DPU_USE_SIMPLE_PANEL \
)

CSS_LOAD_SCP_IMAGES	:=	1

PLAT_MHU		:= MHUv3

BASE_PATH	=	plat/arm/board/basev2

PLAT_INCLUDES		+=	-I${BASE_PATH}/include/ \
				-I${BASE_PATH}/fdts/


# prevent CME related wakups
CSS_ERRATA_SME_POWER_DOWN := 1

WORKAROUND_CVE_2025_10263 := 1
WORKAROUND_CVE_2026_0995 := 1

BASE_CPU_SOURCES	+=	lib/cpus/aarch64/aem_generic.S		\
					lib/cpus/aarch64/c1_pro.S		\
					lib/cpus/aarch64/c1_nano.S		\
					lib/cpus/aarch64/c1_ultra.S		\
					lib/cpus/aarch64/c1_premium.S	\
					lib/cpus/aarch64/c2_ultra.S		\
					lib/cpus/aarch64/caddo.S		\
					lib/cpus/aarch64/rosillo.S		\
					lib/cpus/aarch64/veymont.S		\
					lib/cpus/aarch64/dionysus.S		\
					lib/cpus/aarch64/venom.S		\
					lib/cpus/aarch64/lsc25_p_core.S		\
					lib/cpus/aarch64/lsc25_e_core.S


INTERCONNECT_SOURCES	:=	plat/arm/common/arm_ni.c

PLAT_BL_COMMON_SOURCES	+=	${BASE_PATH}/base_plat.c	\
				${BASE_PATH}/include/base_helpers.S


ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	${BASE_PATH}/base_stack_protector.c
endif

BL1_SOURCES		+=	${INTERCONNECT_SOURCES}	\
				${BASE_CPU_SOURCES}	\
				${BASE_PATH}/base_trusted_boot.c	\
				${BASE_PATH}/base_bl1_setup.c \
				${BASE_PATH}/base_security.c	\
				${BASE_PATH}/base_err.c	\
				drivers/arm/sbsa/sbsa.c

BL2_SOURCES		+=	${BASE_PATH}/base_security.c	\
				${BASE_PATH}/base_err.c		\
				${BASE_PATH}/base_trusted_boot.c		\
				${BASE_PATH}/base_bl2_setup.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${INTERCONNECT_SOURCES}	\
				${BASE_CPU_SOURCES}	\
				${BASE_PATH}/base_bl31_setup.c	\
				${BASE_PATH}/base_topology.c	\
				${BASE_PATH}/base_security.c	\
				lib/fconf/fconf.c			\
				lib/fconf/fconf_dyn_cfg_getter.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c	\
				drivers/arm/sbsa/sbsa.c

ifeq (${WORKAROUND_CVE_2026_0995},1)
BL31_SOURCES		+=	lib/cpus/helpers/aarch64/c1_pro_pubsub.c	\
				${CPU_SVC_SRCS}
endif

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${BASE_PATH}/fdts/${PLAT}_fw_config.dts	\
				${BASE_PATH}/fdts/${PLAT}_tb_fw_config.dts \
				${BASE_PATH}/fdts/${PLAT}_nt_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb
FVP_NT_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))
# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_NT_FW_CONFIG},--nt-fw-config,${FVP_NT_FW_CONFIG}))

ifeq (${SPD},spmd)
ifeq ($(ARM_SPMC_MANIFEST_DTS),)
ARM_SPMC_MANIFEST_DTS	:=	${BASE_PATH}/fdts/${PLAT}_spmc_test_manifest.dts
endif

FDT_SOURCES		+=	${ARM_SPMC_MANIFEST_DTS}
BASE_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${ARM_SPMC_MANIFEST_DTS})).dtb

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${BASE_TOS_FW_CONFIG},--tos-fw-config,${BASE_TOS_FW_CONFIG}))
endif

#Device tree
BASE_HW_CONFIG_DTS	:=	fdts/basev2.dts
FDT_SOURCES		+=	${BASE_HW_CONFIG_DTS}
$(eval BASE_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb,$(BASE_HW_CONFIG_DTS)))

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${BASE_HW_CONFIG},--hw-config,${BASE_HW_CONFIG}))

$(info Including rse_comms.mk)
include drivers/arm/rse/rse_comms.mk

BL1_SOURCES	+=	${RSE_COMMS_SOURCES}
BL2_SOURCES	+=	${RSE_COMMS_SOURCES}
BL31_SOURCES	+=	${RSE_COMMS_SOURCES}

# Include Measured Boot makefile before any Crypto library makefile.
# Crypto library makefile may need default definitions of Measured Boot build
# flags present in Measured Boot makefile.
ifeq (${MEASURED_BOOT},1)
    ifeq (${DICE_PROTECTION_ENVIRONMENT},1)
        $(info Including qcbor.mk)
        include drivers/measured_boot/rse/qcbor.mk
        $(info Including dice_prot_env.mk)
        include drivers/measured_boot/rse/dice_prot_env.mk

	BL1_SOURCES	+=	${QCBOR_SOURCES} \
				${DPE_SOURCES} \
				plat/arm/board/basev2/base_common_dpe.c \
				plat/arm/board/basev2/base_bl1_dpe.c \
				lib/psa/dice_protection_environment.c \
				drivers/arm/css/sds/sds.c \
				drivers/delay_timer/delay_timer.c \
				drivers/delay_timer/generic_delay_timer.c

	BL2_SOURCES	+=	${QCBOR_SOURCES} \
				${DPE_SOURCES} \
				plat/arm/board/basev2/base_common_dpe.c \
				plat/arm/board/basev2/base_bl2_dpe.c \
				lib/psa/dice_protection_environment.c

	PLAT_INCLUDES	+=	-I${QCBOR_INCLUDES} \
				-Iinclude/lib/dice
    else
        $(info Including rse_measured_boot.mk)
        include drivers/measured_boot/rse/rse_measured_boot.mk

	BL1_SOURCES	+=	${MEASURED_BOOT_SOURCES} \
				plat/arm/board/basev2/base_common_measured_boot.c \
				plat/arm/board/basev2/base_bl1_measured_boot.c \
				lib/psa/measured_boot.c

	BL2_SOURCES		+=	${MEASURED_BOOT_SOURCES} \
				plat/arm/board/basev2/base_common_measured_boot.c \
				plat/arm/board/basev2/base_bl2_measured_boot.c \
				lib/psa/measured_boot.c
    endif
endif

BL31_SOURCES	+=	plat/arm/board/basev2/base_trng.c
# PSA lib is dependent on measured boot or trusted boot
ifneq ($(filter 1,${MEASURED_BOOT} ${TRUSTED_BOARD_BOOT}),)
	BL31_SOURCES    +=      lib/psa/rse_platform.c
endif


ifneq (${ENABLE_FEAT_RNG_TRAP},0)
	BL31_SOURCES	+=	plat/arm/board/basev2/base_rng_trap.c
endif

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk
