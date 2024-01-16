#
# Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Trusted Firmware Version
#
VERSION_MAJOR			:= 2
VERSION_MINOR			:= 10
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
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

################################################################################
# Default values for build configurations, and their dependencies
################################################################################

include ${MAKE_HELPERS_DIRECTORY}defaults.mk

################################################################################
# Configure the toolchains used to build TF-A and its tools
################################################################################

include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

# Assertions enabled for DEBUG builds by default
ENABLE_ASSERTIONS		:= ${DEBUG}
ENABLE_PMF			:= ${ENABLE_RUNTIME_INSTRUMENTATION}
PLAT				:= ${DEFAULT_PLAT}

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

# Verbose flag
ifeq (${V},0)
	Q:=@
	ECHO:=@echo
	CHECKCODE_ARGS	+=	--no-summary --terse
else
	Q:=
	ECHO:=$(ECHO_QUIET)
endif

ifneq ($(findstring s,$(filter-out --%,$(MAKEFLAGS))),)
	Q:=@
	ECHO:=$(ECHO_QUIET)
endif

export Q ECHO

################################################################################
# Auxiliary tools (fiptool, cert_create, etc)
################################################################################

# Variables for use with Certificate Generation Tool
CRTTOOLPATH		?=	tools/cert_create
CRTTOOL			?=	${CRTTOOLPATH}/cert_create${BIN_EXT}

# Variables for use with Firmware Encryption Tool
ENCTOOLPATH		?=	tools/encrypt_fw
ENCTOOL			?=	${ENCTOOLPATH}/encrypt_fw${BIN_EXT}

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fiptool
FIPTOOL			?=	${FIPTOOLPATH}/fiptool${BIN_EXT}

# Variables for use with sptool
SPTOOLPATH		?=	tools/sptool
SPTOOL			?=	${SPTOOLPATH}/sptool.py
SP_MK_GEN		?=	${SPTOOLPATH}/sp_mk_generator.py
SP_DTS_LIST_FRAGMENT	?=	${BUILD_PLAT}/sp_list_fragment.dts

# Variables for use with ROMLIB
ROMLIBPATH		?=	lib/romlib

# Variable for use with Python
PYTHON			?=	python3

# Variables for use with documentation build using Sphinx tool
DOCS_PATH		?=	docs

################################################################################
# Compiler Configuration based on ARCH_MAJOR and ARCH_MINOR flags
################################################################################
ifeq (${ARM_ARCH_MAJOR},7)
	target32-directive	= 	-target arm-none-eabi
# Will set march-directive from platform configuration
else
	target32-directive	= 	-target armv8a-none-eabi
endif #(ARM_ARCH_MAJOR)

################################################################################
# Get Architecture Feature Modifiers
################################################################################
arch-features		=	${ARM_ARCH_FEATURE}

ifneq ($(filter %-clang,$($(ARCH)-cc-id)),)
	ifeq ($($(ARCH)-cc-id),arm-clang)
		TF_CFLAGS_aarch32	:=	-target arm-arm-none-eabi
		TF_CFLAGS_aarch64	:=	-target aarch64-arm-none-eabi
	else
		TF_CFLAGS_aarch32	=	$(target32-directive)
		TF_CFLAGS_aarch64	:=	-target aarch64-elf
	endif

else ifeq ($($(ARCH)-cc-id),gnu-gcc)
	# Enable LTO only for aarch64
	ifeq (${ARCH},aarch64)
		LTO_CFLAGS	=	$(if $(filter-out 0,$(ENABLE_LTO)),-flto)
	endif
endif #(clang)

# Process Debug flag
$(eval $(call add_define,DEBUG))
ifneq (${DEBUG}, 0)
	BUILD_TYPE	:=	debug
	TF_CFLAGS	+=	-g -gdwarf-4
	ASFLAGS		+=	-g -Wa,-gdwarf-4

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

ifeq (${AARCH32_INSTRUCTION_SET},A32)
	TF_CFLAGS_aarch32	+=	-marm
else ifeq (${AARCH32_INSTRUCTION_SET},T32)
	TF_CFLAGS_aarch32	+=	-mthumb
else
        $(error Error: Unknown AArch32 instruction set ${AARCH32_INSTRUCTION_SET})
endif #(AARCH32_INSTRUCTION_SET)

TF_CFLAGS_aarch32	+=	-mno-unaligned-access
TF_CFLAGS_aarch64	+=	-mgeneral-regs-only -mstrict-align

##############################################################################
# WARNINGS Configuration
###############################################################################
# General warnings
WARNINGS		:=	-Wall -Wmissing-include-dirs -Wunused	\
				-Wdisabled-optimization -Wvla -Wshadow	\
				-Wredundant-decls
# stricter warnings
WARNINGS		+=	-Wextra -Wno-trigraphs
# too verbose for generic build
WARNINGS		+=	-Wno-missing-field-initializers \
				-Wno-type-limits -Wno-sign-compare \
# on clang this flag gets reset if -Wextra is set after it. No difference on gcc
WARNINGS		+=	-Wno-unused-parameter

# Additional warnings
# Level 1 - infrequent warnings we should have none of
# full -Wextra
WARNING1 += -Wsign-compare
WARNING1 += -Wtype-limits
WARNING1 += -Wmissing-field-initializers

# Level 2 - problematic warnings that we want
# zlib, compiler-rt, coreboot, and mbdedtls blow up with these
# TODO: disable just for them and move into default build
WARNING2 += -Wold-style-definition
WARNING2 += -Wmissing-prototypes
WARNING2 += -Wmissing-format-attribute
# TF-A aims to comply with this eventually. Effort too large at present
WARNING2 += -Wundef
# currently very involved and many platforms set this off
WARNING2 += -Wunused-const-variable=2

