#
# Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

# Use the GICv3 driver on the FVP by default
FVP_USE_GIC_DRIVER	:= FVP_GICV3

# Default cluster count for FVP
FVP_CLUSTER_COUNT	:= 2

# Default number of CPUs per cluster on FVP
FVP_MAX_CPUS_PER_CLUSTER	:= 4

# Default number of threads per CPU on FVP
FVP_MAX_PE_PER_CPU	:= 1

# Disable redistributor frame of inactive/fused CPU cores by marking it as read
# only; enable redistributor frames of all CPU cores by default.
FVP_GICR_REGION_PROTECTION		:= 0

FVP_DT_PREFIX		:= fvp-base-gicv3-psci

# Size (in kilobytes) of the Trusted SRAM region to  utilize when building for
# the FVP platform. This option defaults to 256.
FVP_TRUSTED_SRAM_SIZE	:= 256

# Macro to enable helpers for running SPM tests. Disabled by default.
PLAT_TEST_SPM	:= 0

# This is a very trickly TEMPORARY fix. Enabling ALL features exceeds BL31's
# progbits limit. We need a way to build all useful configurations while waiting
# on the fvp to increase its SRAM size. The problem is twofild:
#  1. the cleanup that introduced these enables cleaned up tf-a a little too
#     well and things that previously (incorrectly) were enabled, no longer are.
#     A bunch of CI configs build subtly incorrectly and this combo makes it
#     necessary to forcefully and unconditionally enable them here.
#  2. the progbits limit is exceeded only when the tsp is involved. However,
#     there are tsp CI configs that run on very high architecture revisions so
#     disabling everything isn't an option.
# The fix is to enable everything, as before. When the tsp is included, though,
# we need to slim the size down. In that case, disable all optional features,
# that will not be present in CI when the tsp is.
# Similarly, DRTM support is only tested on v8.0 models. Disable everything just
# for it.
# TODO: make all of this unconditional (or only base the condition on
# ARM_ARCH_* when the makefile supports it).
ifneq (${DRTM_SUPPORT}, 1)
ifneq (${SPD}, tspd)
	ENABLE_FEAT_AMU			:= 2
	ENABLE_FEAT_AMUv1p1		:= 2
	ENABLE_FEAT_HCX			:= 2
	ENABLE_FEAT_RNG			:= 2
	ENABLE_FEAT_TWED		:= 2
	ENABLE_FEAT_GCS			:= 2
ifeq (${ARCH}, aarch64)
ifneq (${SPD}, spmd)
ifeq (${SPM_MM}, 0)
ifeq (${CTX_INCLUDE_FPREGS}, 0)
	ENABLE_SME_FOR_NS		:= 2
	ENABLE_SME2_FOR_NS		:= 2
endif
endif
endif
endif
endif

# enable unconditionally for all builds
ifeq (${ARCH}, aarch64)
    ENABLE_BRBE_FOR_NS		:= 2
    ENABLE_TRBE_FOR_NS		:= 2
endif
ENABLE_SYS_REG_TRACE_FOR_NS	:= 2
ENABLE_FEAT_CSV2_2		:= 2
ENABLE_FEAT_DIT			:= 2
ENABLE_FEAT_PAN			:= 2
ENABLE_FEAT_MTE_PERM		:= 2
ENABLE_FEAT_VHE			:= 2
CTX_INCLUDE_NEVE_REGS		:= 2
ENABLE_FEAT_SEL2		:= 2
ENABLE_TRF_FOR_NS		:= 2
ENABLE_FEAT_ECV			:= 2
ENABLE_FEAT_FGT			:= 2
ENABLE_FEAT_TCR2		:= 2
ENABLE_FEAT_S2PIE		:= 2
ENABLE_FEAT_S1PIE		:= 2
ENABLE_FEAT_S2POE		:= 2
ENABLE_FEAT_S1POE		:= 2
endif

# The FVP platform depends on this macro to build with correct GIC driver.
$(eval $(call add_define,FVP_USE_GIC_DRIVER))

# Pass FVP_CLUSTER_COUNT to the build system.
$(eval $(call add_define,FVP_CLUSTER_COUNT))

# Pass FVP_MAX_CPUS_PER_CLUSTER to the build system.
$(eval $(call add_define,FVP_MAX_CPUS_PER_CLUSTER))

# Pass FVP_MAX_PE_PER_CPU to the build system.
$(eval $(call add_define,FVP_MAX_PE_PER_CPU))

