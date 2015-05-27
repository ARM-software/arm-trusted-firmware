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
VERSION_MAJOR		:= 1
VERSION_MINOR		:= 1

#
# Default values for build configurations
#

# Build verbosity
V			:= 0
# Debug build
DEBUG			:= 0
# Build platform
DEFAULT_PLAT		:= fvp
PLAT			:= ${DEFAULT_PLAT}
# SPD choice
SPD			:= none
# Base commit to perform code check on
BASE_COMMIT		:= origin/master
# NS timer register save and restore
NS_TIMER_SWITCH		:= 0
# By default, Bl1 acts as the reset handler, not BL31
RESET_TO_BL31		:= 0
# Include FP registers in cpu context
CTX_INCLUDE_FPREGS		:= 0
# Determine the version of ARM GIC architecture to use for interrupt management
# in EL3. The platform port can change this value if needed.
ARM_GIC_ARCH		:=	2
# Determine the version of ARM CCI product used in the platform. The platform
# port can change this value if needed.
ARM_CCI_PRODUCT_ID	:=	400
# Flag used to indicate if ASM_ASSERTION should be enabled for the build.
# This defaults to being present in DEBUG builds only.
ASM_ASSERTION		:=	${DEBUG}
# Build option to choose whether Trusted firmware uses Coherent memory or not.
USE_COHERENT_MEM	:=	1
# Default FIP file name
FIP_NAME		:= fip.bin
# By default, use the -pedantic option in the gcc command line
DISABLE_PEDANTIC	:= 0
# Flags to generate the Chain of Trust
GENERATE_COT		:= 0
CREATE_KEYS		:= 1
# Flags to build TF with Trusted Boot support
TRUSTED_BOARD_BOOT	:= 0
AUTH_MOD		:= none

# Checkpatch ignores
CHECK_IGNORE		=	--ignore COMPLEX_MACRO \
				--ignore GERRIT_CHANGE_ID \
				--ignore GIT_COMMIT_ID

CHECKPATCH_ARGS		=	--no-tree --no-signoff ${CHECK_IGNORE}
CHECKCODE_ARGS		=	--no-patch --no-tree --no-signoff ${CHECK_IGNORE}
# Do not check the coding style on C library files
CHECK_PATHS		=	$(shell ls -I include -I lib) \
				$(addprefix include/,$(shell ls -I stdlib include)) \
				$(addprefix lib/,$(shell ls -I stdlib lib))

ifeq (${V},0)
	Q=@
	CHECKCODE_ARGS	+=	--no-summary --terse
else
	Q=
endif
export Q

ifneq (${DEBUG}, 0)
	BUILD_TYPE	:=	debug
	# Use LOG_LEVEL_INFO by default for debug builds
	LOG_LEVEL	:=	40
else
	BUILD_TYPE	:=	release
	# Use LOG_LEVEL_NOTICE by default for release builds
	LOG_LEVEL	:=	20
endif

# Default build string (git branch and commit)
ifeq (${BUILD_STRING},)
	BUILD_STRING	:=	$(shell git log -n 1 --pretty=format:"%h")
endif

VERSION_STRING		:=	v${VERSION_MAJOR}.${VERSION_MINOR}(${BUILD_TYPE}):${BUILD_STRING}

BL_COMMON_SOURCES	:=	common/bl_common.c			\
				common/tf_printf.c			\
				common/aarch64/debug.S			\
				lib/aarch64/cache_helpers.S		\
				lib/aarch64/misc_helpers.S		\
				lib/aarch64/xlat_helpers.c		\
				lib/stdlib/std.c			\
				plat/common/aarch64/platform_helpers.S

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

# Convenience function for adding build definitions
# $(eval $(call add_define,FOO)) will have:
# -DFOO if $(FOO) is empty; -DFOO=$(FOO) otherwise
define add_define
DEFINES			+=	-D$(1)$(if $(value $(1)),=$(value $(1)),)
endef

# Convenience function for verifying option has a boolean value
# $(eval $(call assert_boolean,FOO)) will assert FOO is 0 or 1
define assert_boolean
$(and $(patsubst 0,,$(value $(1))),$(patsubst 1,,$(value $(1))),$(error $(1) must be boolean))
endef

ifeq (${PLAT},)
  $(error "Error: Unknown platform. Please use PLAT=<platform name> to specify the platform")
endif
PLAT_MAKEFILE_FULL	:=	$(shell find plat/ -wholename '*/${PLAT}/${PLAT_MAKEFILE}')
ifeq ($(PLAT_MAKEFILE_FULL),)
  $(error "Error: Invalid platform. The following platforms are available: ${PLATFORMS}")
