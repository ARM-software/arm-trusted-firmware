#
# Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Report an error if the eval make function is not available.
$(eval eval_available := T)
ifneq (${eval_available},T)
    $(error This makefile only works with a Make program that supports $$(eval))
endif

# Some utility macros for manipulating awkward (whitespace) characters.
blank			:=
space			:=${blank} ${blank}

# A user defined function to recursively search for a filename below a directory
#    $1 is the directory root of the recursive search (blank for current directory).
#    $2 is the file name to search for.
define rwildcard
$(strip $(foreach d,$(wildcard ${1}*),$(call rwildcard,${d}/,${2}) $(filter $(subst *,%,%${2}),${d})))
endef

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

# Convenience function for adding build definitions
# $(eval $(call add_define_val,FOO,BAR)) will have:
# -DFOO=BAR
define add_define_val
    DEFINES			+=	-D$(1)=$(2)
endef

# Convenience function for verifying option has a boolean value
# $(eval $(call assert_boolean,FOO)) will assert FOO is 0 or 1
define assert_boolean
    $(if $(filter-out 0 1,$($1)),$(error $1 must be boolean))
endef

0-9 := 0 1 2 3 4 5 6 7 8 9

# Function to verify that a given option $(1) contains a numeric value
define assert_numeric
$(if $($(1)),,$(error $(1) must not be empty))
$(eval __numeric := $($(1)))
$(foreach d,$(0-9),$(eval __numeric := $(subst $(d),,$(__numeric))))
$(if $(__numeric),$(error $(1) must be numeric))
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

# TOOL_ADD_PAYLOAD appends the command line arguments required by fiptool to
# package a new payload and/or by cert_create to generate certificate.
# Optionally, it adds the dependency on this payload
#   $(1) = payload filename (i.e. bl31.bin)
#   $(2) = command line option for the specified payload (i.e. --soc-fw)
#   $(3) = tool target dependency (optional) (ex. build/fvp/release/bl31.bin)
#   $(4) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)
define TOOL_ADD_PAYLOAD
    $(4)FIP_ARGS += $(2) $(1)
    $(if $(3),$(4)FIP_DEPS += $(3))
    $(4)CRT_ARGS += $(2) $(1)
    $(if $(3),$(4)CRT_DEPS += $(3))
endef

# TOOL_ADD_IMG_PAYLOAD works like TOOL_ADD_PAYLOAD, but applies image filters
# before passing them to host tools if BL*_PRE_TOOL_FILTER is defined.
#   $(1) = image_type (scp_bl2, bl33, etc.)
#   $(2) = payload filepath (ex. build/fvp/release/bl31.bin)
#   $(3) = command line option for the specified payload (ex. --soc-fw)
#   $(4) = tool target dependency (optional) (ex. build/fvp/release/bl31.bin)
#   $(5) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)

define TOOL_ADD_IMG_PAYLOAD

$(eval PRE_TOOL_FILTER := $($(call uppercase,$(1))_PRE_TOOL_FILTER))

ifneq ($(PRE_TOOL_FILTER),)

$(eval PROCESSED_PATH := $(BUILD_PLAT)/$(1).bin$($(PRE_TOOL_FILTER)_SUFFIX))

$(call $(PRE_TOOL_FILTER)_RULE,$(PROCESSED_PATH),$(2))

$(PROCESSED_PATH): $(4)

$(call TOOL_ADD_PAYLOAD,$(PROCESSED_PATH),$(3),$(PROCESSED_PATH),$(5))

else
$(call TOOL_ADD_PAYLOAD,$(2),$(3),$(4),$(5))
endif
endef

# CERT_ADD_CMD_OPT adds a new command line option to the cert_create invocation
#   $(1) = parameter filename
#   $(2) = cert_create command line option for the specified parameter
#   $(3) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)
define CERT_ADD_CMD_OPT
    $(3)CRT_ARGS += $(2) $(1)
endef