# Level 3 - very pedantic, frequently ignored
WARNING3 := -Wbad-function-cast
WARNING3 += -Waggregate-return
WARNING3 += -Wnested-externs
WARNING3 += -Wcast-align
WARNING3 += -Wcast-qual
WARNING3 += -Wconversion
WARNING3 += -Wpacked
WARNING3 += -Wpointer-arith
WARNING3 += -Wswitch-default

# Setting W is quite verbose and most warnings will be pre-existing issues
# outside of the contributor's control. Don't fail the build on them so warnings
# can be seen and hopefully addressed
ifdef W
	ifneq (${W},0)
		E	 ?= 0
	endif
endif

ifeq (${W},1)
	WARNINGS += $(WARNING1)
else ifeq (${W},2)
	WARNINGS += $(WARNING1) $(WARNING2)
else ifeq (${W},3)
	WARNINGS += $(WARNING1) $(WARNING2) $(WARNING3)
endif #(W)

# Compiler specific warnings
ifeq ($(filter %-clang,$($(ARCH)-cc-id)),)
# not using clang
WARNINGS	+=		-Wunused-but-set-variable -Wmaybe-uninitialized	\
				-Wpacked-bitfield-compat -Wshift-overflow=2 \
				-Wlogical-op

# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
TF_CFLAGS		+= 	$(call cc_option, --param=min-pagesize=0)

ifeq ($(HARDEN_SLS), 1)
        TF_CFLAGS_aarch64       +=      $(call cc_option, -mharden-sls=all)
endif

else
# using clang
WARNINGS	+=		-Wshift-overflow -Wshift-sign-overflow \
				-Wlogical-op-parentheses
endif #(Clang Warning)

ifneq (${E},0)
	ERRORS := -Werror
endif #(E)

################################################################################
# Compiler and Linker Directives
################################################################################
CPPFLAGS		=	${DEFINES} ${INCLUDES} ${MBEDTLS_INC} -nostdinc	\
				$(ERRORS) $(WARNINGS)
ASFLAGS			+=	$(CPPFLAGS)                 			\
				-ffreestanding -Wa,--fatal-warnings
TF_CFLAGS		+=	$(CPPFLAGS) $(TF_CFLAGS_$(ARCH))		\
				-ffunction-sections -fdata-sections		\
				-ffreestanding -fno-builtin -fno-common		\
				-Os -std=gnu99

ifeq (${SANITIZE_UB},on)
	TF_CFLAGS	+=	-fsanitize=undefined -fno-sanitize-recover
endif #(${SANITIZE_UB},on)

ifeq (${SANITIZE_UB},trap)
	TF_CFLAGS	+=	-fsanitize=undefined -fno-sanitize-recover	\
				-fsanitize-undefined-trap-on-error
endif #(${SANITIZE_UB},trap)

GCC_V_OUTPUT		:=	$(shell $($(ARCH)-cc) -v 2>&1)

TF_LDFLAGS		+=	-z noexecstack

# LD = armlink
ifeq ($($(ARCH)-ld-id),arm-link)
	TF_LDFLAGS		+=	--diag_error=warning --lto_level=O1
	TF_LDFLAGS		+=	--remove --info=unused,unusedsymbols
	TF_LDFLAGS		+=	$(TF_LDFLAGS_$(ARCH))

# LD = gcc (used when GCC LTO is enabled)
else ifeq ($($(ARCH)-ld-id),gnu-gcc)
	# Pass ld options with Wl or Xlinker switches
	TF_LDFLAGS		+=	$(call ld_option,-Xlinker --no-warn-rwx-segments)
	TF_LDFLAGS		+=	-Wl,--fatal-warnings -O1
	TF_LDFLAGS		+=	-Wl,--gc-sections

	TF_LDFLAGS		+=	-Wl,-z,common-page-size=4096 #Configure page size constants
	TF_LDFLAGS		+=	-Wl,-z,max-page-size=4096
	TF_LDFLAGS		+=	-Wl,--build-id=none

	ifeq ($(ENABLE_LTO),1)
		ifeq (${ARCH},aarch64)
			TF_LDFLAGS	+=	-flto -fuse-linker-plugin
			TF_LDFLAGS      +=	-flto-partition=one
		endif
	endif #(ENABLE_LTO)

# GCC automatically adds fix-cortex-a53-843419 flag when used to link
# which breaks some builds, so disable if errata fix is not explicitly enabled
	ifeq (${ARCH},aarch64)
		ifneq (${ERRATA_A53_843419},1)
			TF_LDFLAGS	+= 	-mno-fix-cortex-a53-843419
		endif
	endif
	TF_LDFLAGS		+= 	-nostdlib
	TF_LDFLAGS		+=	$(subst --,-Xlinker --,$(TF_LDFLAGS_$(ARCH)))

# LD = gcc-ld (ld) or llvm-ld (ld.lld) or other
else
# With ld.bfd version 2.39 and newer new warnings are added. Skip those since we
# are not loaded by a elf loader.
	TF_LDFLAGS		+=	$(call ld_option, --no-warn-rwx-segments)
	TF_LDFLAGS		+=	-O1
	TF_LDFLAGS		+=	--gc-sections

	TF_LDFLAGS		+=	-z common-page-size=4096 # Configure page size constants
	TF_LDFLAGS		+=	-z max-page-size=4096
	TF_LDFLAGS		+=	--build-id=none

# ld.lld doesn't recognize the errata flags,
# therefore don't add those in that case.
# ld.lld reports section type mismatch warnings,
# therefore don't add --fatal-warnings to it.
	ifneq ($($(ARCH)-ld-id),llvm-lld)
		TF_LDFLAGS	+=	$(TF_LDFLAGS_$(ARCH)) --fatal-warnings
	endif

endif #(LD = armlink)

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

