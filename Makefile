#
# Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Trusted Firmware Version
#
VERSION_MAJOR			:= 2
VERSION_MINOR			:= 1

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
INC_LIB_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					include/lib/libfdt		\
					include/lib/libc,		\
					$(wildcard include/lib/*)))
INC_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					include/lib,			\
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
				${LIB_DIRS_TO_CHECK}


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

# Process Debug flag
$(eval $(call add_define,DEBUG))
ifneq (${DEBUG}, 0)
        BUILD_TYPE	:=	debug
        TF_CFLAGS	+= 	-g

        ifneq ($(findstring clang,$(notdir $(CC))),)
             ASFLAGS		+= 	-g
        else
             ASFLAGS		+= 	-g -Wa,--gdwarf-2
        endif

        # Use LOG_LEVEL_INFO by default for debug builds
        LOG_LEVEL	:=	40
else
        BUILD_TYPE	:=	release
        # Use LOG_LEVEL_NOTICE by default for release builds
        LOG_LEVEL	:=	20
endif

# Default build string (git branch and commit)
ifeq (${BUILD_STRING},)
        BUILD_STRING	:=	$(shell git describe --always --dirty --tags 2> /dev/null)
endif
VERSION_STRING		:=	v${VERSION_MAJOR}.${VERSION_MINOR}(${BUILD_TYPE}):${BUILD_STRING}

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
else
        $(error Unknown BRANCH_PROTECTION value ${BRANCH_PROTECTION})
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

# Set the compiler's target architecture profile based on ARM_ARCH_MINOR option
ifeq (${ARM_ARCH_MINOR},0)
march32-directive	= 	-march=armv8-a
march64-directive	= 	-march=armv8-a
else
march32-directive	= 	-march=armv8.${ARM_ARCH_MINOR}-a
march64-directive	= 	-march=armv8.${ARM_ARCH_MINOR}-a
endif
endif

ifneq ($(findstring armclang,$(notdir $(CC))),)
TF_CFLAGS_aarch32	=	-target arm-arm-none-eabi $(march32-directive)
TF_CFLAGS_aarch64	=	-target aarch64-arm-none-eabi $(march64-directive)
LD			=	$(LINKER)
AS			=	$(CC) -c -x assembler-with-cpp $(TF_CFLAGS_$(ARCH))
CPP			=	$(CC) -E $(TF_CFLAGS_$(ARCH))
PP			=	$(CC) -E $(TF_CFLAGS_$(ARCH))
else ifneq ($(findstring clang,$(notdir $(CC))),)
TF_CFLAGS_aarch32	=	$(target32-directive) $(march32-directive)
TF_CFLAGS_aarch64	=	-target aarch64-elf $(march64-directive)
LD			=	$(LINKER)
AS			=	$(CC) -c -x assembler-with-cpp $(TF_CFLAGS_$(ARCH))
CPP			=	$(CC) -E
PP			=	$(CC) -E
else
TF_CFLAGS_aarch32	=	$(march32-directive)
TF_CFLAGS_aarch64	=	$(march64-directive)
LD			=	$(LINKER)
endif

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

WARNING1 := -Wextra
WARNING1 += -Wmissing-declarations
WARNING1 += -Wmissing-format-attribute
WARNING1 += -Wmissing-prototypes
WARNING1 += -Wold-style-definition
WARNING1 += -Wunused-const-variable

WARNING2 := -Waggregate-return
WARNING2 += -Wcast-align
WARNING2 += -Wnested-externs
WARNING2 += -Wshadow
WARNING2 += -Wlogical-op
WARNING2 += -Wmissing-field-initializers
WARNING2 += -Wsign-compare

WARNING3 := -Wbad-function-cast
WARNING3 += -Wcast-qual
WARNING3 += -Wconversion
WARNING3 += -Wpacked
WARNING3 += -Wpadded
WARNING3 += -Wpointer-arith
WARNING3 += -Wredundant-decls
WARNING3 += -Wswitch-default

ifeq (${W},1)
WARNINGS := $(WARNING1)
else ifeq (${W},2)
WARNINGS := $(WARNING1) $(WARNING2)
else ifeq (${W},3)
WARNINGS := $(WARNING1) $(WARNING2) $(WARNING3)
endif

WARNINGS	+=		-Wunused -Wno-unused-parameter	\
				-Wdisabled-optimization		\
				-Wvla

ifeq ($(findstring clang,$(notdir $(CC))),)
# not using clang
WARNINGS	+=		-Wunused-but-set-variable	\
				-Wmaybe-uninitialized		\
				-Wpacked-bitfield-compat	\
				-Wshift-overflow=2
else
# using clang
WARNINGS	+=		-Wshift-overflow -Wshift-sign-overflow
endif

ifneq (${E},0)
ERRORS := -Werror
endif

CPPFLAGS		=	${DEFINES} ${INCLUDES} ${MBEDTLS_INC} -nostdinc		\
				-Wmissing-include-dirs $(ERRORS) $(WARNINGS)
ASFLAGS			+=	$(CPPFLAGS) $(ASFLAGS_$(ARCH))			\
				-ffreestanding -Wa,--fatal-warnings
TF_CFLAGS		+=	$(CPPFLAGS) $(TF_CFLAGS_$(ARCH))		\
				-ffreestanding -fno-builtin -Wall -std=gnu99	\
				-Os -ffunction-sections -fdata-sections

ifeq (${SANITIZE_UB},on)
TF_CFLAGS		+=	-fsanitize=undefined -fno-sanitize-recover
endif
ifeq (${SANITIZE_UB},trap)
TF_CFLAGS		+=	-fsanitize=undefined -fno-sanitize-recover	\
				-fsanitize-undefined-trap-on-error
endif

GCC_V_OUTPUT		:=	$(shell $(CC) -v 2>&1)

ifneq ($(findstring armlink,$(notdir $(LD))),)
TF_LDFLAGS		+=	--diag_error=warning --lto_level=O1
TF_LDFLAGS		+=	--remove --info=unused,unusedsymbols
else
TF_LDFLAGS		+=	--fatal-warnings -O1
TF_LDFLAGS		+=	--gc-sections
endif
TF_LDFLAGS		+=	$(TF_LDFLAGS_$(ARCH))

DTC_FLAGS		+=	-I dts -O dtb
DTC_CPPFLAGS		+=	-nostdinc -Iinclude -undef -x assembler-with-cpp

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

BUILD_BASE		:=	./build
BUILD_PLAT		:=	${BUILD_BASE}/${PLAT}/${BUILD_TYPE}

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
        # We expect to locate an spd.mk under the specified SPD directory
        SPD_MAKE	:=	$(wildcard services/spd/${SPD}/${SPD}.mk)

        ifeq (${SPD_MAKE},)
                $(error Error: No services/spd/${SPD}/${SPD}.mk located)
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
# Include the platform specific Makefile after the SPD Makefile (the platform
# makefile may use all previous definitions in this file)
################################################################################

include ${PLAT_MAKEFILE_FULL}

$(eval $(call MAKE_PREREQ_DIR,${BUILD_PLAT}))

ifeq (${ARM_ARCH_MAJOR},7)
include make_helpers/armv7-a-cpus.mk
endif

ifeq ($(ENABLE_PIE),1)
    TF_CFLAGS		+=	-fpie
    TF_LDFLAGS		+=	-pie --no-dynamic-linker
else
    PIE_FOUND		:=	$(findstring --enable-default-pie,${GCC_V_OUTPUT})
    ifneq ($(PIE_FOUND),)
        TF_CFLAGS		+=	-fno-PIE
    endif
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
    else
        $(info CTX_INCLUDE_PAUTH_REGS is an experimental feature)
    endif
endif

ifeq ($(ENABLE_PAUTH),1)
    $(info Pointer Authentication is an experimental feature)
endif

ifeq ($(ENABLE_BTI),1)
    $(info Branch Protection is an experimental feature)
endif

ifeq ($(CTX_INCLUDE_MTE_REGS),1)
    ifneq (${ARCH},aarch64)
        $(error CTX_INCLUDE_MTE_REGS requires AArch64)
    else
        $(info CTX_INCLUDE_MTE_REGS is an experimental feature)
    endif
endif

################################################################################
# Process platform overrideable behaviour
################################################################################

# Using BL2 implies that a BL33 image also needs to be supplied for the FIP and
# Certificate generation tools. This flag can be overridden by the platform.
ifdef BL2_SOURCES
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

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fiptool
FIPTOOL			?=	${FIPTOOLPATH}/fiptool${BIN_EXT}

# Variables for use with sptool
SPTOOLPATH		?=	tools/sptool
SPTOOL			?=	${SPTOOLPATH}/sptool${BIN_EXT}

# Variables for use with ROMLIB
ROMLIBPATH		?=	lib/romlib

################################################################################
# Include BL specific makefiles
################################################################################
ifdef BL1_SOURCES
NEED_BL1 := yes
include bl1/bl1.mk
endif

ifdef BL2_SOURCES
NEED_BL2 := yes
include bl2/bl2.mk
endif

ifdef BL2U_SOURCES
NEED_BL2U := yes
include bl2u/bl2u.mk
endif

ifeq (${NEED_BL31},yes)
ifdef BL31_SOURCES
include bl31/bl31.mk
endif
endif

ifdef FDT_SOURCES
NEED_FDT := yes
endif

################################################################################
# Build options checks
################################################################################

$(eval $(call assert_boolean,COLD_BOOT_SINGLE_CPU))
$(eval $(call assert_boolean,CREATE_KEYS))
$(eval $(call assert_boolean,CTX_INCLUDE_AARCH32_REGS))
$(eval $(call assert_boolean,CTX_INCLUDE_FPREGS))
$(eval $(call assert_boolean,CTX_INCLUDE_PAUTH_REGS))
$(eval $(call assert_boolean,CTX_INCLUDE_MTE_REGS))
$(eval $(call assert_boolean,DEBUG))
$(eval $(call assert_boolean,DYN_DISABLE_AUTH))
$(eval $(call assert_boolean,EL3_EXCEPTION_HANDLING))
$(eval $(call assert_boolean,ENABLE_AMU))
$(eval $(call assert_boolean,ENABLE_ASSERTIONS))
$(eval $(call assert_boolean,ENABLE_MPAM_FOR_LOWER_ELS))
$(eval $(call assert_boolean,ENABLE_PIE))
$(eval $(call assert_boolean,ENABLE_PMF))
$(eval $(call assert_boolean,ENABLE_PSCI_STAT))
$(eval $(call assert_boolean,ENABLE_RUNTIME_INSTRUMENTATION))
$(eval $(call assert_boolean,ENABLE_SPE_FOR_LOWER_ELS))
$(eval $(call assert_boolean,ENABLE_SPM))
$(eval $(call assert_boolean,ENABLE_SVE_FOR_NS))
$(eval $(call assert_boolean,ERROR_DEPRECATED))
$(eval $(call assert_boolean,FAULT_INJECTION_SUPPORT))
$(eval $(call assert_boolean,GENERATE_COT))
$(eval $(call assert_boolean,GICV2_G0_FOR_EL3))
$(eval $(call assert_boolean,HANDLE_EA_EL3_FIRST))
$(eval $(call assert_boolean,HW_ASSISTED_COHERENCY))
$(eval $(call assert_boolean,NS_TIMER_SWITCH))
$(eval $(call assert_boolean,OVERRIDE_LIBC))
$(eval $(call assert_boolean,PL011_GENERIC_UART))
$(eval $(call assert_boolean,PROGRAMMABLE_RESET_ADDRESS))
$(eval $(call assert_boolean,PSCI_EXTENDED_STATE_ID))
$(eval $(call assert_boolean,RAS_EXTENSION))
$(eval $(call assert_boolean,RESET_TO_BL31))
$(eval $(call assert_boolean,SAVE_KEYS))
$(eval $(call assert_boolean,SEPARATE_CODE_AND_RODATA))
$(eval $(call assert_boolean,SPIN_ON_BL1_EXIT))
$(eval $(call assert_boolean,SPM_MM))
$(eval $(call assert_boolean,TRUSTED_BOARD_BOOT))
$(eval $(call assert_boolean,USE_COHERENT_MEM))
$(eval $(call assert_boolean,USE_ROMLIB))
$(eval $(call assert_boolean,USE_TBBR_DEFS))
$(eval $(call assert_boolean,WARMBOOT_ENABLE_DCACHE_EARLY))
$(eval $(call assert_boolean,BL2_AT_EL3))
$(eval $(call assert_boolean,BL2_IN_XIP_MEM))

$(eval $(call assert_numeric,ARM_ARCH_MAJOR))
$(eval $(call assert_numeric,ARM_ARCH_MINOR))
$(eval $(call assert_numeric,BRANCH_PROTECTION))

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

$(eval $(call add_define,ARM_ARCH_MAJOR))
$(eval $(call add_define,ARM_ARCH_MINOR))
$(eval $(call add_define,COLD_BOOT_SINGLE_CPU))
$(eval $(call add_define,CTX_INCLUDE_AARCH32_REGS))
$(eval $(call add_define,CTX_INCLUDE_FPREGS))
$(eval $(call add_define,CTX_INCLUDE_PAUTH_REGS))
$(eval $(call add_define,EL3_EXCEPTION_HANDLING))
$(eval $(call add_define,CTX_INCLUDE_MTE_REGS))
$(eval $(call add_define,ENABLE_AMU))
$(eval $(call add_define,ENABLE_ASSERTIONS))
$(eval $(call add_define,ENABLE_BTI))
$(eval $(call add_define,ENABLE_MPAM_FOR_LOWER_ELS))
$(eval $(call add_define,ENABLE_PAUTH))
$(eval $(call add_define,ENABLE_PIE))
$(eval $(call add_define,ENABLE_PMF))
$(eval $(call add_define,ENABLE_PSCI_STAT))
$(eval $(call add_define,ENABLE_RUNTIME_INSTRUMENTATION))
$(eval $(call add_define,ENABLE_SPE_FOR_LOWER_ELS))
$(eval $(call add_define,ENABLE_SPM))
$(eval $(call add_define,ENABLE_SVE_FOR_NS))
$(eval $(call add_define,ERROR_DEPRECATED))
$(eval $(call add_define,FAULT_INJECTION_SUPPORT))
$(eval $(call add_define,GICV2_G0_FOR_EL3))
$(eval $(call add_define,HANDLE_EA_EL3_FIRST))
$(eval $(call add_define,HW_ASSISTED_COHERENCY))
$(eval $(call add_define,LOG_LEVEL))
$(eval $(call add_define,NS_TIMER_SWITCH))
$(eval $(call add_define,PL011_GENERIC_UART))
$(eval $(call add_define,PLAT_${PLAT}))
$(eval $(call add_define,PROGRAMMABLE_RESET_ADDRESS))
$(eval $(call add_define,PSCI_EXTENDED_STATE_ID))
$(eval $(call add_define,RAS_EXTENSION))
$(eval $(call add_define,RESET_TO_BL31))
$(eval $(call add_define,SEPARATE_CODE_AND_RODATA))
$(eval $(call add_define,RECLAIM_INIT_CODE))
$(eval $(call add_define,SPD_${SPD}))
$(eval $(call add_define,SPIN_ON_BL1_EXIT))
$(eval $(call add_define,SPM_MM))
$(eval $(call add_define,TRUSTED_BOARD_BOOT))
$(eval $(call add_define,USE_COHERENT_MEM))
$(eval $(call add_define,USE_ROMLIB))
$(eval $(call add_define,USE_TBBR_DEFS))
$(eval $(call add_define,WARMBOOT_ENABLE_DCACHE_EARLY))
$(eval $(call add_define,BL2_AT_EL3))
$(eval $(call add_define,BL2_IN_XIP_MEM))

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

################################################################################
# Build targets
################################################################################

.PHONY:	all msg_start clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool sptool fip fwu_fip certtool dtbs
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
# __ASSEMBLY__ is deprecated in favor of the compiler-builtin __ASSEMBLER__.
ASFLAGS	+= -D__ASSEMBLY__
# AARCH32/AARCH64 macros are deprecated in favor of the compiler-builtin __aarch64__.
ifeq (${ARCH},aarch32)
        $(eval $(call add_define,AARCH32))
else
        $(eval $(call add_define,AARCH64))
endif
endif # !ERROR_DEPRECATED

$(eval $(call MAKE_LIB_DIRS))
$(eval $(call MAKE_LIB,c))

# Expand build macros for the different images
ifeq (${NEED_BL1},yes)
$(eval $(call MAKE_BL,1))
endif

ifeq (${NEED_BL2},yes)
ifeq (${BL2_AT_EL3}, 0)
FIP_BL2_ARGS := tb-fw
endif

$(if ${BL2}, $(eval $(call TOOL_ADD_IMG,bl2,--${FIP_BL2_ARGS})),\
	$(eval $(call MAKE_BL,2,${FIP_BL2_ARGS})))
endif

ifeq (${NEED_SCP_BL2},yes)
$(eval $(call TOOL_ADD_IMG,scp_bl2,--scp-fw))
endif

ifeq (${NEED_BL31},yes)
BL31_SOURCES += ${SPD_SOURCES}
$(if ${BL31}, $(eval $(call TOOL_ADD_IMG,bl31,--soc-fw)),\
	$(eval $(call MAKE_BL,31,soc-fw)))
endif

# If a BL32 image is needed but neither BL32 nor BL32_SOURCES is defined, the
# build system will call TOOL_ADD_IMG to print a warning message and abort the
# process. Note that the dependency on BL32 applies to the FIP only.
ifeq (${NEED_BL32},yes)

BUILD_BL32 := $(if $(BL32),,$(if $(BL32_SOURCES),1))

$(if ${BUILD_BL32}, $(eval $(call MAKE_BL,32,tos-fw)),\
	$(eval $(call TOOL_ADD_IMG,bl32,--tos-fw)))
endif

# Add the BL33 image if required by the platform
ifeq (${NEED_BL33},yes)
$(eval $(call TOOL_ADD_IMG,bl33,--nt-fw))
endif

ifeq (${NEED_BL2U},yes)
$(if ${BL2U}, $(eval $(call TOOL_ADD_IMG,bl2u,--ap-fwu-cfg,FWU_)),\
	$(eval $(call MAKE_BL,2u,ap-fwu-cfg,FWU_)))
endif

# Expand build macros for the different images
ifeq (${NEED_FDT},yes)
    $(eval $(call MAKE_DTBS,$(BUILD_PLAT)/fdts,$(FDT_SOURCES)))
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
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH} clean
	${Q}${MAKE} --no-print-directory -C ${ROMLIBPATH} clean

realclean distclean:
	@echo "  REALCLEAN"
	$(call SHELL_REMOVE_DIR,${BUILD_BASE})
	$(call SHELL_DELETE_ALL, ${CURDIR}/cscope.*)
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
	${Q}${MAKE} --no-print-directory -C ${SPTOOLPATH} clean
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH} clean
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
	for commit in `git rev-list $$COMMON_COMMIT..HEAD`; do		\
		printf "\n[*] Checking style of '$$commit'\n\n";	\
		git log --format=email "$$commit~..$$commit"		\
			-- ${CHECK_PATHS} |				\
			${CHECKPATCH} ${CHECKPATCH_OPTS} - || true;	\
		git diff --format=email "$$commit~..$$commit"		\
			-- ${CHECK_PATHS} |				\
			${CHECKPATCH}  ${CHECKPATCH_OPTS} - || true;	\
	done

certtool: ${CRTTOOL}

.PHONY: ${CRTTOOL}
${CRTTOOL}:
	${Q}${MAKE} PLAT=${PLAT} USE_TBBR_DEFS=${USE_TBBR_DEFS} --no-print-directory -C ${CRTTOOLPATH}
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
	${Q}${FIPTOOL} create ${FWU_FIP_ARGS} $@
	${Q}${FIPTOOL} info $@
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

fiptool: ${FIPTOOL}
fip: ${BUILD_PLAT}/${FIP_NAME}
fwu_fip: ${BUILD_PLAT}/${FWU_FIP_NAME}

.PHONY: ${FIPTOOL}
${FIPTOOL}:
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" --no-print-directory -C ${FIPTOOLPATH}

sptool: ${SPTOOL}
.PHONY: ${SPTOOL}
${SPTOOL}:
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" --no-print-directory -C ${SPTOOLPATH}

.PHONY: libraries
romlib.bin: libraries
	${Q}${MAKE} PLAT_DIR=${PLAT_DIR} BUILD_PLAT=${BUILD_PLAT} ENABLE_BTI=${ENABLE_BTI} ARM_ARCH_MINOR=${ARM_ARCH_MINOR} INCLUDES='${INCLUDES}' DEFINES='${DEFINES}' --no-print-directory -C ${ROMLIBPATH} all

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
	@echo "  fiptool        Build the Firmware Image Package (FIP) creation tool"
	@echo "  sptool         Build the Secure Partition Package creation tool"
	@echo "  dtbs           Build the Device Tree Blobs (if required for the platform)"
	@echo ""
	@echo "Note: most build targets require PLAT to be set to a specific platform."
	@echo ""
	@echo "example: build all targets for the FVP platform:"
	@echo "  CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp all"
