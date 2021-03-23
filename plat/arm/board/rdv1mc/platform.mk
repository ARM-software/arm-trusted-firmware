# Copyright (c) 2020-2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable GICv4 extension with multichip driver
GIC_ENABLE_V4_EXTN		:=	1
GICV3_IMPL_GIC600_MULTICHIP	:=	1

include plat/arm/css/sgi/sgi-common.mk

RDV1MC_BASE	=	plat/arm/board/rdv1mc

PLAT_INCLUDES		+=	-I${RDV1MC_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_v1.S

PLAT_BL_COMMON_SOURCES	+=	${CSS_ENT_BASE}/sgi_plat.c

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDV1MC_BASE}/rdv1mc_err.c

BL2_SOURCES		+=	${RDV1MC_BASE}/rdv1mc_plat.c	\
				${RDV1MC_BASE}/rdv1mc_security.c	\
				${RDV1MC_BASE}/rdv1mc_err.c	\
				drivers/arm/tzc/tzc400.c	\
				plat/arm/common/arm_tzc400.c	\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDV1MC_BASE}/rdv1mc_plat.c	\
				${RDV1MC_BASE}/rdv1mc_topology.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				drivers/arm/gic/v3/gic600_multichip.c	\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${RDV1MC_BASE}/rdv1mc_trusted_boot.c
BL2_SOURCES		+=	${RDV1MC_BASE}/rdv1mc_trusted_boot.c
endif

# Enable dynamic addition of MMAP regions in BL31
BL31_CFLAGS		+=	-DPLAT_XLAT_TABLES_DYNAMIC

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDV1MC_BASE}/fdts/${PLAT}_fw_config.dts	\
				${RDV1MC_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

$(eval $(call CREATE_SEQ,SEQ,4))
ifneq ($(CSS_SGI_CHIP_COUNT),$(filter $(CSS_SGI_CHIP_COUNT),$(SEQ)))
 $(error  "Chip count for RD-V1-MC should be either $(SEQ) \
 currently it is set to ${CSS_SGI_CHIP_COUNT}.")
endif

FDT_SOURCES		+=	${RDV1MC_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config,${NT_FW_CONFIG}))

override CTX_INCLUDE_AARCH32_REGS	:= 0
override ENABLE_AMU			:= 1

ifneq ($(CSS_SGI_PLATFORM_VARIANT),0)
 $(error "CSS_SGI_PLATFORM_VARIANT for RD-V1-MC should always be 0, \
     currently set to ${CSS_SGI_PLATFORM_VARIANT}.")
endif
