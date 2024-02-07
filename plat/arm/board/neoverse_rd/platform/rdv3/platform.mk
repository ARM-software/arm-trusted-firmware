# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RD_V3_VARIANTS := 0 1 2
ifneq ($(NRD_PLATFORM_VARIANT),						\
	$(filter $(NRD_PLATFORM_VARIANT),$(RD_V3_VARIANTS)))
	$(error "NRD_PLATFORM_VARIANT for RD-V3 should be 0, 1, or 2,"
	"currently set to ${NRD_PLATFORM_VARIANT}.")
endif

$(eval $(call CREATE_SEQ,SEQ,4))
ifneq ($(NRD_CHIP_COUNT),$(filter $(NRD_CHIP_COUNT),$(SEQ)))
	$(error  "Chip count for RD-V3-MC should be either $(SEQ) \
	currently it is set to ${NRD_CHIP_COUNT}.")
endif

# Build options
# Major and Minor versions
override ARM_ARCH_MAJOR			:= 8
override ARM_ARCH_MINOR			:= 7

# Misc options
override CTX_INCLUDE_AARCH32_REGS	:= 0

ifeq (${PLAT_RESET_TO_BL31}, 1)
# Support for BL31 boot flow
override RESET_TO_BL31			:= 1

# arm_common.mk sets ENABLE_PIE=1, but Makefile blocks PIE for RME
override ENABLE_PIE			:= 0

# Non Trusted Firmware parameters
override ARM_PRELOADED_DTB_BASE		:= 0xF3000000
override ARM_LINUX_KERNEL_AS_BL33	:= 1
override PRELOADED_BL33_BASE		:= 0xE0000000

# These are internal build flags but as of now RESET_TO_BL31 won't work without defining them
override NEED_BL1			:= no
override NEED_BL2			:= no
override NEED_BL32			:= no
endif

# RD-V3 platform uses GIC-700 which is based on GICv4.1
GIC_ENABLE_V4_EXTN			:= 1

# Enable GIC multichip extension only for multichip platforms
ifeq (${NRD_PLATFORM_VARIANT}, 2)
GICV3_IMPL_GIC600_MULTICHIP	:= 1
endif

# RD-V3 uses MHUv3
PLAT_MHU_VERSION := 3

include plat/arm/board/neoverse_rd/common/nrd-common.mk
include drivers/arm/rse/rse_comms.mk
include drivers/auth/mbedtls/mbedtls_common.mk
ifeq (${MEASURED_BOOT},1)
include drivers/measured_boot/rse/rse_measured_boot.mk
endif

RDV3_BASE	=	plat/arm/board/neoverse_rd/platform/rdv3

PLAT_INCLUDES	+=	-I${NRD_COMMON_BASE}/include/nrd3/		\
			-I${RDV3_BASE}/include/			\
			-Iinclude/lib/psa

NRD_CPU_SOURCES	:=	lib/cpus/aarch64/neoverse_v3.S

# Source files for RD-V3 variants
PLAT_BL_COMMON_SOURCES							\
		+=	${NRD_COMMON_BASE}/nrd_plat3.c			\
			${RDV3_BASE}/rdv3_common.c

PLAT_MEASURED_BOOT_SOURCES						\
		:=	${MEASURED_BOOT_SOURCES} 			\
			${RSE_COMMS_SOURCES}				\
			${RDV3_BASE}/rdv3_common_measured_boot.c \
			lib/psa/measured_boot.c

BL1_SOURCES	+=	${NRD_CPU_SOURCES}				\
			${RDV3_BASE}/rdv3_err.c		\
			${RDV3_BASE}/rdv3_mhuv3.c
ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES	+=	${RDV3_BASE}/rdv3_trusted_boot.c
endif
ifeq (${MEASURED_BOOT},1)
BL1_SOURCES	+=	${PLAT_MEASURED_BOOT_SOURCES}			\
			${RDV3_BASE}/rdv3_bl1_measured_boot.c
endif

BL2_SOURCES	+=	${RDV3_BASE}/rdv3_bl2_setup.c		\
			${RDV3_BASE}/rdv3_err.c		\
			${RDV3_BASE}/rdv3_mhuv3.c		\
			${RDV3_BASE}/rdv3_security.c		\
			lib/utils/mem_region.c				\
			plat/arm/common/arm_nor_psci_mem_protect.c
ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL2_SOURCES	+=	${RDV3_BASE}/rdv3_trusted_boot.c
endif
ifeq (${MEASURED_BOOT},1)
BL2_SOURCES	+=	${PLAT_MEASURED_BOOT_SOURCES}			\
			${RDV3_BASE}/rdv3_bl2_measured_boot.c
endif

ifeq (${PLAT_RESET_TO_BL31}, 1)
BL31_SOURCES	+=	${RDV3_BASE}/rdv3_security.c
endif

BL31_SOURCES	+=	${NRD_CPU_SOURCES}				\
			${MBEDTLS_SOURCES}				\
			${RSE_COMMS_SOURCES}				\
			${RDV3_BASE}/rdv3_bl31_setup.c	\
			${RDV3_BASE}/rdv3_mhuv3.c		\
			${RDV3_BASE}/rdv3_topology.c		\
			${RDV3_BASE}/rdv3_plat_attest_token.c	\
			${RDV3_BASE}/rdv3_realm_attest_key.c	\
			drivers/arm/smmu/smmu_v3.c			\
			drivers/cfi/v2m/v2m_flash.c			\
			lib/psa/cca_attestation.c			\
			lib/psa/delegated_attestation.c			\
			lib/utils/mem_region.c				\
			plat/arm/common/arm_dyn_cfg.c			\
			plat/arm/common/arm_nor_psci_mem_protect.c
ifeq (${NRD_PLATFORM_VARIANT}, 2)
BL31_SOURCES	+=	drivers/arm/gic/v3/gic600_multichip.c
endif

# XLAT options for RD-V3 variants
BL31_CFLAGS	+=      -DPLAT_XLAT_TABLES_DYNAMIC
BL2_CFLAGS	+=      -DPLAT_XLAT_TABLES_DYNAMIC

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES	+=	${RDV3_BASE}/fdts/${PLAT}_fw_config.dts	\
			${RDV3_BASE}/fdts/${PLAT}_tb_fw_config.dts \
			${RDV3_BASE}/fdts/${PLAT}_nt_fw_config.dts

FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb
NT_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))
# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config))

# Features for RD-V3 variants
override ENABLE_FEAT_MPAM	:= 2
override ENABLE_FEAT_AMU	:= 2
override ENABLE_SVE_FOR_SWD	:= 1
override ENABLE_SVE_FOR_NS	:= 2
override ENABLE_FEAT_MTE2	:= 2
override CTX_INCLUDE_SVE_REGS	:= 1