# TOOL_ADD_IMG allows the platform to specify an external image to be packed
# in the FIP and/or for which certificate is generated. It also adds a
# dependency on the image file, aborting the build if the file does not exist.
#   $(1) = image_type (scp_bl2, bl33, etc.)
#   $(2) = command line option for fiptool (--scp-fw, --nt-fw, etc)
#   $(3) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)
# Example:
#   $(eval $(call TOOL_ADD_IMG,bl33,--nt-fw))
define TOOL_ADD_IMG
    # Build option to specify the image filename (SCP_BL2, BL33, etc)
    # This is the uppercase form of the first parameter
    $(eval _V := $(call uppercase,$(1)))

    $(3)CRT_DEPS += check_$(1)
    $(3)FIP_DEPS += check_$(1)
    $(call TOOL_ADD_IMG_PAYLOAD,$(1),$(value $(_V)),$(2),,$(3))

.PHONY: check_$(1)
check_$(1):
	$$(if $(value $(_V)),,$$(error "Platform '${PLAT}' requires $(_V). Please set $(_V) to point to the right file"))
	$$(if $(wildcard $(value $(_V))),,$$(error '$(_V)=$(value $(_V))' was specified, but '$(value $(_V))' does not exist))
endef

################################################################################
# Generic image processing filters
################################################################################

# GZIP
define GZIP_RULE
$(1): $(2)
	@echo "  GZIP    $$@"
	$(Q)gzip -n -f -9 $$< --stdout > $$@
endef

GZIP_SUFFIX := .gz

################################################################################
# Auxiliary macros to build TF images from sources
################################################################################

MAKE_DEP = -Wp,-MD,$(DEP) -MT $$@ -MP


# MAKE_C_LIB builds a C source file and generates the dependency file
#   $(1) = output directory
#   $(2) = source file (%.c)
#   $(3) = library name
define MAKE_C_LIB
$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))

$(OBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | lib$(3)_dirs
	@echo "  CC      $$<"
	$$(Q)$$(CC) $$(TF_CFLAGS) $$(CFLAGS) $(MAKE_DEP) -c $$< -o $$@

-include $(DEP)

endef


# MAKE_C builds a C source file and generates the dependency file
#   $(1) = output directory
#   $(2) = source file (%.c)
#   $(3) = BL stage (2, 2u, 30, 31, 32, 33)
define MAKE_C

$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))
$(eval IMAGE := IMAGE_BL$(call uppercase,$(3)))

$(OBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | bl$(3)_dirs
	@echo "  CC      $$<"
	$$(Q)$$(CC) $$(TF_CFLAGS) $$(CFLAGS) -D$(IMAGE) $(MAKE_DEP) -c $$< -o $$@

-include $(DEP)

endef


# MAKE_S builds an assembly source file and generates the dependency file
#   $(1) = output directory
#   $(2) = assembly file (%.S)
#   $(3) = BL stage (2, 2u, 30, 31, 32, 33)
define MAKE_S

$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))
$(eval IMAGE := IMAGE_BL$(call uppercase,$(3)))

