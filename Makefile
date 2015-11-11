#
# Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
VERSION_MINOR			:= 1

# Default goal is build all images
.DEFAULT_GOAL			:= all

include make_helpers/build_macros.mk

################################################################################
# Default values for build configurations
################################################################################

# Build verbosity
V				:= 0
# Debug build
DEBUG				:= 0
# Build platform
DEFAULT_PLAT			:= fvp
PLAT				:= ${DEFAULT_PLAT}
# SPD choice
SPD				:= none
# Base commit to perform code check on
BASE_COMMIT			:= origin/master
# NS timer register save and restore
NS_TIMER_SWITCH			:= 0
# By default, Bl1 acts as the reset handler, not BL31
RESET_TO_BL31			:= 0
# Include FP registers in cpu context
CTX_INCLUDE_FPREGS		:= 0
# Determine the version of ARM GIC architecture to use for interrupt management
# in EL3. The platform port can change this value if needed.
ARM_GIC_ARCH			:= 2
# Determine the version of ARM CCI product used in the platform. The platform
# port can change this value if needed.
ARM_CCI_PRODUCT_ID		:= 400
# Flag used to indicate if ASM_ASSERTION should be enabled for the build.
# This defaults to being present in DEBUG builds only.
ASM_ASSERTION			:= ${DEBUG}
# Build option to choose whether Trusted firmware uses Coherent memory or not.
USE_COHERENT_MEM		:= 1
# Flag used to choose the power state format viz Extended State-ID or the Original
# format.
PSCI_EXTENDED_STATE_ID		:= 0
# Default FIP file name
FIP_NAME			:= fip.bin
# By default, use the -pedantic option in the gcc command line
DISABLE_PEDANTIC		:= 0
# Flags to generate the Chain of Trust
GENERATE_COT			:= 0
CREATE_KEYS			:= 1
SAVE_KEYS			:= 0
# Flags to build TF with Trusted Boot support
TRUSTED_BOARD_BOOT		:= 0
# By default, consider that the platform's reset address is not programmable.
# The platform Makefile is free to override this value.
PROGRAMMABLE_RESET_ADDRESS	:= 0
# Build flag to warn about usage of deprecated platform and framework APIs
WARN_DEPRECATED			:= 0


################################################################################
# Checkpatch script options
################################################################################

CHECK_IGNORE		:=	--ignore COMPLEX_MACRO \
				--ignore GERRIT_CHANGE_ID \
				--ignore GIT_COMMIT_ID
CHECKPATCH_ARGS		:=	--no-tree --no-signoff ${CHECK_IGNORE}
CHECKCODE_ARGS		:=	--no-patch --no-tree --no-signoff ${CHECK_IGNORE}
# Do not check the coding style on C library files
CHECK_PATHS		:=	$(shell ls -I include -I lib) \
				$(addprefix include/,$(shell ls -I stdlib include)) \
				$(addprefix lib/,$(shell ls -I stdlib lib))


################################################################################
# Process build options
################################################################################

# Verbose flag
ifeq (${V},0)
        Q=@
        CHECKCODE_ARGS	+=	--no-summary --terse
else
        Q=
endif
export Q

# Process Debug flag
$(eval $(call add_define,DEBUG))
ifneq (${DEBUG}, 0)
        BUILD_TYPE	:=	debug
        CFLAGS		+= 	-g
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

ASFLAGS			+= 	-nostdinc -ffreestanding -Wa,--fatal-warnings	\
				-Werror -Wmissing-include-dirs			\
				-mgeneral-regs-only -D__ASSEMBLY__		\
				${DEFINES} ${INCLUDES}
CFLAGS			+= 	-nostdinc -ffreestanding -Wall			\
				-Werror -Wmissing-include-dirs			\
				-mgeneral-regs-only -mstrict-align		\
				-std=c99 -c -Os					\
				${DEFINES} ${INCLUDES}
CFLAGS			+=	-ffunction-sections -fdata-sections

LDFLAGS			+=	--fatal-warnings -O1
LDFLAGS			+=	--gc-sections


################################################################################
# Common sources and include directories
################################################################################