DTC_FLAGS		+=	-I dts -O dtb
DTC_CPPFLAGS		+=	-P -nostdinc $(INCLUDES) -Ifdts -undef \
				-x assembler-with-cpp $(DEFINES)

include common/backtrace/backtrace.mk

################################################################################
# Generic definitions
################################################################################
include ${MAKE_HELPERS_DIRECTORY}plat_helpers.mk

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
	ifeq (${ARCH},aarch32)
                $(error "Error: SPD is incompatible with AArch32.")
	endif

	ifdef EL3_PAYLOAD_BASE
                $(warning "SPD and EL3_PAYLOAD_BASE are incompatible build options.")
                $(warning "The SPD and its BL32 companion will be present but \
                ignored.")
	endif

	ifeq (${SPD},spmd)
	# SPMD is located in std_svc directory
		SPD_DIR := std_svc

		ifeq ($(SPMD_SPM_AT_SEL2),1)
			CTX_INCLUDE_EL2_REGS := 1
			ifeq ($(SPMC_AT_EL3),1)
                                $(error SPM cannot be enabled in both S-EL2 and EL3.)
			endif
		endif

		ifeq ($(findstring optee_sp,$(ARM_SPMC_MANIFEST_DTS)),optee_sp)
			DTC_CPPFLAGS	+=	-DOPTEE_SP_FW_CONFIG
		endif

		ifeq ($(findstring trusty_sp,$(ARM_SPMC_MANIFEST_DTS)),trusty_sp)
			DTC_CPPFLAGS	+=	-DTRUSTY_SP_FW_CONFIG
		endif

		ifeq ($(TS_SP_FW_CONFIG),1)
			DTC_CPPFLAGS	+=	-DTS_SP_FW_CONFIG
		endif

		ifneq ($(ARM_BL2_SP_LIST_DTS),)
		DTC_CPPFLAGS += -DARM_BL2_SP_LIST_DTS=$(ARM_BL2_SP_LIST_DTS)
		endif

		ifneq ($(SP_LAYOUT_FILE),)
		BL2_ENABLE_SP_LOAD := 1
		endif

		ifeq ($(SPMC_AT_EL3_SEL0_SP),1)
			ifneq ($(SPMC_AT_EL3),1)
			$(error SEL0 SP cannot be enabled without SPMC at EL3)
			endif
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

ifeq (${ENABLE_SPMD_LP}, 1)
ifneq (${SPD},spmd)
        $(error Error: ENABLE_SPMD_LP requires SPD=spmd.)
endif
ifeq ($(SPMC_AT_EL3),1)
        $(error SPMC at EL3 not supported when enabling SPMD Logical partitions.)
endif
endif

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
else
        $(error Unknown BRANCH_PROTECTION value ${BRANCH_PROTECTION})
endif #(BRANCH_PROTECTION)

ifeq ($(ENABLE_PAUTH),1)
	CTX_INCLUDE_PAUTH_REGS := 1
endif
ifneq (${BP_OPTION},none)
	TF_CFLAGS_aarch64	+=	-mbranch-protection=${BP_OPTION}
endif #(BP_OPTION)

# Pointer Authentication sources
ifeq (${ENABLE_PAUTH}, 1)
# arm/common/aarch64/arm_pauth.c contains a sample platform hook to complete the
# Pauth support. As it's not secure, it must be reimplemented for real platforms
	BL_COMMON_SOURCES	+=	lib/extensions/pauth/pauth_helpers.S
endif

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

####################################################
# Enable required options for Memory Stack Tagging.
####################################################

# Currently, these options are enabled only for clang and armclang compiler.
ifeq (${SUPPORT_STACK_MEMTAG},yes)
    ifdef mem_tag_arch_support
        # Check for armclang and clang compilers
        ifneq ($(filter %-clang,$($(ARCH)-cc-id)),)
        # Add "memtag" architecture feature modifier if not specified
            ifeq ( ,$(findstring memtag,$(arch-features)))
                arch-features	:=	$(arch-features)+memtag
            endif	# memtag
            ifeq ($($(ARCH)-cc-id),arm-clang)
                TF_CFLAGS	+=	-mmemtag-stack
            else ifeq ($($(ARCH)-cc-id),llvm-clang)
                TF_CFLAGS	+=	-fsanitize=memtag
            endif	# armclang
        endif
    else
        $(error "Error: stack memory tagging is not supported for  \
        architecture ${ARCH},armv${ARM_ARCH_MAJOR}.${ARM_ARCH_MINOR}-a")
	endif #(mem_tag_arch_support)
endif #(SUPPORT_STACK_MEMTAG)

################################################################################
# RME dependent flags configuration, Enable optional features for RME.
################################################################################
# FEAT_RME
ifeq (${ENABLE_RME},1)
	# RME doesn't support BRBE
	ENABLE_BRBE_FOR_NS := 0

	# RME doesn't support PIE
	ifneq (${ENABLE_PIE},0)
                $(error ENABLE_RME does not support PIE)
	endif

	# RME doesn't support BRBE
	ifneq (${ENABLE_BRBE_FOR_NS},0)
                $(error ENABLE_RME does not support BRBE.)
	endif

	# RME requires AARCH64
	ifneq (${ARCH},aarch64)
                $(error ENABLE_RME requires AArch64)
	endif

	# RME requires el2 context to be saved for now.
	CTX_INCLUDE_EL2_REGS := 1
	CTX_INCLUDE_AARCH32_REGS := 0
	CTX_INCLUDE_PAUTH_REGS := 1

	# RME enables CSV2_2 extension by default.
	ENABLE_FEAT_CSV2_2 = 1
endif #(FEAT_RME)