$(OBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | bl$(3)_dirs
	@echo "  AS      $$<"
	$$(Q)$$(AS) $$(ASFLAGS) -D$(IMAGE) $(MAKE_DEP) -c $$< -o $$@

-include $(DEP)

endef


# MAKE_LD generate the linker script using the C preprocessor
#   $(1) = output linker script
#   $(2) = input template
#   $(3) = BL stage (2, 2u, 30, 31, 32, 33)
define MAKE_LD

$(eval DEP := $(1).d)
$(eval IMAGE := IMAGE_BL$(call uppercase,$(3)))

$(1): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | bl$(3)_dirs
	@echo "  PP      $$<"
	$$(Q)$$(CPP) $$(CPPFLAGS) -P -D__ASSEMBLY__ -D__LINKER__ $(MAKE_DEP) -D$(IMAGE) -o $$@ $$<

-include $(DEP)

endef

# MAKE_LIB_OBJS builds both C source files
#   $(1) = output directory
#   $(2) = list of source files
#   $(3) = name of the library
define MAKE_LIB_OBJS
        $(eval C_OBJS := $(filter %.c,$(2)))
        $(eval REMAIN := $(filter-out %.c,$(2)))
        $(eval $(foreach obj,$(C_OBJS),$(call MAKE_C_LIB,$(1),$(obj),$(3))))

        $(and $(REMAIN),$(error Unexpected source files present: $(REMAIN)))
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
# Also bear this issue in mind if extending the list of supported filetypes.
define SOURCES_TO_OBJS
        $(notdir $(patsubst %.c,%.o,$(filter %.c,$(1)))) \
        $(notdir $(patsubst %.S,%.o,$(filter %.S,$(1))))
endef

# Allow overriding the timestamp, for example for reproducible builds, or to
# synchronize timestamps across multiple projects.
# This must be set to a C string (including quotes where applicable).
BUILD_MESSAGE_TIMESTAMP ?= __TIME__", "__DATE__

.PHONY: libraries

# MAKE_LIB_DIRS macro defines the target for the directory where
# libraries are created
define MAKE_LIB_DIRS
        $(eval LIB_DIR    := ${BUILD_PLAT}/lib)
        $(eval ROMLIB_DIR    := ${BUILD_PLAT}/romlib)
        $(eval LIBWRAPPER_DIR := ${BUILD_PLAT}/libwrapper)
        $(eval $(call MAKE_PREREQ_DIR,${LIB_DIR},${BUILD_PLAT}))
        $(eval $(call MAKE_PREREQ_DIR,${ROMLIB_DIR},${BUILD_PLAT}))
        $(eval $(call MAKE_PREREQ_DIR,${LIBWRAPPER_DIR},${BUILD_PLAT}))
endef

# MAKE_LIB macro defines the targets and options to build each BL image.
# Arguments:
#   $(1) = Library name
define MAKE_LIB
        $(eval BUILD_DIR  := ${BUILD_PLAT}/lib$(1))
        $(eval LIB_DIR    := ${BUILD_PLAT}/lib)
        $(eval ROMLIB_DIR    := ${BUILD_PLAT}/romlib)
        $(eval SOURCES    := $(LIB$(call uppercase,$(1))_SRCS))
        $(eval OBJS       := $(addprefix $(BUILD_DIR)/,$(call SOURCES_TO_OBJS,$(SOURCES))))

$(eval $(call MAKE_PREREQ_DIR,${BUILD_DIR},${BUILD_PLAT}))
$(eval $(call MAKE_LIB_OBJS,$(BUILD_DIR),$(SOURCES),$(1)))

.PHONY : lib${1}_dirs
lib${1}_dirs: | ${BUILD_DIR} ${LIB_DIR}  ${ROMLIB_DIR} ${LIBWRAPPER_DIR}
libraries: ${LIB_DIR}/lib$(1).a
LDPATHS = -L${LIB_DIR}
LDLIBS += -l$(1)

ifeq ($(USE_ROMLIB),1)
LDLIBS := -lwrappers -lc
endif

all: ${LIB_DIR}/lib$(1).a

${LIB_DIR}/lib$(1).a: $(OBJS)
	@echo "  AR      $$@"
	$$(Q)$$(AR) cr $$@ $$?
endef

# MAKE_BL macro defines the targets and options to build each BL image.
# Arguments:
#   $(1) = BL stage (2, 2u, 30, 31, 32, 33)
#   $(2) = FIP command line option (if empty, image will not be included in the FIP)
#   $(3) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)
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
        $(eval BL_LIBS    := $(BL$(call uppercase,$(1))_LIBS))
        # We use sort only to get a list of unique object directory names.
        # ordering is not relevant but sort removes duplicates.
        $(eval TEMP_OBJ_DIRS := $(sort $(dir ${OBJS} ${LINKERFILE})))
        # The $(dir ) function leaves a trailing / on the directory names
        # Rip off the / to match directory names with make rule targets.
        $(eval OBJ_DIRS   := $(patsubst %/,%,$(TEMP_OBJ_DIRS)))

# Create generators for object directory structure

$(eval $(call MAKE_PREREQ_DIR,${BUILD_DIR},${BUILD_PLAT}))

$(eval $(foreach objd,${OBJ_DIRS},$(call MAKE_PREREQ_DIR,${objd},${BUILD_DIR})))

.PHONY : bl${1}_dirs

# We use order-only prerequisites to ensure that directories are created,
# but do not cause re-builds every time a file is written.
bl${1}_dirs: | ${OBJ_DIRS}

$(eval $(call MAKE_OBJS,$(BUILD_DIR),$(SOURCES),$(1)))
$(eval $(call MAKE_LD,$(LINKERFILE),$(BL_LINKERFILE),$(1)))

ifeq ($(USE_ROMLIB),1)
$(ELF): romlib.bin
endif

$(ELF): $(OBJS) $(LINKERFILE) | bl$(1)_dirs libraries $(BL_LIBS)
	@echo "  LD      $$@"
ifdef MAKE_BUILD_STRINGS
	$(call MAKE_BUILD_STRINGS, $(BUILD_DIR)/build_message.o)
else
	@echo 'const char build_message[] = "Built : "$(BUILD_MESSAGE_TIMESTAMP); \
	       const char version_string[] = "${VERSION_STRING}";' | \
		$$(CC) $$(TF_CFLAGS) $$(CFLAGS) -xc -c - -o $(BUILD_DIR)/build_message.o
endif
	$$(Q)$$(LD) -o $$@ $$(TF_LDFLAGS) $$(LDFLAGS) -Map=$(MAPFILE) \
		--script $(LINKERFILE) $(BUILD_DIR)/build_message.o \
		$(OBJS) $(LDPATHS) $(LDLIBS) $(BL_LIBS)

$(DUMP): $(ELF)
	@echo "  OD      $$@"
	$${Q}$${OD} -dx $$< > $$@

$(BIN): $(ELF)
	@echo "  BIN     $$@"
	$$(Q)$$(OC) -O binary $$< $$@
	@${ECHO_BLANK_LINE}
	@echo "Built $$@ successfully"
	@${ECHO_BLANK_LINE}

.PHONY: bl$(1)
bl$(1): $(BIN) $(DUMP)

all: bl$(1)

$(if $(2),$(call TOOL_ADD_IMG_PAYLOAD,bl$(1),$(BIN),--$(2),$(BIN),$(3)))

endef

# Convert device tree source file names to matching blobs
#   $(1) = input dts
define SOURCES_TO_DTBS
        $(notdir $(patsubst %.dts,%.dtb,$(filter %.dts,$(1))))
endef

# MAKE_FDT_DIRS macro creates the prerequisite directories that host the
# FDT binaries
#   $(1) = output directory
#   $(2) = input dts
define MAKE_FDT_DIRS
        $(eval DTBS       := $(addprefix $(1)/,$(call SOURCES_TO_DTBS,$(2))))
        $(eval TEMP_DTB_DIRS := $(sort $(dir ${DTBS})))
        # The $(dir ) function leaves a trailing / on the directory names
        # Rip off the / to match directory names with make rule targets.
        $(eval DTB_DIRS   := $(patsubst %/,%,$(TEMP_DTB_DIRS)))

$(eval $(foreach objd,${DTB_DIRS},$(call MAKE_PREREQ_DIR,${objd},${BUILD_DIR})))

fdt_dirs: ${DTB_DIRS}
endef

# MAKE_DTB generate the Flattened device tree binary
#   $(1) = output directory
#   $(2) = input dts
define MAKE_DTB

$(eval DOBJ := $(addprefix $(1)/,$(call SOURCES_TO_DTBS,$(2))))
$(eval DPRE := $(addprefix $(1)/,$(patsubst %.dts,%.pre.dts,$(notdir $(2)))))
$(eval DEP := $(patsubst %.dtb,%.d,$(DOBJ)))

$(DOBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | fdt_dirs
	@echo "  CPP     $$<"
	$$(Q)$$(CPP) $$(CPPFLAGS) -x assembler-with-cpp -o $(DPRE) $$<
	@echo "  DTC     $$<"
	$$(Q)$$(DTC) $$(DTC_FLAGS) -i fdts -d $(DEP) -o $$@ $(DPRE)

-include $(DEP)

endef

# MAKE_DTBS builds flattened device tree sources
#   $(1) = output directory
#   $(2) = list of flattened device tree source files
define MAKE_DTBS
        $(eval DOBJS := $(filter %.dts,$(2)))
        $(eval REMAIN := $(filter-out %.dts,$(2)))
        $(and $(REMAIN),$(error FDT_SOURCES contain non-DTS files: $(REMAIN)))
        $(eval $(foreach obj,$(DOBJS),$(call MAKE_DTB,$(1),$(obj))))

        $(eval $(call MAKE_FDT_DIRS,$(1),$(2)))

dtbs: $(DTBS)
all: dtbs
endef