BL_COMMON_SOURCES	+=	common/bl_common.c			\
				common/tf_printf.c			\
				common/aarch64/debug.S			\
				lib/aarch64/cache_helpers.S		\
				lib/aarch64/misc_helpers.S		\
				lib/aarch64/xlat_helpers.c		\
				lib/stdlib/std.c			\
				plat/common/aarch64/platform_helpers.S

INCLUDES		+=	-Iinclude/bl31			\
				-Iinclude/bl31/services		\
				-Iinclude/common		\
				-Iinclude/drivers		\
				-Iinclude/drivers/arm		\
				-Iinclude/drivers/auth		\
				-Iinclude/drivers/io		\
				-Iinclude/drivers/ti/uart	\
				-Iinclude/lib			\
				-Iinclude/lib/aarch64		\
				-Iinclude/lib/cpus/aarch64	\
				-Iinclude/plat/common		\
				-Iinclude/stdlib		\
				-Iinclude/stdlib/sys		\
				${PLAT_INCLUDES}		\
				${SPD_INCLUDES}


################################################################################
# Generic definitions
################################################################################

BUILD_BASE		:=	./build
BUILD_PLAT		:=	${BUILD_BASE}/${PLAT}/${BUILD_TYPE}

PLAT_MAKEFILE		:=	platform.mk
# Generate the platforms list by recursively searching for all directories
# under /plat containing a PLAT_MAKEFILE. Append each platform with a `|`
# char and strip out the final '|'.
PLATFORMS		:=	$(shell find plat/ -name '${PLAT_MAKEFILE}' -print0 |			\
					sed -r 's%[^\x00]*\/([^/]*)\/${PLAT_MAKEFILE}\x00%\1|%g' |	\
					sed -r 's/\|$$//')
SPDS			:=	$(shell ls -I none services/spd)

# Platforms providing their own TBB makefile may override this value
INCLUDE_TBBR_MK		:=	1


################################################################################
# Include SPD Makefile if one has been specified
################################################################################

ifneq (${SPD},none)
        # We expect to locate an spd.mk under the specified SPD directory
        SPD_MAKE	:=	$(shell m="services/spd/${SPD}/${SPD}.mk"; [ -f "$$m" ] && echo "$$m")

        ifeq (${SPD_MAKE},)
                $(error Error: No services/spd/${SPD}/${SPD}.mk located)
        endif
        $(info Including ${SPD_MAKE})
        include ${SPD_MAKE}

        # If there's BL3-2 companion for the chosen SPD, and the SPD wants to build the
        # BL3-2 from source, we expect that the SPD's Makefile would set NEED_BL32
        # variable to "yes". In case the BL3-2 is a binary which needs to be included in
        # fip, then the NEED_BL32 needs to be set and BL3-2 would need to point to the bin.
endif


################################################################################
# Include the platform specific Makefile after the SPD Makefile (the platform
# makefile may use all previous definitions in this file)
################################################################################

ifeq (${PLAT},)
        $(error "Error: Unknown platform. Please use PLAT=<platform name> to specify the platform")
endif
PLAT_MAKEFILE_FULL	:=	$(shell find plat/ -wholename '*/${PLAT}/${PLAT_MAKEFILE}')
ifeq ($(PLAT_MAKEFILE_FULL),)
        $(error "Error: Invalid platform. The following platforms are available: ${PLATFORMS}")
endif

include ${PLAT_MAKEFILE_FULL}

# If the platform has not defined ENABLE_PLAT_COMPAT, then enable it by default
ifndef ENABLE_PLAT_COMPAT
ENABLE_PLAT_COMPAT := 1
endif

# Include the platform compatibility helpers for PSCI
ifneq (${ENABLE_PLAT_COMPAT}, 0)
include plat/compat/plat_compat.mk
endif

# Include the CPU specific operations makefile. By default all CPU errata
# workarounds and CPU specifc optimisations are disabled. This can be
# overridden by the platform.
include lib/cpus/cpu-ops.mk


################################################################################
# Process platform overrideable behaviour
################################################################################

