#
# Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/arm/css/sgi/sgi-common.mk

SGI575_BASE		=	plat/arm/board/sgi575

PLAT_INCLUDES		+=	-I${SGI575_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/cortex_a75.S

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${SGI575_BASE}/sgi575_err.c

BL2_SOURCES		+=	${SGI575_BASE}/sgi575_plat.c		\
				${SGI575_BASE}/sgi575_security.c	\
				${SGI575_BASE}/sgi575_err.c		\
				drivers/arm/tzc/tzc_dmc620.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${SGI575_BASE}/sgi575_plat.c		\
				${SGI575_BASE}/sgi575_topology.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${SGI575_BASE}/sgi575_trusted_boot.c
BL2_SOURCES		+=	${SGI575_BASE}/sgi575_trusted_boot.c
endif

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${SGI575_BASE}/fdts/${PLAT}_fw_config.dts	\
				${SGI575_BASE}/fdts/${PLAT}_tb_fw_config.dts

FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

FDT_SOURCES		+=	${SGI575_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config,${NT_FW_CONFIG}))

ifneq ($(CSS_SGI_CHIP_COUNT),1)
 $(error  "Chip count for SGI575 should be 1, currently set to \
   ${CSS_SGI_CHIP_COUNT}.")
endif
