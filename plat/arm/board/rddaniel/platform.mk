# Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# RD-Daniel platform uses GIC-Clayton which is based on GICv4.1
GIC_ENABLE_V4_EXTN	:=	1

include plat/arm/css/sgi/sgi-common.mk

RDDANIEL_BASE		=	plat/arm/board/rddaniel

PLAT_INCLUDES		+=	-I${RDDANIEL_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_v1.S

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDDANIEL_BASE}/rddaniel_err.c

BL2_SOURCES		+=	${RDDANIEL_BASE}/rddaniel_plat.c	\
				${RDDANIEL_BASE}/rddaniel_security.c	\
				${RDDANIEL_BASE}/rddaniel_err.c		\
				lib/utils/mem_region.c			\
				drivers/arm/tzc/tzc400.c		\
				plat/arm/common/arm_tzc400.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDDANIEL_BASE}/rddaniel_plat.c	\
				${RDDANIEL_BASE}/rddaniel_topology.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${RDDANIEL_BASE}/rddaniel_trusted_boot.c
BL2_SOURCES		+=	${RDDANIEL_BASE}/rddaniel_trusted_boot.c
endif

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDDANIEL_BASE}/fdts/${PLAT}_fw_config.dts	\
				${RDDANIEL_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

FDT_SOURCES		+=	${RDDANIEL_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config,${NT_FW_CONFIG}))

override CTX_INCLUDE_AARCH32_REGS	:= 0