# Pass FVP_GICR_REGION_PROTECTION to the build system.
$(eval $(call add_define,FVP_GICR_REGION_PROTECTION))

# Pass FVP_TRUSTED_SRAM_SIZE to the build system.
$(eval $(call add_define,FVP_TRUSTED_SRAM_SIZE))

# Sanity check the cluster count and if FVP_CLUSTER_COUNT <= 2,
# choose the CCI driver , else the CCN driver
ifeq ($(FVP_CLUSTER_COUNT), 0)
$(error "Incorrect cluster count specified for FVP port")
else ifeq ($(FVP_CLUSTER_COUNT),$(filter $(FVP_CLUSTER_COUNT),1 2))
FVP_INTERCONNECT_DRIVER := FVP_CCI
else
FVP_INTERCONNECT_DRIVER := FVP_CCN
endif

$(eval $(call add_define,FVP_INTERCONNECT_DRIVER))

# Choose the GIC sources depending upon the how the FVP will be invoked
ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV3)

# The GIC model (GIC-600 or GIC-500) will be detected at runtime
GICV3_SUPPORT_GIC600		:=	1
GICV3_OVERRIDE_DISTIF_PWR_OPS	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

FVP_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c

	ifeq ($(filter 1,${RESET_TO_BL2} \
		${RESET_TO_BL31} ${RESET_TO_SP_MIN}),)
		FVP_GIC_SOURCES += plat/arm/board/fvp/fvp_gicv3.c
	endif

else ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV2)

# No GICv4 extension
GIC_ENABLE_V4_EXTN	:=	0
$(eval $(call add_define,GIC_ENABLE_V4_EXTN))

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk

FVP_GIC_SOURCES		:=	${GICV2_SOURCES}			\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c

FVP_DT_PREFIX		:=	fvp-base-gicv2-psci
else
$(error "Incorrect GIC driver chosen on FVP port")
endif

ifeq (${FVP_INTERCONNECT_DRIVER}, FVP_CCI)
FVP_INTERCONNECT_SOURCES	:= 	drivers/arm/cci/cci.c
else ifeq (${FVP_INTERCONNECT_DRIVER}, FVP_CCN)
FVP_INTERCONNECT_SOURCES	:= 	drivers/arm/ccn/ccn.c		\
					plat/arm/common/arm_ccn.c
else
$(error "Incorrect CCN driver chosen on FVP port")
endif

FVP_SECURITY_SOURCES	:=	drivers/arm/tzc/tzc400.c		\
				plat/arm/board/fvp/fvp_security.c	\
				plat/arm/common/arm_tzc400.c


PLAT_INCLUDES		:=	-Iplat/arm/board/fvp/include		\
				-Iinclude/lib/psa


PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/fvp/fvp_common.c

FVP_CPU_LIBS		:=	lib/cpus/${ARCH}/aem_generic.S

ifeq (${ARCH}, aarch64)

# select a different set of CPU files, depending on whether we compile for
# hardware assisted coherency cores or not
ifeq (${HW_ASSISTED_COHERENCY}, 0)
# Cores used without DSU
	FVP_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a35.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				lib/cpus/aarch64/cortex_a73.S
else
# Cores used with DSU only
	ifeq (${CTX_INCLUDE_AARCH32_REGS}, 0)
	# AArch64-only cores
	# TODO: add all cores to the appropriate lists
		FVP_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a65.S		\
					lib/cpus/aarch64/cortex_a65ae.S		\
					lib/cpus/aarch64/cortex_a76.S		\
					lib/cpus/aarch64/cortex_a76ae.S		\
					lib/cpus/aarch64/cortex_a77.S		\
					lib/cpus/aarch64/cortex_a78.S		\
					lib/cpus/aarch64/cortex_a78_ae.S	\
					lib/cpus/aarch64/cortex_a78c.S		\
					lib/cpus/aarch64/cortex_a710.S		\
					lib/cpus/aarch64/cortex_a715.S		\
					lib/cpus/aarch64/cortex_a720.S		\
					lib/cpus/aarch64/neoverse_n_common.S	\
					lib/cpus/aarch64/neoverse_n1.S		\
					lib/cpus/aarch64/neoverse_n2.S		\
					lib/cpus/aarch64/neoverse_v1.S		\
					lib/cpus/aarch64/neoverse_e1.S		\
					lib/cpus/aarch64/cortex_x2.S		\
					lib/cpus/aarch64/cortex_x4.S		\
					lib/cpus/aarch64/cortex_gelas.S		\
					lib/cpus/aarch64/nevis.S		\
					lib/cpus/aarch64/travis.S
	endif
	# AArch64/AArch32 cores
	FVP_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a55.S		\
				lib/cpus/aarch64/cortex_a75.S