################################################################################
# Include rmmd Makefile if RME is enabled
################################################################################
ifneq (${ENABLE_RME},0)
	ifneq (${ARCH},aarch64)
                $(error ENABLE_RME requires AArch64)
	endif
	ifeq ($(SPMC_AT_EL3),1)
                $(error SPMC_AT_EL3 and ENABLE_RME cannot both be enabled.)
	endif

	ifneq (${SPD}, none)
		ifneq (${SPD}, spmd)
                        $(error ENABLE_RME is incompatible with SPD=${SPD}. Use SPD=spmd)
		endif
	endif
include services/std_svc/rmmd/rmmd.mk
$(warning "RME is an experimental feature")
endif

ifeq (${CTX_INCLUDE_EL2_REGS}, 1)
	ifeq (${SPD},none)
		ifeq (${ENABLE_RME},0)
                        $(error CTX_INCLUDE_EL2_REGS is available only when SPD \
                        or RME is enabled)
		endif
	endif
endif

################################################################################
# Platform specific Makefile might provide us ARCH_MAJOR/MINOR use that to come
# up with appropriate march values for compiler.
################################################################################
include ${MAKE_HELPERS_DIRECTORY}march.mk

TF_CFLAGS   +=	$(march-directive)
ASFLAGS		+=	$(march-directive)

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

$(eval $(call MAKE_PREREQ_DIR,${BUILD_PLAT}))

ifeq (${ARM_ARCH_MAJOR},7)
include make_helpers/armv7-a-cpus.mk
endif

PIE_FOUND		:=	$(findstring --enable-default-pie,${GCC_V_OUTPUT})
ifneq ($(PIE_FOUND),)
	TF_CFLAGS	+=	-fno-PIE
ifeq ($($(ARCH)-ld-id),gnu-gcc)
	TF_LDFLAGS	+=	-no-pie
endif
endif #(PIE_FOUND)

ifeq ($($(ARCH)-ld-id),gnu-gcc)
	PIE_LDFLAGS	+=	-Wl,-pie -Wl,--no-dynamic-linker
else
	PIE_LDFLAGS	+=	-pie --no-dynamic-linker
endif

ifeq ($(ENABLE_PIE),1)
	ifeq ($(RESET_TO_BL2),1)
		ifneq ($(BL2_IN_XIP_MEM),1)
			BL2_CPPFLAGS	+=	-fpie
			BL2_CFLAGS	+=	-fpie
			BL2_LDFLAGS	+=	$(PIE_LDFLAGS)
		endif #(BL2_IN_XIP_MEM)
	endif #(RESET_TO_BL2)
	BL31_CPPFLAGS	+=	-fpie
	BL31_CFLAGS 	+=	-fpie
	BL31_LDFLAGS	+=	$(PIE_LDFLAGS)

	BL32_CPPFLAGS	+=	-fpie
	BL32_CFLAGS	+=	-fpie
	BL32_LDFLAGS	+=	$(PIE_LDFLAGS)
endif #(ENABLE_PIE)

BL1_CPPFLAGS  += -DREPORT_ERRATA=${DEBUG}
BL31_CPPFLAGS += -DREPORT_ERRATA=${DEBUG}
BL32_CPPFLAGS += -DREPORT_ERRATA=${DEBUG}

BL1_CPPFLAGS += -DIMAGE_AT_EL3
ifeq ($(RESET_TO_BL2),1)
	BL2_CPPFLAGS += -DIMAGE_AT_EL3
else
	BL2_CPPFLAGS += -DIMAGE_AT_EL1
endif #(RESET_TO_BL2)

ifeq (${ARCH},aarch64)
	BL2U_CPPFLAGS += -DIMAGE_AT_EL1
	BL31_CPPFLAGS += -DIMAGE_AT_EL3
	BL32_CPPFLAGS += -DIMAGE_AT_EL1
else
	BL32_CPPFLAGS += -DIMAGE_AT_EL3
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

################################################################################
# Check incompatible options and dependencies
################################################################################

# USE_DEBUGFS experimental feature recommended only in debug builds
ifeq (${USE_DEBUGFS},1)
        ifeq (${DEBUG},1)
                $(warning DEBUGFS experimental feature is enabled.)
        else
                $(warning DEBUGFS experimental, recommended in DEBUG builds ONLY)
        endif
endif #(USE_DEBUGFS)

# USE_SPINLOCK_CAS requires AArch64 build
ifeq (${USE_SPINLOCK_CAS},1)
        ifneq (${ARCH},aarch64)
               $(error USE_SPINLOCK_CAS requires AArch64)
        endif
endif #(USE_SPINLOCK_CAS)

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

ifdef EL3_PAYLOAD_BASE
	ifdef PRELOADED_BL33_BASE
                $(warning "PRELOADED_BL33_BASE and EL3_PAYLOAD_BASE are \
		incompatible build options. EL3_PAYLOAD_BASE has priority.")
	endif
	ifneq (${GENERATE_COT},0)
                $(error "GENERATE_COT and EL3_PAYLOAD_BASE are incompatible \
                build options.")
	endif
	ifneq (${TRUSTED_BOARD_BOOT},0)
                $(error "TRUSTED_BOARD_BOOT and EL3_PAYLOAD_BASE are \
                incompatible \ build options.")
	endif
endif #(EL3_PAYLOAD_BASE)

ifeq (${NEED_BL33},yes)
	ifdef EL3_PAYLOAD_BASE
                $(warning "BL33 image is not needed when option \
                BL33_PAYLOAD_BASE is used and won't be added to the FIP file.")
	endif
	ifdef PRELOADED_BL33_BASE
                $(warning "BL33 image is not needed when option \
                PRELOADED_BL33_BASE is used and won't be added to the FIP file.")
	endif
endif #(NEED_BL33)

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
ifeq ($(HW_ASSISTED_COHERENCY)-$(USE_COHERENT_MEM),1-1)
        $(error USE_COHERENT_MEM cannot be enabled with HW_ASSISTED_COHERENCY)
