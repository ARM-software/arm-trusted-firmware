#
# Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

#
# Trusted Firmware Version
#
VERSION_MAJOR			:= 1
VERSION_MINOR			:= 3

# Default goal is build all images
.DEFAULT_GOAL			:= all

MAKE_HELPERS_DIRECTORY := make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

################################################################################
# Default values for build configurations, and their dependencies
################################################################################

include ${MAKE_HELPERS_DIRECTORY}defaults.mk

# ASM_ASSERTION enabled for DEBUG builds only
ASM_ASSERTION			:= ${DEBUG}
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
					lib/libfdt			\
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
        BUILD_STRING	:=	$(shell git log -n 1 --pretty=format:"%h")
endif
VERSION_STRING		:=	v${VERSION_MAJOR}.${VERSION_MINOR}(${BUILD_TYPE}):${BUILD_STRING}

# The cert_create tool cannot generate certificates individually, so we use the
# target 'certificates' to create them all
ifneq (${GENERATE_COT},0)
        FIP_DEPS += certificates
        FWU_FIP_DEPS += fwu_certificates
endif

# For AArch32, enable new version of image loading.
ifeq (${ARCH},aarch32)
        LOAD_IMAGE_V2	:=	1
endif

################################################################################
# Toolchain
################################################################################

CC			:=	${CROSS_COMPILE}gcc
CPP			:=	${CROSS_COMPILE}cpp
AS			:=	${CROSS_COMPILE}gcc
AR			:=	${CROSS_COMPILE}ar
LD			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump
NM			:=	${CROSS_COMPILE}nm
PP			:=	${CROSS_COMPILE}gcc -E

ASFLAGS_aarch64		=	-mgeneral-regs-only
TF_CFLAGS_aarch64	=	-mgeneral-regs-only -mstrict-align

ASFLAGS_aarch32		=	-march=armv8-a
TF_CFLAGS_aarch32	=	-march=armv8-a

ASFLAGS			+= 	-nostdinc -ffreestanding -Wa,--fatal-warnings	\
				-Werror -Wmissing-include-dirs			\
				-D__ASSEMBLY__ $(ASFLAGS_$(ARCH))		\
				${DEFINES} ${INCLUDES}
TF_CFLAGS		+= 	-nostdinc -ffreestanding -Wall			\
				-Werror -Wmissing-include-dirs			\
				-std=c99 -c -Os					\
				$(TF_CFLAGS_$(ARCH))				\
				${DEFINES} ${INCLUDES}
TF_CFLAGS		+=	-ffunction-sections -fdata-sections

LDFLAGS			+=	--fatal-warnings -O1
LDFLAGS			+=	--gc-sections


################################################################################
# Common sources and include directories
################################################################################
include lib/stdlib/stdlib.mk

BL_COMMON_SOURCES	+=	common/bl_common.c			\
				common/tf_printf.c			\
				common/${ARCH}/debug.S			\
				lib/${ARCH}/cache_helpers.S		\
				lib/${ARCH}/misc_helpers.S		\
				plat/common/${ARCH}/platform_helpers.S	\
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
				-Iinclude/lib/cpus/${ARCH}		\
				-Iinclude/lib/el3_runtime		\
				-Iinclude/lib/el3_runtime/${ARCH}	\
				-Iinclude/lib/pmf			\
				-Iinclude/lib/psci			\
				-Iinclude/plat/common			\
				-Iinclude/services			\
				${PLAT_INCLUDES}			\
				${SPD_INCLUDES}


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


################################################################################
# Check incompatible options
################################################################################

ifdef EL3_PAYLOAD_BASE
        ifdef PRELOADED_BL33_BASE
                $(warning "PRELOADED_BL33_BASE and EL3_PAYLOAD_BASE are \
                incompatible build options. EL3_PAYLOAD_BASE has priority.")
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

# TRUSTED_BOARD_BOOT is currently not supported when LOAD_IMAGE_V2 is enabled.
ifeq (${LOAD_IMAGE_V2},1)
        ifeq (${TRUSTED_BOARD_BOOT},1)
                $(error "TRUSTED_BOARD_BOOT is currently not supported	\
                for LOAD_IMAGE_V2=1")
        endif
endif

# For AArch32, LOAD_IMAGE_V2 must be enabled.
ifeq (${ARCH},aarch32)
    ifeq (${LOAD_IMAGE_V2}, 0)
        $(error "For AArch32, LOAD_IMAGE_V2 must be enabled.")
    endif
endif


################################################################################
# Process platform overrideable behaviour
################################################################################

# Check if -pedantic option should be used
ifeq (${DISABLE_PEDANTIC},0)
        TF_CFLAGS	+= 	-pedantic
endif

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

# Make sure PMF is enabled if PSCI STAT is enabled.
ifeq (${ENABLE_PSCI_STAT},1)
ENABLE_PMF			:= 1
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
# Build options checks
################################################################################