endif

else
FVP_CPU_LIBS		+=	lib/cpus/aarch32/cortex_a32.S			\
				lib/cpus/aarch32/cortex_a57.S			\
				lib/cpus/aarch32/cortex_a53.S
endif

BL1_SOURCES		+=	drivers/arm/smmu/smmu_v3.c			\
				drivers/arm/sp805/sp805.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/io/io_semihosting.c			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/board/fvp/${ARCH}/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_bl1_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c		\
				${FVP_CPU_LIBS}					\
				${FVP_INTERCONNECT_SOURCES}

ifeq (${USE_SP804_TIMER},1)
BL1_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
else
BL1_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c
endif


BL2_SOURCES		+=	drivers/arm/sp805/sp805.c			\
				drivers/io/io_semihosting.c			\
				lib/utils/mem_region.c				\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/board/fvp/fvp_bl2_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c	\
				${FVP_SECURITY_SOURCES}


ifeq (${COT_DESC_IN_DTB},1)
BL2_SOURCES		+=	plat/arm/common/fconf/fconf_nv_cntr_getter.c
endif

ifeq (${ENABLE_RME},1)
BL2_SOURCES		+=	plat/arm/board/fvp/aarch64/fvp_helpers.S

BL31_SOURCES		+=	plat/arm/board/fvp/fvp_plat_attest_token.c	\
				plat/arm/board/fvp/fvp_realm_attest_key.c
endif

ifeq (${ENABLE_FEAT_RNG_TRAP},1)
BL31_SOURCES		+=	plat/arm/board/fvp/fvp_sync_traps.c
endif

ifeq (${RESET_TO_BL2},1)
BL2_SOURCES		+=	plat/arm/board/fvp/${ARCH}/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_bl2_el3_setup.c		\
				${FVP_CPU_LIBS}					\
				${FVP_INTERCONNECT_SOURCES}
endif

ifeq (${USE_SP804_TIMER},1)
BL2_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
endif

BL2U_SOURCES		+=	plat/arm/board/fvp/fvp_bl2u_setup.c		\
				${FVP_SECURITY_SOURCES}

ifeq (${USE_SP804_TIMER},1)
BL2U_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
endif

BL31_SOURCES		+=	drivers/arm/fvp/fvp_pwrc.c			\
				drivers/arm/smmu/smmu_v3.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/cfi/v2m/v2m_flash.c			\
				lib/utils/mem_region.c				\
				plat/arm/board/fvp/fvp_bl31_setup.c		\
				plat/arm/board/fvp/fvp_console.c		\
				plat/arm/board/fvp/fvp_pm.c			\
				plat/arm/board/fvp/fvp_topology.c		\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/common/arm_nor_psci_mem_protect.c	\
				${FVP_CPU_LIBS}					\
				${FVP_GIC_SOURCES}				\
				${FVP_INTERCONNECT_SOURCES}			\
				${FVP_SECURITY_SOURCES}

# Support for fconf in BL31
# Added separately from the above list for better readability
ifeq ($(filter 1,${RESET_TO_BL2} ${RESET_TO_BL31}),)
BL31_SOURCES		+=	lib/fconf/fconf.c				\
				lib/fconf/fconf_dyn_cfg_getter.c		\
				plat/arm/board/fvp/fconf/fconf_hw_config_getter.c

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

ifeq (${SEC_INT_DESC_IN_FCONF},1)
BL31_SOURCES		+=	plat/arm/common/fconf/fconf_sec_intr_config.c
endif

endif

ifeq (${USE_SP804_TIMER},1)
BL31_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
else
BL31_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c
endif

# Add the FDT_SOURCES and options for Dynamic Config (only for Unix env)
ifdef UNIX_MK
FVP_HW_CONFIG_DTS	:=	fdts/${FVP_DT_PREFIX}.dts
FDT_SOURCES		+=	$(addprefix plat/arm/board/fvp/fdts/,	\
					${PLAT}_fw_config.dts		\
					${PLAT}_tb_fw_config.dts	\
					${PLAT}_soc_fw_config.dts	\
					${PLAT}_nt_fw_config.dts	\
				)

