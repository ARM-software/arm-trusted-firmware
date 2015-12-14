#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

# This table is used in converting lower case to upper case.
uppercase_table:=a,A b,B c,C d,D e,E f,F g,G h,H i,I j,J k,K l,L m,M n,N o,O p,P q,Q r,R s,S t,T u,U v,V w,W x,X y,Y z,Z

# Internal macro used for converting lower case to upper case.
#   $(1) = upper case table
#   $(2) = String to convert
define uppercase_internal
$(if $(1),$$(subst $(firstword $(1)),$(call uppercase_internal,$(wordlist 2,$(words $(1)),$(1)),$(2))),$(2))
endef

# A macro for converting a string to upper case
#   $(1) = String to convert
define uppercase
$(eval uppercase_result:=$(call uppercase_internal,$(uppercase_table),$(1)))$(uppercase_result)
endef

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

# IMG_LINKERFILE defines the linker script corresponding to a BL stage
#   $(1) = BL stage (2, 30, 31, 32, 33)
define IMG_LINKERFILE
    ${BUILD_DIR}/bl$(1).ld
endef

# IMG_MAPFILE defines the output file describing the memory map corresponding
# to a BL stage
#   $(1) = BL stage (2, 30, 31, 32, 33)
define IMG_MAPFILE
    ${BUILD_DIR}/bl$(1).map
endef

# IMG_ELF defines the elf file corresponding to a BL stage
#   $(1) = BL stage (2, 30, 31, 32, 33)
define IMG_ELF
    ${BUILD_DIR}/bl$(1).elf
endef

# IMG_DUMP defines the symbols dump file corresponding to a BL stage
#   $(1) = BL stage (2, 30, 31, 32, 33)
define IMG_DUMP
    ${BUILD_DIR}/bl$(1).dump
endef

# IMG_BIN defines the default image file corresponding to a BL stage
#   $(1) = BL stage (2, 30, 31, 32, 33)
define IMG_BIN
    ${BUILD_PLAT}/bl$(1).bin
endef

# FIP_ADD_PAYLOAD appends the command line arguments required by the FIP tool
# to package a new payload. Optionally, it adds the dependency on this payload
#   $(1) = payload filename (i.e. bl31.bin)
#   $(2) = command line option for the specified payload (i.e. --bl31)
#   $(3) = fip target dependency (optional) (i.e. bl31)
define FIP_ADD_PAYLOAD
    $(eval FIP_ARGS += $(2) $(1))
    $(eval $(if $(3),FIP_DEPS += $(3)))
endef

# CERT_ADD_CMD_OPT adds a new command line option to the cert_create invocation
#   $(1) = parameter filename
#   $(2) = cert_create command line option for the specified parameter
#   $(3) = input parameter (false if empty)
define CERT_ADD_CMD_OPT
    $(eval $(if $(3),CRT_DEPS += $(1)))
    $(eval CRT_ARGS += $(2) $(1))
endef

# FIP_ADD_IMG allows the platform to specify an image to be packed in the FIP
# using a build option. It also adds a dependency on the image file, aborting
# the build if the file does not exist.
#   $(1) = build option to specify the image filename (SCP_BL2, BL33, etc)
#   $(2) = command line option for the fip_create tool (scp_bl2, bl33, etc)
# Example:
#   $(eval $(call FIP_ADD_IMG,BL33,--bl33))
define FIP_ADD_IMG
    CRT_DEPS += check_$(1)
    FIP_DEPS += check_$(1)
    $(call FIP_ADD_PAYLOAD,$(value $(1)),$(2))

check_$(1):
	$$(if $(value $(1)),,$$(error "Platform '${PLAT}' requires $(1). Please set $(1) to point to the right file"))
endef