endif

all: msg_start

msg_start:
	@echo "Building ${PLAT}"

include ${PLAT_MAKEFILE_FULL}

# Include the CPU specific operations makefile. By default all CPU errata
# workarounds and CPU specifc optimisations are disabled. This can be
# overridden by the platform.
include lib/cpus/cpu-ops.mk

ifdef BL1_SOURCES
NEED_BL1 := yes
include bl1/bl1.mk
endif

ifdef BL2_SOURCES
NEED_BL2 := yes
include bl2/bl2.mk
# Using the ARM Trusted Firmware BL2 implies that a BL3-3 image also need to be supplied for the FIP.
# This flag can be overridden by the platform.
NEED_BL33 ?= yes
endif

ifdef BL31_SOURCES
NEED_BL31 := yes
include bl31/bl31.mk
endif

# Include SPD Makefile if one has been specified
ifneq (${SPD},none)
  # We expect to locate an spd.mk under the specified SPD directory
  SPD_MAKE		:=	$(shell m="services/spd/${SPD}/${SPD}.mk"; [ -f "$$m" ] && echo "$$m")

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

.PHONY:			all msg_start clean realclean distclean cscope locate-checkpatch checkcodebase checkpatch fiptool fip certtool
.SUFFIXES:

INCLUDES		+=	-Iinclude/bl31			\
				-Iinclude/bl31/services		\
				-Iinclude/common		\
				-Iinclude/drivers		\
				-Iinclude/drivers/arm		\
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

# Process DEBUG flag
$(eval $(call assert_boolean,DEBUG))
$(eval $(call add_define,DEBUG))
ifeq (${DEBUG},0)
  $(eval $(call add_define,NDEBUG))
else
CFLAGS			+= 	-g
ASFLAGS			+= 	-g -Wa,--gdwarf-2
endif

# Process PLAT flag
$(eval $(call add_define,PLAT_${PLAT}))

# Process NS_TIMER_SWITCH flag
$(eval $(call assert_boolean,NS_TIMER_SWITCH))
$(eval $(call add_define,NS_TIMER_SWITCH))

# Process RESET_TO_BL31 flag
$(eval $(call assert_boolean,RESET_TO_BL31))
$(eval $(call add_define,RESET_TO_BL31))

# Process CTX_INCLUDE_FPREGS flag
$(eval $(call assert_boolean,CTX_INCLUDE_FPREGS))
$(eval $(call add_define,CTX_INCLUDE_FPREGS))

# Process ARM_GIC_ARCH flag
$(eval $(call add_define,ARM_GIC_ARCH))

# Process ARM_CCI_PRODUCT_ID flag
$(eval $(call add_define,ARM_CCI_PRODUCT_ID))

# Process ASM_ASSERTION flag
$(eval $(call assert_boolean,ASM_ASSERTION))
$(eval $(call add_define,ASM_ASSERTION))

# Process LOG_LEVEL flag
$(eval $(call add_define,LOG_LEVEL))

# Process USE_COHERENT_MEM flag
$(eval $(call assert_boolean,USE_COHERENT_MEM))
$(eval $(call add_define,USE_COHERENT_MEM))

# Process Generate CoT flags
$(eval $(call assert_boolean,GENERATE_COT))
$(eval $(call assert_boolean,CREATE_KEYS))

# Process TRUSTED_BOARD_BOOT flag
$(eval $(call assert_boolean,TRUSTED_BOARD_BOOT))
$(eval $(call add_define,TRUSTED_BOARD_BOOT))

ASFLAGS			+= 	-nostdinc -ffreestanding -Wa,--fatal-warnings	\
				-Werror -Wmissing-include-dirs			\
				-mgeneral-regs-only -D__ASSEMBLY__		\
				${DEFINES} ${INCLUDES}
CFLAGS			+= 	-nostdinc -ffreestanding -Wall			\
				-Werror -Wmissing-include-dirs			\
				-mgeneral-regs-only -std=c99 -c -Os		\
				${DEFINES} ${INCLUDES}
CFLAGS			+=	-ffunction-sections -fdata-sections

LDFLAGS			+=	--fatal-warnings -O1
LDFLAGS			+=	--gc-sections


CC			:=	${CROSS_COMPILE}gcc
CPP			:=	${CROSS_COMPILE}cpp
AS			:=	${CROSS_COMPILE}gcc
AR			:=	${CROSS_COMPILE}ar
LD			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump
NM			:=	${CROSS_COMPILE}nm
PP			:=	${CROSS_COMPILE}gcc -E ${CFLAGS}