endif

#For now, BL2_IN_XIP_MEM is only supported when RESET_TO_BL2 is 1.
ifeq ($(RESET_TO_BL2)-$(BL2_IN_XIP_MEM),0-1)
        $(error "BL2_IN_XIP_MEM is only supported when RESET_TO_BL2 is enabled")
endif

# RAS_EXTENSION is deprecated, provide alternate build options
ifeq ($(RAS_EXTENSION),1)
        $(error "RAS_EXTENSION is now deprecated, please use ENABLE_FEAT_RAS \
        and HANDLE_EA_EL3_FIRST_NS instead")
endif


# When FAULT_INJECTION_SUPPORT is used, require that FEAT_RAS is enabled
ifeq ($(FAULT_INJECTION_SUPPORT),1)
	ifeq ($(ENABLE_FEAT_RAS),0)
                $(error For FAULT_INJECTION_SUPPORT, ENABLE_FEAT_RAS must not be 0)
	endif
endif #(FAULT_INJECTION_SUPPORT)

# DYN_DISABLE_AUTH can be set only when TRUSTED_BOARD_BOOT=1
ifeq ($(DYN_DISABLE_AUTH), 1)
	ifeq (${TRUSTED_BOARD_BOOT}, 0)
                $(error "TRUSTED_BOARD_BOOT must be enabled for DYN_DISABLE_AUTH \
                to be set.")
	endif
endif #(DYN_DISABLE_AUTH)

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

# SDEI_IN_FCONF is only supported when SDEI_SUPPORT is enabled.
ifeq ($(SDEI_SUPPORT)-$(SDEI_IN_FCONF),0-1)
        $(error "SDEI_IN_FCONF is only supported when SDEI_SUPPORT is enabled")
endif

# If pointer authentication is used in the firmware, make sure that all the
# registers associated to it are also saved and restored.
# Not doing it would leak the value of the keys used by EL3 to EL1 and S-EL1.
ifeq ($(ENABLE_PAUTH),1)
	ifeq ($(CTX_INCLUDE_PAUTH_REGS),0)
                $(error Pointer Authentication requires CTX_INCLUDE_PAUTH_REGS=1)
	endif
endif #(ENABLE_PAUTH)

ifeq ($(CTX_INCLUDE_PAUTH_REGS),1)
	ifneq (${ARCH},aarch64)
                $(error CTX_INCLUDE_PAUTH_REGS requires AArch64)
	endif
endif #(CTX_INCLUDE_PAUTH_REGS)

ifeq ($(PSA_FWU_SUPPORT),1)
        $(info PSA_FWU_SUPPORT is an experimental feature)
endif #(PSA_FWU_SUPPORT)

ifeq ($(FEATURE_DETECTION),1)
        $(info FEATURE_DETECTION is an experimental feature)
endif #(FEATURE_DETECTION)

ifneq ($(ENABLE_SME2_FOR_NS), 0)
	ifeq (${ENABLE_SME_FOR_NS}, 0)
                $(warning "ENABLE_SME2_FOR_NS requires ENABLE_SME_FOR_NS also \
                to be set")
                $(warning "Forced ENABLE_SME_FOR_NS=1")
		override ENABLE_SME_FOR_NS	:= 1
	endif
endif #(ENABLE_SME2_FOR_NS)

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
	ifeq (${ALLOW_RO_XLAT_TABLES}, 1)
                $(error "ALLOW_RO_XLAT_TABLES requires translation tables \
                library v2")
	endif
endif #(ARM_XLAT_TABLES_LIB_V1)

ifneq (${DECRYPTION_SUPPORT},none)
	ifeq (${TRUSTED_BOARD_BOOT}, 0)
                $(error TRUSTED_BOARD_BOOT must be enabled for DECRYPTION_SUPPORT \
                to be set)
	endif
endif #(DECRYPTION_SUPPORT)

# Ensure that no Aarch64-only features are enabled in Aarch32 build
ifeq (${ARCH},aarch32)

	# SME/SVE only supported on AArch64
	ifneq (${ENABLE_SME_FOR_NS},0)
                $(error "ENABLE_SME_FOR_NS cannot be used with ARCH=aarch32")
	endif

	ifeq (${ENABLE_SVE_FOR_NS},1)
		# Warning instead of error due to CI dependency on this
                $(error "ENABLE_SVE_FOR_NS cannot be used with ARCH=aarch32")
	endif

	# BRBE is not supported in AArch32
	ifeq (${ENABLE_BRBE_FOR_NS},1)
                $(error "ENABLE_BRBE_FOR_NS cannot be used with ARCH=aarch32")
	endif

	# FEAT_RNG_TRAP is not supported in AArch32
	ifeq (${ENABLE_FEAT_RNG_TRAP},1)
                $(error "ENABLE_FEAT_RNG_TRAP cannot be used with ARCH=aarch32")
	endif
endif #(ARCH=aarch32)

ifneq (${ENABLE_SME_FOR_NS},0)
	ifeq (${ENABLE_SVE_FOR_NS},0)
                $(error "ENABLE_SME_FOR_NS requires ENABLE_SVE_FOR_NS")
	endif
endif #(ENABLE_SME_FOR_NS)

# Secure SME/SVE requires the non-secure component as well
ifeq (${ENABLE_SME_FOR_SWD},1)
	ifeq (${ENABLE_SME_FOR_NS},0)
                $(error "ENABLE_SME_FOR_SWD requires ENABLE_SME_FOR_NS")
	endif
	ifeq (${ENABLE_SVE_FOR_SWD},0)
                $(error "ENABLE_SME_FOR_SWD requires ENABLE_SVE_FOR_SWD")
	endif
endif #(ENABLE_SME_FOR_SWD)