# FWU_FIP_ADD_PAYLOAD appends the command line arguments required by the FIP tool
# to package a new FWU payload. Optionally, it  adds the dependency on this payload
#   $(1) = payload filename (e.g. ns_bl2u.bin)
#   $(2) = command line option for the specified payload (e.g. --ns_bl2u)
#   $(3) = fip target dependency (optional) (e.g. ns_bl2u)
define FWU_FIP_ADD_PAYLOAD
    $(eval $(if $(3),FWU_FIP_DEPS += $(3)))
    $(eval FWU_FIP_ARGS += $(2) $(1))
endef

# FWU_CERT_ADD_CMD_OPT adds a new command line option to the cert_create invocation
#   $(1) = parameter filename
#   $(2) = cert_create command line option for the specified parameter
#   $(3) = input parameter (false if empty)
define FWU_CERT_ADD_CMD_OPT
    $(eval $(if $(3),FWU_CRT_DEPS += $(1)))
    $(eval FWU_CRT_ARGS += $(2) $(1))
endef

# FWU_FIP_ADD_IMG allows the platform to pack a binary image in the FWU FIP
#   $(1) build option to specify the image filename (BL2U, NS_BL2U, etc)
#   $(2) command line option for the fip_create tool (bl2u, ns_bl2u, etc)
# Example:
#   $(eval $(call FWU_FIP_ADD_IMG,BL2U,--bl2u))
define FWU_FIP_ADD_IMG
    FWU_CRT_DEPS += check_$(1)
    FWU_FIP_DEPS += check_$(1)
    $(call FWU_FIP_ADD_PAYLOAD,$(value $(1)),$(2))

check_$(1):
	$$(if $(value $(1)),,$$(error "Platform '${PLAT}' requires $(1). Please set $(1) to point to the right file"))
endef

################################################################################
# Auxiliary macros to build TF images from sources
################################################################################

# If no goal is specified in the command line, .DEFAULT_GOAL is used.
# .DEFAULT_GOAL is defined in the main Makefile before including this file.
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS := $(.DEFAULT_GOAL)
endif

define match_goals
$(strip $(foreach goal,$(1),$(filter $(goal),$(MAKECMDGOALS))))
endef

# List of rules that involve building things
BUILD_TARGETS := all bl1 bl2 bl2u bl31 bl32 certificates fip

# Does the list of goals specified on the command line include a build target?
ifneq ($(call match_goals,${BUILD_TARGETS}),)
IS_ANYTHING_TO_BUILD := 1
endif


# MAKE_C builds a C source file and generates the dependency file
#   $(1) = output directory
#   $(2) = source file (%.c)
#   $(3) = BL stage (2, 2u, 30, 31, 32, 33)
define MAKE_C

$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
$(eval PREREQUISITES := $(patsubst %.o,%.d,$(OBJ)))
$(eval IMAGE := IMAGE_BL$(call uppercase,$(3)))

$(OBJ): $(2)
	@echo "  CC      $$<"
	$$(Q)$$(CC) $$(CFLAGS) -D$(IMAGE) -c $$< -o $$@


$(PREREQUISITES): $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $(1)
	$$(Q)$$(CC) $$(CFLAGS) -M -MT $(OBJ) -MF $$@ $$<

ifdef IS_ANYTHING_TO_BUILD
-include $(PREREQUISITES)
endif

endef


# MAKE_S builds an assembly source file and generates the dependency file
#   $(1) = output directory
#   $(2) = assembly file (%.S)
#   $(3) = BL stage (2, 2u, 30, 31, 32, 33)
define MAKE_S

$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))
$(eval PREREQUISITES := $(patsubst %.o,%.d,$(OBJ)))
$(eval IMAGE := IMAGE_BL$(call uppercase,$(3)))

$(OBJ): $(2)
	@echo "  AS      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -D$(IMAGE) -c $$< -o $$@

$(PREREQUISITES): $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $(1)
	$$(Q)$$(AS) $$(ASFLAGS) -M -MT $(OBJ) -MF $$@ $$<

ifdef IS_ANYTHING_TO_BUILD
-include $(PREREQUISITES)
endif

