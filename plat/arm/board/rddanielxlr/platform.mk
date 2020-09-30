# Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable GICv4 extension with multichip driver
GIC_ENABLE_V4_EXTN		:=	1
GICV3_IMPL_GIC600_MULTICHIP	:=	1

include plat/arm/css/sgi/sgi-common.mk

RDDANIELXLR_BASE	=	plat/arm/board/rddanielxlr

PLAT_INCLUDES		+=	-I${RDDANIELXLR_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_v1.S

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDDANIELXLR_BASE}/rddanielxlr_err.c

BL2_SOURCES		+=	${RDDANIELXLR_BASE}/rddanielxlr_plat.c	\
				${RDDANIELXLR_BASE}/rddanielxlr_security.c	\
				${RDDANIELXLR_BASE}/rddanielxlr_err.c	\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDDANIELXLR_BASE}/rddanielxlr_plat.c	\
				${RDDANIELXLR_BASE}/rddanielxlr_topology.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				drivers/arm/gic/v3/gic600_multichip.c	\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${RDDANIELXLR_BASE}/rddanielxlr_trusted_boot.c
BL2_SOURCES		+=	${RDDANIELXLR_BASE}/rddanielxlr_trusted_boot.c
endif

# Enable dynamic addition of MMAP regions in BL31
BL31_CFLAGS		+=	-DPLAT_XLAT_TABLES_DYNAMIC

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDDANIELXLR_BASE}/fdts/${PLAT}_fw_config.dts	\
				${RDDANIELXLR_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

$(eval $(call CREATE_SEQ,SEQ,4))
ifneq ($(CSS_SGI_CHIP_COUNT),$(filter $(CSS_SGI_CHIP_COUNT),$(SEQ)))
 $(error  "Chip count for RD-Daniel Config-XLR should be either $(SEQ) \
 currently it is set to ${CSS_SGI_CHIP_COUNT}.")
endif

FDT_SOURCES		+=	${RDDANIELXLR_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config,${NT_FW_CONFIG}))

override CTX_INCLUDE_AARCH32_REGS	:= 0
