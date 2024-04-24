# Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# RD-V1 platform uses GIC-700 which is based on GICv4.1
GIC_ENABLE_V4_EXTN	:=	1

include plat/arm/board/neoverse_rd/common/nrd-common.mk

RDV1_BASE		=	plat/arm/board/neoverse_rd/platform/rdv1

PLAT_INCLUDES		+=	-I${NRD_COMMON_BASE}/include/nrd1/	\
				-I${RDV1_BASE}/include/

NRD_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_v1.S

PLAT_BL_COMMON_SOURCES	+=	${NRD_COMMON_BASE}/nrd_plat1.c

BL1_SOURCES		+=	${NRD_CPU_SOURCES}			\
				${RDV1_BASE}/rdv1_err.c

BL2_SOURCES		+=	${RDV1_BASE}/rdv1_plat.c	\
				${RDV1_BASE}/rdv1_security.c	\
				${RDV1_BASE}/rdv1_err.c		\
				lib/utils/mem_region.c			\
				drivers/arm/tzc/tzc400.c		\
				plat/arm/common/arm_tzc400.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${NRD_CPU_SOURCES}			\
				${RDV1_BASE}/rdv1_plat.c	\
				${RDV1_BASE}/rdv1_topology.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${RDV1_BASE}/rdv1_trusted_boot.c
BL2_SOURCES		+=	${RDV1_BASE}/rdv1_trusted_boot.c
endif

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDV1_BASE}/fdts/${PLAT}_fw_config.dts	\
				${RDV1_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

FDT_SOURCES		+=	${RDV1_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config,${NT_FW_CONFIG}))

override CTX_INCLUDE_AARCH32_REGS	:= 0
override ENABLE_FEAT_AMU		:= 2
override SPMD_SPM_AT_SEL2		:= 0

ifneq ($(NRD_PLATFORM_VARIANT),0)
 $(error "NRD_PLATFORM_VARIANT for RD-V1 should always be 0, \
     currently set to ${NRD_PLATFORM_VARIANT}.")
endif

# Enable the flag since RD-V1 has a system level cache
NEOVERSE_Nx_EXTERNAL_LLC		:=	1