endef


# MAKE_LD generate the linker script using the C preprocessor
#   $(1) = output linker script
#   $(2) = input template
define MAKE_LD

$(eval PREREQUISITES := $(1).d)

$(1): $(2)
	@echo "  PP      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -P -E -D__LINKER__ -o $$@ $$<

$(PREREQUISITES): $(2)
	@echo "  DEPS    $$@"
	@mkdir -p $$(dir $$@)
	$$(Q)$$(AS) $$(ASFLAGS) -M -MT $(1) -MF $$@ $$<

ifdef IS_ANYTHING_TO_BUILD
-include $(PREREQUISITES)
endif

endef


# MAKE_OBJS builds both C and assembly source files
#   $(1) = output directory
#   $(2) = list of source files (both C and assembly)
#   $(3) = BL stage (2, 30, 31, 32, 33)
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


# MAKE_TOOL_ARGS macro defines the command line arguments for the FIP tool for
# each BL image. Arguments:
#   $(1) = BL stage (2, 30, 31, 32, 33)
#   $(2) = Binary file
#   $(3) = In FIP (false if empty)
define MAKE_TOOL_ARGS
        $(if $(3),$(eval $(call FIP_ADD_PAYLOAD,$(2),--bl$(1),bl$(1))))
endef


# MAKE_BL macro defines the targets and options to build each BL image.
# Arguments:
#   $(1) = BL stage (2, 2u, 30, 31, 32, 33)
#   $(2) = In FIP (false if empty)
define MAKE_BL
        $(eval BUILD_DIR  := ${BUILD_PLAT}/bl$(1))
        $(eval BL_SOURCES := $(BL$(call uppercase,$(1))_SOURCES))
        $(eval SOURCES    := $(BL_SOURCES) $(BL_COMMON_SOURCES) $(PLAT_BL_COMMON_SOURCES))
        $(eval OBJS       := $(addprefix $(BUILD_DIR)/,$(call SOURCES_TO_OBJS,$(SOURCES))))
        $(eval LINKERFILE := $(call IMG_LINKERFILE,$(1)))
        $(eval MAPFILE    := $(call IMG_MAPFILE,$(1)))
        $(eval ELF        := $(call IMG_ELF,$(1)))
        $(eval DUMP       := $(call IMG_DUMP,$(1)))
        $(eval BIN        := $(call IMG_BIN,$(1)))
        $(eval BL_LINKERFILE := $(BL$(call uppercase,$(1))_LINKERFILE))

        $(eval $(call MAKE_OBJS,$(BUILD_DIR),$(SOURCES),$(1)))
        $(eval $(call MAKE_LD,$(LINKERFILE),$(BL_LINKERFILE)))

$(BUILD_DIR):
	$$(Q)mkdir -p "$$@"

$(ELF): $(OBJS) $(LINKERFILE)
	@echo "  LD      $$@"
	@echo 'const char build_message[] = "Built : "__TIME__", "__DATE__; \
	       const char version_string[] = "${VERSION_STRING}";' | \
		$$(CC) $$(CFLAGS) -xc - -o $(BUILD_DIR)/build_message.o
	$$(Q)$$(LD) -o $$@ $$(LDFLAGS) -Map=$(MAPFILE) --script $(LINKERFILE) \
					$(BUILD_DIR)/build_message.o $(OBJS)

$(DUMP): $(ELF)
	@echo "  OD      $$@"
	$${Q}$${OD} -dx $$< > $$@

$(BIN): $(ELF)
	@echo "  BIN     $$@"
	$$(Q)$$(OC) -O binary $$< $$@
	@echo
	@echo "Built $$@ successfully"
	@echo

.PHONY: bl$(1)
bl$(1): $(BUILD_DIR) $(BIN) $(DUMP)

all: bl$(1)

$(eval $(call MAKE_TOOL_ARGS,$(1),$(BIN),$(2)))

endef