# Variables for use with Firmware Image Package
FIPTOOLPATH		?=	tools/fip_create
FIPTOOL			?=	${FIPTOOLPATH}/fip_create
fiptool:		${FIPTOOL}
fip:			${BUILD_PLAT}/${FIP_NAME}

# Variables for use with Certificate Generation Tool
CRTTOOLPATH		?=	tools/cert_create
CRTTOOL			?=	${CRTTOOLPATH}/cert_create
certtool:		${CRTTOOL}

# CoT generation tool default parameters
TRUSTED_KEY_CERT	:=	${BUILD_PLAT}/trusted_key.crt

# Pass the private keys to the CoT generation tool in the command line
# If CREATE_KEYS is set, the '-n' option will be added, indicating the tool to create new keys
ifneq (${GENERATE_COT},0)
    $(eval CERTS := yes)

    $(eval FIP_DEPS += certificates)
    $(eval FIP_ARGS += --trusted-key-cert ${TRUSTED_KEY_CERT})

    ifneq (${CREATE_KEYS},0)
        $(eval CRT_ARGS += -n)
    endif
    $(eval CRT_ARGS += $(if ${ROT_KEY}, --rot-key ${ROT_KEY}))
    $(eval CRT_ARGS += $(if ${TRUSTED_WORLD_KEY}, --trusted-world-key ${TRUSTED_WORLD_KEY}))
    $(eval CRT_ARGS += $(if ${NON_TRUSTED_WORLD_KEY}, --non-trusted-world-key ${NON_TRUSTED_WORLD_KEY}))
    $(eval CRT_ARGS += --trusted-key-cert ${TRUSTED_KEY_CERT})
endif

# Check Trusted Board Boot options
ifneq (${TRUSTED_BOARD_BOOT},0)
    ifeq (${AUTH_MOD},none)
        $(error Error: When TRUSTED_BOARD_BOOT=1, AUTH_MOD has to be the name of a valid authentication module)
    else
        # We expect to locate an *.mk file under the specified AUTH_MOD directory
        AUTH_MAKE := $(shell m="common/auth/${AUTH_MOD}/${AUTH_MOD}.mk"; [ -f "$$m" ] && echo "$$m")
        ifeq (${AUTH_MAKE},)
            $(error Error: No common/auth/${AUTH_MOD}/${AUTH_MOD}.mk located)
        endif
        $(info Including ${AUTH_MAKE})
        include ${AUTH_MAKE}
    endif

    BL_COMMON_SOURCES	+=	common/auth.c
endif

# Check if -pedantic option should be used
ifeq (${DISABLE_PEDANTIC},0)
    CFLAGS		+= 	-pedantic
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

.PHONY: ${CRTTOOL}
${CRTTOOL}:
			${Q}${MAKE} PLAT=${PLAT} --no-print-directory -C ${CRTTOOLPATH}
			@echo
			@echo "Built $@ successfully"
			@echo

.PHONY: ${FIPTOOL}
${FIPTOOL}:
			${Q}${MAKE} --no-print-directory -C ${FIPTOOLPATH}

define match_goals
$(strip $(foreach goal,$(1),$(filter $(goal),$(MAKECMDGOALS))))
endef

# List of rules that involve building things
BUILD_TARGETS := all bl1 bl2 bl31 bl32 fip

# Does the list of goals specified on the command line include a build target?
ifneq ($(call match_goals,${BUILD_TARGETS}),)
IS_ANYTHING_TO_BUILD := 1
endif

define MAKE_C

$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
$(eval PREREQUISITES := $(patsubst %.o,%.d,$(OBJ)))

$(OBJ) : $(2)
	@echo "  CC      $$<"
	$$(Q)$$(CC) $$(CFLAGS) -DIMAGE_BL$(3) -c $$< -o $$@


$(PREREQUISITES) : $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $(1)
	$$(Q)$$(CC) $$(CFLAGS) -M -MT $(OBJ) -MF $$@ $$<

ifdef IS_ANYTHING_TO_BUILD
-include $(PREREQUISITES)
endif

endef


define MAKE_S

$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))
$(eval PREREQUISITES := $(patsubst %.o,%.d,$(OBJ)))

$(OBJ) : $(2)
	@echo "  AS      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -DIMAGE_BL$(3) -c $$< -o $$@

$(PREREQUISITES) : $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $(1)
	$$(Q)$$(AS) $$(ASFLAGS) -M -MT $(OBJ) -MF $$@ $$<