FVP_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
FVP_TB_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb
FVP_SOC_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_soc_fw_config.dtb
FVP_NT_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

ifeq (${SPD},tspd)
FDT_SOURCES		+=	plat/arm/board/fvp/fdts/${PLAT}_tsp_fw_config.dts
FVP_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_tsp_fw_config.dtb

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_TOS_FW_CONFIG},--tos-fw-config,${FVP_TOS_FW_CONFIG}))
endif

ifeq (${TRANSFER_LIST}, 1)
include lib/transfer_list/transfer_list.mk
endif

ifeq (${SPD},spmd)

ifeq ($(ARM_SPMC_MANIFEST_DTS),)
ARM_SPMC_MANIFEST_DTS	:=	plat/arm/board/fvp/fdts/${PLAT}_spmc_manifest.dts
endif

FDT_SOURCES		+=	${ARM_SPMC_MANIFEST_DTS}
FVP_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${ARM_SPMC_MANIFEST_DTS})).dtb

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_TOS_FW_CONFIG},--tos-fw-config,${FVP_TOS_FW_CONFIG}))
endif

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_FW_CONFIG},--fw-config,${FVP_FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_TB_FW_CONFIG},--tb-fw-config,${FVP_TB_FW_CONFIG}))
# Add the SOC_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_SOC_FW_CONFIG},--soc-fw-config,${FVP_SOC_FW_CONFIG}))
# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_NT_FW_CONFIG},--nt-fw-config,${FVP_NT_FW_CONFIG}))

FDT_SOURCES		+=	${FVP_HW_CONFIG_DTS}
$(eval FVP_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb,$(FVP_HW_CONFIG_DTS)))

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_HW_CONFIG},--hw-config,${FVP_HW_CONFIG}))
endif

# Enable dynamic mitigation support by default
DYNAMIC_WORKAROUND_CVE_2018_3639	:=	1

ifneq (${ENABLE_FEAT_AMU},0)
BL31_SOURCES		+=	lib/cpus/aarch64/cpuamu.c		\
				lib/cpus/aarch64/cpuamu_helpers.S

ifeq (${HW_ASSISTED_COHERENCY}, 1)
BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a75_pubsub.c	\
				lib/cpus/aarch64/neoverse_n1_pubsub.c
endif
endif

ifeq (${HANDLE_EA_EL3_FIRST_NS},1)
ifeq (${ENABLE_FEAT_RAS},1)
BL31_SOURCES		+=	plat/arm/board/fvp/aarch64/fvp_ras.c
else
BL31_SOURCES		+= 	plat/arm/board/fvp/aarch64/fvp_ea.c
endif
endif

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/board/fvp/fvp_stack_protector.c
endif

# Enable the dynamic translation tables library.
ifeq ($(filter 1,${RESET_TO_BL2} ${ARM_XLAT_TABLES_LIB_V1}),)
    ifeq (${ARCH},aarch32)
        BL32_CPPFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC
    else # AArch64
        BL31_CPPFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC
    endif
endif

ifeq (${ALLOW_RO_XLAT_TABLES}, 1)
    ifeq (${ARCH},aarch32)
        BL32_CPPFLAGS	+=	-DPLAT_RO_XLAT_TABLES
    else # AArch64
        BL31_CPPFLAGS	+=	-DPLAT_RO_XLAT_TABLES
        ifeq (${SPD},tspd)
            BL32_CPPFLAGS +=	-DPLAT_RO_XLAT_TABLES
        endif
    endif
endif

ifeq (${USE_DEBUGFS},1)
    BL31_CPPFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC
endif

# Add support for platform supplied linker script for BL31 build
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

ifneq (${RESET_TO_BL2}, 0)
    override BL1_SOURCES =
endif

# RSS is not supported on FVP right now. Thus, we use the mocked version
# of the provided PSA APIs. They return with success and hard-coded token/key.
PLAT_RSS_NOT_SUPPORTED	:= 1

