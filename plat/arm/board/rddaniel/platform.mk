# Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/arm/css/sgi/sgi-common.mk

RDDANIEL_BASE		=	plat/arm/board/rddaniel

PLAT_INCLUDES		+=	-I${RDDANIEL_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_zeus.S

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDDANIEL_BASE}/rddaniel_err.c

BL2_SOURCES		+=	${RDDANIEL_BASE}/rddaniel_plat.c	\
				${RDDANIEL_BASE}/rddaniel_security.c	\
				${RDDANIEL_BASE}/rddaniel_err.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDDANIEL_BASE}/rddaniel_plat.c	\
				${RDDANIEL_BASE}/rddaniel_topology.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDDANIEL_BASE}/fdts/${PLAT}_fw_config.dts
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb

# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config))

FDT_SOURCES		+=	${RDDANIEL_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config))

override CTX_INCLUDE_AARCH32_REGS	:= 0
