#
# Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Trusted Firmware Version
#
VERSION_MAJOR			:= 2
VERSION_MINOR			:= 13
# VERSION_PATCH is only used for LTS releases
VERSION_PATCH			:= 0
VERSION				:= ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}

# Default goal is build all images
.DEFAULT_GOAL			:= all

# Avoid any implicit propagation of command line variable definitions to
# sub-Makefiles, like CFLAGS that we reserved for the firmware images'
# usage. Other command line options like "-s" are still propagated as usual.
MAKEOVERRIDES =

MAKE_HELPERS_DIRECTORY := make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build-rules.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk

################################################################################
# Default values for build configurations, and their dependencies
################################################################################

include ${MAKE_HELPERS_DIRECTORY}defaults.mk
PLAT				:= ${DEFAULT_PLAT}
include ${MAKE_HELPERS_DIRECTORY}plat_helpers.mk

# To be able to set platform specific defaults
ifneq ($(PLAT_DEFAULTS_MAKEFILE_FULL),)
include ${PLAT_DEFAULTS_MAKEFILE_FULL}
endif

################################################################################
# Configure the toolchains used to build TF-A and its tools
################################################################################

include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

# Assertions enabled for DEBUG builds by default
ENABLE_ASSERTIONS		:= ${DEBUG}
ENABLE_PMF			:= ${ENABLE_RUNTIME_INSTRUMENTATION}

################################################################################
# Checkpatch script options
################################################################################