ifdef IS_ANYTHING_TO_BUILD
-include $(PREREQUISITES)
endif

endef


define MAKE_LD

$(eval PREREQUISITES := $(1).d)

$(1) : $(2)
	@echo "  PP      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -P -E -D__LINKER__ -o $$@ $$<

$(PREREQUISITES) : $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $$(dir $$@)
	$$(Q)$$(AS) $$(ASFLAGS) -M -MT $(1) -MF $$@ $$<

ifdef IS_ANYTHING_TO_BUILD
-include $(PREREQUISITES)
endif

endef


define MAKE_OBJS
	$(eval C_OBJS := $(filter %.c,$(2)))
	$(eval REMAIN := $(filter-out %.c,$(2)))
	$(eval $(foreach obj,$(C_OBJS),$(call MAKE_C,$(1),$(obj),$(3))))

	$(eval S_OBJS := $(filter %.S,$(REMAIN)))
	$(eval REMAIN := $(filter-out %.S,$(REMAIN)))
	$(eval $(foreach obj,$(S_OBJS),$(call MAKE_S,$(1),$(obj),$(3))))

	$(and $(REMAIN),$(error Unexpected source files present: $(REMAIN)))
endef


# NOTE: The line continuation '\' is required in the next define otherwise we
# end up with a line-feed characer at the end of the last c filename.
# Also bare this issue in mind if extending the list of supported filetypes.
define SOURCES_TO_OBJS
	$(notdir $(patsubst %.c,%.o,$(filter %.c,$(1)))) \
	$(notdir $(patsubst %.S,%.o,$(filter %.S,$(1))))
endef


# MAKE_TOOL_ARGS macro defines the command line arguments for the FIP and CRT
# tools at each BL stage. Arguments:
#   $(1) = BL stage (2, 30, 31, 32, 33)
#   $(2) = Binary file
#   $(3) = In FIP (false if empty)
#   $(4) = Create certificates (false if empty)
#   $(5) = Create key certificate (false if empty)
#   $(6) = Private key (optional)
define MAKE_TOOL_ARGS

$(eval FIP_DEPS += $(if $3,$(2),))
$(eval FIP_ARGS += $(if $3,--bl$(1) $(2),))
$(eval FIP_ARGS += $(if $4,--bl$(1)-cert $(BUILD_PLAT)/bl$(1).crt))
$(eval FIP_ARGS += $(if $4,$(if $5,--bl$(1)-key-cert $(BUILD_PLAT)/bl$(1)_key.crt)))

$(eval CRT_DEPS += $(if $4,$(2),))
$(eval CRT_DEPS += $(if $4,$(if $6,$(6),)))
$(eval CRT_ARGS += $(if $4,--bl$(1) $(2)))
$(eval CRT_ARGS += $(if $4,$(if $6,--bl$(1)-key $(6))))
$(eval CRT_ARGS += $(if $4,--bl$(1)-cert $(BUILD_PLAT)/bl$(1).crt))
$(eval CRT_ARGS += $(if $4,$(if $5,--bl$(1)-key-cert $(BUILD_PLAT)/bl$(1)_key.crt)))

endef


# MAKE_BL macro defines the targets and options to build each BL image.
# Arguments:
#   $(1) = BL stage (2, 30, 31, 32, 33)
#   $(2) = In FIP (false if empty)
#   $(3) = Create certificates (false if empty)
#   $(4) = Create key certificate (false if empty)
#   $(5) = Private key (optional)
define MAKE_BL
	$(eval BUILD_DIR  := ${BUILD_PLAT}/bl$(1))
	$(eval SOURCES    := $(BL$(1)_SOURCES) $(BL_COMMON_SOURCES) $(PLAT_BL_COMMON_SOURCES))
	$(eval OBJS       := $(addprefix $(BUILD_DIR)/,$(call SOURCES_TO_OBJS,$(SOURCES))))
	$(eval LINKERFILE := $(BUILD_DIR)/bl$(1).ld)
	$(eval MAPFILE    := $(BUILD_DIR)/bl$(1).map)
	$(eval ELF        := $(BUILD_DIR)/bl$(1).elf)
	$(eval DUMP       := $(BUILD_DIR)/bl$(1).dump)
	$(eval BIN        := $(BUILD_PLAT)/bl$(1).bin)

	$(eval $(call MAKE_OBJS,$(BUILD_DIR),$(SOURCES),$(1)))
	$(eval $(call MAKE_LD,$(LINKERFILE),$(BL$(1)_LINKERFILE)))

