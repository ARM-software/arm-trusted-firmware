# Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RD_N2_VARIANTS	:= 0 1 2 3
ifneq ($(CSS_SGI_PLATFORM_VARIANT),\
	$(filter $(CSS_SGI_PLATFORM_VARIANT),$(RD_N2_VARIANTS)))
 $(error "CSS_SGI_PLATFORM_VARIANT for RD-N2 should be 0, 1, 2 or 3, currently \
	set to ${CSS_SGI_PLATFORM_VARIANT}.")
endif

$(eval $(call CREATE_SEQ,SEQ,4))
ifneq ($(CSS_SGI_CHIP_COUNT),$(filter $(CSS_SGI_CHIP_COUNT),$(SEQ)))
 $(error  "Chip count for RD-N2-MC should be either $(SEQ) \
 currently it is set to ${CSS_SGI_CHIP_COUNT}.")
endif

# RD-N2 platform uses GIC-700 which is based on GICv4.1
GIC_ENABLE_V4_EXTN	:=	1
GIC_EXT_INTID		:=	1

#Enable GIC Multichip Extension only for Multichip Platforms
ifeq (${CSS_SGI_PLATFORM_VARIANT}, 2)
GICV3_IMPL_GIC600_MULTICHIP	:=	1
endif

override CSS_SYSTEM_GRACEFUL_RESET	:= 1
override EL3_EXCEPTION_HANDLING		:= 1

include plat/arm/css/sgi/sgi-common.mk

RDN2_BASE		=	plat/arm/board/rdn2

PLAT_INCLUDES		+=	-I${RDN2_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/neoverse_n2.S \
				lib/cpus/aarch64/neoverse_v2.S

PLAT_BL_COMMON_SOURCES	+=	${CSS_ENT_BASE}/sgi_plat_v2.c

BL1_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDN2_BASE}/rdn2_err.c

BL2_SOURCES		+=	${RDN2_BASE}/rdn2_plat.c		\
				${RDN2_BASE}/rdn2_security.c		\
				${RDN2_BASE}/rdn2_err.c			\
				lib/utils/mem_region.c			\
				drivers/arm/tzc/tzc400.c		\
				plat/arm/common/arm_tzc400.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				${RDN2_BASE}/rdn2_plat.c		\
				${RDN2_BASE}/rdn2_topology.c		\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${RDN2_BASE}/rdn2_trusted_boot.c
BL2_SOURCES		+=	${RDN2_BASE}/rdn2_trusted_boot.c
endif

ifeq (${CSS_SGI_PLATFORM_VARIANT}, 2)
BL31_SOURCES	+=	drivers/arm/gic/v3/gic600_multichip.c

# Enable dynamic addition of MMAP regions in BL31
BL31_CFLAGS		+=	-DPLAT_XLAT_TABLES_DYNAMIC
endif

ifeq (${ENABLE_FEAT_RAS}-${HANDLE_EA_EL3_FIRST_NS},1-1)
BL31_SOURCES		+=	${RDN2_BASE}/rdn2_ras.c			\
				${CSS_ENT_BASE}/ras/sgi_ras_common.c	\
				${CSS_ENT_BASE}/ras/sgi_ras_sram.c	\
				${CSS_ENT_BASE}/ras/sgi_ras_cpu.c
endif

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${RDN2_BASE}/fdts/${PLAT}_fw_config.dts	\
				${RDN2_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

FDT_SOURCES		+=	${RDN2_BASE}/fdts/${PLAT}_nt_fw_config.dts
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config))

override CTX_INCLUDE_AARCH32_REGS	:= 0
override ENABLE_FEAT_AMU		:= 1

# Enable N2 CPU errata workarounds
ERRATA_N2_2002655	:=	1
ERRATA_N2_2009478	:=	1
ERRATA_N2_2067956	:=	1
ERRATA_N2_2025414	:=	1
ERRATA_N2_2189731	:=	1
ERRATA_N2_2138956	:=	1
ERRATA_N2_2242415	:=	1
ERRATA_N2_2138958	:=	1
ERRATA_N2_2242400	:=	1
ERRATA_N2_2280757	:=	1
ERRATA_N2_2326639	:=	1
ERRATA_N2_2340933	:=	1
ERRATA_N2_2346952	:=	1
ERRATA_N2_2376738	:=	1
ERRATA_N2_2388450	:=	1
ERRATA_N2_2743014	:=	1
ERRATA_N2_2743089	:=	1
ERRATA_N2_2728475	:=	1
ERRATA_N2_2779511	:=	1
