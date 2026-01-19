# Copyright (c) 2025-2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# RD-Aspen platform.

RDASPEN_BASE		 =	plat/arm/board/automotive_rd/platform/rdaspen
RDASPEN_CPU_SOURCES	:=	lib/cpus/aarch64/cortex_a720_ae.S

PLAT_INCLUDES		+=	-I${RDASPEN_BASE}/include/ 	\
				-I${RDASPEN_BASE}/ras/include/	\
				-Idrivers/arm/mhu

override ARM_FW_CONFIG_LOAD_ENABLE		:=	1
override ARM_PLAT_MT				:=	1
override ARM_PLAT_PROVIDES_BL2_MEM_PARAMS	:=	1
override ARM_RECOM_STATE_ID_ENC			:=	1
override CSS_LOAD_SCP_IMAGES			:=	0
override CTX_INCLUDE_AARCH32_REGS		:=	0
override NEED_BL1				:=	0
override NEED_BL2U				:=	0
override PSCI_EXTENDED_STATE_ID			:=	1

# SVE related flags
override CTX_INCLUDE_FPREGS			:=	1
override CTX_INCLUDE_SVE_REGS			:=	1
override ENABLE_SVE_FOR_NS			:=	1
override ENABLE_SVE_FOR_SWD			:=	1

ARM_ARCH_MAJOR					:=	9
ARM_ARCH_MINOR					:=	2
CSS_USE_SCMI_SDS_DRIVER				:=	1
# Enable runtime feature detection for emulation environments
ENABLE_FEAT_AMU					:=	2
ENABLE_FEAT_ECV					:=	2
ENABLE_FEAT_FGT					:=	2
ENABLE_FEAT_MTE2				:=	2
ENABLE_MPAM_FOR_LOWER_ELS			:=	1
GIC_ENABLE_V4_EXTN				:=	1
GICV3_SUPPORT_GIC600				:=	1
HW_ASSISTED_COHERENCY				:=	1
NEED_BL32					?=	yes
PLAT_MHU_VERSION				:=	3
RESET_TO_BL2					:=	1
SVE_VECTOR_LEN					:=	128
USE_GIC_DRIVER					:=	3
USE_COHERENT_MEM				:=	0
# Enable RSE communication support in BL2. This is required by measured boot,
# and may also be enabled independently when BL2 needs to communicate with RSE.
RDASPEN_ENABLE_RSE_COMMS_BL2			?=	0

PLAT_MHU 					:= 	MHUv3
RSE_COMMS_BOOT_MK				:= 	drivers/arm/rse/rse_comms.mk

# Enable the DSU driver and save DSU PMU registers on cluster off
# and restore them on cluster on
USE_DSU_DRIVER				:=	1
PRESERVE_DSU_PMU_REGS			:=	1

# RAS Enablement
ENABLE_FEAT_RAS				:= 	1
HANDLE_EA_EL3_FIRST_NS			:=	1
EL3_EXCEPTION_HANDLING			:=	1
FAULT_INJECTION_SUPPORT			?=	1


# ERRATA
ERRATA_A720_AE_3699562			:=	1

include ${RSE_COMMS_BOOT_MK}

PLAT_BL_COMMON_SOURCES	+=	${RDASPEN_BASE}/rdaspen_plat.c	\
				${RDASPEN_BASE}/include/rdaspen_helpers.S

BL2_SOURCES	+=	${RDASPEN_CPU_SOURCES}	\
			${RDASPEN_BASE}/rdaspen_err.c	\
			${RDASPEN_BASE}/rdaspen_bl2_mem_params_desc.c	\
			lib/utils/mem_region.c	\
			drivers/arm/sbsa/sbsa.c	\
			plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES	+=	${RDASPEN_CPU_SOURCES}	\
			${RDASPEN_BASE}/rdaspen_bl31_setup.c	\
			${RDASPEN_BASE}/rdaspen_topology.c	\
			${RDASPEN_BASE}/ras/rdaspen_ras.c	\
			${RDASPEN_BASE}/ras/rdaspen_ras_helpers.S \
			drivers/cfi/v2m/v2m_flash.c		\
			lib/utils/mem_region.c	\
			plat/arm/common/arm_nor_psci_mem_protect.c \
			drivers/arm/dsu/dsu.c

ifeq ($(PLAT_MHU),MHUv3)
BL31_SOURCES	+=	$(addprefix drivers/arm/mhu/,		\
				    mhu_v3_x.c			\
				    mhu_wrapper_v3_x.c		\
				)
endif

ifeq ($(ENABLE_FEAT_RAS),1)
ifeq ($(HANDLE_EA_EL3_FIRST_NS),1)
BL31_SOURCES 	+=	$(RDASPEN_BASE)/../common/cper.c
PLAT_INCLUDES	+=	-I$(RDASPEN_BASE)/../common/include
endif
endif

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL2_SOURCES	+=	${RDASPEN_BASE}/rdaspen_trusted_board_boot.c
FIP_BL2_ARGS	:=	tb-fw
$(eval $(call TOOL_ADD_PAYLOAD,${BUILD_PLAT}/tb_fw.crt,--tb-fw-cert))
endif

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES	+=	${RDASPEN_BASE}/fdts/${PLAT}_fw_config.dts	\
			fdts/${PLAT}.dts \
			${RDASPEN_BASE}/fdts/${PLAT}_optee_spmc_manifest.dts

FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
HW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}.dtb
TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_optee_spmc_manifest.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${HW_CONFIG},--hw-config,${HW_CONFIG}))

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
ifeq (${NEED_BL32},yes)
$(eval $(call TOOL_ADD_PAYLOAD,${TOS_FW_CONFIG},--tos-fw-config,${TOS_FW_CONFIG}))
endif

# Using graceful flag to send SCMI system power set command
# the css_scp_system_off() use forceful flag by default
$(eval $(call add_define_val,CSS_SCP_SYSTEM_OFF_GRACEFUL,1))

ifdef PLATFORM_CORE_COUNT
# Pass PLATFORM_CORE_COUNT to the build system.
$(eval $(call add_define,PLATFORM_CORE_COUNT))
endif

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk

# Exclude arm common board_arm_helpers.S to override the default
# plat_report_exception, to get rid of legacy V2M registers
PLAT_BL_COMMON_SOURCES	:= $(filter-out						\
			     plat/arm/board/common/${ARCH}/board_arm_helpers.S, \
			     $(PLAT_BL_COMMON_SOURCES))

# Include Measured Boot makefile and source
ifeq (${MEASURED_BOOT},1)
	MEASURED_BOOT_MK	:= drivers/measured_boot/rse/rse_measured_boot.mk
	include ${MEASURED_BOOT_MK}
	MEASURED_BOOT_SOURCES	+= lib/psa/measured_boot.c
	RDASPEN_ENABLE_RSE_COMMS_BL2	:= 1
	BL2_SOURCES		+= ${MEASURED_BOOT_SOURCES}
	PLAT_BL_COMMON_SOURCES	+= ${RDASPEN_BASE}/rdaspen_measured_boot.c
endif

ifeq ($(RDASPEN_ENABLE_RSE_COMMS_BL2),1)
BL2_SOURCES	+=	${RDASPEN_BASE}/rdaspen_rse_comms.c	\
			${RSE_COMMS_SOURCES}
endif