ifeq (${ENABLE_SVE_FOR_SWD},1)
	ifeq (${ENABLE_SVE_FOR_NS},0)
                $(error "ENABLE_SVE_FOR_SWD requires ENABLE_SVE_FOR_NS")
	endif
endif #(ENABLE_SVE_FOR_SWD)

# SVE and SME cannot be used with CTX_INCLUDE_FPREGS since secure manager does
# its own context management including FPU registers.
ifeq (${CTX_INCLUDE_FPREGS},1)
	ifneq (${ENABLE_SME_FOR_NS},0)
                $(error "ENABLE_SME_FOR_NS cannot be used with CTX_INCLUDE_FPREGS")
	endif

	ifeq (${ENABLE_SVE_FOR_NS},1)
		# Warning instead of error due to CI dependency on this
                $(warning "ENABLE_SVE_FOR_NS cannot be used with CTX_INCLUDE_FPREGS")
                $(warning "Forced ENABLE_SVE_FOR_NS=0")
		override ENABLE_SVE_FOR_NS	:= 0
	endif
endif #(CTX_INCLUDE_FPREGS)

ifeq ($(DRTM_SUPPORT),1)
        $(info DRTM_SUPPORT is an experimental feature)
endif

ifeq (${TRANSFER_LIST},1)
        $(info TRANSFER_LIST is an experimental feature)
endif

ifeq (${ENABLE_RME},1)
	ifneq (${SEPARATE_CODE_AND_RODATA},1)
                $(error `ENABLE_RME=1` requires `SEPARATE_CODE_AND_RODATA=1`)
	endif
endif

ifeq ($(PSA_CRYPTO),1)
        $(info PSA_CRYPTO is an experimental feature)
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
	CTX_INCLUDE_EL2_REGS \
	CTX_INCLUDE_MPAM_REGS \
	DEBUG \
	DYN_DISABLE_AUTH \
	EL3_EXCEPTION_HANDLING \
	ENABLE_AMU_AUXILIARY_COUNTERS \
	ENABLE_AMU_FCONF \
	AMU_RESTRICT_COUNTERS \
	ENABLE_ASSERTIONS \
	ENABLE_PIE \
	ENABLE_PMF \
	ENABLE_PSCI_STAT \
	ENABLE_RUNTIME_INSTRUMENTATION \
	ENABLE_SME_FOR_SWD \
	ENABLE_SVE_FOR_SWD \
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
	DICE_PROTECTION_ENVIRONMENT \
	DRTM_SUPPORT \
	NS_TIMER_SWITCH \
	OVERRIDE_LIBC \
	PL011_GENERIC_UART \
	PROGRAMMABLE_RESET_ADDRESS \
	PSCI_EXTENDED_STATE_ID \
	PSCI_OS_INIT_MODE \
	RESET_TO_BL31 \
	SAVE_KEYS \
	SEPARATE_CODE_AND_RODATA \
	SEPARATE_BL2_NOLOAD_REGION \
	SEPARATE_NOBITS_REGION \
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
	RAS_TRAP_NS_ERR_REC_ACCESS \
	COT_DESC_IN_DTB \
	USE_SP804_TIMER \
	PSA_FWU_SUPPORT \
	PSA_FWU_METADATA_FW_STORE_DESC \
	ENABLE_MPMM \
	ENABLE_MPMM_FCONF \
	FEATURE_DETECTION \
	TRNG_SUPPORT \
	ERRATA_ABI_SUPPORT \
	ERRATA_NON_ARM_INTERCONNECT \
	CONDITIONAL_CMO \
	PSA_CRYPTO	\
	ENABLE_CONSOLE_GETC \
	INIT_UNUSED_NS_EL2	\
	PLATFORM_REPORT_CTX_MEM_USE \
	EARLY_CONSOLE \
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
	ENABLE_FEAT_AMU \
	ENABLE_FEAT_AMUv1p1 \
	ENABLE_FEAT_CSV2_2 \
	ENABLE_FEAT_CSV2_3 \
	ENABLE_FEAT_DIT \
	ENABLE_FEAT_ECV \
	ENABLE_FEAT_FGT \
	ENABLE_FEAT_HCX \
	ENABLE_FEAT_MTE2 \
	ENABLE_FEAT_PAN \
	ENABLE_FEAT_RNG \
	ENABLE_FEAT_RNG_TRAP \
	ENABLE_FEAT_SEL2 \
	ENABLE_FEAT_TCR2 \
	ENABLE_FEAT_SB \
	ENABLE_FEAT_S2PIE \
	ENABLE_FEAT_S1PIE \
	ENABLE_FEAT_S2POE \
	ENABLE_FEAT_S1POE \
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
	CTX_INCLUDE_PAUTH_REGS \
	CTX_INCLUDE_MPAM_REGS \
	EL3_EXCEPTION_HANDLING \
	CTX_INCLUDE_EL2_REGS \
	CTX_INCLUDE_NEVE_REGS \
	DECRYPTION_SUPPORT_${DECRYPTION_SUPPORT} \
	DISABLE_MTPMU \
	ENABLE_FEAT_AMU \
	ENABLE_AMU_AUXILIARY_COUNTERS \
	ENABLE_AMU_FCONF \
	AMU_RESTRICT_COUNTERS \
	ENABLE_ASSERTIONS \
	ENABLE_BTI \
	ENABLE_FEAT_MPAM \
	ENABLE_PAUTH \
	ENABLE_PIE \
	ENABLE_PMF \
	ENABLE_PSCI_STAT \
	ENABLE_RME \
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
	DICE_PROTECTION_ENVIRONMENT \
	DRTM_SUPPORT \
	NS_TIMER_SWITCH \
	PL011_GENERIC_UART \
	PLAT_${PLAT} \
	PROGRAMMABLE_RESET_ADDRESS \
	PSCI_EXTENDED_STATE_ID \
	PSCI_OS_INIT_MODE \
	RESET_TO_BL31 \
	SEPARATE_CODE_AND_RODATA \
	SEPARATE_BL2_NOLOAD_REGION \
	SEPARATE_NOBITS_REGION \
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
	ENABLE_MPMM_FCONF \
	ENABLE_FEAT_FGT \
	ENABLE_FEAT_ECV \
	ENABLE_FEAT_AMUv1p1 \
	ENABLE_FEAT_SEL2 \
	ENABLE_FEAT_VHE \
	ENABLE_FEAT_CSV2_2 \
	ENABLE_FEAT_CSV2_3 \
	ENABLE_FEAT_PAN \
	ENABLE_FEAT_TCR2 \
	ENABLE_FEAT_S2PIE \
	ENABLE_FEAT_S1PIE \
	ENABLE_FEAT_S2POE \
	ENABLE_FEAT_S1POE \
	ENABLE_FEAT_GCS \
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
# Build targets
################################################################################