# Include Measured Boot makefile before any Crypto library makefile.
# Crypto library makefile may need default definitions of Measured Boot build
# flags present in Measured Boot makefile.
ifeq (${MEASURED_BOOT},1)
    RSS_MEASURED_BOOT_MK := drivers/measured_boot/rss/rss_measured_boot.mk
    $(info Including ${RSS_MEASURED_BOOT_MK})
    include ${RSS_MEASURED_BOOT_MK}

    ifneq (${MBOOT_RSS_HASH_ALG}, sha256)
        $(eval $(call add_define,TF_MBEDTLS_MBOOT_USE_SHA512))
    endif

    BL1_SOURCES		+=	${MEASURED_BOOT_SOURCES}
    BL2_SOURCES		+=	${MEASURED_BOOT_SOURCES}
endif

include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk

ifeq (${MEASURED_BOOT},1)
BL1_SOURCES		+=	plat/arm/board/fvp/fvp_common_measured_boot.c	\
				plat/arm/board/fvp/fvp_bl1_measured_boot.c	\
				lib/psa/measured_boot.c

BL2_SOURCES		+=	plat/arm/board/fvp/fvp_common_measured_boot.c	\
				plat/arm/board/fvp/fvp_bl2_measured_boot.c	\
				lib/psa/measured_boot.c

# Even though RSS is not supported on FVP (see above), we support overriding
# PLAT_RSS_NOT_SUPPORTED from the command line, just for the purpose of building
# the code to detect any build regressions. The resulting firmware will not be
# functional.
ifneq (${PLAT_RSS_NOT_SUPPORTED},1)
    $(warning "RSS is not supported on FVP. The firmware will not be functional.")
    include drivers/arm/rss/rss_comms.mk
    BL1_SOURCES		+=	${RSS_COMMS_SOURCES}
    BL2_SOURCES		+=	${RSS_COMMS_SOURCES}
    BL31_SOURCES	+=	${RSS_COMMS_SOURCES}

    BL1_CFLAGS		+=	-DPLAT_RSS_COMMS_PAYLOAD_MAX_SIZE=0
    BL2_CFLAGS		+=	-DPLAT_RSS_COMMS_PAYLOAD_MAX_SIZE=0
    BL31_CFLAGS		+=	-DPLAT_RSS_COMMS_PAYLOAD_MAX_SIZE=0
endif

endif

ifeq (${DRTM_SUPPORT}, 1)
BL31_SOURCES   += plat/arm/board/fvp/fvp_drtm_addr.c	\
		  plat/arm/board/fvp/fvp_drtm_dma_prot.c	\
		  plat/arm/board/fvp/fvp_drtm_err.c	\
		  plat/arm/board/fvp/fvp_drtm_measurement.c	\
		  plat/arm/board/fvp/fvp_drtm_stub.c	\
		  plat/arm/common/arm_dyn_cfg.c		\
		  plat/arm/board/fvp/fvp_err.c
endif

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	plat/arm/board/fvp/fvp_trusted_boot.c
BL2_SOURCES		+=	plat/arm/board/fvp/fvp_trusted_boot.c

# FVP being a development platform, enable capability to disable Authentication
# dynamically if TRUSTED_BOARD_BOOT is set.
DYN_DISABLE_AUTH	:=	1
endif

ifeq (${SPMC_AT_EL3}, 1)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/board/fvp/fvp_el3_spmc.c
endif

PSCI_OS_INIT_MODE	:=	1

ifeq (${SPD},spmd)
BL31_SOURCES	+=	plat/arm/board/fvp/fvp_spmd.c
endif

# Test specific macros, keep them at bottom of this file
$(eval $(call add_define,PLATFORM_TEST_EA_FFH))
ifeq (${PLATFORM_TEST_EA_FFH}, 1)
    ifeq (${FFH_SUPPORT}, 0)
         $(error "PLATFORM_TEST_EA_FFH expects FFH_SUPPORT to be 1")
    endif

endif

$(eval $(call add_define,PLATFORM_TEST_RAS_FFH))
ifeq (${PLATFORM_TEST_RAS_FFH}, 1)
    ifeq (${ENABLE_FEAT_RAS}, 0)
         $(error "PLATFORM_TEST_RAS_FFH expects ENABLE_FEAT_RAS to be 1")
    endif
    ifeq (${HANDLE_EA_EL3_FIRST_NS}, 0)
         $(error "PLATFORM_TEST_RAS_FFH expects HANDLE_EA_EL3_FIRST_NS to be 1")
    endif
endif

ifeq (${ERRATA_ABI_SUPPORT}, 1)
include plat/arm/board/fvp/fvp_cpu_errata.mk
endif

# Build macro necessary for running SPM tests on FVP platform
$(eval $(call add_define,PLAT_TEST_SPM))
