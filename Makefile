#
# Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Trusted Firmware Version
#
VERSION_MAJOR			:= 2
VERSION_MINOR			:= 7

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

# Assertions enabled for DEBUG builds by default
ENABLE_ASSERTIONS		:= ${DEBUG}
ENABLE_PMF			:= ${ENABLE_RUNTIME_INSTRUMENTATION}
PLAT				:= ${DEFAULT_PLAT}

################################################################################
# Checkpatch script options
################################################################################

CHECKCODE_ARGS		:=	--no-patch
# Do not check the coding style on imported library files or documentation files
INC_ARM_DIRS_TO_CHECK	:=	$(sort $(filter-out                     \
					include/drivers/arm/cryptocell,	\
					$(wildcard include/drivers/arm/*)))
INC_ARM_DIRS_TO_CHECK	+=	include/drivers/arm/cryptocell/*.h
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

# The cert_create tool cannot generate certificates individually, so we use the
# target 'certificates' to create them all
ifneq (${GENERATE_COT},0)
        FIP_DEPS += certificates
        FWU_FIP_DEPS += fwu_certificates
endif

# Process BRANCH_PROTECTION value and set
# Pointer Authentication and Branch Target Identification flags
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
endif

# FEAT_RME
ifeq (${ENABLE_RME},1)
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
ARM_ARCH_MAJOR := 8
ARM_ARCH_MINOR := 6
endif

# USE_SPINLOCK_CAS requires AArch64 build
ifeq (${USE_SPINLOCK_CAS},1)
ifneq (${ARCH},aarch64)
        $(error USE_SPINLOCK_CAS requires AArch64)
endif
endif

# USE_DEBUGFS experimental feature recommended only in debug builds
ifeq (${USE_DEBUGFS},1)
ifeq (${DEBUG},1)
        $(warning DEBUGFS experimental feature is enabled.)
else
        $(warning DEBUGFS experimental, recommended in DEBUG builds ONLY)
endif
endif

ifneq (${DECRYPTION_SUPPORT},none)
ENC_ARGS += -f ${FW_ENC_STATUS}
ENC_ARGS += -k ${ENC_KEY}
ENC_ARGS += -n ${ENC_NONCE}
FIP_DEPS += enctool
FWU_FIP_DEPS += enctool
endif

################################################################################
# Toolchain
################################################################################

HOSTCC			:=	gcc
export HOSTCC

CC			:=	${CROSS_COMPILE}gcc
CPP			:=	${CROSS_COMPILE}cpp
AS			:=	${CROSS_COMPILE}gcc
AR			:=	${CROSS_COMPILE}ar
LINKER			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump
NM			:=	${CROSS_COMPILE}nm
PP			:=	${CROSS_COMPILE}gcc -E
DTC			:=	dtc

# Use ${LD}.bfd instead if it exists (as absolute path or together with $PATH).
ifneq ($(strip $(wildcard ${LD}.bfd) \
	$(foreach dir,$(subst :, ,${PATH}),$(wildcard ${dir}/${LINKER}.bfd))),)
LINKER			:=	${LINKER}.bfd
endif

ifeq (${ARM_ARCH_MAJOR},7)
target32-directive	= 	-target arm-none-eabi
# Will set march32-directive from platform configuration
else
target32-directive	= 	-target armv8a-none-eabi

# Set the compiler's target architecture profile based on
# ARM_ARCH_MAJOR ARM_ARCH_MINOR options
ifeq (${ARM_ARCH_MINOR},0)
march32-directive	= 	-march=armv${ARM_ARCH_MAJOR}-a
march64-directive	= 	-march=armv${ARM_ARCH_MAJOR}-a
else
march32-directive	= 	-march=armv${ARM_ARCH_MAJOR}.${ARM_ARCH_MINOR}-a
march64-directive	= 	-march=armv${ARM_ARCH_MAJOR}.${ARM_ARCH_MINOR}-a
endif
endif

# Memory tagging is supported in architecture Armv8.5-A AArch64 and onwards
ifeq ($(ARCH), aarch64)
# Check if revision is greater than or equal to 8.5
ifeq "8.5" "$(word 1, $(sort 8.5 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
mem_tag_arch_support	= 	yes
endif
endif

# Get architecture feature modifiers
arch-features		=	${ARM_ARCH_FEATURE}

# Enable required options for memory stack tagging.
# Currently, these options are enabled only for clang and armclang compiler.
ifeq (${SUPPORT_STACK_MEMTAG},yes)
ifdef mem_tag_arch_support
# Check for armclang and clang compilers
ifneq ( ,$(filter $(notdir $(CC)),armclang clang))
# Add "memtag" architecture feature modifier if not specified
ifeq ( ,$(findstring memtag,$(arch-features)))
arch-features       	:=       $(arch-features)+memtag
endif	# memtag
ifeq ($(notdir $(CC)),armclang)
TF_CFLAGS		+=	-mmemtag-stack
else ifeq ($(notdir $(CC)),clang)
TF_CFLAGS		+=	-fsanitize=memtag
endif	# armclang
endif	# armclang clang
else
$(error "Error: stack memory tagging is not supported for architecture \
	${ARCH},armv${ARM_ARCH_MAJOR}.${ARM_ARCH_MINOR}-a")
endif	# mem_tag_arch_support
endif	# SUPPORT_STACK_MEMTAG

# Set the compiler's architecture feature modifiers
ifneq ($(arch-features), none)
# Strip "none+" from arch-features
arch-features		:=	$(subst none+,,$(arch-features))
ifeq ($(ARCH), aarch32)
march32-directive	:=	$(march32-directive)+$(arch-features)
else
march64-directive	:=	$(march64-directive)+$(arch-features)
endif
# Print features
$(info Arm Architecture Features specified: $(subst +, ,$(arch-features)))
endif	# arch-features

# Determine if FEAT_RNG is supported
ENABLE_FEAT_RNG		=	$(if $(findstring rng,${arch-features}),1,0)

# Determine if FEAT_SB is supported
ENABLE_FEAT_SB		=	$(if $(findstring sb,${arch-features}),1,0)

ifneq ($(findstring clang,$(notdir $(CC))),)
	ifneq ($(findstring armclang,$(notdir $(CC))),)
		TF_CFLAGS_aarch32	:=	-target arm-arm-none-eabi $(march32-directive)
		TF_CFLAGS_aarch64	:=	-target aarch64-arm-none-eabi $(march64-directive)
		LD			:=	$(LINKER)
	else
		TF_CFLAGS_aarch32	:=	$(target32-directive) $(march32-directive)
		TF_CFLAGS_aarch64	:=	-target aarch64-elf $(march64-directive)
		LD			:=	$(shell $(CC) --print-prog-name ld.lld)

		AR			:=	$(shell $(CC) --print-prog-name llvm-ar)
		OD			:=	$(shell $(CC) --print-prog-name llvm-objdump)
		OC			:=	$(shell $(CC) --print-prog-name llvm-objcopy)
	endif

	CPP		:=	$(CC) -E $(TF_CFLAGS_$(ARCH))
	PP		:=	$(CC) -E $(TF_CFLAGS_$(ARCH))
	AS		:=	$(CC) -c -x assembler-with-cpp $(TF_CFLAGS_$(ARCH))
else ifneq ($(findstring gcc,$(notdir $(CC))),)
TF_CFLAGS_aarch32	=	$(march32-directive)
TF_CFLAGS_aarch64	=	$(march64-directive)
ifeq ($(ENABLE_LTO),1)
	# Enable LTO only for aarch64
	ifeq (${ARCH},aarch64)
		LTO_CFLAGS	=	-flto
		# Use gcc as a wrapper for the ld, recommended for LTO
		LINKER		:=	${CROSS_COMPILE}gcc
	endif
endif
LD			=	$(LINKER)
else
TF_CFLAGS_aarch32	=	$(march32-directive)
TF_CFLAGS_aarch64	=	$(march64-directive)
LD			=	$(LINKER)
endif

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
endif

# Default build string (git branch and commit)
ifeq (${BUILD_STRING},)
        BUILD_STRING  :=  $(shell git describe --always --dirty --tags 2> /dev/null)
endif
VERSION_STRING    :=  v${VERSION_MAJOR}.${VERSION_MINOR}(${BUILD_TYPE}):${BUILD_STRING}

ifeq (${AARCH32_INSTRUCTION_SET},A32)
TF_CFLAGS_aarch32	+=	-marm
else ifeq (${AARCH32_INSTRUCTION_SET},T32)
TF_CFLAGS_aarch32	+=	-mthumb
else
$(error Error: Unknown AArch32 instruction set ${AARCH32_INSTRUCTION_SET})
endif

TF_CFLAGS_aarch32	+=	-mno-unaligned-access
TF_CFLAGS_aarch64	+=	-mgeneral-regs-only -mstrict-align

ifneq (${BP_OPTION},none)
TF_CFLAGS_aarch64	+=	-mbranch-protection=${BP_OPTION}
endif

ASFLAGS_aarch32		=	$(march32-directive)
ASFLAGS_aarch64		=	$(march64-directive)

# General warnings
WARNINGS		:=	-Wall -Wmissing-include-dirs -Wunused	\
				-Wdisabled-optimization -Wvla -Wshadow	\
				-Wno-unused-parameter -Wredundant-decls

# Additional warnings
# Level 1
WARNING1 := -Wextra
WARNING1 += -Wmissing-format-attribute
WARNING1 += -Wmissing-prototypes
WARNING1 += -Wold-style-definition

# Level 2
WARNING2 := -Waggregate-return
WARNING2 += -Wcast-align
WARNING2 += -Wnested-externs

WARNING3 := -Wbad-function-cast
WARNING3 += -Wcast-qual
WARNING3 += -Wconversion
WARNING3 += -Wpacked
WARNING3 += -Wpointer-arith
WARNING3 += -Wswitch-default

ifeq (${W},1)
WARNINGS += $(WARNING1)
else ifeq (${W},2)
WARNINGS += $(WARNING1) $(WARNING2)
else ifeq (${W},3)
WARNINGS += $(WARNING1) $(WARNING2) $(WARNING3)
endif

# Compiler specific warnings
ifeq ($(findstring clang,$(notdir $(CC))),)
# not using clang
WARNINGS	+=		-Wunused-but-set-variable -Wmaybe-uninitialized	\
				-Wpacked-bitfield-compat -Wshift-overflow=2 \
				-Wlogical-op
else
# using clang
WARNINGS	+=		-Wshift-overflow -Wshift-sign-overflow \
				-Wlogical-op-parentheses
endif

ifneq (${E},0)
ERRORS := -Werror
endif

CPPFLAGS		=	${DEFINES} ${INCLUDES} ${MBEDTLS_INC} -nostdinc	\
				$(ERRORS) $(WARNINGS)
ASFLAGS			+=	$(CPPFLAGS) $(ASFLAGS_$(ARCH))			\
				-ffreestanding -Wa,--fatal-warnings
TF_CFLAGS		+=	$(CPPFLAGS) $(TF_CFLAGS_$(ARCH))		\
				-ffunction-sections -fdata-sections		\
				-ffreestanding -fno-builtin -fno-common		\
				-Os -std=gnu99

ifeq (${SANITIZE_UB},on)
TF_CFLAGS		+=	-fsanitize=undefined -fno-sanitize-recover
endif
ifeq (${SANITIZE_UB},trap)
TF_CFLAGS		+=	-fsanitize=undefined -fno-sanitize-recover	\
				-fsanitize-undefined-trap-on-error
endif

GCC_V_OUTPUT		:=	$(shell $(CC) -v 2>&1)

# LD = armlink
ifneq ($(findstring armlink,$(notdir $(LD))),)
TF_LDFLAGS		+=	--diag_error=warning --lto_level=O1
TF_LDFLAGS		+=	--remove --info=unused,unusedsymbols
TF_LDFLAGS		+=	$(TF_LDFLAGS_$(ARCH))

# LD = gcc (used when GCC LTO is enabled)
else ifneq ($(findstring gcc,$(notdir $(LD))),)
# Pass ld options with Wl or Xlinker switches
TF_LDFLAGS		+=	-Wl,--fatal-warnings -O1
TF_LDFLAGS		+=	-Wl,--gc-sections
ifeq ($(ENABLE_LTO),1)
	ifeq (${ARCH},aarch64)
		TF_LDFLAGS	+=	-flto -fuse-linker-plugin
	endif
endif
# GCC automatically adds fix-cortex-a53-843419 flag when used to link
# which breaks some builds, so disable if errata fix is not explicitly enabled
ifneq (${ERRATA_A53_843419},1)
	TF_LDFLAGS	+= 	-mno-fix-cortex-a53-843419
endif
TF_LDFLAGS		+= 	-nostdlib
TF_LDFLAGS		+=	$(subst --,-Xlinker --,$(TF_LDFLAGS_$(ARCH)))

# LD = gcc-ld (ld) or llvm-ld (ld.lld) or other
else
TF_LDFLAGS		+=	--fatal-warnings -O1
TF_LDFLAGS		+=	--gc-sections
# ld.lld doesn't recognize the errata flags,
# therefore don't add those in that case
ifeq ($(findstring ld.lld,$(notdir $(LD))),)
TF_LDFLAGS		+=	$(TF_LDFLAGS_$(ARCH))
endif
endif

DTC_FLAGS		+=	-I dts -O dtb
DTC_CPPFLAGS		+=	-P -nostdinc -Iinclude -Ifdts -undef \
				-x assembler-with-cpp $(DEFINES)

################################################################################
# Common sources and include directories
################################################################################
include ${MAKE_HELPERS_DIRECTORY}arch_features.mk
include lib/compiler-rt/compiler-rt.mk

BL_COMMON_SOURCES	+=	common/bl_common.c			\
				common/tf_log.c				\
				common/${ARCH}/debug.S			\
				drivers/console/multi_console.c		\
				lib/${ARCH}/cache_helpers.S		\
				lib/${ARCH}/misc_helpers.S		\
				plat/common/plat_bl_common.c		\
				plat/common/plat_log_common.c		\
				plat/common/${ARCH}/plat_common.c	\
				plat/common/${ARCH}/platform_helpers.S	\
				${COMPILER_RT_SRCS}

ifeq ($(notdir $(CC)),armclang)
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
        $(warning "The SPD and its BL32 companion will be present but ignored.")
    endif

    ifeq (${SPD},spmd)
        # SPMD is located in std_svc directory
        SPD_DIR := std_svc

        ifeq ($(SPMD_SPM_AT_SEL2),1)
            ifeq ($(CTX_INCLUDE_EL2_REGS),0)
                $(error SPMD with SPM at S-EL2 requires CTX_INCLUDE_EL2_REGS option)
            endif
	    ifeq ($(SPMC_AT_EL3),1)
                $(error SPM cannot be enabled in both S-EL2 and EL3.)
            endif
        endif

        ifeq ($(findstring optee_sp,$(ARM_SPMC_MANIFEST_DTS)),optee_sp)
            DTC_CPPFLAGS	+=	-DOPTEE_SP_FW_CONFIG
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
    else
        # All other SPDs in spd directory
        SPD_DIR := spd
    endif

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
endif

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
include services/std_svc/rmmd/rmmd.mk
$(warning "RME is an experimental feature")
endif

################################################################################
# Include the platform specific Makefile after the SPD Makefile (the platform
# makefile may use all previous definitions in this file)
################################################################################

include ${PLAT_MAKEFILE_FULL}

$(eval $(call MAKE_PREREQ_DIR,${BUILD_PLAT}))

ifeq (${ARM_ARCH_MAJOR},7)
include make_helpers/armv7-a-cpus.mk
endif

PIE_FOUND		:=	$(findstring --enable-default-pie,${GCC_V_OUTPUT})
ifneq ($(PIE_FOUND),)
	TF_CFLAGS	+=	-fno-PIE
endif

ifneq ($(findstring gcc,$(notdir $(LD))),)
	PIE_LDFLAGS	+=	-Wl,-pie -Wl,--no-dynamic-linker
else
	PIE_LDFLAGS	+=	-pie --no-dynamic-linker
endif

ifeq ($(ENABLE_PIE),1)
ifeq ($(BL2_AT_EL3),1)
ifneq ($(BL2_IN_XIP_MEM),1)
	BL2_CFLAGS	+=	-fpie
	BL2_LDFLAGS	+=	$(PIE_LDFLAGS)
endif
endif
	BL31_CFLAGS	+=	-fpie
	BL31_LDFLAGS	+=	$(PIE_LDFLAGS)
	BL32_CFLAGS	+=	-fpie
	BL32_LDFLAGS	+=	$(PIE_LDFLAGS)
endif

ifeq (${ARCH},aarch64)
BL1_CPPFLAGS += -DIMAGE_AT_EL3
ifeq ($(BL2_AT_EL3),1)
BL2_CPPFLAGS += -DIMAGE_AT_EL3
else
BL2_CPPFLAGS += -DIMAGE_AT_EL1
endif
BL2U_CPPFLAGS += -DIMAGE_AT_EL1
BL31_CPPFLAGS += -DIMAGE_AT_EL3
BL32_CPPFLAGS += -DIMAGE_AT_EL1
endif

# Include the CPU specific operations makefile, which provides default
# values for all CPU errata workarounds and CPU specific optimisations.
# This can be overridden by the platform.
include lib/cpus/cpu-ops.mk

ifeq (${ARCH},aarch32)
NEED_BL32 := yes

################################################################################
# Build `AARCH32_SP` as BL32 image for AArch32
################################################################################
ifneq (${AARCH32_SP},none)
# We expect to locate an sp.mk under the specified AARCH32_SP directory
AARCH32_SP_MAKE	:=	$(wildcard bl32/${AARCH32_SP}/${AARCH32_SP}.mk)

ifeq (${AARCH32_SP_MAKE},)
  $(error Error: No bl32/${AARCH32_SP}/${AARCH32_SP}.mk located)
endif

$(info Including ${AARCH32_SP_MAKE})
include ${AARCH32_SP_MAKE}
endif

endif

################################################################################
# Include libc if not overridden
################################################################################
ifeq (${OVERRIDE_LIBC},0)
include lib/libc/libc.mk
endif

################################################################################
# Check incompatible options
################################################################################

ifdef EL3_PAYLOAD_BASE
        ifdef PRELOADED_BL33_BASE
                $(warning "PRELOADED_BL33_BASE and EL3_PAYLOAD_BASE are \
                incompatible build options. EL3_PAYLOAD_BASE has priority.")
        endif
        ifneq (${GENERATE_COT},0)
                $(error "GENERATE_COT and EL3_PAYLOAD_BASE are incompatible build options.")
        endif
        ifneq (${TRUSTED_BOARD_BOOT},0)
                $(error "TRUSTED_BOARD_BOOT and EL3_PAYLOAD_BASE are incompatible build options.")
        endif
endif

ifeq (${NEED_BL33},yes)
        ifdef EL3_PAYLOAD_BASE
                $(warning "BL33 image is not needed when option \
                BL33_PAYLOAD_BASE is used and won't be added to the FIP file.")
        endif
        ifdef PRELOADED_BL33_BASE
                $(warning "BL33 image is not needed when option \
                PRELOADED_BL33_BASE is used and won't be added to the FIP \
                file.")
        endif
endif

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
ifeq ($(HW_ASSISTED_COHERENCY)-$(USE_COHERENT_MEM),1-1)
$(error USE_COHERENT_MEM cannot be enabled with HW_ASSISTED_COHERENCY)
endif

#For now, BL2_IN_XIP_MEM is only supported when BL2_AT_EL3 is 1.
ifeq ($(BL2_AT_EL3)-$(BL2_IN_XIP_MEM),0-1)
$(error "BL2_IN_XIP_MEM is only supported when BL2_AT_EL3 is enabled")
endif

# For RAS_EXTENSION, require that EAs are handled in EL3 first
ifeq ($(RAS_EXTENSION),1)
    ifneq ($(HANDLE_EA_EL3_FIRST),1)
        $(error For RAS_EXTENSION, HANDLE_EA_EL3_FIRST must also be 1)
    endif
endif

# When FAULT_INJECTION_SUPPORT is used, require that RAS_EXTENSION is enabled
ifeq ($(FAULT_INJECTION_SUPPORT),1)
    ifneq ($(RAS_EXTENSION),1)
        $(error For FAULT_INJECTION_SUPPORT, RAS_EXTENSION must also be 1)
    endif
endif

# DYN_DISABLE_AUTH can be set only when TRUSTED_BOARD_BOOT=1
ifeq ($(DYN_DISABLE_AUTH), 1)
    ifeq (${TRUSTED_BOARD_BOOT}, 0)
        $(error "TRUSTED_BOARD_BOOT must be enabled for DYN_DISABLE_AUTH to be set.")
    endif
endif

ifneq ($(filter 1,${MEASURED_BOOT} ${TRUSTED_BOARD_BOOT}),)
    CRYPTO_SUPPORT := 1
else
    CRYPTO_SUPPORT := 0
endif

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
endif

ifeq ($(CTX_INCLUDE_PAUTH_REGS),1)
    ifneq (${ARCH},aarch64)
        $(error CTX_INCLUDE_PAUTH_REGS requires AArch64)
    endif
endif

ifeq ($(CTX_INCLUDE_MTE_REGS),1)
    ifneq (${ARCH},aarch64)
        $(error CTX_INCLUDE_MTE_REGS requires AArch64)
    endif
endif

ifeq ($(PSA_FWU_SUPPORT),1)
    $(info PSA_FWU_SUPPORT is an experimental feature)
endif

ifeq ($(FEATURE_DETECTION),1)
    $(info FEATURE_DETECTION is an experimental feature)
endif

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
    ifeq (${ALLOW_RO_XLAT_TABLES}, 1)
        $(error "ALLOW_RO_XLAT_TABLES requires translation tables library v2")
    endif
endif

ifneq (${DECRYPTION_SUPPORT},none)
    ifeq (${TRUSTED_BOARD_BOOT}, 0)
        $(error TRUSTED_BOARD_BOOT must be enabled for DECRYPTION_SUPPORT to be set)
    endif
endif

# Ensure that no Aarch64-only features are enabled in Aarch32 build
ifeq (${ARCH},aarch32)

    # SME/SVE only supported on AArch64
    ifeq (${ENABLE_SME_FOR_NS},1)
        $(error "ENABLE_SME_FOR_NS cannot be used with ARCH=aarch32")
    endif
    ifeq (${ENABLE_SVE_FOR_NS},1)
        # Warning instead of error due to CI dependency on this
        $(error "ENABLE_SVE_FOR_NS cannot be used with ARCH=aarch32")
    endif

    # BRBE is not supported in Aarch32
    ifeq (${ENABLE_BRBE_FOR_NS},1)
        $(error "ENABLE_BRBE_FOR_NS cannot be used with ARCH=aarch32")
    endif

endif

# Ensure ENABLE_RME is not used with SME
ifeq (${ENABLE_RME},1)
    ifeq (${ENABLE_SME_FOR_NS},1)
        $(error "ENABLE_SME_FOR_NS cannot be used with ENABLE_RME")
    endif
endif

# Secure SME/SVE requires the non-secure component as well
ifeq (${ENABLE_SME_FOR_SWD},1)
    ifeq (${ENABLE_SME_FOR_NS},0)
        $(error "ENABLE_SME_FOR_SWD requires ENABLE_SME_FOR_NS")
    endif
endif
ifeq (${ENABLE_SVE_FOR_SWD},1)
    ifeq (${ENABLE_SVE_FOR_NS},0)
        $(error "ENABLE_SVE_FOR_SWD requires ENABLE_SVE_FOR_NS")
    endif
endif

# SVE and SME cannot be used with CTX_INCLUDE_FPREGS since secure manager does
# its own context management including FPU registers.
ifeq (${CTX_INCLUDE_FPREGS},1)
    ifeq (${ENABLE_SME_FOR_NS},1)
        $(error "ENABLE_SME_FOR_NS cannot be used with CTX_INCLUDE_FPREGS")
    endif
    ifeq (${ENABLE_SVE_FOR_NS},1)
        # Warning instead of error due to CI dependency on this
        $(warning "ENABLE_SVE_FOR_NS cannot be used with CTX_INCLUDE_FPREGS")
        $(warning "Forced ENABLE_SVE_FOR_NS=0")
        override ENABLE_SVE_FOR_NS	:= 0
    endif
endif

ifeq ($(DRTM_SUPPORT),1)
    $(info DRTM_SUPPORT is an experimental feature)
endif

################################################################################
# Process platform overrideable behaviour
################################################################################

ifdef BL1_SOURCES
NEED_BL1 := yes
endif

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
endif

ifdef BL2U_SOURCES
NEED_BL2U := yes
endif

# If SCP_BL2 is given, we always want FIP to include it.
ifdef SCP_BL2
        NEED_SCP_BL2		:=	yes
endif

# For AArch32, BL31 is not currently supported.
ifneq (${ARCH},aarch32)
    ifdef BL31_SOURCES
        # When booting an EL3 payload, there is no need to compile the BL31 image nor
        # put it in the FIP.
        ifndef EL3_PAYLOAD_BASE
            NEED_BL31 := yes
        endif
    endif
endif

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
endif

ifneq (${FIP_ALIGN},0)
FIP_ARGS += --align ${FIP_ALIGN}
endif

ifdef FDT_SOURCES
NEED_FDT := yes
endif

################################################################################
# Include libraries' Makefile that are used in all BL
################################################################################

include lib/stack_protector/stack_protector.mk

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

# Variables for use with ROMLIB
ROMLIBPATH		?=	lib/romlib

# Variable for use with Python
PYTHON			?=	python3

# Variables for use with PRINT_MEMORY_MAP
PRINT_MEMORY_MAP_PATH		?=	tools/memory
PRINT_MEMORY_MAP		?=	${PRINT_MEMORY_MAP_PATH}/print_memory_map.py

# Variables for use with documentation build using Sphinx tool
DOCS_PATH		?=	docs

# Defination of SIMICS flag
SIMICS_BUILD	?=	0

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

$(eval $(call assert_booleans,\
    $(sort \
        ALLOW_RO_XLAT_TABLES \
        BL2_ENABLE_SP_LOAD \
        COLD_BOOT_SINGLE_CPU \
        CREATE_KEYS \
        CTX_INCLUDE_AARCH32_REGS \
        CTX_INCLUDE_FPREGS \
        CTX_INCLUDE_EL2_REGS \
        DEBUG \
        DISABLE_MTPMU \
        DYN_DISABLE_AUTH \
        EL3_EXCEPTION_HANDLING \
        ENABLE_AMU \
        ENABLE_AMU_AUXILIARY_COUNTERS \
        ENABLE_AMU_FCONF \
        AMU_RESTRICT_COUNTERS \
        ENABLE_ASSERTIONS \
        ENABLE_PIE \
        ENABLE_PMF \
        ENABLE_PSCI_STAT \
        ENABLE_RUNTIME_INSTRUMENTATION \
        ENABLE_SME_FOR_NS \
        ENABLE_SME_FOR_SWD \
        ENABLE_SPE_FOR_LOWER_ELS \
        ENABLE_SVE_FOR_NS \
        ENABLE_SVE_FOR_SWD \
        ERROR_DEPRECATED \
        FAULT_INJECTION_SUPPORT \
        GENERATE_COT \
        GICV2_G0_FOR_EL3 \
        HANDLE_EA_EL3_FIRST \
        HW_ASSISTED_COHERENCY \
        INVERTED_MEMMAP \
        MEASURED_BOOT \
        DRTM_SUPPORT \
        NS_TIMER_SWITCH \
        OVERRIDE_LIBC \
        PL011_GENERIC_UART \
        PLAT_RSS_NOT_SUPPORTED \
        PROGRAMMABLE_RESET_ADDRESS \
        PSCI_EXTENDED_STATE_ID \
        RESET_TO_BL31 \
        RESET_TO_BL31_WITH_PARAMS \
        SAVE_KEYS \
        SEPARATE_CODE_AND_RODATA \
        SEPARATE_BL2_NOLOAD_REGION \
        SEPARATE_NOBITS_REGION \
        SPIN_ON_BL1_EXIT \
        SPM_MM \
        SPMC_AT_EL3 \
        SPMD_SPM_AT_SEL2 \
        TRUSTED_BOARD_BOOT \
        CRYPTO_SUPPORT \
        USE_COHERENT_MEM \
        USE_DEBUGFS \
        ARM_IO_IN_DTB \
        SDEI_IN_FCONF \
        SEC_INT_DESC_IN_FCONF \
        USE_ROMLIB \
        USE_TBBR_DEFS \
        WARMBOOT_ENABLE_DCACHE_EARLY \
        BL2_AT_EL3 \
        BL2_IN_XIP_MEM \
        BL2_INV_DCACHE \
        USE_SPINLOCK_CAS \
        ENCRYPT_BL31 \
        ENCRYPT_BL32 \
        ERRATA_SPECULATIVE_AT \
        RAS_TRAP_LOWER_EL_ERR_ACCESS \
        COT_DESC_IN_DTB \
        USE_SP804_TIMER \
        PSA_FWU_SUPPORT \
        ENABLE_BRBE_FOR_NS \
        ENABLE_TRBE_FOR_NS \
        ENABLE_SYS_REG_TRACE_FOR_NS \
        ENABLE_MPMM \
        ENABLE_MPMM_FCONF \
        SIMICS_BUILD \
        FEATURE_DETECTION \
)))

$(eval $(call assert_numerics,\
    $(sort \
        ARM_ARCH_MAJOR \
        ARM_ARCH_MINOR \
        BRANCH_PROTECTION \
        CTX_INCLUDE_PAUTH_REGS \
        CTX_INCLUDE_MTE_REGS \
        CTX_INCLUDE_NEVE_REGS \
        ENABLE_BTI \
        ENABLE_PAUTH \
        ENABLE_FEAT_AMUv1 \
        ENABLE_FEAT_AMUv1p1 \
        ENABLE_FEAT_CSV2_2 \
        ENABLE_FEAT_DIT \
        ENABLE_FEAT_ECV \
        ENABLE_FEAT_FGT \
        ENABLE_FEAT_HCX \
        ENABLE_FEAT_PAN \
        ENABLE_FEAT_RNG \
        ENABLE_FEAT_SB \
        ENABLE_FEAT_SEL2 \
        ENABLE_FEAT_VHE \
        ENABLE_MPAM_FOR_LOWER_ELS \
        ENABLE_RME \
        ENABLE_TRF_FOR_NS \
        FW_ENC_STATUS \
        NR_OF_FW_BANKS \
        NR_OF_IMAGES_IN_FW_BANK \
        RAS_EXTENSION \
        TWED_DELAY \
        ENABLE_FEAT_TWED \
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
        EL3_EXCEPTION_HANDLING \
        CTX_INCLUDE_MTE_REGS \
        CTX_INCLUDE_EL2_REGS \
        CTX_INCLUDE_NEVE_REGS \
        DECRYPTION_SUPPORT_${DECRYPTION_SUPPORT} \
        DISABLE_MTPMU \
        ENABLE_AMU \
        ENABLE_AMU_AUXILIARY_COUNTERS \
        ENABLE_AMU_FCONF \
        AMU_RESTRICT_COUNTERS \
        ENABLE_ASSERTIONS \
        ENABLE_BTI \
        ENABLE_MPAM_FOR_LOWER_ELS \
        ENABLE_PAUTH \
        ENABLE_PIE \
        ENABLE_PMF \
        ENABLE_PSCI_STAT \
        ENABLE_RME \
        ENABLE_RUNTIME_INSTRUMENTATION \
        ENABLE_SME_FOR_NS \
        ENABLE_SME_FOR_SWD \
        ENABLE_SPE_FOR_LOWER_ELS \
        ENABLE_SVE_FOR_NS \
        ENABLE_SVE_FOR_SWD \
        ENCRYPT_BL31 \
        ENCRYPT_BL32 \
        ERROR_DEPRECATED \
        FAULT_INJECTION_SUPPORT \
        GICV2_G0_FOR_EL3 \
        HANDLE_EA_EL3_FIRST \
        HW_ASSISTED_COHERENCY \
        LOG_LEVEL \
        MEASURED_BOOT \
        DRTM_SUPPORT \
        NS_TIMER_SWITCH \
        PL011_GENERIC_UART \
        PLAT_${PLAT} \
        PLAT_RSS_NOT_SUPPORTED \
        PROGRAMMABLE_RESET_ADDRESS \
        PSCI_EXTENDED_STATE_ID \
        RAS_EXTENSION \
        RESET_TO_BL31 \
        RESET_TO_BL31_WITH_PARAMS \
        SEPARATE_CODE_AND_RODATA \
        SEPARATE_BL2_NOLOAD_REGION \
        SEPARATE_NOBITS_REGION \
        RECLAIM_INIT_CODE \
        SPD_${SPD} \
        SPIN_ON_BL1_EXIT \
        SPM_MM \
        SPMC_AT_EL3 \
        SPMD_SPM_AT_SEL2 \
        TRUSTED_BOARD_BOOT \
        CRYPTO_SUPPORT \
        TRNG_SUPPORT \
        USE_COHERENT_MEM \
        USE_DEBUGFS \
        ARM_IO_IN_DTB \
        SDEI_IN_FCONF \
        SEC_INT_DESC_IN_FCONF \
        USE_ROMLIB \
        USE_TBBR_DEFS \
        WARMBOOT_ENABLE_DCACHE_EARLY \
        BL2_AT_EL3 \
        BL2_IN_XIP_MEM \
        BL2_INV_DCACHE \
        USE_SPINLOCK_CAS \
        ERRATA_SPECULATIVE_AT \
        RAS_TRAP_LOWER_EL_ERR_ACCESS \
        COT_DESC_IN_DTB \
        USE_SP804_TIMER \
        ENABLE_FEAT_RNG \
        ENABLE_FEAT_SB \
        ENABLE_FEAT_DIT \
        NR_OF_FW_BANKS \
        NR_OF_IMAGES_IN_FW_BANK \
        PSA_FWU_SUPPORT \
        ENABLE_BRBE_FOR_NS \
        ENABLE_TRBE_FOR_NS \
        ENABLE_SYS_REG_TRACE_FOR_NS \
        ENABLE_TRF_FOR_NS \
        ENABLE_FEAT_HCX \
        ENABLE_MPMM \
        ENABLE_MPMM_FCONF \
        ENABLE_FEAT_FGT \
        ENABLE_FEAT_AMUv1 \
        ENABLE_FEAT_ECV \
        SIMICS_BUILD \
        ENABLE_FEAT_AMUv1p1 \
        ENABLE_FEAT_SEL2 \
        ENABLE_FEAT_VHE \
        ENABLE_FEAT_CSV2_2 \
        ENABLE_FEAT_PAN \
        FEATURE_DETECTION \
        TWED_DELAY \
        ENABLE_FEAT_TWED \
)))

ifeq (${SANITIZE_UB},trap)
        $(eval $(call add_define,MONITOR_TRAPS))
endif

# Define the EL3_PAYLOAD_BASE flag only if it is provided.
ifdef EL3_PAYLOAD_BASE
        $(eval $(call add_define,EL3_PAYLOAD_BASE))
else
        # Define the PRELOADED_BL33_BASE flag only if it is provided and
        # EL3_PAYLOAD_BASE is not defined, as it has priority.
        ifdef PRELOADED_BL33_BASE
                $(eval $(call add_define,PRELOADED_BL33_BASE))
        endif
endif

# Define the DYN_DISABLE_AUTH flag only if set.
ifeq (${DYN_DISABLE_AUTH},1)
$(eval $(call add_define,DYN_DISABLE_AUTH))
endif

ifneq ($(findstring armlink,$(notdir $(LD))),)
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
endif
endif

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
ifneq ($(findstring clang,$(notdir $(CC))),)
    CPPFLAGS		+= 	-Wno-error=deprecated-declarations
else
    CPPFLAGS		+= 	-Wno-error=deprecated-declarations -Wno-error=cpp
endif
endif # !ERROR_DEPRECATED

$(eval $(call MAKE_LIB_DIRS))
$(eval $(call MAKE_LIB,c))

# Expand build macros for the different images
ifeq (${NEED_BL1},yes)
BL1_SOURCES := $(sort ${BL1_SOURCES})

$(eval $(call MAKE_BL,bl1))
endif

ifeq (${NEED_BL2},yes)
ifeq (${BL2_AT_EL3}, 0)
FIP_BL2_ARGS := tb-fw
endif

BL2_SOURCES := $(sort ${BL2_SOURCES})

$(if ${BL2}, $(eval $(call TOOL_ADD_IMG,bl2,--${FIP_BL2_ARGS})),\
	$(eval $(call MAKE_BL,bl2,${FIP_BL2_ARGS})))
endif

ifeq (${NEED_SCP_BL2},yes)
$(eval $(call TOOL_ADD_IMG,scp_bl2,--scp-fw))
endif

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
endif
endif

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
endif
endif

# If RMM image is needed but RMM is not defined, Test Realm Payload (TRP)
# needs to be built from RMM_SOURCES.
ifeq (${NEED_RMM},yes)
# Sort RMM source files to remove duplicates
RMM_SOURCES := $(sort ${RMM_SOURCES})
BUILD_RMM := $(if $(RMM),,$(if $(RMM_SOURCES),1))

$(if ${BUILD_RMM}, $(eval $(call MAKE_BL,rmm,rmm-fw)),\
         $(eval $(call TOOL_ADD_IMG,rmm,--rmm-fw)))
endif

# Add the BL33 image if required by the platform
ifeq (${NEED_BL33},yes)
$(eval $(call TOOL_ADD_IMG,bl33,--nt-fw))
endif

ifeq (${NEED_BL2U},yes)
$(if ${BL2U}, $(eval $(call TOOL_ADD_IMG,bl2u,--ap-fwu-cfg,FWU_)),\
	$(eval $(call MAKE_BL,bl2u,ap-fwu-cfg,FWU_)))
endif

# Expand build macros for the different images
ifeq (${NEED_FDT},yes)
    $(eval $(call MAKE_DTBS,$(BUILD_PLAT)/fdts,$(FDT_SOURCES)))
endif

# Add Secure Partition packages
ifeq (${NEED_SP_PKG},yes)
$(BUILD_PLAT)/sp_gen.mk: ${SP_MK_GEN} ${SP_LAYOUT_FILE} | ${BUILD_PLAT}
	${Q}${PYTHON} "$<" "$@" $(filter-out $<,$^) $(BUILD_PLAT) ${COT}
sp: $(DTBS) $(BUILD_PLAT)/sp_gen.mk $(SP_PKGS)
	@${ECHO_BLANK_LINE}
	@echo "Built SP Images successfully"
	@${ECHO_BLANK_LINE}
endif

locate-checkpatch:
ifndef CHECKPATCH
	$(error "Please set CHECKPATCH to point to the Linux checkpatch.pl file, eg: CHECKPATCH=../linux/scripts/checkpatch.pl")
else
ifeq (,$(wildcard ${CHECKPATCH}))
	$(error "The file CHECKPATCH points to cannot be found, use eg: CHECKPATCH=../linux/scripts/checkpatch.pl")
endif
endif

clean:
	@echo "  CLEAN"
	$(call SHELL_REMOVE_DIR,${BUILD_PLAT})
ifdef UNIX_MK
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
else
# Clear the MAKEFLAGS as we do not want
# to pass the gnumake flags to nmake.
	${Q}set MAKEFLAGS= && ${MSVC_NMAKE} /nologo /f ${FIPTOOLPATH}/Makefile.msvc FIPTOOLPATH=$(subst /,\,$(FIPTOOLPATH)) FIPTOOL=$(subst /,\,$(FIPTOOL)) clean
endif
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
endif
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
	${Q}${MAKE} PLAT=${PLAT} USE_TBBR_DEFS=${USE_TBBR_DEFS} COT=${COT} OPENSSL_DIR=${OPENSSL_DIR} CRTTOOL=${CRTTOOL} --no-print-directory -C ${CRTTOOLPATH}
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
endif

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
endif

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
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" FIPTOOL=${FIPTOOL} OPENSSL_DIR=${OPENSSL_DIR} --no-print-directory -C ${FIPTOOLPATH}
else
# Clear the MAKEFLAGS as we do not want
# to pass the gnumake flags to nmake.
	${Q}set MAKEFLAGS= && ${MSVC_NMAKE} /nologo /f ${FIPTOOLPATH}/Makefile.msvc FIPTOOLPATH=$(subst /,\,$(FIPTOOLPATH)) FIPTOOL=$(subst /,\,$(FIPTOOL))
endif

romlib.bin: libraries FORCE
	${Q}${MAKE} PLAT_DIR=${PLAT_DIR} BUILD_PLAT=${BUILD_PLAT} ENABLE_BTI=${ENABLE_BTI} ARM_ARCH_MINOR=${ARM_ARCH_MINOR} INCLUDES='${INCLUDES}' DEFINES='${DEFINES}' --no-print-directory -C ${ROMLIBPATH} all

# Call print_memory_map tool
memmap: all
	${Q}${PYTHON} ${PRINT_MEMORY_MAP} ${BUILD_PLAT} ${INVERTED_MEMMAP}

doc:
	@echo "  BUILD DOCUMENTATION"
	${Q}${MAKE} --no-print-directory -C ${DOCS_PATH} html

enctool: ${ENCTOOL}

${ENCTOOL}: FORCE
	${Q}${MAKE} PLAT=${PLAT} BUILD_INFO=0 OPENSSL_DIR=${OPENSSL_DIR} ENCTOOL=${ENCTOOL} --no-print-directory -C ${ENCTOOLPATH}
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
