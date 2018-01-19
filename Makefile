#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Trusted Firmware Version
#
VERSION_MAJOR			:= 1
VERSION_MINOR			:= 4

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

ifdef ASM_ASSERTION
        $(warning ASM_ASSERTION is removed, use ENABLE_ASSERTIONS instead.)
endif

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
					include/lib/stdlib,		\
					$(wildcard include/lib/*)))
INC_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					include/lib,			\
					$(wildcard include/*)))
LIB_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					lib/compiler-rt			\
					lib/libfdt%			\
					lib/stdlib,			\
					$(wildcard lib/*)))
ROOT_DIRS_TO_CHECK	:=	$(sort $(filter-out			\
					lib				\
					include				\
					docs				\
					%.md,				\
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
        CHECKCODE_ARGS	+=	--no-summary --terse
else
        Q:=
endif
export Q

# Process Debug flag
$(eval $(call add_define,DEBUG))
ifneq (${DEBUG}, 0)
        BUILD_TYPE	:=	debug
        TF_CFLAGS	+= 	-g
        ASFLAGS		+= 	-g -Wa,--gdwarf-2
        # Use LOG_LEVEL_INFO by default for debug builds
        LOG_LEVEL	:=	40
else
        BUILD_TYPE	:=	release
        $(eval $(call add_define,NDEBUG))
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


################################################################################
# Toolchain
################################################################################

HOSTCC			:=	gcc
export HOSTCC

CC			:=	${CROSS_COMPILE}gcc
CPP			:=	${CROSS_COMPILE}cpp
AS			:=	${CROSS_COMPILE}gcc
AR			:=	${CROSS_COMPILE}ar
LD			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump
NM			:=	${CROSS_COMPILE}nm
PP			:=	${CROSS_COMPILE}gcc -E
DTC			?=	dtc

ifeq (${ARM_ARCH_MAJOR},7)
target32-directive	= 	-target arm-none-eabi
# Will set march32-directive from platform configuration
else
target32-directive	= 	-target armv8a-none-eabi
march32-directive	= 	-march=armv8-a
endif

ifeq ($(notdir $(CC)),armclang)
TF_CFLAGS_aarch32	=	-target arm-arm-none-eabi $(march32-directive)
TF_CFLAGS_aarch64	=	-target aarch64-arm-none-eabi -march=armv8-a
else ifneq ($(findstring clang,$(notdir $(CC))),)
TF_CFLAGS_aarch32	=	$(target32-directive)
TF_CFLAGS_aarch64	=	-target aarch64-elf
else
TF_CFLAGS_aarch32	=	$(march32-directive)
TF_CFLAGS_aarch64	=	-march=armv8-a
endif

TF_CFLAGS_aarch64	+=	-mgeneral-regs-only -mstrict-align

ASFLAGS_aarch32		=	$(march32-directive)
ASFLAGS_aarch64		=	-march=armv8-a

CPPFLAGS		=	${DEFINES} ${INCLUDES} -nostdinc		\
				-Wmissing-include-dirs -Werror
ASFLAGS			+=	$(CPPFLAGS) $(ASFLAGS_$(ARCH))			\
				-D__ASSEMBLY__ -ffreestanding 			\
				-Wa,--fatal-warnings
TF_CFLAGS		+=	$(CPPFLAGS) $(TF_CFLAGS_$(ARCH))		\
				-ffreestanding -fno-builtin -Wall -std=gnu99	\
				-Os -ffunction-sections -fdata-sections

GCC_V_OUTPUT		:=	$(shell $(CC) -v 2>&1)
PIE_FOUND		:=	$(findstring --enable-default-pie,${GCC_V_OUTPUT})

ifeq ($(PIE_FOUND),1)
TF_CFLAGS		+=	-fno-PIE
endif

TF_LDFLAGS		+=	--fatal-warnings -O1
TF_LDFLAGS		+=	--gc-sections
TF_LDFLAGS		+=	$(TF_LDFLAGS_$(ARCH))

DTC_FLAGS		+=	-I dts -O dtb

################################################################################
# Common sources and include directories
################################################################################
include lib/compiler-rt/compiler-rt.mk
include lib/stdlib/stdlib.mk

BL_COMMON_SOURCES	+=	common/bl_common.c			\
				common/tf_log.c				\
				common/tf_printf.c			\
				common/tf_snprintf.c			\
				common/${ARCH}/debug.S			\
				lib/${ARCH}/cache_helpers.S		\
				lib/${ARCH}/misc_helpers.S		\
				plat/common/plat_log_common.c		\
				plat/common/${ARCH}/plat_common.c	\
				plat/common/${ARCH}/platform_helpers.S	\
				${COMPILER_RT_SRCS}			\
				${STDLIB_SRCS}

INCLUDES		+=	-Iinclude/bl1				\
				-Iinclude/bl31				\
				-Iinclude/common			\
				-Iinclude/common/${ARCH}		\
				-Iinclude/drivers			\
				-Iinclude/drivers/arm			\
				-Iinclude/drivers/auth			\
				-Iinclude/drivers/io			\
				-Iinclude/drivers/ti/uart		\
				-Iinclude/lib				\
				-Iinclude/lib/${ARCH}			\
				-Iinclude/lib/cpus			\
				-Iinclude/lib/cpus/${ARCH}		\
				-Iinclude/lib/el3_runtime		\
				-Iinclude/lib/el3_runtime/${ARCH}	\
				-Iinclude/lib/extensions		\
				-Iinclude/lib/pmf			\
				-Iinclude/lib/psci			\
				-Iinclude/lib/xlat_tables		\
				-Iinclude/plat/common			\
				-Iinclude/services			\
				${PLAT_INCLUDES}			\
				${SPD_INCLUDES}				\
				-Iinclude/tools_share


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
# Include libraries' Makefile that are used in all BL
################################################################################

include lib/stack_protector/stack_protector.mk


################################################################################
# Include the platform specific Makefile after the SPD Makefile (the platform
# makefile may use all previous definitions in this file)
################################################################################

include ${PLAT_MAKEFILE_FULL}

$(eval $(call MAKE_PREREQ_DIR,${BUILD_PLAT}))

ifeq (${ARM_ARCH_MAJOR},7)
include make_helpers/armv7-a-cpus.mk
endif

# Platform compatibility is not supported in AArch32
ifneq (${ARCH},aarch32)
# If the platform has not defined ENABLE_PLAT_COMPAT, then enable it by default
ifndef ENABLE_PLAT_COMPAT
ENABLE_PLAT_COMPAT := 1
endif

# Include the platform compatibility helpers for PSCI
ifneq (${ENABLE_PLAT_COMPAT}, 0)
include plat/compat/plat_compat.mk
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

# For AArch32, LOAD_IMAGE_V2 must be enabled.
ifeq (${ARCH},aarch32)
    ifeq (${LOAD_IMAGE_V2}, 0)
        $(error "For AArch32, LOAD_IMAGE_V2 must be enabled.")
    endif
endif

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
ifeq ($(HW_ASSISTED_COHERENCY)-$(USE_COHERENT_MEM),1-1)
$(error USE_COHERENT_MEM cannot be enabled with HW_ASSISTED_COHERENCY)
endif

################################################################################
# Process platform overrideable behaviour
################################################################################

# Using the ARM Trusted Firmware BL2 implies that a BL33 image also needs to be
# supplied for the FIP and Certificate generation tools. This flag can be
# overridden by the platform.
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
# Auxiliary tools (fiptool, cert_create, etc)
################################################################################

# Variables for use with Certificate Generation Tool
CRTTOOLPATH		?=	tools/cert_create
CRTTOOL			?=	${CRTTOOLPATH}/cert_create${BIN_EXT}

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fiptool
FIPTOOL			?=	${FIPTOOLPATH}/fiptool${BIN_EXT}

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
$(eval $(call assert_boolean,DEBUG))
$(eval $(call assert_boolean,DISABLE_PEDANTIC))
$(eval $(call assert_boolean,ENABLE_AMU))
$(eval $(call assert_boolean,ENABLE_ASSERTIONS))
$(eval $(call assert_boolean,ENABLE_PLAT_COMPAT))
$(eval $(call assert_boolean,ENABLE_PMF))
$(eval $(call assert_boolean,ENABLE_PSCI_STAT))
$(eval $(call assert_boolean,ENABLE_RUNTIME_INSTRUMENTATION))
$(eval $(call assert_boolean,ENABLE_SPE_FOR_LOWER_ELS))
$(eval $(call assert_boolean,ENABLE_SVE_FOR_NS))
$(eval $(call assert_boolean,ERROR_DEPRECATED))
$(eval $(call assert_boolean,GENERATE_COT))
$(eval $(call assert_boolean,GICV2_G0_FOR_EL3))
$(eval $(call assert_boolean,HW_ASSISTED_COHERENCY))
$(eval $(call assert_boolean,LOAD_IMAGE_V2))
$(eval $(call assert_boolean,NS_TIMER_SWITCH))
$(eval $(call assert_boolean,PL011_GENERIC_UART))
$(eval $(call assert_boolean,PROGRAMMABLE_RESET_ADDRESS))
$(eval $(call assert_boolean,PSCI_EXTENDED_STATE_ID))
$(eval $(call assert_boolean,RESET_TO_BL31))
$(eval $(call assert_boolean,SAVE_KEYS))
$(eval $(call assert_boolean,SEPARATE_CODE_AND_RODATA))
$(eval $(call assert_boolean,SPIN_ON_BL1_EXIT))
$(eval $(call assert_boolean,TRUSTED_BOARD_BOOT))
$(eval $(call assert_boolean,USE_COHERENT_MEM))
$(eval $(call assert_boolean,USE_TBBR_DEFS))
$(eval $(call assert_boolean,WARMBOOT_ENABLE_DCACHE_EARLY))
$(eval $(call assert_boolean,BL2_AT_EL3))

$(eval $(call assert_numeric,ARM_ARCH_MAJOR))
$(eval $(call assert_numeric,ARM_ARCH_MINOR))

################################################################################
# Add definitions to the cpp preprocessor based on the current build options.
# This is done after including the platform specific makefile to allow the
# platform to overwrite the default options
################################################################################

$(eval $(call add_define,ARM_ARCH_MAJOR))
$(eval $(call add_define,ARM_ARCH_MINOR))
$(eval $(call add_define,ARM_GIC_ARCH))
$(eval $(call add_define,COLD_BOOT_SINGLE_CPU))
$(eval $(call add_define,CTX_INCLUDE_AARCH32_REGS))
$(eval $(call add_define,CTX_INCLUDE_FPREGS))
$(eval $(call add_define,ENABLE_AMU))
$(eval $(call add_define,ENABLE_ASSERTIONS))
$(eval $(call add_define,ENABLE_PLAT_COMPAT))
$(eval $(call add_define,ENABLE_PMF))
$(eval $(call add_define,ENABLE_PSCI_STAT))
$(eval $(call add_define,ENABLE_RUNTIME_INSTRUMENTATION))
$(eval $(call add_define,ENABLE_SPE_FOR_LOWER_ELS))
$(eval $(call add_define,ENABLE_SVE_FOR_NS))
$(eval $(call add_define,ERROR_DEPRECATED))
$(eval $(call add_define,GICV2_G0_FOR_EL3))
$(eval $(call add_define,HW_ASSISTED_COHERENCY))
$(eval $(call add_define,LOAD_IMAGE_V2))
$(eval $(call add_define,LOG_LEVEL))
$(eval $(call add_define,NS_TIMER_SWITCH))
$(eval $(call add_define,PL011_GENERIC_UART))
$(eval $(call add_define,PLAT_${PLAT}))
$(eval $(call add_define,PROGRAMMABLE_RESET_ADDRESS))
$(eval $(call add_define,PSCI_EXTENDED_STATE_ID))
$(eval $(call add_define,RESET_TO_BL31))
$(eval $(call add_define,SEPARATE_CODE_AND_RODATA))
$(eval $(call add_define,ENABLE_SPM))
$(eval $(call add_define,SPD_${SPD}))
$(eval $(call add_define,SPIN_ON_BL1_EXIT))
$(eval $(call add_define,TRUSTED_BOARD_BOOT))
$(eval $(call add_define,USE_COHERENT_MEM))
$(eval $(call add_define,USE_TBBR_DEFS))
$(eval $(call add_define,WARMBOOT_ENABLE_DCACHE_EARLY))
$(eval $(call add_define,BL2_AT_EL3))

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
# Define the AARCH32/AARCH64 flag based on the ARCH flag
ifeq (${ARCH},aarch32)
        $(eval $(call add_define,AARCH32))
else
        $(eval $(call add_define,AARCH64))
endif

################################################################################
# Build targets
################################################################################

.PHONY:	all msg_start clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool fip fwu_fip certtool dtbs
.SUFFIXES:

all: msg_start

msg_start:
	@echo "Building ${PLAT}"

# Check if deprecated declarations should be treated as error or not.
ifeq (${ERROR_DEPRECATED},0)
    TF_CFLAGS		+= 	-Wno-error=deprecated-declarations
endif

# Expand build macros for the different images
ifeq (${NEED_BL1},yes)
$(eval $(call MAKE_BL,1))
endif

ifeq (${NEED_BL2},yes)
ifeq (${BL2_AT_EL3}, 0)
FIP_BL2_ARGS := tb-fw
endif

$(if ${BL2}, $(eval $(call MAKE_TOOL_ARGS,2,${BL2},${FIP_BL2_ARGS})),\
	$(eval $(call MAKE_BL,2,${FIP_BL2_ARGS})))
endif

ifeq (${NEED_SCP_BL2},yes)
$(eval $(call FIP_ADD_IMG,SCP_BL2,--scp-fw))
endif

ifeq (${NEED_BL31},yes)
BL31_SOURCES += ${SPD_SOURCES}
$(if ${BL31}, $(eval $(call MAKE_TOOL_ARGS,31,${BL31},soc-fw)),\
	$(eval $(call MAKE_BL,31,soc-fw)))
endif

# If a BL32 image is needed but neither BL32 nor BL32_SOURCES is defined, the
# build system will call FIP_ADD_IMG to print a warning message and abort the
# process. Note that the dependency on BL32 applies to the FIP only.
ifeq (${NEED_BL32},yes)
$(if ${BL32}, $(eval $(call MAKE_TOOL_ARGS,32,${BL32},tos-fw)),\
	$(if ${BL32_SOURCES}, $(eval $(call MAKE_BL,32,tos-fw)),\
		$(eval $(call FIP_ADD_IMG,BL32,--tos-fw))))
endif

# Add the BL33 image if required by the platform
ifeq (${NEED_BL33},yes)
$(eval $(call FIP_ADD_IMG,BL33,--nt-fw))
endif

ifeq (${NEED_BL2U},yes)
BL2U_PATH	:= $(if ${BL2U},${BL2U},$(call IMG_BIN,2u))
$(if ${BL2U}, ,$(eval $(call MAKE_BL,2u)))
$(eval $(call FWU_FIP_ADD_PAYLOAD,${BL2U_PATH},--ap-fwu-cfg))
endif

# Expand build macros for the different images
ifeq (${NEED_FDT},yes)
$(eval $(call MAKE_DTBS,$(BUILD_PLAT)/fdts,$(FDT_SOURCES)))
$(eval $(call MAKE_FDT))
dtbs: $(DTBS)
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

realclean distclean:
	@echo "  REALCLEAN"
	$(call SHELL_REMOVE_DIR,${BUILD_BASE})
	$(call SHELL_DELETE_ALL, ${CURDIR}/cscope.*)
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH} clean

checkcodebase:		locate-checkpatch
	@echo "  CHECKING STYLE"
	@if test -d .git ; then						\
		git ls-files | grep -E -v 'libfdt|stdlib|docs|\.md' |	\
		while read GIT_FILE ;					\
		do ${CHECKPATCH} ${CHECKCODE_ARGS} -f $$GIT_FILE ;	\
		done ;							\
	else								\
		 find . -type f -not -iwholename "*.git*"		\
		 -not -iwholename "*build*"				\
		 -not -iwholename "*libfdt*"				\
		 -not -iwholename "*stdlib*"				\
		 -not -iwholename "*docs*"				\
		 -not -iwholename "*.md"				\
		 -exec ${CHECKPATCH} ${CHECKCODE_ARGS} -f {} \; ;	\
	fi

checkpatch:		locate-checkpatch
	@echo "  CHECKING STYLE"
	${Q}git format-patch --stdout ${BASE_COMMIT}..HEAD -- ${CHECK_PATHS} | ${CHECKPATCH} - || true

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

cscope:
	@echo "  CSCOPE"
	${Q}find ${CURDIR} -name "*.[chsS]" > cscope.files
	${Q}cscope -b -q -k

help:
	@echo "usage: ${MAKE} PLAT=<${PLATFORM_LIST}> [OPTIONS] [TARGET]"
	@echo ""
	@echo "PLAT is used to specify which platform you wish to build."
	@echo "If no platform is specified, PLAT defaults to: ${DEFAULT_PLAT}"
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
	@echo "  dtbs           Build the Flattened device tree (if required for the platform)"
	@echo ""
	@echo "Note: most build targets require PLAT to be set to a specific platform."
	@echo ""
	@echo "example: build all targets for the FVP platform:"
	@echo "  CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp all"