.PHONY:	all msg_start clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool sptool fip sp fwu_fip certtool dtbs memmap doc enctool
.SUFFIXES:

all: msg_start

msg_start:
	@echo "Building ${PLAT}"

ifeq (${ERROR_DEPRECATED},0)
# Check if deprecated declarations and cpp warnings should be treated as error or not.
ifneq ($(filter %-clang,$($(ARCH)-cc-id)),)
    CPPFLAGS		+= 	-Wno-error=deprecated-declarations
else
    CPPFLAGS		+= 	-Wno-error=deprecated-declarations -Wno-error=cpp
endif
endif #(!ERROR_DEPRECATED)

$(eval $(call MAKE_LIB_DIRS))
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
endif #(NEED_FDT)

# Add Secure Partition packages
ifeq (${NEED_SP_PKG},yes)
$(BUILD_PLAT)/sp_gen.mk: ${SP_MK_GEN} ${SP_LAYOUT_FILE} | ${BUILD_PLAT}
	@${PYTHON} "$<" "$@" $(filter-out $<,$^) $(BUILD_PLAT) ${COT} ${SP_DTS_LIST_FRAGMENT}
sp: $(DTBS) $(BUILD_PLAT)/sp_gen.mk $(SP_PKGS)
	@${ECHO_BLANK_LINE}
	@echo "Built SP Images successfully"
	@${ECHO_BLANK_LINE}
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
	@echo "  CLEAN"
	$(call SHELL_REMOVE_DIR,${BUILD_PLAT})
ifdef UNIX_MK
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
else
# Clear the MAKEFLAGS as we do not want
# to pass the gnumake flags to nmake.
	${Q}set MAKEFLAGS= && ${MSVC_NMAKE} /nologo /f ${FIPTOOLPATH}/Makefile.msvc FIPTOOLPATH=$(subst /,\,$(FIPTOOLPATH)) FIPTOOL=$(subst /,\,$(FIPTOOL)) clean
endif #(UNIX_MK)
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH} clean
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${ENCTOOLPATH} clean
	${Q}${MAKE} --no-print-directory -C ${ROMLIBPATH} clean

realclean distclean:
	@echo "  REALCLEAN"
	$(call SHELL_REMOVE_DIR,${BUILD_BASE})
	$(call SHELL_DELETE_ALL, ${CURDIR}/cscope.*)
ifdef UNIX_MK
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
else
# Clear the MAKEFLAGS as we do not want
# to pass the gnumake flags to nmake.
	${Q}set MAKEFLAGS= && ${MSVC_NMAKE} /nologo /f ${FIPTOOLPATH}/Makefile.msvc FIPTOOLPATH=$(subst /,\,$(FIPTOOLPATH)) FIPTOOL=$(subst /,\,$(FIPTOOL)) realclean
endif #(UNIX_MK)
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH} realclean
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${ENCTOOLPATH} realclean
	${Q}${MAKE} --no-print-directory -C ${ROMLIBPATH} clean

checkcodebase:		locate-checkpatch
	@echo "  CHECKING STYLE"
	@if test -d .git ; then						\
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
	@echo "  CHECKING STYLE"
	@if test -n "${CHECKPATCH_OPTS}"; then				\
		echo "    with ${CHECKPATCH_OPTS} option(s)";		\
	fi
	${Q}COMMON_COMMIT=$$(git merge-base HEAD ${BASE_COMMIT});	\
	for commit in `git rev-list --no-merges $$COMMON_COMMIT..HEAD`;	\
	do								\
		printf "\n[*] Checking style of '$$commit'\n\n";	\
		git log --format=email "$$commit~..$$commit"		\
			-- ${CHECK_PATHS} |				\
			${CHECKPATCH} ${CHECKPATCH_OPTS} - || true;	\
		git diff --format=email "$$commit~..$$commit"		\
			-- ${CHECK_PATHS} |				\
			${CHECKPATCH}  ${CHECKPATCH_OPTS} - || true;	\
	done

certtool: ${CRTTOOL}

${CRTTOOL}: FORCE
	${Q}${MAKE} PLAT=${PLAT} USE_TBBR_DEFS=${USE_TBBR_DEFS} COT=${COT} OPENSSL_DIR=${OPENSSL_DIR} CRTTOOL=${CRTTOOL} DEBUG=${DEBUG} V=${V} --no-print-directory -C ${CRTTOOLPATH} all
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

