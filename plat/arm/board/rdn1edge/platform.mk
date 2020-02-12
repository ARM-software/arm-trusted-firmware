#
# Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/arm/css/sgi/sgi-common.mk

RDN1EDGE_BASE		=	plat/arm/board/rdn1edge

PLAT_INCLUDES		+=	-I${RDN1EDGE_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_n1.S

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDN1EDGE_BASE}/rdn1edge_err.c

BL2_SOURCES		+=	${RDN1EDGE_BASE}/rdn1edge_plat.c	\
				${RDN1EDGE_BASE}/rdn1edge_security.c	\
				${RDN1EDGE_BASE}/rdn1edge_err.c		\
				drivers/arm/tzc/tzc_dmc620.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDN1EDGE_BASE}/rdn1edge_plat.c	\
				${RDN1EDGE_BASE}/rdn1edge_topology.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				drivers/arm/gic/v3/gic600_multichip.c	\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${RDN1EDGE_BASE}/rdn1edge_trusted_boot.c
BL2_SOURCES		+=	${RDN1EDGE_BASE}/rdn1edge_trusted_boot.c
endif

# Enable dynamic addition of MMAP regions in BL31
BL31_CFLAGS		+=	-DPLAT_XLAT_TABLES_DYNAMIC=1

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDN1EDGE_BASE}/fdts/${PLAT}_fw_config.dts
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb

# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config))

FDT_SOURCES		+=	${RDN1EDGE_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config))

$(eval $(call CREATE_SEQ,SEQ,2))
ifneq ($(CSS_SGI_CHIP_COUNT),$(filter $(CSS_SGI_CHIP_COUNT),$(SEQ)))
 $(error  "Chip count for RDN1Edge platform should be one of $(SEQ), currently \
   set to ${CSS_SGI_CHIP_COUNT}.")
endif

override CTX_INCLUDE_AARCH32_REGS	:= 0
