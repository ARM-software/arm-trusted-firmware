# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

ifeq ($(filter ${TARGET_PLATFORM}, 0 1),)
        $(error TARGET_PLATFORM must be 0 or 1)
endif

CSS_LOAD_SCP_IMAGES	:=	1

CSS_USE_SCMI_SDS_DRIVER	:=	1

RAS_EXTENSION		:=	0

SDEI_SUPPORT		:=	0

EL3_EXCEPTION_HANDLING	:=	0

HANDLE_EA_EL3_FIRST	:=	0

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM	:=	0

GIC_ENABLE_V4_EXTN	:=      1

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1

# Enable SVE
ENABLE_SVE_FOR_NS	:=	1
ENABLE_SVE_FOR_SWD	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

ENT_GIC_SOURCES		:=	${GICV3_SOURCES}		\
				plat/common/plat_gicv3.c	\
				plat/arm/common/arm_gicv3.c

override NEED_BL2U	:=	no

override ARM_PLAT_MT	:=	1

TC_BASE	=	plat/arm/board/tc

PLAT_INCLUDES		+=	-I${TC_BASE}/include/

# Common CPU libraries
TC_CPU_SOURCES	:=	lib/cpus/aarch64/cortex_a510.S

# CPU libraries for TARGET_PLATFORM=0
ifeq (${TARGET_PLATFORM}, 0)
TC_CPU_SOURCES	+=	lib/cpus/aarch64/cortex_a710.S \
			lib/cpus/aarch64/cortex_x2.S
endif

# CPU libraries for TARGET_PLATFORM=1
ifeq (${TARGET_PLATFORM}, 1)
TC_CPU_SOURCES	+=	lib/cpus/aarch64/cortex_makalu.S \
			lib/cpus/aarch64/cortex_makalu_elp_arm.S
endif

INTERCONNECT_SOURCES	:=	${TC_BASE}/tc_interconnect.c

PLAT_BL_COMMON_SOURCES	+=	${TC_BASE}/tc_plat.c	\
				${TC_BASE}/include/tc_helpers.S

BL1_SOURCES		+=	${INTERCONNECT_SOURCES}	\
				${TC_CPU_SOURCES}	\
				${TC_BASE}/tc_trusted_boot.c	\
				${TC_BASE}/tc_err.c	\
				drivers/arm/sbsa/sbsa.c


BL2_SOURCES		+=	${TC_BASE}/tc_security.c	\
				${TC_BASE}/tc_err.c		\
				${TC_BASE}/tc_trusted_boot.c		\
				${TC_BASE}/tc_bl2_setup.c		\
				lib/utils/mem_region.c			\
				drivers/arm/tzc/tzc400.c		\
				plat/arm/common/arm_tzc400.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${INTERCONNECT_SOURCES}	\
				${TC_CPU_SOURCES}	\
				${ENT_GIC_SOURCES}			\
				${TC_BASE}/tc_bl31_setup.c	\
				${TC_BASE}/tc_topology.c	\
				lib/fconf/fconf.c			\
				lib/fconf/fconf_dyn_cfg_getter.c	\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${TC_BASE}/fdts/${PLAT}_fw_config.dts	\
				${TC_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

ifeq (${SPD},spmd)
ifeq ($(ARM_SPMC_MANIFEST_DTS),)
ARM_SPMC_MANIFEST_DTS	:=	${TC_BASE}/fdts/${PLAT}_spmc_manifest.dts
endif

FDT_SOURCES		+=	${ARM_SPMC_MANIFEST_DTS}
TC_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${ARM_SPMC_MANIFEST_DTS})).dtb

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TC_TOS_FW_CONFIG},--tos-fw-config,${TC_TOS_FW_CONFIG}))
endif

#Device tree
TC_HW_CONFIG_DTS	:=	fdts/tc.dts
TC_HW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}.dtb
FDT_SOURCES		+=	${TC_HW_CONFIG_DTS}
$(eval TC_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb,$(TC_HW_CONFIG_DTS)))

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TC_HW_CONFIG},--hw-config,${TC_HW_CONFIG}))

override CTX_INCLUDE_AARCH32_REGS	:= 0

override CTX_INCLUDE_PAUTH_REGS	:= 1

override ENABLE_SPE_FOR_LOWER_ELS	:= 0

override ENABLE_AMU := 1
override ENABLE_AMU_AUXILIARY_COUNTERS := 1
override ENABLE_AMU_FCONF := 1

override ENABLE_MPMM := 1
override ENABLE_MPMM_FCONF := 1

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/soc/common/soc_css.mk
include plat/arm/board/common/board_common.mk