ifneq (${GENERATE_COT},0)
certificates: ${CRT_DEPS} ${CRTTOOL}
	${Q}${CRTTOOL} ${CRT_ARGS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@echo "Certificates can be found in ${BUILD_PLAT}"
	@${ECHO_BLANK_LINE}
endif #(GENERATE_COT)

${BUILD_PLAT}/${FIP_NAME}: ${FIP_DEPS} ${FIPTOOL}
	$(eval ${CHECK_FIP_CMD})
	${Q}${FIPTOOL} create ${FIP_ARGS} $@
	${Q}${FIPTOOL} info $@
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

ifneq (${GENERATE_COT},0)
fwu_certificates: ${FWU_CRT_DEPS} ${CRTTOOL}
	${Q}${CRTTOOL} ${FWU_CRT_ARGS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@echo "FWU certificates can be found in ${BUILD_PLAT}"
	@${ECHO_BLANK_LINE}
endif #(GENERATE_COT)

${BUILD_PLAT}/${FWU_FIP_NAME}: ${FWU_FIP_DEPS} ${FIPTOOL}
	$(eval ${CHECK_FWU_FIP_CMD})
	${Q}${FIPTOOL} create ${FWU_FIP_ARGS} $@
	${Q}${FIPTOOL} info $@
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

fiptool: ${FIPTOOL}
fip: ${BUILD_PLAT}/${FIP_NAME}
fwu_fip: ${BUILD_PLAT}/${FWU_FIP_NAME}

${FIPTOOL}: FORCE
ifdef UNIX_MK
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" FIPTOOL=${FIPTOOL} OPENSSL_DIR=${OPENSSL_DIR} DEBUG=${DEBUG} V=${V} --no-print-directory -C ${FIPTOOLPATH} all
else
# Clear the MAKEFLAGS as we do not want
# to pass the gnumake flags to nmake.
	${Q}set MAKEFLAGS= && ${MSVC_NMAKE} /nologo /f ${FIPTOOLPATH}/Makefile.msvc FIPTOOLPATH=$(subst /,\,$(FIPTOOLPATH)) FIPTOOL=$(subst /,\,$(FIPTOOL))
endif #(UNIX_MK)

romlib.bin: libraries FORCE
	${Q}${MAKE} PLAT_DIR=${PLAT_DIR} BUILD_PLAT=${BUILD_PLAT} ENABLE_BTI=${ENABLE_BTI} ARM_ARCH_MINOR=${ARM_ARCH_MINOR} INCLUDES='${INCLUDES}' DEFINES='${DEFINES}' --no-print-directory -C ${ROMLIBPATH} all

memmap: all
ifdef UNIX_MK
	${Q}PYTHONPATH=${CURDIR}/tools/memory \
		${PYTHON} -m memory.memmap -sr ${BUILD_PLAT}
else
	${Q}set PYTHONPATH=${CURDIR}/tools/memory && \
		${PYTHON} -m memory.memmap -sr ${BUILD_PLAT}
endif

doc:
	@echo "  BUILD DOCUMENTATION"
	${Q}${MAKE} --no-print-directory -C ${DOCS_PATH} html

enctool: ${ENCTOOL}

${ENCTOOL}: FORCE
	${Q}${MAKE} PLAT=${PLAT} BUILD_INFO=0 OPENSSL_DIR=${OPENSSL_DIR} ENCTOOL=${ENCTOOL} DEBUG=${DEBUG} V=${V} --no-print-directory -C ${ENCTOOLPATH} all
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

cscope:
	@echo "  CSCOPE"
	${Q}find ${CURDIR} -name "*.[chsS]" > cscope.files
	${Q}cscope -b -q -k

help:
	@echo "usage: ${MAKE} [PLAT=<platform>] [OPTIONS] [TARGET]"
	@echo ""
	@echo "PLAT is used to specify which platform you wish to build."
	@echo "If no platform is specified, PLAT defaults to: ${DEFAULT_PLAT}"
	@echo ""
	@echo "platform = ${PLATFORM_LIST}"
	@echo ""
	@echo "Please refer to the User Guide for a list of all supported options."
	@echo "Note that the build system doesn't track dependencies for build "
	@echo "options. Therefore, if any of the build options are changed "
	@echo "from a previous build, a clean build must be performed."
	@echo ""
	@echo "Supported Targets:"
	@echo "  all            Build all individual bootloader binaries"
	@echo "  bl1            Build the BL1 binary"
	@echo "  bl2            Build the BL2 binary"
	@echo "  bl2u           Build the BL2U binary"
	@echo "  bl31           Build the BL31 binary"
	@echo "  bl32           Build the BL32 binary. If ARCH=aarch32, then "
	@echo "                 this builds secure payload specified by AARCH32_SP"
	@echo "  certificates   Build the certificates (requires 'GENERATE_COT=1')"
	@echo "  fip            Build the Firmware Image Package (FIP)"
	@echo "  fwu_fip        Build the FWU Firmware Image Package (FIP)"
	@echo "  checkcodebase  Check the coding style of the entire source tree"
	@echo "  checkpatch     Check the coding style on changes in the current"
	@echo "                 branch against BASE_COMMIT (default origin/master)"
	@echo "  clean          Clean the build for the selected platform"
	@echo "  cscope         Generate cscope index"
	@echo "  distclean      Remove all build artifacts for all platforms"
	@echo "  certtool       Build the Certificate generation tool"
	@echo "  enctool        Build the Firmware encryption tool"
	@echo "  fiptool        Build the Firmware Image Package (FIP) creation tool"
	@echo "  sp             Build the Secure Partition Packages"
	@echo "  sptool         Build the Secure Partition Package creation tool"
	@echo "  dtbs           Build the Device Tree Blobs (if required for the platform)"
	@echo "  memmap         Print the memory map of the built binaries"
	@echo "  doc            Build html based documentation using Sphinx tool"
	@echo ""
	@echo "Note: most build targets require PLAT to be set to a specific platform."
	@echo ""
	@echo "example: build all targets for the FVP platform:"
	@echo "  CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp all"

.PHONY: FORCE
FORCE:;