$(eval $(call assert_boolean,ASM_ASSERTION))
$(eval $(call assert_boolean,COLD_BOOT_SINGLE_CPU))
$(eval $(call assert_boolean,CREATE_KEYS))
$(eval $(call assert_boolean,CTX_INCLUDE_AARCH32_REGS))
$(eval $(call assert_boolean,CTX_INCLUDE_FPREGS))
$(eval $(call assert_boolean,DEBUG))
$(eval $(call assert_boolean,DISABLE_PEDANTIC))
$(eval $(call assert_boolean,ENABLE_PLAT_COMPAT))
$(eval $(call assert_boolean,ENABLE_PMF))
$(eval $(call assert_boolean,ENABLE_PSCI_STAT))
$(eval $(call assert_boolean,ENABLE_RUNTIME_INSTRUMENTATION))
$(eval $(call assert_boolean,ERROR_DEPRECATED))
$(eval $(call assert_boolean,GENERATE_COT))
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

################################################################################
# Add definitions to the cpp preprocessor based on the current build options.
# This is done after including the platform specific makefile to allow the
# platform to overwrite the default options
################################################################################

$(eval $(call add_define,ARM_CCI_PRODUCT_ID))
$(eval $(call add_define,ARM_GIC_ARCH))
$(eval $(call add_define,ASM_ASSERTION))
$(eval $(call add_define,COLD_BOOT_SINGLE_CPU))
$(eval $(call add_define,CTX_INCLUDE_AARCH32_REGS))
$(eval $(call add_define,CTX_INCLUDE_FPREGS))
$(eval $(call add_define,ENABLE_PLAT_COMPAT))
$(eval $(call add_define,ENABLE_PMF))
$(eval $(call add_define,ENABLE_PSCI_STAT))
$(eval $(call add_define,ENABLE_RUNTIME_INSTRUMENTATION))
$(eval $(call add_define,ERROR_DEPRECATED))
$(eval $(call add_define,LOAD_IMAGE_V2))
$(eval $(call add_define,LOG_LEVEL))
$(eval $(call add_define,NS_TIMER_SWITCH))
$(eval $(call add_define,PL011_GENERIC_UART))
$(eval $(call add_define,PLAT_${PLAT}))
$(eval $(call add_define,PROGRAMMABLE_RESET_ADDRESS))
$(eval $(call add_define,PSCI_EXTENDED_STATE_ID))
$(eval $(call add_define,RESET_TO_BL31))
$(eval $(call add_define,SEPARATE_CODE_AND_RODATA))
$(eval $(call add_define,SPD_${SPD}))
$(eval $(call add_define,SPIN_ON_BL1_EXIT))
$(eval $(call add_define,TRUSTED_BOARD_BOOT))
$(eval $(call add_define,USE_COHERENT_MEM))

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

# For AArch32, BL31 is not applicable, and BL2U is not supported at present.
ifneq (${ARCH},aarch32)
ifdef BL2U_SOURCES
NEED_BL2U := yes
include bl2u/bl2u.mk
endif

ifdef BL31_SOURCES
# When booting an EL3 payload, there is no need to compile the BL31 image nor
# put it in the FIP.
ifndef EL3_PAYLOAD_BASE
NEED_BL31 := yes
include bl31/bl31.mk
endif
endif
endif

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
# Build targets
################################################################################

.PHONY:	all msg_start clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool fip fwu_fip certtool
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
$(if ${BL2}, $(eval $(call MAKE_TOOL_ARGS,2,${BL2},tb-fw)),\
	$(eval $(call MAKE_BL,2,tb-fw)))
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

locate-checkpatch:
ifndef CHECKPATCH
	$(error "Please set CHECKPATCH to point to the Linux checkpatch.pl file, eg: CHECKPATCH=../linux/script/checkpatch.pl")
else
ifeq (,$(wildcard ${CHECKPATCH}))
	$(error "The file CHECKPATCH points to cannot be found, use eg: CHECKPATCH=../linux/script/checkpatch.pl")
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
		git ls-files | grep -E -v libfdt\|stdlib\|docs\|\.md |	\
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
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH}
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
	@echo
	@echo "Built $@ successfully"
	@echo "FWU certificates can be found in ${BUILD_PLAT}"
	@echo
endif

${BUILD_PLAT}/${FWU_FIP_NAME}: ${FWU_FIP_DEPS} ${FIPTOOL}
	${Q}${FIPTOOL} create ${FWU_FIP_ARGS} $@
	${Q}${FIPTOOL} info $@
	@echo
	@echo "Built $@ successfully"
	@echo

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
	@echo ""
	@echo "Note: most build targets require PLAT to be set to a specific platform."
	@echo ""
	@echo "example: build all targets for the FVP platform:"
	@echo "  CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp all"