CHECKCODE_ARGS		:=	--no-patch
# Do not check the coding style on imported library files or documentation files
INC_DRV_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					include/drivers/arm,		\
					$(wildcard include/drivers/*)))
INC_LIB_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					include/lib/libfdt		\
					include/lib/libc,		\
					$(wildcard include/lib/*)))
INC_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					include/lib			\
					include/drivers,		\
					$(wildcard include/*)))
LIB_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					lib/compiler-rt			\
					lib/libfdt%			\
					lib/libc,			\
					lib/zlib			\
					$(wildcard lib/*)))
ROOT_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					lib				\
					include				\
					docs				\
					%.rst,				\
					$(wildcard *)))
CHECK_PATHS		:=	${ROOT_DIRS_TO_CHECK}			\
				${INC_DIRS_TO_CHECK}			\
				${INC_LIB_DIRS_TO_CHECK}		\
				${LIB_DIRS_TO_CHECK}			\
				${INC_DRV_DIRS_TO_CHECK}		\
				${INC_ARM_DIRS_TO_CHECK}

################################################################################
# Process build options
################################################################################

ifeq ($(verbose),)
	CHECKCODE_ARGS	+=	--no-summary --terse
endif

################################################################################
# Auxiliary tools (fiptool, cert_create, etc)
################################################################################

# Variables for use with Certificate Generation Tool
CRTTOOLPATH		?=	tools/cert_create
CRTTOOL			?=	${BUILD_PLAT}/${CRTTOOLPATH}/cert_create$(.exe)

# Variables for use with Firmware Encryption Tool
ENCTOOLPATH		?=	tools/encrypt_fw
ENCTOOL			?=	${BUILD_PLAT}/${ENCTOOLPATH}/encrypt_fw$(.exe)

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fiptool
FIPTOOL			?=	${BUILD_PLAT}/${FIPTOOLPATH}/fiptool$(.exe)

# Variables for use with sptool
SPTOOLPATH		?=	tools/sptool
SPTOOL			?=	${SPTOOLPATH}/sptool.py
SP_MK_GEN		?=	${SPTOOLPATH}/sp_mk_generator.py
SP_DTS_LIST_FRAGMENT	?=	${BUILD_PLAT}/sp_list_fragment.dts

# Variables for use with sptool
TLCTOOL 		?=	poetry run tlc

# Variables for use with ROMLIB
ROMLIBPATH		?=	lib/romlib

# Variable for use with Python
PYTHON			?=	python3

# Variables for use with documentation build using Sphinx tool
DOCS_PATH		?=	docs

# Process Debug flag
ifneq (${DEBUG}, 0)
	BUILD_TYPE	:=	debug
	# Use LOG_LEVEL_INFO by default for debug builds
	LOG_LEVEL	:=	40
else
	BUILD_TYPE	:=	release
	# Use LOG_LEVEL_NOTICE by default for release builds
	LOG_LEVEL	:=	20
endif #(Debug)

# Default build string (git branch and commit)
ifeq (${BUILD_STRING},)
	BUILD_STRING  :=  $(shell git describe --always --dirty --tags 2> /dev/null)
endif
VERSION_STRING    :=  v${VERSION}(${BUILD_TYPE}):${BUILD_STRING}

# Setting W is quite verbose and most warnings will be pre-existing issues
# outside of the contributor's control. Don't fail the build on them so warnings
# can be seen and hopefully addressed
ifdef W
	ifneq (${W},0)
		E	 ?= 0
	endif
endif

################################################################################
# Setup ARCH_MAJOR/MINOR before parsing arch_features.
################################################################################
ifeq (${ENABLE_RME},1)
	ARM_ARCH_MAJOR := 9
	ARM_ARCH_MINOR := 2
endif

################################################################################
# Common sources and include directories
################################################################################
include lib/compiler-rt/compiler-rt.mk

# Allow overriding the timestamp, for example for reproducible builds, or to
# synchronize timestamps across multiple projects.
# This must be set to a C string (including quotes where applicable).
BUILD_MESSAGE_TIMESTAMP ?= __TIME__", "__DATE__

DEFINES += -DBUILD_MESSAGE_TIMESTAMP='$(BUILD_MESSAGE_TIMESTAMP)'
DEFINES += -DBUILD_MESSAGE_VERSION_STRING='"$(VERSION_STRING)"'
DEFINES += -DBUILD_MESSAGE_VERSION='"$(VERSION)"'

BL_COMMON_SOURCES	+=	common/bl_common.c			\
				common/tf_log.c				\
				common/${ARCH}/debug.S			\
				drivers/console/multi_console.c		\
				lib/${ARCH}/cache_helpers.S		\
				lib/${ARCH}/misc_helpers.S		\
				lib/extensions/pmuv3/${ARCH}/pmuv3.c	\
				plat/common/plat_bl_common.c		\
				plat/common/plat_log_common.c		\
				plat/common/${ARCH}/plat_common.c	\
				plat/common/${ARCH}/platform_helpers.S	\
				${COMPILER_RT_SRCS}

ifeq ($($(ARCH)-cc-id),arm-clang)
	BL_COMMON_SOURCES	+=	lib/${ARCH}/armclang_printf.S
endif

ifeq (${SANITIZE_UB},on)
	BL_COMMON_SOURCES	+=	plat/common/ubsan.c
endif

INCLUDES		+=	-Iinclude				\
				-Iinclude/arch/${ARCH}			\
				-Iinclude/lib/cpus/${ARCH}		\
				-Iinclude/lib/el3_runtime/${ARCH}	\
				${PLAT_INCLUDES}			\
				${SPD_INCLUDES}

include common/backtrace/backtrace.mk

################################################################################
# Generic definitions
################################################################################

ifeq (${BUILD_BASE},)
     BUILD_BASE		:=	./build
endif
BUILD_PLAT		:=	$(abspath ${BUILD_BASE})/${PLAT}/${BUILD_TYPE}

SPDS			:=	$(sort $(filter-out none, $(patsubst services/spd/%,%,$(wildcard services/spd/*))))

# Platforms providing their own TBB makefile may override this value
INCLUDE_TBBR_MK		:=	1

################################################################################
# Include SPD Makefile if one has been specified
################################################################################

ifneq (${SPD},none)
	ifeq (${SPD},spmd)
	# SPMD is located in std_svc directory
		SPD_DIR := std_svc

		ifeq ($(SPMD_SPM_AT_SEL2),1)
			CTX_INCLUDE_EL2_REGS := 1
		endif

		ifneq ($(SP_LAYOUT_FILE),)
		BL2_ENABLE_SP_LOAD := 1
		endif
	else
		# All other SPDs in spd directory
		SPD_DIR := spd
	endif #(SPD)

	# We expect to locate an spd.mk under the specified SPD directory
	SPD_MAKE	:=	$(wildcard services/${SPD_DIR}/${SPD}/${SPD}.mk)

	ifeq (${SPD_MAKE},)
                $(error Error: No services/${SPD_DIR}/${SPD}/${SPD}.mk located)
	endif
        $(info Including ${SPD_MAKE})
        include ${SPD_MAKE}

	# If there's BL32 companion for the chosen SPD, we expect that the SPD's
	# Makefile would set NEED_BL32 to "yes". In this case, the build system
	# supports two mutually exclusive options:
	# * BL32 is built from source: then BL32_SOURCES must contain the list
	#   of source files to build BL32
	# * BL32 is a prebuilt binary: then BL32 must point to the image file
	#   that will be included in the FIP
	# If both BL32_SOURCES and BL32 are defined, the binary takes precedence
	# over the sources.
endif #(SPD=none)

################################################################################
# Include the platform specific Makefile after the SPD Makefile (the platform
# makefile may use all previous definitions in this file)
################################################################################
include ${PLAT_MAKEFILE_FULL}

################################################################################
# Setup arch_features based on ARM_ARCH_MAJOR, ARM_ARCH_MINOR provided from
# platform.
################################################################################

include ${MAKE_HELPERS_DIRECTORY}arch_features.mk
################################################################################
# Process BRANCH_PROTECTION value and set
# Pointer Authentication and Branch Target Identification flags
################################################################################
ifeq (${BRANCH_PROTECTION},0)
	# Default value turns off all types of branch protection
	BP_OPTION := none
else ifneq (${ARCH},aarch64)
        $(error BRANCH_PROTECTION requires AArch64)
else ifeq (${BRANCH_PROTECTION},1)
	# Enables all types of branch protection features
	BP_OPTION := standard
	ENABLE_BTI := 1
	ENABLE_PAUTH := 1
else ifeq (${BRANCH_PROTECTION},2)
	# Return address signing to its standard level
	BP_OPTION := pac-ret
	ENABLE_PAUTH := 1
else ifeq (${BRANCH_PROTECTION},3)
	# Extend the signing to include leaf functions
	BP_OPTION := pac-ret+leaf
	ENABLE_PAUTH := 1
else ifeq (${BRANCH_PROTECTION},4)
	# Turn on branch target identification mechanism
	BP_OPTION := bti
	ENABLE_BTI := 1
else ifeq (${BRANCH_PROTECTION},5)
	# Turn on branch target identification mechanism
	BP_OPTION := standard
	ENABLE_BTI := 2
	ENABLE_PAUTH := 2
else
        $(error Unknown BRANCH_PROTECTION value ${BRANCH_PROTECTION})
endif #(BRANCH_PROTECTION)

ifneq ($(ENABLE_PAUTH),0)
	CTX_INCLUDE_PAUTH_REGS	:= ${ENABLE_PAUTH}
endif

# Pointer Authentication sources
ifneq (${ENABLE_PAUTH},0)
# arm/common/aarch64/arm_pauth.c contains a sample platform hook to complete the
# Pauth support. As it's not secure, it must be reimplemented for real platforms
	BL_COMMON_SOURCES	+=	lib/extensions/pauth/pauth.c
endif
#
ifneq (${ENABLE_FEAT_PAUTH_LR},0)
# Currently, FEAT_PAUTH_LR is only supported by arm/clang compilers
# TODO implement for GCC when support is added
ifeq ($($(ARCH)-cc-id),arm-clang)
	arch-features	:= $(arch-features)+pauth-lr
else
	$(error Error: ENABLE_FEAT_PAUTH_LR not supported for GCC compiler)
endif
endif

################################################################################
# RME dependent flags configuration, Enable optional features for RME.
################################################################################
# FEAT_RME
ifeq (${ENABLE_RME},1)
	# RME requires el2 context to be saved for now.
	CTX_INCLUDE_EL2_REGS := 1
	CTX_INCLUDE_AARCH32_REGS := 0
	CTX_INCLUDE_PAUTH_REGS := 1

	ifneq ($(ENABLE_FEAT_MPAM), 0)
		CTX_INCLUDE_MPAM_REGS := 1
	endif

	# RME enables CSV2_2 extension by default.
	ENABLE_FEAT_CSV2_2 = 1
endif #(FEAT_RME)

################################################################################
# Include rmmd Makefile if RME is enabled
################################################################################
ifneq (${ENABLE_RME},0)
include services/std_svc/rmmd/rmmd.mk
$(warning "RME is an experimental feature")
endif

################################################################################
# Make 128-Bit sysreg read/writes availabe when FEAT_D128 is enabled.
################################################################################
ifneq (${ENABLE_FEAT_D128}, 0)
        BL_COMMON_SOURCES       +=      lib/extensions/sysreg128/sysreg128.S
endif

# This internal flag is common option which is set to 1 for scenarios
# when the BL2 is running in EL3 level. This occurs in two scenarios -
# 4 world system running BL2 at EL3 and two world system without BL1 running
# BL2 in EL3

ifeq (${RESET_TO_BL2},1)
	BL2_RUNS_AT_EL3	:=	1
	ifeq (${ENABLE_RME},1)
                $(error RESET_TO_BL2=1 and ENABLE_RME=1 configuration is not \
                supported at the moment.)
	endif
else ifeq (${ENABLE_RME},1)
	BL2_RUNS_AT_EL3	:=	1
else
	BL2_RUNS_AT_EL3	:=	0
endif

# This internal flag is set to 1 when Firmware First handling of External aborts
# is required by lowe ELs. Currently only NS requires this support.
ifeq ($(HANDLE_EA_EL3_FIRST_NS),1)
	FFH_SUPPORT := 1
else
	FFH_SUPPORT := 0
endif

# Include the CPU specific operations makefile, which provides default
# values for all CPU errata workarounds and CPU specific optimisations.
# This can be overridden by the platform.
include lib/cpus/cpu-ops.mk

################################################################################
# Build `AARCH32_SP` as BL32 image for AArch32
################################################################################
ifeq (${ARCH},aarch32)
        NEED_BL32 := yes

        ifneq (${AARCH32_SP},none)
        # We expect to locate an sp.mk under the specified AARCH32_SP directory
		AARCH32_SP_MAKE	:=	$(wildcard bl32/${AARCH32_SP}/${AARCH32_SP}.mk)

                ifeq (${AARCH32_SP_MAKE},)
                        $(error Error: No bl32/${AARCH32_SP}/${AARCH32_SP}.mk located)
                endif
                $(info Including ${AARCH32_SP_MAKE})
                include ${AARCH32_SP_MAKE}
        endif
endif #(ARCH=aarch32)

################################################################################
# Include libc if not overridden
################################################################################
ifeq (${OVERRIDE_LIBC},0)
include lib/libc/libc.mk
endif

ifneq (${USE_GIC_DRIVER},0)
include drivers/arm/gic/gic.mk
endif

################################################################################
# Check incompatible options and dependencies
################################################################################
include ${MAKE_HELPERS_DIRECTORY}constraints.mk

# The cert_create tool cannot generate certificates individually, so we use the
# target 'certificates' to create them all
ifneq (${GENERATE_COT},0)
        FIP_DEPS += certificates
        FWU_FIP_DEPS += fwu_certificates
endif

ifneq (${DECRYPTION_SUPPORT},none)
	ENC_ARGS += -f ${FW_ENC_STATUS}
	ENC_ARGS += -k ${ENC_KEY}
	ENC_ARGS += -n ${ENC_NONCE}
	FIP_DEPS += enctool
	FWU_FIP_DEPS += enctool
endif #(DECRYPTION_SUPPORT)

ifeq ($(MEASURED_BOOT)-$(TRUSTED_BOARD_BOOT),1-1)
# Support authentication verification and hash calculation
	CRYPTO_SUPPORT := 3
else ifeq ($(DRTM_SUPPORT)-$(TRUSTED_BOARD_BOOT),1-1)
# Support authentication verification and hash calculation
	CRYPTO_SUPPORT := 3
else ifneq ($(filter 1,${MEASURED_BOOT} ${DRTM_SUPPORT}),)
# Support hash calculation only
	CRYPTO_SUPPORT := 2
else ifeq (${TRUSTED_BOARD_BOOT},1)
# Support authentication verification only
	CRYPTO_SUPPORT := 1
else
	CRYPTO_SUPPORT := 0
endif #($(MEASURED_BOOT)-$(TRUSTED_BOARD_BOOT))

ifneq ($(filter 1 2 3,$(CRYPTO_SUPPORT)),)
CRYPTO_LIB := $(BUILD_PLAT)/lib/libmbedtls.a
endif

################################################################################
# Process platform overrideable behaviour
################################################################################

ifdef BL1_SOURCES
	NEED_BL1 := yes
endif #(BL1_SOURCES)

ifdef BL2_SOURCES
	NEED_BL2 := yes

	# Using BL2 implies that a BL33 image also needs to be supplied for the FIP and
	# Certificate generation tools. This flag can be overridden by the platform.
	ifdef EL3_PAYLOAD_BASE
		# If booting an EL3 payload there is no need for a BL33 image
		# in the FIP file.
		NEED_BL33		:=	no
	else
		ifdef PRELOADED_BL33_BASE
			# If booting a BL33 preloaded image there is no need of
			# another one in the FIP file.
			NEED_BL33		:=	no
		else
			NEED_BL33		?=	yes
		endif
	endif
endif #(BL2_SOURCES)

ifdef BL2U_SOURCES
	NEED_BL2U := yes
endif #(BL2U_SOURCES)

# If SCP_BL2 is given, we always want FIP to include it.
ifdef SCP_BL2
	NEED_SCP_BL2		:=	yes
endif #(SCP_BL2)

# For AArch32, BL31 is not currently supported.
ifneq (${ARCH},aarch32)
	ifdef BL31_SOURCES
	# When booting an EL3 payload, there is no need to compile the BL31
	# image nor put it in the FIP.
		ifndef EL3_PAYLOAD_BASE
			NEED_BL31 := yes
		endif
	endif
endif #(ARCH=aarch64)

# Process TBB related flags
ifneq (${GENERATE_COT},0)
	# Common cert_create options
	ifneq (${CREATE_KEYS},0)
                $(eval CRT_ARGS += -n)
                $(eval FWU_CRT_ARGS += -n)
		ifneq (${SAVE_KEYS},0)
                        $(eval CRT_ARGS += -k)
                        $(eval FWU_CRT_ARGS += -k)
		endif
	endif
	# Include TBBR makefile (unless the platform indicates otherwise)
	ifeq (${INCLUDE_TBBR_MK},1)
                include make_helpers/tbbr/tbbr_tools.mk
	endif
endif #(GENERATE_COT)

ifneq (${FIP_ALIGN},0)
	FIP_ARGS += --align ${FIP_ALIGN}
endif #(FIP_ALIGN)

ifdef FDT_SOURCES
	NEED_FDT := yes
endif #(FDT_SOURCES)

################################################################################
# Include libraries' Makefile that are used in all BL
################################################################################

include lib/stack_protector/stack_protector.mk

################################################################################
# Include BL specific makefiles
################################################################################

ifeq (${NEED_BL1},yes)
include bl1/bl1.mk
endif

ifeq (${NEED_BL2},yes)
include bl2/bl2.mk
endif

ifeq (${NEED_BL2U},yes)
include bl2u/bl2u.mk
endif

ifeq (${NEED_BL31},yes)
include bl31/bl31.mk
endif

################################################################################
# Build options checks
################################################################################

# Boolean_Flags
$(eval $(call assert_booleans,\
    $(sort \
	ALLOW_RO_XLAT_TABLES \
	BL2_ENABLE_SP_LOAD \
	COLD_BOOT_SINGLE_CPU \
	CREATE_KEYS \
	CTX_INCLUDE_AARCH32_REGS \
	CTX_INCLUDE_FPREGS \
	CTX_INCLUDE_SVE_REGS \
	CTX_INCLUDE_EL2_REGS \
	CTX_INCLUDE_MPAM_REGS \
	DEBUG \
	DYN_DISABLE_AUTH \
	EL3_EXCEPTION_HANDLING \
	ENABLE_AMU_AUXILIARY_COUNTERS \
	AMU_RESTRICT_COUNTERS \
	ENABLE_ASSERTIONS \
	ENABLE_PIE \
	ENABLE_PMF \
	ENABLE_PSCI_STAT \
	ENABLE_RUNTIME_INSTRUMENTATION \
	ENABLE_SME_FOR_SWD \
	ENABLE_SVE_FOR_SWD \
	ENABLE_FEAT_GCIE \
	ENABLE_FEAT_RAS	\
	FFH_SUPPORT	\
	ERROR_DEPRECATED \
	FAULT_INJECTION_SUPPORT \
	GENERATE_COT \
	GICV2_G0_FOR_EL3 \
	HANDLE_EA_EL3_FIRST_NS \
	HARDEN_SLS \
	HW_ASSISTED_COHERENCY \
	MEASURED_BOOT \
	DISCRETE_TPM \
	DICE_PROTECTION_ENVIRONMENT \
	RMMD_ENABLE_EL3_TOKEN_SIGN \
	RMMD_ENABLE_IDE_KEY_PROG \
	DRTM_SUPPORT \
	NS_TIMER_SWITCH \
	OVERRIDE_LIBC \
	PL011_GENERIC_UART \
	PROGRAMMABLE_RESET_ADDRESS \
	PSCI_EXTENDED_STATE_ID \
	PSCI_OS_INIT_MODE \
	ARCH_FEATURE_AVAILABILITY \
	RESET_TO_BL31 \
	SAVE_KEYS \
	SEPARATE_CODE_AND_RODATA \
	SEPARATE_BL2_NOLOAD_REGION \
	SEPARATE_NOBITS_REGION \
	SEPARATE_RWDATA_REGION \
	SEPARATE_SIMD_SECTION \
	SPIN_ON_BL1_EXIT \
	SPM_MM \
	SPMC_AT_EL3 \
	SPMC_AT_EL3_SEL0_SP \
	SPMD_SPM_AT_SEL2 \
	ENABLE_SPMD_LP \
	TRANSFER_LIST \
	TRUSTED_BOARD_BOOT \
	USE_COHERENT_MEM \
	USE_DEBUGFS \
	ARM_IO_IN_DTB \
	SDEI_IN_FCONF \
	SEC_INT_DESC_IN_FCONF \
	USE_ROMLIB \
	USE_TBBR_DEFS \
	WARMBOOT_ENABLE_DCACHE_EARLY \
	RESET_TO_BL2 \
	BL2_IN_XIP_MEM \
	BL2_INV_DCACHE \
	USE_SPINLOCK_CAS \
	ENCRYPT_BL31 \
	ENCRYPT_BL32 \
	ERRATA_SPECULATIVE_AT \
	ERRATA_SME_POWER_DOWN \
	RAS_TRAP_NS_ERR_REC_ACCESS \
	COT_DESC_IN_DTB \
	USE_SP804_TIMER \
	PSA_FWU_SUPPORT \
	PSA_FWU_METADATA_FW_STORE_DESC \
	ENABLE_MPMM \
	FEATURE_DETECTION \
	TRNG_SUPPORT \
	ENABLE_ERRATA_ALL \
	ERRATA_ABI_SUPPORT \
	ERRATA_NON_ARM_INTERCONNECT \
	CONDITIONAL_CMO \
	PSA_CRYPTO	\
	ENABLE_CONSOLE_GETC \
	INIT_UNUSED_NS_EL2	\
	PLATFORM_REPORT_CTX_MEM_USE \
	EARLY_CONSOLE \
	PRESERVE_DSU_PMU_REGS \
	HOB_LIST \
	LFA_SUPPORT \
)))

# Numeric_Flags
$(eval $(call assert_numerics,\
    $(sort \
	ARM_ARCH_MAJOR \
	ARM_ARCH_MINOR \
	BRANCH_PROTECTION \
	CTX_INCLUDE_PAUTH_REGS \
	CTX_INCLUDE_NEVE_REGS \
	CRYPTO_SUPPORT \
	DISABLE_MTPMU \
	ENABLE_BRBE_FOR_NS \
	ENABLE_TRBE_FOR_NS \
	ENABLE_BTI \
	ENABLE_PAUTH \
	ENABLE_FEAT_PAUTH_LR \
	ENABLE_FEAT_AMU \
	ENABLE_FEAT_AMUv1p1 \
	ENABLE_FEAT_CSV2_2 \
	ENABLE_FEAT_CSV2_3 \
	ENABLE_FEAT_DEBUGV8P9 \
	ENABLE_FEAT_DIT \
	ENABLE_FEAT_ECV \
	ENABLE_FEAT_FGT \
	ENABLE_FEAT_FGT2 \
	ENABLE_FEAT_FGWTE3 \
	ENABLE_FEAT_FPMR \
	ENABLE_FEAT_HCX \
	ENABLE_FEAT_LS64_ACCDATA \
	ENABLE_FEAT_MEC \
	ENABLE_FEAT_MOPS \
	ENABLE_FEAT_MTE2 \
	ENABLE_FEAT_PAN \
	ENABLE_FEAT_RNG \
	ENABLE_FEAT_RNG_TRAP \
	ENABLE_FEAT_SEL2 \
	ENABLE_FEAT_TCR2 \
	ENABLE_FEAT_THE \
	ENABLE_FEAT_SB \
	ENABLE_FEAT_S2PIE \
	ENABLE_FEAT_S1PIE \
	ENABLE_FEAT_S2POE \
	ENABLE_FEAT_S1POE \
	ENABLE_FEAT_SCTLR2 \
	ENABLE_FEAT_D128 \
	ENABLE_FEAT_GCS \
	ENABLE_FEAT_VHE \
	ENABLE_FEAT_MPAM \
	ENABLE_RME \
	ENABLE_SPE_FOR_NS \
	ENABLE_SYS_REG_TRACE_FOR_NS \
	ENABLE_SME_FOR_NS \
	ENABLE_SME2_FOR_NS \
	ENABLE_SVE_FOR_NS \
	ENABLE_TRF_FOR_NS \
	FW_ENC_STATUS \
	NR_OF_FW_BANKS \
	NR_OF_IMAGES_IN_FW_BANK \
	TWED_DELAY \
	ENABLE_FEAT_TWED \
	SVE_VECTOR_LEN \
	IMPDEF_SYSREG_TRAP \
	W \
)))

ifdef KEY_SIZE
        $(eval $(call assert_numeric,KEY_SIZE))
endif

ifeq ($(filter $(SANITIZE_UB), on off trap),)
        $(error "Invalid value for SANITIZE_UB: can be one of on, off, trap")
endif

################################################################################
# Add definitions to the cpp preprocessor based on the current build options.
# This is done after including the platform specific makefile to allow the
# platform to overwrite the default options
################################################################################

$(eval $(call add_defines,\
    $(sort \
	ALLOW_RO_XLAT_TABLES \
	ARM_ARCH_MAJOR \
	ARM_ARCH_MINOR \
	BL2_ENABLE_SP_LOAD \
	COLD_BOOT_SINGLE_CPU \
	CTX_INCLUDE_AARCH32_REGS \
	CTX_INCLUDE_FPREGS \
	CTX_INCLUDE_SVE_REGS \
	CTX_INCLUDE_PAUTH_REGS \
	CTX_INCLUDE_MPAM_REGS \
	EL3_EXCEPTION_HANDLING \
	CTX_INCLUDE_EL2_REGS \
	CTX_INCLUDE_NEVE_REGS \
	DEBUG \
	DECRYPTION_SUPPORT_${DECRYPTION_SUPPORT} \
	DISABLE_MTPMU \
	ENABLE_FEAT_AMU \
	ENABLE_AMU_AUXILIARY_COUNTERS \
	AMU_RESTRICT_COUNTERS \
	ENABLE_ASSERTIONS \
	ENABLE_BTI \
	ENABLE_FEAT_DEBUGV8P9 \
	ENABLE_FEAT_MPAM \
	ENABLE_PAUTH \
	ENABLE_FEAT_PAUTH_LR \
	ENABLE_PIE \
	ENABLE_PMF \
	ENABLE_PSCI_STAT \
	ENABLE_RME \
	RMMD_ENABLE_EL3_TOKEN_SIGN \
	RMMD_ENABLE_IDE_KEY_PROG \
	ENABLE_RUNTIME_INSTRUMENTATION \
	ENABLE_SME_FOR_NS \
	ENABLE_SME2_FOR_NS \
	ENABLE_SME_FOR_SWD \
	ENABLE_SPE_FOR_NS \
	ENABLE_SVE_FOR_NS \
	ENABLE_SVE_FOR_SWD \
	ENABLE_FEAT_RAS \
	FFH_SUPPORT \
	ENCRYPT_BL31 \
	ENCRYPT_BL32 \
	ERROR_DEPRECATED \
	FAULT_INJECTION_SUPPORT \
	GICV2_G0_FOR_EL3 \
	HANDLE_EA_EL3_FIRST_NS \
	HW_ASSISTED_COHERENCY \
	LOG_LEVEL \
	MEASURED_BOOT \
	DISCRETE_TPM \
	DICE_PROTECTION_ENVIRONMENT \
	DRTM_SUPPORT \
	NS_TIMER_SWITCH \
	PL011_GENERIC_UART \
	PLAT_${PLAT} \
	PROGRAMMABLE_RESET_ADDRESS \
	PSCI_EXTENDED_STATE_ID \
	PSCI_OS_INIT_MODE \
	ARCH_FEATURE_AVAILABILITY \
	RESET_TO_BL31 \
	RME_GPT_BITLOCK_BLOCK \
	RME_GPT_MAX_BLOCK \
	SEPARATE_CODE_AND_RODATA \
	SEPARATE_BL2_NOLOAD_REGION \
	SEPARATE_NOBITS_REGION \
	SEPARATE_RWDATA_REGION \
	SEPARATE_SIMD_SECTION \
	RECLAIM_INIT_CODE \
	SPD_${SPD} \
	SPIN_ON_BL1_EXIT \
	SPM_MM \
	SPMC_AT_EL3 \
	SPMC_AT_EL3_SEL0_SP \
	SPMD_SPM_AT_SEL2 \
	TRANSFER_LIST \
	TRUSTED_BOARD_BOOT \
	CRYPTO_SUPPORT \
	TRNG_SUPPORT \
	ERRATA_ABI_SUPPORT \
	ERRATA_NON_ARM_INTERCONNECT \
	USE_COHERENT_MEM \
	USE_DEBUGFS \
	ARM_IO_IN_DTB \
	SDEI_IN_FCONF \
	SEC_INT_DESC_IN_FCONF \
	USE_ROMLIB \
	USE_TBBR_DEFS \
	WARMBOOT_ENABLE_DCACHE_EARLY \
	RESET_TO_BL2 \
	BL2_RUNS_AT_EL3	\
	BL2_IN_XIP_MEM \
	BL2_INV_DCACHE \
	USE_SPINLOCK_CAS \
	ERRATA_SPECULATIVE_AT \
	ERRATA_SME_POWER_DOWN \
	RAS_TRAP_NS_ERR_REC_ACCESS \
	COT_DESC_IN_DTB \
	USE_SP804_TIMER \
	ENABLE_FEAT_RNG \
	ENABLE_FEAT_RNG_TRAP \
	ENABLE_FEAT_SB \
	ENABLE_FEAT_DIT \
	NR_OF_FW_BANKS \
	NR_OF_IMAGES_IN_FW_BANK \
	PSA_FWU_SUPPORT \
	PSA_FWU_METADATA_FW_STORE_DESC \
	ENABLE_BRBE_FOR_NS \
	ENABLE_TRBE_FOR_NS \
	ENABLE_SYS_REG_TRACE_FOR_NS \
	ENABLE_TRF_FOR_NS \
	ENABLE_FEAT_HCX \
	ENABLE_MPMM \
	ENABLE_FEAT_FGT \
	ENABLE_FEAT_FGT2 \
	ENABLE_FEAT_FGWTE3 \
	ENABLE_FEAT_FPMR \
	ENABLE_FEAT_ECV \
	ENABLE_FEAT_AMUv1p1 \
	ENABLE_FEAT_SEL2 \
	ENABLE_FEAT_VHE \
	ENABLE_FEAT_CSV2_2 \
	ENABLE_FEAT_CSV2_3 \
	ENABLE_FEAT_LS64_ACCDATA \
	ENABLE_FEAT_MEC \
	ENABLE_FEAT_PAN \
	ENABLE_FEAT_TCR2 \
	ENABLE_FEAT_THE \
	ENABLE_FEAT_S2PIE \
	ENABLE_FEAT_S1PIE \
	ENABLE_FEAT_S2POE \
	ENABLE_FEAT_S1POE \
	ENABLE_FEAT_SCTLR2 \
	ENABLE_FEAT_D128 \
	ENABLE_FEAT_GCS \
	ENABLE_FEAT_MOPS \
	ENABLE_FEAT_GCIE \
	ENABLE_FEAT_MTE2 \
	FEATURE_DETECTION \
	TWED_DELAY \
	ENABLE_FEAT_TWED \
	CONDITIONAL_CMO \
	IMPDEF_SYSREG_TRAP \
	SVE_VECTOR_LEN \
	ENABLE_SPMD_LP \
	PSA_CRYPTO	\
	ENABLE_CONSOLE_GETC \
	INIT_UNUSED_NS_EL2	\
	PLATFORM_REPORT_CTX_MEM_USE \
	EARLY_CONSOLE \
	PRESERVE_DSU_PMU_REGS \
	HOB_LIST \
	LFA_SUPPORT \
)))

ifeq (${PLATFORM_REPORT_CTX_MEM_USE}, 1)
ifeq (${DEBUG}, 0)
        $(warning "PLATFORM_REPORT_CTX_MEM_USE can be applied when DEBUG=1 only")
        override PLATFORM_REPORT_CTX_MEM_USE := 0
endif
endif

ifeq (${SANITIZE_UB},trap)
        $(eval $(call add_define,MONITOR_TRAPS))
endif #(SANITIZE_UB)

# Define the EL3_PAYLOAD_BASE flag only if it is provided.
ifdef EL3_PAYLOAD_BASE
        $(eval $(call add_define,EL3_PAYLOAD_BASE))
else
# Define the PRELOADED_BL33_BASE flag only if it is provided and
# EL3_PAYLOAD_BASE is not defined, as it has priority.
	ifdef PRELOADED_BL33_BASE
                $(eval $(call add_define,PRELOADED_BL33_BASE))
	endif
endif #(EL3_PAYLOAD_BASE)

# Define the DYN_DISABLE_AUTH flag only if set.
ifeq (${DYN_DISABLE_AUTH},1)
        $(eval $(call add_define,DYN_DISABLE_AUTH))
endif

ifeq ($($(ARCH)-ld-id),arm-link)
        $(eval $(call add_define,USE_ARM_LINK))
endif

# Generate and include sp_gen.mk if SPD is spmd and SP_LAYOUT_FILE is defined
ifeq (${SPD},spmd)
ifdef SP_LAYOUT_FILE
	-include $(BUILD_PLAT)/sp_gen.mk
	FIP_DEPS += sp
	CRT_DEPS += sp
	NEED_SP_PKG := yes
else
	ifeq (${SPMD_SPM_AT_SEL2},1)
                $(error "SPMD with SPM at S-EL2 require SP_LAYOUT_FILE")
	endif
endif #(SP_LAYOUT_FILE)
endif #(SPD)

################################################################################
# Configure the flags for the specified compiler and linker
################################################################################
include ${MAKE_HELPERS_DIRECTORY}cflags.mk

################################################################################
# Build targets
################################################################################

.PHONY:	all msg_start clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool sptool fip sp tl fwu_fip certtool dtbs memmap doc enctool

all: msg_start

msg_start:
	$(s)echo "Building ${PLAT}"

$(eval $(call MAKE_LIB,c))

# Expand build macros for the different images
ifeq (${NEED_BL1},yes)
BL1_SOURCES := $(sort ${BL1_SOURCES})
$(eval $(call MAKE_BL,bl1))
endif #(NEED_BL1)

ifeq (${NEED_BL2},yes)

ifeq (${RESET_TO_BL2}, 0)
FIP_BL2_ARGS := tb-fw
endif

BL2_SOURCES := $(sort ${BL2_SOURCES})

$(if ${BL2}, $(eval $(call TOOL_ADD_IMG,bl2,--${FIP_BL2_ARGS})),\
	$(eval $(call MAKE_BL,bl2,${FIP_BL2_ARGS})))

endif #(NEED_BL2)

ifeq (${NEED_SCP_BL2},yes)
$(eval $(call TOOL_ADD_IMG,scp_bl2,--scp-fw))
endif #(NEED_SCP_BL2)

ifeq (${NEED_BL31},yes)
BL31_SOURCES += ${SPD_SOURCES}
# Sort BL31 source files to remove duplicates
BL31_SOURCES := $(sort ${BL31_SOURCES})
ifneq (${DECRYPTION_SUPPORT},none)
$(if ${BL31}, $(eval $(call TOOL_ADD_IMG,bl31,--soc-fw,,$(ENCRYPT_BL31))),\
	$(eval $(call MAKE_BL,bl31,soc-fw,,$(ENCRYPT_BL31))))
else
$(if ${BL31}, $(eval $(call TOOL_ADD_IMG,bl31,--soc-fw)),\
	$(eval $(call MAKE_BL,bl31,soc-fw)))
endif #(DECRYPTION_SUPPORT)
endif #(NEED_BL31)

# If a BL32 image is needed but neither BL32 nor BL32_SOURCES is defined, the
# build system will call TOOL_ADD_IMG to print a warning message and abort the
# process. Note that the dependency on BL32 applies to the FIP only.
ifeq (${NEED_BL32},yes)
# Sort BL32 source files to remove duplicates
BL32_SOURCES := $(sort ${BL32_SOURCES})
BUILD_BL32 := $(if $(BL32),,$(if $(BL32_SOURCES),1))

ifneq (${DECRYPTION_SUPPORT},none)
$(if ${BUILD_BL32}, $(eval $(call MAKE_BL,bl32,tos-fw,,$(ENCRYPT_BL32))),\
	$(eval $(call TOOL_ADD_IMG,bl32,--tos-fw,,$(ENCRYPT_BL32))))
else
$(if ${BUILD_BL32}, $(eval $(call MAKE_BL,bl32,tos-fw)),\
	$(eval $(call TOOL_ADD_IMG,bl32,--tos-fw)))
endif #(DECRYPTION_SUPPORT)
endif #(NEED_BL32)

# If RMM image is needed but RMM is not defined, Test Realm Payload (TRP)
# needs to be built from RMM_SOURCES.
ifeq (${NEED_RMM},yes)
# Sort RMM source files to remove duplicates
RMM_SOURCES := $(sort ${RMM_SOURCES})
BUILD_RMM := $(if $(RMM),,$(if $(RMM_SOURCES),1))

$(if ${BUILD_RMM}, $(eval $(call MAKE_BL,rmm,rmm-fw)),\
	 $(eval $(call TOOL_ADD_IMG,rmm,--rmm-fw)))
endif #(NEED_RMM)

# Add the BL33 image if required by the platform
ifeq (${NEED_BL33},yes)
$(eval $(call TOOL_ADD_IMG,bl33,--nt-fw))
endif #(NEED_BL33)

ifeq (${NEED_BL2U},yes)
$(if ${BL2U}, $(eval $(call TOOL_ADD_IMG,bl2u,--ap-fwu-cfg,FWU_)),\
	$(eval $(call MAKE_BL,bl2u,ap-fwu-cfg,FWU_)))
endif #(NEED_BL2U)

# Expand build macros for the different images
ifeq (${NEED_FDT},yes)
    $(eval $(call MAKE_DTBS,$(BUILD_PLAT)/fdts,$(FDT_SOURCES)))

    ifneq (${INITRD_SIZE}${INITRD_PATH},)
        ifndef INITRD_BASE
            $(error INITRD_BASE must be set when inserting initrd properties to the DTB.)
        endif

        INITRD_SIZE ?= $(shell printf "0x%x\n" $$(stat -Lc %s $(INITRD_PATH)))
        initrd_end = $(shell printf "0x%x\n" $$(expr $$(($(INITRD_BASE) + $(INITRD_SIZE)))))

        define $(HW_CONFIG)-after +=
            $(s)echo "  INITRD  $(HW_CONFIG)"
            $(q)fdtput -t x $@ /chosen linux,initrd-start $(INITRD_BASE)
            $(q)fdtput -t x $@ /chosen linux,initrd-end $(initrd_end)
        endef
    endif
endif #(NEED_FDT)

# Add Secure Partition packages
ifeq (${NEED_SP_PKG},yes)
$(BUILD_PLAT)/sp_gen.mk: ${SP_MK_GEN} ${SP_LAYOUT_FILE} | $$(@D)/
	$(if $(host-poetry),$(q)poetry -q install --no-root)
	$(q)$(if $(host-poetry),poetry run )${PYTHON} "$<" "$@" $(filter-out $<,$^) $(BUILD_PLAT) ${COT} ${SP_DTS_LIST_FRAGMENT}
sp: $(DTBS) $(BUILD_PLAT)/sp_gen.mk $(SP_PKGS)
	$(s)echo
	$(s)echo "Built SP Images successfully"
	$(s)echo
endif #(NEED_SP_PKG)

locate-checkpatch:
ifndef CHECKPATCH
	$(error "Please set CHECKPATCH to point to the Linux checkpatch.pl file, eg: CHECKPATCH=../linux/scripts/checkpatch.pl")
else
ifeq (,$(wildcard ${CHECKPATCH}))
	$(error "The file CHECKPATCH points to cannot be found, use eg: CHECKPATCH=../linux/scripts/checkpatch.pl")
endif
endif #(CHECKPATCH)

clean:
	$(s)echo "  CLEAN"
	$(q)rm -rf $(BUILD_PLAT)
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=${BUILD_PLAT} --no-print-directory -C ${FIPTOOLPATH} clean
	$(q)rm -rf ${FIPTOOLPATH}/fiptool
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=${BUILD_PLAT} --no-print-directory -C ${CRTTOOLPATH} clean
	$(q)rm -rf ${CRTTOOLPATH}/cert_create
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=${BUILD_PLAT} --no-print-directory -C ${ENCTOOLPATH} clean
	$(q)${MAKE} --no-print-directory -C ${ROMLIBPATH} clean

realclean distclean:
	$(s)echo "  REALCLEAN"
	$(q)rm -rf $(BUILD_BASE)
	$(q)rm -rf $(CURDIR)/cscope.*
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=${BUILD_PLAT} --no-print-directory -C ${FIPTOOLPATH} clean
	$(q)rm -rf ${FIPTOOLPATH}/fiptool
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=${BUILD_PLAT} --no-print-directory -C ${CRTTOOLPATH} clean
	$(q)rm -rf ${CRTTOOLPATH}/cert_create
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=${BUILD_PLAT} --no-print-directory -C ${ENCTOOLPATH} clean
	$(q)${MAKE} --no-print-directory -C ${ROMLIBPATH} clean

checkcodebase:		locate-checkpatch
	$(s)echo "  CHECKING STYLE"
	$(q)if test -d .git ; then						\
		git ls-files | grep -E -v 'libfdt|libc|docs|\.rst' |	\
		while read GIT_FILE ;					\
		do ${CHECKPATCH} ${CHECKCODE_ARGS} -f $$GIT_FILE ;	\
		done ;							\
	else								\
		 find . -type f -not -iwholename "*.git*"		\
		 -not -iwholename "*build*"				\
		 -not -iwholename "*libfdt*"				\
		 -not -iwholename "*libc*"				\
		 -not -iwholename "*docs*"				\
		 -not -iwholename "*.rst"				\
		 -exec ${CHECKPATCH} ${CHECKCODE_ARGS} -f {} \; ;	\
	fi

checkpatch:		locate-checkpatch
	$(s)echo "  CHECKING STYLE"
	$(q)if test -n "${CHECKPATCH_OPTS}"; then				\
		echo "    with ${CHECKPATCH_OPTS} option(s)";		\
	fi
	$(q)COMMON_COMMIT=$$(git merge-base HEAD ${BASE_COMMIT});	\
	for commit in `git rev-list --no-merges $$COMMON_COMMIT..HEAD`;	\
	do								\
		printf "\n[*] Checking style of '$$commit'\n\n";	\
		( git log --format=email "$$commit~..$$commit"		\
			-- ${CHECK_PATHS} ;				\
		  git diff --format=email "$$commit~..$$commit"		\
			-- ${CHECK_PATHS}; ) |				\
			${CHECKPATCH}  ${CHECKPATCH_OPTS} - || true;	\
	done

certtool: ${CRTTOOL}

${CRTTOOL}: FORCE
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=$(abspath ${BUILD_PLAT}) USE_TBBR_DEFS=${USE_TBBR_DEFS} COT=${COT} OPENSSL_DIR=${OPENSSL_DIR} DEBUG=${DEBUG} --no-print-directory -C ${CRTTOOLPATH} all
	$(q)ln -sf ${CRTTOOL} ${CRTTOOLPATH}/cert_create
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

ifneq (${GENERATE_COT},0)
certificates: ${CRT_DEPS} ${CRTTOOL} ${DTBS}
	$(q)${CRTTOOL} ${CRT_ARGS}
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo "Certificates can be found in ${BUILD_PLAT}"
	$(s)echo
endif #(GENERATE_COT)

${BUILD_PLAT}/${FIP_NAME}: ${FIP_DEPS} ${FIPTOOL}
	$(eval ${CHECK_FIP_CMD})
	$(q)${FIPTOOL} create ${FIP_ARGS} $@
	$(q)${FIPTOOL} info $@
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

ifneq (${GENERATE_COT},0)
fwu_certificates: ${FWU_CRT_DEPS} ${CRTTOOL}
	$(q)${CRTTOOL} ${FWU_CRT_ARGS}
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo "FWU certificates can be found in ${BUILD_PLAT}"
	$(s)echo
endif #(GENERATE_COT)

${BUILD_PLAT}/${FWU_FIP_NAME}: ${FWU_FIP_DEPS} ${FIPTOOL}
	$(eval ${CHECK_FWU_FIP_CMD})
	$(q)${FIPTOOL} create ${FWU_FIP_ARGS} $@
	$(q)${FIPTOOL} info $@
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

fiptool: ${FIPTOOL}
fip: ${BUILD_PLAT}/${FIP_NAME}
fwu_fip: ${BUILD_PLAT}/${FWU_FIP_NAME}

# symlink for compatibility before tools were in the build directory
${FIPTOOL}: FORCE
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=$(abspath ${BUILD_PLAT}) CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" OPENSSL_DIR=${OPENSSL_DIR} DEBUG=${DEBUG} --no-print-directory -C ${FIPTOOLPATH} all
	$(q)ln -sf ${FIPTOOL} ${FIPTOOLPATH}/fiptool

$(BUILD_PLAT)/romlib/romlib.bin $(BUILD_PLAT)/lib/libwrappers.a $&: $(BUILD_PLAT)/lib/libfdt.a $(BUILD_PLAT)/lib/libc.a $(CRYPTO_LIB)
	$(q)${MAKE} PLAT_DIR=${PLAT_DIR} BUILD_PLAT=${BUILD_PLAT} ENABLE_BTI=${ENABLE_BTI} CRYPTO_SUPPORT=${CRYPTO_SUPPORT} ARM_ARCH_MINOR=${ARM_ARCH_MINOR} INCLUDES=$(call escape-shell,$(INCLUDES)) DEFINES=$(call escape-shell,$(DEFINES)) --no-print-directory -C ${ROMLIBPATH} all

memmap: all
	$(if $(host-poetry),$(q)poetry -q install --no-root)
	$(q)$(if $(host-poetry),poetry run )memory --root ${BUILD_PLAT} symbols

tl: ${BUILD_PLAT}/tl.bin
${BUILD_PLAT}/tl.bin: ${HW_CONFIG}
	$(if $(host-poetry),$(q)poetry -q install --no-root)
	$(q)$(if $(host-poetry),poetry run )tlc create --fdt $< -s ${FW_HANDOFF_SIZE} $@

doc:
	$(s)echo "  BUILD DOCUMENTATION"
	$(if $(host-poetry),$(q)poetry -q install --with docs --no-root)
	$(q)$(if $(host-poetry),poetry run )${MAKE} --no-print-directory -C ${DOCS_PATH} html

enctool: ${ENCTOOL}

${ENCTOOL}: FORCE
	$(q)${MAKE} PLAT=${PLAT} BUILD_PLAT=$(abspath ${BUILD_PLAT}) BUILD_INFO=0 OPENSSL_DIR=${OPENSSL_DIR} DEBUG=${DEBUG} --no-print-directory -C ${ENCTOOLPATH} all
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

cscope:
	$(s)echo "  CSCOPE"
	$(q)find ${CURDIR} -name "*.[chsS]" > cscope.files
	$(q)cscope -b -q -k

help:
	$(s)echo "usage: ${MAKE} [PLAT=<platform>] [OPTIONS] [TARGET]"
	$(s)echo ""
	$(s)echo "PLAT is used to specify which platform you wish to build."
	$(s)echo "If no platform is specified, PLAT defaults to: ${DEFAULT_PLAT}"
	$(s)echo ""
	$(s)echo "platform = ${PLATFORM_LIST}"
	$(s)echo ""
	$(s)echo "Please refer to the User Guide for a list of all supported options."
	$(s)echo "Note that the build system doesn't track dependencies for build "
	$(s)echo "options. Therefore, if any of the build options are changed "
	$(s)echo "from a previous build, a clean build must be performed."
	$(s)echo ""
	$(s)echo "Supported Targets:"
	$(s)echo "  all            Build all individual bootloader binaries"
	$(s)echo "  bl1            Build the BL1 binary"
	$(s)echo "  bl2            Build the BL2 binary"
	$(s)echo "  bl2u           Build the BL2U binary"
	$(s)echo "  bl31           Build the BL31 binary"
	$(s)echo "  bl32           Build the BL32 binary. If ARCH=aarch32, then "
	$(s)echo "                 this builds secure payload specified by AARCH32_SP"
	$(s)echo "  certificates   Build the certificates (requires 'GENERATE_COT=1')"
	$(s)echo "  fip            Build the Firmware Image Package (FIP)"
	$(s)echo "  fwu_fip        Build the FWU Firmware Image Package (FIP)"
	$(s)echo "  checkcodebase  Check the coding style of the entire source tree"
	$(s)echo "  checkpatch     Check the coding style on changes in the current"
	$(s)echo "                 branch against BASE_COMMIT (default origin/master)"
	$(s)echo "  clean          Clean the build for the selected platform"
	$(s)echo "  cscope         Generate cscope index"
	$(s)echo "  distclean      Remove all build artifacts for all platforms"
	$(s)echo "  certtool       Build the Certificate generation tool"
	$(s)echo "  enctool        Build the Firmware encryption tool"
	$(s)echo "  fiptool        Build the Firmware Image Package (FIP) creation tool"
	$(s)echo "  sp             Build the Secure Partition Packages"
	$(s)echo "  sptool         Build the Secure Partition Package creation tool"
	$(s)echo "  dtbs           Build the Device Tree Blobs (if required for the platform)"
	$(s)echo "  memmap         Print the memory map of the built binaries"
	$(s)echo "  doc            Build html based documentation using Sphinx tool"
	$(s)echo ""
	$(s)echo "Note: most build targets require PLAT to be set to a specific platform."
	$(s)echo ""
	$(s)echo "example: build all targets for the FVP platform:"
	$(s)echo "  CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp all"

.PHONY: FORCE
FORCE:;
