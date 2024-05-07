# Copyright (c) 2021-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

TARGET_FLAVOUR			:=	fvp
# DPU with SCMI may not necessarily work, so allow its independence
TC_DPU_USE_SCMI_CLK		:=	1
# SCMI power domain control enable
TC_SCMI_PD_CTRL_EN		:=	1
# IOMMU: Enable the use of system or individual MMUs
TC_IOMMU_EN			:=	1

# System setup
CSS_USE_SCMI_SDS_DRIVER		:=	1
HW_ASSISTED_COHERENCY		:=	1
USE_COHERENT_MEM		:=	0
GIC_ENABLE_V4_EXTN		:=      1
GICV3_SUPPORT_GIC600		:=	1
override NEED_BL2U		:=	no
override ARM_PLAT_MT		:=	1

# CPU setup
ARM_ARCH_MINOR			:=	7
BRANCH_PROTECTION		:=	1
ENABLE_FEAT_MPAM		:=	1 # default is 2, optimise
ENABLE_SVE_FOR_NS		:=	2 # to show we use it
ENABLE_SVE_FOR_SWD		:=	1
ENABLE_TRBE_FOR_NS		:=	1
ENABLE_SYS_REG_TRACE_FOR_NS	:=	1
ENABLE_FEAT_AMU			:=	1
ENABLE_AMU_FCONF		:=	1
ENABLE_AMU_AUXILIARY_COUNTERS	:=	1
ENABLE_MPMM			:=	1
ENABLE_MPMM_FCONF		:=	1

CTX_INCLUDE_AARCH32_REGS	:=	0

ifeq (${SPD},spmd)
	SPMD_SPM_AT_SEL2	:=	1
	ENABLE_FEAT_MTE2	:=	1
	CTX_INCLUDE_PAUTH_REGS	:=	1
endif


ifneq ($(shell expr $(TARGET_PLATFORM) \<= 1), 0)
        $(warning Platform ${PLAT}$(TARGET_PLATFORM) is deprecated. \
          Some of the features might not work as expected)
endif

ifeq ($(shell expr $(TARGET_PLATFORM) \<= 3), 0)
        $(error TARGET_PLATFORM must be less than or equal to 3)
endif

ifeq ($(filter ${TARGET_FLAVOUR}, fvp fpga),)
        $(error TARGET_FLAVOUR must be fvp or fpga)
endif

$(eval $(call add_defines, \
	TARGET_PLATFORM \
	TARGET_FLAVOUR_$(call uppercase,${TARGET_FLAVOUR}) \
	TC_DPU_USE_SCMI_CLK \
	TC_SCMI_PD_CTRL_EN \
	TC_IOMMU_EN \
))

CSS_LOAD_SCP_IMAGES	:=	1

# Save DSU PMU registers on cluster off and restore them on cluster on
PRESERVE_DSU_PMU_REGS		:= 1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

ENT_GIC_SOURCES		:=	${GICV3_SOURCES}		\
				plat/common/plat_gicv3.c	\
				plat/arm/common/arm_gicv3.c

TC_BASE	=	plat/arm/board/tc

PLAT_INCLUDES		+=	-I${TC_BASE}/include/ \
				-I${TC_BASE}/fdts/

# CPU libraries for TARGET_PLATFORM=1
ifeq (${TARGET_PLATFORM}, 1)
TC_CPU_SOURCES	+=	lib/cpus/aarch64/cortex_a510.S \
			lib/cpus/aarch64/cortex_a715.S \
			lib/cpus/aarch64/cortex_x3.S
endif

# CPU libraries for TARGET_PLATFORM=2
ifeq (${TARGET_PLATFORM}, 2)
TC_CPU_SOURCES	+=	lib/cpus/aarch64/cortex_a520.S \
			lib/cpus/aarch64/cortex_a720.S \
			lib/cpus/aarch64/cortex_x4.S
endif

# CPU libraries for TARGET_PLATFORM=3
ifeq (${TARGET_PLATFORM}, 3)
TC_CPU_SOURCES	+=	lib/cpus/aarch64/cortex_a520.S \
			lib/cpus/aarch64/cortex_chaberton.S \
			lib/cpus/aarch64/cortex_blackhawk.S
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
				drivers/arm/css/dsu/dsu.c			\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c	\
				drivers/arm/sbsa/sbsa.c

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${TC_BASE}/fdts/${PLAT}_fw_config.dts	\
				${TC_BASE}/fdts/${PLAT}_tb_fw_config.dts \
				${TC_BASE}/fdts/${PLAT}_nt_fw_config.dts
FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb
FVP_NT_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))
# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_NT_FW_CONFIG},--nt-fw-config,${FVP_NT_FW_CONFIG}))

ifeq (${SPD},spmd)
ifeq ($(ARM_SPMC_MANIFEST_DTS),)
ARM_SPMC_MANIFEST_DTS	:=	${TC_BASE}/fdts/${PLAT}_spmc_test_manifest.dts
endif

FDT_SOURCES		+=	${ARM_SPMC_MANIFEST_DTS}
TC_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${ARM_SPMC_MANIFEST_DTS})).dtb

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TC_TOS_FW_CONFIG},--tos-fw-config,${TC_TOS_FW_CONFIG}))
endif

#Device tree
TC_HW_CONFIG_DTS	:=	fdts/${PLAT}${TARGET_PLATFORM}.dts
TC_HW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}.dtb
FDT_SOURCES		+=	${TC_HW_CONFIG_DTS}
$(eval TC_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb,$(TC_HW_CONFIG_DTS)))

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TC_HW_CONFIG},--hw-config,${TC_HW_CONFIG}))

# Include Measured Boot makefile before any Crypto library makefile.
# Crypto library makefile may need default definitions of Measured Boot build
# flags present in Measured Boot makefile.
$(info Including rse_comms.mk)
ifeq (${MEASURED_BOOT},1)
        $(info Including rse_comms.mk)
        include drivers/arm/rse/rse_comms.mk

	BL1_SOURCES	+=	${RSE_COMMS_SOURCES}
	BL2_SOURCES	+=	${RSE_COMMS_SOURCES}
	PLAT_INCLUDES	+=	-Iinclude/lib/psa

    ifeq (${DICE_PROTECTION_ENVIRONMENT},1)
        $(info Including qcbor.mk)
        include drivers/measured_boot/rse/qcbor.mk
        $(info Including dice_prot_env.mk)
        include drivers/measured_boot/rse/dice_prot_env.mk

	BL1_SOURCES	+=	${QCBOR_SOURCES} \
				${DPE_SOURCES} \
				plat/arm/board/tc/tc_common_dpe.c \
				plat/arm/board/tc/tc_bl1_dpe.c \
				lib/psa/dice_protection_environment.c \
				drivers/arm/css/sds/sds.c \
				drivers/delay_timer/delay_timer.c \
				drivers/delay_timer/generic_delay_timer.c

	BL2_SOURCES	+=	${QCBOR_SOURCES} \
				${DPE_SOURCES} \
				plat/arm/board/tc/tc_common_dpe.c \
				plat/arm/board/tc/tc_bl2_dpe.c \
				lib/psa/dice_protection_environment.c

	PLAT_INCLUDES	+=	-I${QCBOR_INCLUDES} \
				-Iinclude/lib/dice
    else
        $(info Including rse_measured_boot.mk)
        include drivers/measured_boot/rse/rse_measured_boot.mk

	BL1_SOURCES	+=	${MEASURED_BOOT_SOURCES} \
				plat/arm/board/tc/tc_common_measured_boot.c \
				plat/arm/board/tc/tc_bl1_measured_boot.c \
				lib/psa/measured_boot.c

	BL2_SOURCES		+=	${MEASURED_BOOT_SOURCES} \
				plat/arm/board/tc/tc_common_measured_boot.c \
				plat/arm/board/tc/tc_bl2_measured_boot.c \
				lib/psa/measured_boot.c
    endif
endif

ifeq (${TRNG_SUPPORT},1)
	BL31_SOURCES	+=	plat/arm/board/tc/tc_trng.c
endif

ifneq (${PLATFORM_TEST},)
    # Add this include as first, before arm_common.mk. This is necessary
    # because arm_common.mk builds Mbed TLS, and platform_test.mk can
    # change the list of Mbed TLS files that are to be compiled
    # (LIBMBEDTLS_SRCS).
    include plat/arm/board/tc/platform_test.mk
endif


include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/soc/common/soc_css.mk
include plat/arm/board/common/board_common.mk