# Check if -pedantic option should be used
ifeq (${DISABLE_PEDANTIC},0)
        CFLAGS		+= 	-pedantic
endif

# Using the ARM Trusted Firmware BL2 implies that a BL3-3 image also need to be
# supplied for the FIP and Certificate generation tools. This flag can be
# overridden by the platform.
ifdef BL2_SOURCES
NEED_BL33		?=	yes
endif

# Process TBB related flags
ifneq (${GENERATE_COT},0)
        # Common cert_create options
        ifneq (${CREATE_KEYS},0)
                $(eval CRT_ARGS += -n)
                ifneq (${SAVE_KEYS},0)
                        $(eval CRT_ARGS += -k)
                endif
        endif
        # Include TBBR makefile (unless the platform indicates otherwise)
        ifeq (${INCLUDE_TBBR_MK},1)
                include make_helpers/tbbr/tbbr_tools.mk
        endif
endif


################################################################################
# Auxiliary tools (fip_create, cert_create, etc)
################################################################################

# Variables for use with Certificate Generation Tool
CRTTOOLPATH		?=	tools/cert_create
CRTTOOL			?=	${CRTTOOLPATH}/cert_create

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fip_create
FIPTOOL			?=	${FIPTOOLPATH}/fip_create


################################################################################
# Build options checks
################################################################################

$(eval $(call assert_boolean,DEBUG))
$(eval $(call assert_boolean,NS_TIMER_SWITCH))
$(eval $(call assert_boolean,RESET_TO_BL31))
$(eval $(call assert_boolean,CTX_INCLUDE_FPREGS))
$(eval $(call assert_boolean,ASM_ASSERTION))
$(eval $(call assert_boolean,USE_COHERENT_MEM))
$(eval $(call assert_boolean,DISABLE_PEDANTIC))
$(eval $(call assert_boolean,GENERATE_COT))
$(eval $(call assert_boolean,CREATE_KEYS))
$(eval $(call assert_boolean,SAVE_KEYS))
$(eval $(call assert_boolean,TRUSTED_BOARD_BOOT))
$(eval $(call assert_boolean,PROGRAMMABLE_RESET_ADDRESS))
$(eval $(call assert_boolean,PSCI_EXTENDED_STATE_ID))
$(eval $(call assert_boolean,WARN_DEPRECATED))
$(eval $(call assert_boolean,ENABLE_PLAT_COMPAT))


################################################################################
# Add definitions to the cpp preprocessor based on the current build options.
# This is done after including the platform specific makefile to allow the
# platform to overwrite the default options
################################################################################

$(eval $(call add_define,PLAT_${PLAT}))
$(eval $(call add_define,NS_TIMER_SWITCH))
$(eval $(call add_define,RESET_TO_BL31))
$(eval $(call add_define,CTX_INCLUDE_FPREGS))
$(eval $(call add_define,ARM_GIC_ARCH))
$(eval $(call add_define,ARM_CCI_PRODUCT_ID))
$(eval $(call add_define,ASM_ASSERTION))
$(eval $(call add_define,LOG_LEVEL))
$(eval $(call add_define,USE_COHERENT_MEM))
$(eval $(call add_define,TRUSTED_BOARD_BOOT))
$(eval $(call add_define,PROGRAMMABLE_RESET_ADDRESS))
$(eval $(call add_define,PSCI_EXTENDED_STATE_ID))
$(eval $(call add_define,WARN_DEPRECATED))
$(eval $(call add_define,ENABLE_PLAT_COMPAT))


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

ifdef BL31_SOURCES
NEED_BL31 := yes
include bl31/bl31.mk
endif


################################################################################
# Build targets
################################################################################

.PHONY:	all msg_start clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool fip certtool
.SUFFIXES:

all: msg_start

msg_start:
	@echo "Building ${PLAT}"

# Expand build macros for the different images
ifeq (${NEED_BL1},yes)
$(eval $(call MAKE_BL,1))
endif

ifeq (${NEED_BL2},yes)
$(if ${BL2}, $(eval $(call MAKE_TOOL_ARGS,2,${BL2},in_fip)),\
	$(eval $(call MAKE_BL,2,in_fip)))