$(BUILD_DIR) :
	$$(Q)mkdir -p "$$@"

$(ELF) : $(OBJS) $(LINKERFILE)
	@echo "  LD      $$@"
	@echo 'const char build_message[] = "Built : "__TIME__", "__DATE__; \
	       const char version_string[] = "${VERSION_STRING}";' | \
		$$(CC) $$(CFLAGS) -xc - -o $(BUILD_DIR)/build_message.o
	$$(Q)$$(LD) -o $$@ $$(LDFLAGS) -Map=$(MAPFILE) --script $(LINKERFILE) \
					$(BUILD_DIR)/build_message.o $(OBJS)

$(DUMP) : $(ELF)
	@echo "  OD      $$@"
	$${Q}$${OD} -dx $$< > $$@

$(BIN) : $(ELF)
	@echo "  BIN     $$@"
	$$(Q)$$(OC) -O binary $$< $$@
	@echo
	@echo "Built $$@ successfully"
	@echo

.PHONY : bl$(1)
bl$(1) : $(BUILD_DIR) $(BIN) $(DUMP)

all : bl$(1)

$(eval $(call MAKE_TOOL_ARGS,$(1),$(BIN),$(2),$(3),$(4),$(5)))

endef


ifeq (${NEED_BL1},yes)
$(eval $(call MAKE_BL,1))
endif

ifeq (${NEED_BL2},yes)
$(if ${BL2}, $(eval $(call MAKE_TOOL_ARGS,2,${BL2},in_fip,${CERTS})),\
	$(eval $(call MAKE_BL,2,in_fip,${CERTS})))
endif

ifeq (${NEED_BL31},yes)
BL31_SOURCES += ${SPD_SOURCES}
$(if ${BL31}, $(eval $(call MAKE_TOOL_ARGS,31,${BL31},in_fip,${CERTS},${CERTS},${BL31_KEY})),\
	$(eval $(call MAKE_BL,31,in_fip,${CERTS},${CERTS},${BL31_KEY})))
endif

ifeq (${NEED_BL32},yes)
$(if ${BL32}, $(eval $(call MAKE_TOOL_ARGS,32,${BL32},in_fip,${CERTS},${CERTS},${BL32_KEY})),\
	$(eval $(call MAKE_BL,32,in_fip,${CERTS},${CERTS},${BL32_KEY})))
endif

ifeq (${NEED_BL30},yes)
$(if ${BL30}, $(eval $(call MAKE_TOOL_ARGS,30,${BL30},in_fip,${CERTS},${CERTS},${BL30_KEY})))

# If BL3-0 is needed by the platform then 'BL30' variable must be defined.
check_bl30:
	$(if ${BL30},,$(error "To build a FIP for platform ${PLAT}, please set BL30 to point to the SCP firmware"))
else

# If BL3-0 is not needed by the platform but the user still specified the path
# to a BL3-0 image then warn him that it will be ignored.
check_bl30:
	$(if ${BL30},$(warning "BL3-0 is not supported on platform ${PLAT}, it will just be ignored"),)
endif

ifeq (${NEED_BL33},yes)
$(if ${BL33}, $(eval $(call MAKE_TOOL_ARGS,33,${BL33},in_fip,${CERTS},${CERTS},${BL33_KEY})))

# If BL3-3 is needed by the platform then 'BL33' variable must be defined.
check_bl33:
	$(if ${BL33},,$(error "To build a FIP, please set BL33 to point to the Normal World binary, eg: BL33=../uefi/FVP_AARCH64_EFI.fd"))
else

# If BL3-3 is not needed by the platform but the user still specified the path
# to a BL3-3 image then warn him that it will be ignored.
check_bl33:
	$(if ${BL33},$(warning "BL3-3 is not supported on platform ${PLAT}, it will just be ignored"),)
endif

# Add the dependency on the certificates
ifneq (${GENERATE_COT},0)
    fip: certificates
endif

certificates: ${CRT_DEPS} ${CRTTOOL} check_bl30 check_bl33
			${Q}${CRTTOOL} ${CRT_ARGS}
			@echo
			@echo "Built $@ successfully"
			@echo "Certificates can be found in ${BUILD_PLAT}"
			@echo

${BUILD_PLAT}/${FIP_NAME}: ${FIP_DEPS} ${FIPTOOL} check_bl30 check_bl33
			${Q}${FIPTOOL} --dump \
				${FIP_ARGS} \
				$@
			@echo
			@echo "Built $@ successfully"
			@echo


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