endif

ifeq (${NEED_BL31},yes)
BL31_SOURCES += ${SPD_SOURCES}
$(if ${BL31}, $(eval $(call MAKE_TOOL_ARGS,31,${BL31},in_fip)),\
	$(eval $(call MAKE_BL,31,in_fip)))
endif

ifeq (${NEED_BL32},yes)
$(if ${BL32}, $(eval $(call MAKE_TOOL_ARGS,32,${BL32},in_fip)),\
	$(eval $(call MAKE_BL,32,in_fip)))
endif

# Add the BL33 image if required by the platform
ifeq (${NEED_BL33},yes)
$(eval $(call FIP_ADD_IMG,BL33,--bl33))
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
	${Q}rm -rf ${BUILD_PLAT}
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH} clean

realclean distclean:
	@echo "  REALCLEAN"
	${Q}rm -rf ${BUILD_BASE}
	${Q}rm -f ${CURDIR}/cscope.*
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH} clean
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH} clean

checkcodebase:		locate-checkpatch
	@echo "  CHECKING STYLE"
	@if test -d .git ; then	\
		git ls-files | grep -v stdlib | while read GIT_FILE ; do ${CHECKPATCH} ${CHECKCODE_ARGS} -f $$GIT_FILE ; done ;	\
	 else			\
		 find . -type f -not -iwholename "*.git*" -not -iwholename "*build*" -not -iwholename "*stdlib*" -exec ${CHECKPATCH} ${CHECKCODE_ARGS} -f {} \; ;	\
	 fi

checkpatch:		locate-checkpatch
	@echo "  CHECKING STYLE"
	${Q}git log -p ${BASE_COMMIT}..HEAD -- ${CHECK_PATHS} | ${CHECKPATCH} ${CHECKPATCH_ARGS} - || true

certtool: ${CRTTOOL}

.PHONY: ${CRTTOOL}
${CRTTOOL}:
	${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH}
	@echo
	@echo "Built $@ successfully"
	@echo

ifneq (${GENERATE_COT},0)
certificates: ${CRT_DEPS} ${CRTTOOL}
	${Q}${CRTTOOL} ${CRT_ARGS}
	@echo
	@echo "Built $@ successfully"
	@echo "Certificates can be found in ${BUILD_PLAT}"
	@echo
endif

${BUILD_PLAT}/${FIP_NAME}: ${FIP_DEPS} ${FIPTOOL}
	${Q}${FIPTOOL} --dump ${FIP_ARGS} $@
	@echo
	@echo "Built $@ successfully"
	@echo

fiptool: ${FIPTOOL}
fip: ${BUILD_PLAT}/${FIP_NAME}

.PHONY: ${FIPTOOL}
${FIPTOOL}:
	${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH}

cscope:
	@echo "  CSCOPE"
	${Q}find ${CURDIR} -name "*.[chsS]" > cscope.files
	${Q}cscope -b -q -k

help:
	@echo "usage: ${MAKE} PLAT=<${PLATFORMS}> [OPTIONS] [TARGET]"
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
	@echo "  bl31           Build the BL3-1 binary"
	@echo "  bl32           Build the BL3-2 binary"
	@echo "  certificates   Build the certificates (requires 'GENERATE_COT=1')"
	@echo "  fip            Build the Firmware Image Package (FIP)"
	@echo "  checkcodebase  Check the coding style of the entire source tree"
	@echo "  checkpatch     Check the coding style on changes in the current"
	@echo "                 branch against BASE_COMMIT (default origin/master)"
	@echo "  clean          Clean the build for the selected platform"
	@echo "  cscope         Generate cscope index"
	@echo "  distclean      Remove all build artifacts for all platforms"
	@echo "  certtool       Build the Certificate generation tool"
	@echo "  fiptool        Build the Firmware Image Package(FIP) creation tool"
	@echo ""
	@echo "Note: most build targets require PLAT to be set to a specific platform."
	@echo ""
	@echo "example: build all targets for the FVP platform:"
	@echo "  CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp all"
