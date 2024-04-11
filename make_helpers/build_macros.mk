#
# Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Report an error if the eval make function is not available.
$(eval eval_available := T)
ifneq (${eval_available},T)
    $(error This makefile only works with a Make program that supports $$(eval))
endif

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

# Convenience function for setting a variable to 0 if not previously set
# $(eval $(call default_zero,FOO))
define default_zero
	$(eval $(1) ?= 0)
endef

# Convenience function for setting a list of variables to 0 if not previously set
# $(eval $(call default_zeros,FOO BAR))
define default_zeros
	$(foreach var,$1,$(eval $(call default_zero,$(var))))
endef

# Convenience function for setting a variable to 1 if not previously set
# $(eval $(call default_one,FOO))
define default_one
	$(eval $(1) ?= 1)
endef

# Convenience function for setting a list of variables to 1 if not previously set
# $(eval $(call default_ones,FOO BAR))
define default_ones
	$(foreach var,$1,$(eval $(call default_one,$(var))))
endef

# Convenience function for adding build definitions
# $(eval $(call add_define,FOO)) will have:
# -DFOO if $(FOO) is empty; -DFOO=$(FOO) otherwise
define add_define
    DEFINES			+=	-D$(1)$(if $(value $(1)),=$(value $(1)),)
endef

# Convenience function for addding multiple build definitions
# $(eval $(call add_defines,FOO BOO))
define add_defines
    $(foreach def,$1,$(eval $(call add_define,$(def))))
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
    $(if $($(1)),,$(error $(1) must not be empty))
    $(if $(filter-out 0 1,$($1)),$(error $1 must be boolean))
endef

# Convenience function for verifying options have boolean values
# $(eval $(call assert_booleans,FOO BOO)) will assert FOO and BOO for 0 or 1 values
define assert_booleans
    $(foreach bool,$1,$(eval $(call assert_boolean,$(bool))))
endef

0-9 := 0 1 2 3 4 5 6 7 8 9

# Function to verify that a given option $(1) contains a numeric value
define assert_numeric
$(if $($(1)),,$(error $(1) must not be empty))
$(eval __numeric := $($(1)))
$(foreach d,$(0-9),$(eval __numeric := $(subst $(d),,$(__numeric))))
$(if $(__numeric),$(error $(1) must be numeric))
endef

# Convenience function for verifying options have numeric values
# $(eval $(call assert_numerics,FOO BOO)) will assert FOO and BOO contain numeric values
define assert_numerics
    $(foreach num,$1,$(eval $(call assert_numeric,$(num))))
endef

# Convenience function to check for a given linker option. An call to
# $(call ld_option, --no-XYZ) will return --no-XYZ if supported by the linker
ld_option = $(shell $($(ARCH)-ld) $(1) -Wl,--version >/dev/null 2>&1 || $($(ARCH)-ld) $(1) -v >/dev/null 2>&1 && echo $(1))

# Convenience function to check for a given compiler option. A call to
# $(call cc_option, --no-XYZ) will return --no-XYZ if supported by the compiler
define cc_option
	$(shell if $($(ARCH)-cc) $(1) -c -x c /dev/null -o /dev/null >/dev/null 2>&1; then echo $(1); fi )
endef

# CREATE_SEQ is a recursive function to create sequence of numbers from 1 to
# $(2) and assign the sequence to $(1)
define CREATE_SEQ
$(if $(word $(2), $($(1))),\
  $(eval $(1) += $(words $($(1))))\
  $(eval $(1) := $(filter-out 0,$($(1)))),\
  $(eval $(1) += $(words $($(1))))\
  $(call CREATE_SEQ,$(1),$(2))\
)
endef

# IMG_MAPFILE defines the output file describing the memory map corresponding
# to a BL stage
#   $(1) = BL stage
define IMG_MAPFILE
    ${BUILD_DIR}/$(1).map
endef

# IMG_ELF defines the elf file corresponding to a BL stage
#   $(1) = BL stage
define IMG_ELF
    ${BUILD_DIR}/$(1).elf
endef

# IMG_DUMP defines the symbols dump file corresponding to a BL stage
#   $(1) = BL stage
define IMG_DUMP
    ${BUILD_DIR}/$(1).dump
endef

# IMG_BIN defines the default image file corresponding to a BL stage
#   $(1) = BL stage
define IMG_BIN
    ${BUILD_PLAT}/$(1).bin
endef

# IMG_ENC_BIN defines the default encrypted image file corresponding to a
# BL stage
#   $(1) = BL stage
define IMG_ENC_BIN
    ${BUILD_PLAT}/$(1)_enc.bin
endef

# ENCRYPT_FW invokes enctool to encrypt firmware binary
#   $(1) = input firmware binary
#   $(2) = output encrypted firmware binary
define ENCRYPT_FW
$(2): $(1) enctool
	$$(ECHO) "  ENC     $$<"
	$$(Q)$$(ENCTOOL) $$(ENC_ARGS) -i $$< -o $$@
endef

# TOOL_ADD_PAYLOAD appends the command line arguments required by fiptool to
# package a new payload and/or by cert_create to generate certificate.
# Optionally, it adds the dependency on this payload
#   $(1) = payload filename (i.e. bl31.bin)
#   $(2) = command line option for the specified payload (i.e. --soc-fw)
#   $(3) = tool target dependency (optional) (ex. build/fvp/release/bl31.bin)
#   $(4) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)
#   $(5) = encrypted payload (optional) (ex. build/fvp/release/bl31_enc.bin)
define TOOL_ADD_PAYLOAD
ifneq ($(5),)
    $(4)FIP_ARGS += $(2) $(5)
    $(if $(3),$(4)CRT_DEPS += $(1))
else
    $(4)FIP_ARGS += $(2) $(1)
    $(if $(3),$(4)CRT_DEPS += $(3))
endif
    $(if $(3),$(4)FIP_DEPS += $(3))
    $(4)CRT_ARGS += $(2) $(1)
endef

# TOOL_ADD_IMG_PAYLOAD works like TOOL_ADD_PAYLOAD, but applies image filters
# before passing them to host tools if BL*_PRE_TOOL_FILTER is defined.
#   $(1) = image_type (scp_bl2, bl33, etc.)
#   $(2) = payload filepath (ex. build/fvp/release/bl31.bin)
#   $(3) = command line option for the specified payload (ex. --soc-fw)
#   $(4) = tool target dependency (optional) (ex. build/fvp/release/bl31.bin)
#   $(5) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)
#   $(6) = encrypted payload (optional) (ex. build/fvp/release/bl31_enc.bin)

define TOOL_ADD_IMG_PAYLOAD

$(eval PRE_TOOL_FILTER := $($(call uppercase,$(1))_PRE_TOOL_FILTER))

ifneq ($(PRE_TOOL_FILTER),)

$(eval PROCESSED_PATH := $(BUILD_PLAT)/$(1).bin$($(PRE_TOOL_FILTER)_SUFFIX))

$(call $(PRE_TOOL_FILTER)_RULE,$(PROCESSED_PATH),$(2))

$(PROCESSED_PATH): $(4)

$(call TOOL_ADD_PAYLOAD,$(PROCESSED_PATH),$(3),$(PROCESSED_PATH),$(5),$(6))

else
$(call TOOL_ADD_PAYLOAD,$(2),$(3),$(4),$(5),$(6))
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
#   $(4) = Image encryption flag (optional) (0, 1)
# Example:
#   $(eval $(call TOOL_ADD_IMG,bl33,--nt-fw))
define TOOL_ADD_IMG
    # Build option to specify the image filename (SCP_BL2, BL33, etc)
    # This is the uppercase form of the first parameter
    $(eval _V := $(call uppercase,$(1)))

    # $(check_$(1)_cmd) variable is executed in the check_$(1) target and also
    # is put into the ${CHECK_$(3)FIP_CMD} variable which is executed by the
    # target ${BUILD_PLAT}/${$(3)FIP_NAME}.
    $(eval check_$(1)_cmd := \
        $(if $(value $(_V)),,$$$$(error "Platform '${PLAT}' requires $(_V). Please set $(_V) to point to the right file")) \
        $(if $(wildcard $(value $(_V))),,$$$$(error '$(_V)=$(value $(_V))' was specified, but '$(value $(_V))' does not exist)) \
    )

    $(3)CRT_DEPS += check_$(1)
    CHECK_$(3)FIP_CMD += $$(check_$(1)_cmd)
ifeq ($(4),1)
    $(eval ENC_BIN := ${BUILD_PLAT}/$(1)_enc.bin)
    $(call ENCRYPT_FW,$(value $(_V)),$(ENC_BIN))
    $(call TOOL_ADD_IMG_PAYLOAD,$(1),$(value $(_V)),$(2),$(ENC_BIN),$(3), \
		$(ENC_BIN))
else
    $(call TOOL_ADD_IMG_PAYLOAD,$(1),$(value $(_V)),$(2),$(if $(wildcard $(value $(_V))),$(value $(_V)),FORCE),$(3))
endif

.PHONY: check_$(1)
check_$(1):
	$(check_$(1)_cmd)
endef

# SELECT_OPENSSL_API_VERSION selects the OpenSSL API version to be used to
# build the host tools by checking the version of OpenSSL located under
# the path defined by the OPENSSL_DIR variable. It receives no parameters.
define SELECT_OPENSSL_API_VERSION
    # Set default value for USING_OPENSSL3 macro to 0
    $(eval USING_OPENSSL3 = 0)
    # Obtain the OpenSSL version for the build located under OPENSSL_DIR
    $(eval OPENSSL_INFO := $(shell LD_LIBRARY_PATH=${OPENSSL_DIR}:${OPENSSL_DIR}/lib ${OPENSSL_BIN_PATH}/openssl version))
    $(eval OPENSSL_CURRENT_VER = $(word 2, ${OPENSSL_INFO}))
    $(eval OPENSSL_CURRENT_VER_MAJOR = $(firstword $(subst ., ,$(OPENSSL_CURRENT_VER))))
    # If OpenSSL version is 3.x, then set USING_OPENSSL3 flag to 1
    $(if $(filter 3,$(OPENSSL_CURRENT_VER_MAJOR)), $(eval USING_OPENSSL3 = 1))
endef

################################################################################
# Generic image processing filters
################################################################################

# GZIP
define GZIP_RULE
$(1): $(2)
	$(ECHO) "  GZIP    $$@"
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
$(eval LIB := $(call uppercase, $(notdir $(1))))

$(OBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | lib$(3)_dirs
	$$(ECHO) "  CC      $$<"
	$$(Q)$($(ARCH)-cc) $$($(LIB)_CFLAGS) $$(TF_CFLAGS) $$(CFLAGS) $(MAKE_DEP) -c $$< -o $$@

-include $(DEP)

endef

# MAKE_S_LIB builds an assembly source file and generates the dependency file
#   $(1) = output directory
#   $(2) = source file (%.S)
#   $(3) = library name
define MAKE_S_LIB
$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))

$(OBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | lib$(3)_dirs
	$$(ECHO) "  AS      $$<"
	$$(Q)$($(ARCH)-as) -x assembler-with-cpp $$(TF_CFLAGS_$(ARCH)) $$(ASFLAGS) $(MAKE_DEP) -c $$< -o $$@

-include $(DEP)

endef


# MAKE_C builds a C source file and generates the dependency file
#   $(1) = output directory
#   $(2) = source file (%.c)
#   $(3) = BL stage
define MAKE_C

$(eval OBJ := $(1)/$(patsubst %.c,%.o,$(notdir $(2))))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))

$(eval BL_DEFINES := IMAGE_$(call uppercase,$(3)) $($(call uppercase,$(3))_DEFINES) $(PLAT_BL_COMMON_DEFINES))
$(eval BL_INCLUDE_DIRS := $($(call uppercase,$(3))_INCLUDE_DIRS) $(PLAT_BL_COMMON_INCLUDE_DIRS))
$(eval BL_CPPFLAGS := $($(call uppercase,$(3))_CPPFLAGS) $(addprefix -D,$(BL_DEFINES)) $(addprefix -I,$(BL_INCLUDE_DIRS)) $(PLAT_BL_COMMON_CPPFLAGS))
$(eval BL_CFLAGS := $($(call uppercase,$(3))_CFLAGS) $(PLAT_BL_COMMON_CFLAGS))

$(OBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | $(3)_dirs
	$$(ECHO) "  CC      $$<"
	$$(Q)$($(ARCH)-cc) $$(LTO_CFLAGS) $$(TF_CFLAGS) $$(CFLAGS) $(BL_CPPFLAGS) $(BL_CFLAGS) $(MAKE_DEP) -c $$< -o $$@

-include $(DEP)

endef


# MAKE_S builds an assembly source file and generates the dependency file
#   $(1) = output directory
#   $(2) = assembly file (%.S)
#   $(3) = BL stage
define MAKE_S

$(eval OBJ := $(1)/$(patsubst %.S,%.o,$(notdir $(2))))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))

$(eval BL_DEFINES := IMAGE_$(call uppercase,$(3)) $($(call uppercase,$(3))_DEFINES) $(PLAT_BL_COMMON_DEFINES))
$(eval BL_INCLUDE_DIRS := $($(call uppercase,$(3))_INCLUDE_DIRS) $(PLAT_BL_COMMON_INCLUDE_DIRS))
$(eval BL_CPPFLAGS := $($(call uppercase,$(3))_CPPFLAGS) $(addprefix -D,$(BL_DEFINES)) $(addprefix -I,$(BL_INCLUDE_DIRS)) $(PLAT_BL_COMMON_CPPFLAGS))
$(eval BL_ASFLAGS := $($(call uppercase,$(3))_ASFLAGS) $(PLAT_BL_COMMON_ASFLAGS))

$(OBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | $(3)_dirs
	$$(ECHO) "  AS      $$<"
	$$(Q)$($(ARCH)-as) -x assembler-with-cpp $$(TF_CFLAGS_$(ARCH)) $$(ASFLAGS) $(BL_CPPFLAGS) $(BL_ASFLAGS) $(MAKE_DEP) -c $$< -o $$@

-include $(DEP)

endef


# MAKE_LD generate the linker script using the C preprocessor
#   $(1) = output linker script
#   $(2) = input template
#   $(3) = BL stage
define MAKE_LD

$(eval DEP := $(1).d)

$(eval BL_DEFINES := IMAGE_$(call uppercase,$(3)) $($(call uppercase,$(3))_DEFINES) $(PLAT_BL_COMMON_DEFINES))
$(eval BL_INCLUDE_DIRS := $($(call uppercase,$(3))_INCLUDE_DIRS) $(PLAT_BL_COMMON_INCLUDE_DIRS))
$(eval BL_CPPFLAGS := $($(call uppercase,$(3))_CPPFLAGS) $(addprefix -D,$(BL_DEFINES)) $(addprefix -I,$(BL_INCLUDE_DIRS)) $(PLAT_BL_COMMON_CPPFLAGS))

$(1): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | $(3)_dirs
	$$(ECHO) "  PP      $$<"
	$$(Q)$($(ARCH)-cpp) -E $$(CPPFLAGS) $(BL_CPPFLAGS) $(TF_CFLAGS_$(ARCH)) -P -x assembler-with-cpp -D__LINKER__ $(MAKE_DEP) -o $$@ $$<

-include $(DEP)

endef

# MAKE_LIB_OBJS builds both C and assembly source files
#   $(1) = output directory
#   $(2) = list of source files
#   $(3) = name of the library
define MAKE_LIB_OBJS
        $(eval C_OBJS := $(filter %.c,$(2)))
        $(eval REMAIN := $(filter-out %.c,$(2)))
        $(eval $(foreach obj,$(C_OBJS),$(call MAKE_C_LIB,$(1),$(obj),$(3))))

        $(eval S_OBJS := $(filter %.S,$(REMAIN)))
        $(eval REMAIN := $(filter-out %.S,$(REMAIN)))
        $(eval $(foreach obj,$(S_OBJS),$(call MAKE_S_LIB,$(1),$(obj),$(3))))

        $(and $(REMAIN),$(error Unexpected source files present: $(REMAIN)))
endef


# MAKE_OBJS builds both C and assembly source files
#   $(1) = output directory
#   $(2) = list of source files (both C and assembly)
#   $(3) = BL stage
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
ifeq ($($(ARCH)-ld-id),arm-link)
LDPATHS = --userlibpath=${LIB_DIR}
LDLIBS += --library=$(1)
else
LDPATHS = -L${LIB_DIR}
LDLIBS += -l$(1)
endif

ifeq ($(USE_ROMLIB),1)
LIBWRAPPER = -lwrappers
endif

all: ${LIB_DIR}/lib$(1).a

${LIB_DIR}/lib$(1).a: $(OBJS)
	$$(ECHO) "  AR      $$@"
	$$(Q)$($(ARCH)-ar) cr $$@ $$?
endef

# Generate the path to one or more preprocessed linker scripts given the paths
# of their sources.
#
# Arguments:
#   $(1) = path to one or more linker script sources
define linker_script_path
        $(patsubst %.S,$(BUILD_DIR)/%,$(1))
endef

# MAKE_BL macro defines the targets and options to build each BL image.
# Arguments:
#   $(1) = BL stage
#   $(2) = FIP command line option (if empty, image will not be included in the FIP)
#   $(3) = FIP prefix (optional) (if FWU_, target is fwu_fip instead of fip)
#   $(4) = BL encryption flag (optional) (0, 1)
define MAKE_BL
        $(eval BUILD_DIR  := ${BUILD_PLAT}/$(1))
        $(eval BL_SOURCES := $($(call uppercase,$(1))_SOURCES))
        $(eval SOURCES    := $(sort $(BL_SOURCES) $(BL_COMMON_SOURCES) $(PLAT_BL_COMMON_SOURCES)))
        $(eval OBJS       := $(addprefix $(BUILD_DIR)/,$(call SOURCES_TO_OBJS,$(SOURCES))))
        $(eval MAPFILE    := $(call IMG_MAPFILE,$(1)))
        $(eval ELF        := $(call IMG_ELF,$(1)))
        $(eval DUMP       := $(call IMG_DUMP,$(1)))
        $(eval BIN        := $(call IMG_BIN,$(1)))
        $(eval ENC_BIN    := $(call IMG_ENC_BIN,$(1)))
        $(eval BL_LIBS    := $($(call uppercase,$(1))_LIBS))

        $(eval DEFAULT_LINKER_SCRIPT_SOURCE := $($(call uppercase,$(1))_DEFAULT_LINKER_SCRIPT_SOURCE))
        $(eval DEFAULT_LINKER_SCRIPT := $(call linker_script_path,$(DEFAULT_LINKER_SCRIPT_SOURCE)))

        $(eval LINKER_SCRIPT_SOURCES := $($(call uppercase,$(1))_LINKER_SCRIPT_SOURCES))
        $(eval LINKER_SCRIPTS := $(call linker_script_path,$(LINKER_SCRIPT_SOURCES)))

        # We use sort only to get a list of unique object directory names.
        # ordering is not relevant but sort removes duplicates.
        $(eval TEMP_OBJ_DIRS := $(sort $(dir ${OBJS} ${DEFAULT_LINKER_SCRIPT} ${LINKER_SCRIPTS})))
        # The $(dir ) function leaves a trailing / on the directory names
        # Rip off the / to match directory names with make rule targets.
        $(eval OBJ_DIRS   := $(patsubst %/,%,$(TEMP_OBJ_DIRS)))

# Create generators for object directory structure

$(eval $(call MAKE_PREREQ_DIR,${BUILD_DIR},${BUILD_PLAT}))

$(eval $(foreach objd,${OBJ_DIRS},
        $(call MAKE_PREREQ_DIR,${objd},${BUILD_DIR})))

.PHONY : ${1}_dirs

# We use order-only prerequisites to ensure that directories are created,
# but do not cause re-builds every time a file is written.
${1}_dirs: | ${OBJ_DIRS}

$(eval $(call MAKE_OBJS,$(BUILD_DIR),$(SOURCES),$(1)))

# Generate targets to preprocess each required linker script
$(eval $(foreach source,$(DEFAULT_LINKER_SCRIPT_SOURCE) $(LINKER_SCRIPT_SOURCES), \
        $(call MAKE_LD,$(call linker_script_path,$(source)),$(source),$(1))))

$(eval BL_LDFLAGS := $($(call uppercase,$(1))_LDFLAGS))

ifeq ($(USE_ROMLIB),1)
$(ELF): romlib.bin
endif

# MODULE_OBJS can be assigned by vendors with different compiled
# object file path, and prebuilt object file path.
$(eval OBJS += $(MODULE_OBJS))

$(ELF): $(OBJS) $(DEFAULT_LINKER_SCRIPT) $(LINKER_SCRIPTS) | $(1)_dirs libraries $(BL_LIBS)
	$$(ECHO) "  LD      $$@"
ifdef MAKE_BUILD_STRINGS
	$(call MAKE_BUILD_STRINGS,$(BUILD_DIR)/build_message.o)
else
	@echo 'const char build_message[] = "Built : "$(BUILD_MESSAGE_TIMESTAMP); \
	       const char version_string[] = "${VERSION_STRING}"; \
	       const char version[] = "${VERSION}";' | \
		$($(ARCH)-cc) $$(TF_CFLAGS) $$(CFLAGS) -xc -c - -o $(BUILD_DIR)/build_message.o
endif
ifeq ($($(ARCH)-ld-id),arm-link)
	$$(Q)$($(ARCH)-ld) -o $$@ $$(TF_LDFLAGS) $$(LDFLAGS) $(BL_LDFLAGS) --entry=${1}_entrypoint \
		--predefine="-D__LINKER__=$(__LINKER__)" \
		--predefine="-DTF_CFLAGS=$(TF_CFLAGS)" \
		--map --list="$(MAPFILE)" --scatter=${PLAT_DIR}/scat/${1}.scat \
		$(LDPATHS) $(LIBWRAPPER) $(LDLIBS) $(BL_LIBS) \
		$(BUILD_DIR)/build_message.o $(OBJS)
else ifeq ($($(ARCH)-ld-id),gnu-gcc)
	$$(Q)$($(ARCH)-ld) -o $$@ $$(TF_LDFLAGS) $$(LDFLAGS) $(BL_LDFLAGS) -Wl,-Map=$(MAPFILE) \
		$(addprefix -Wl$(comma)--script$(comma),$(LINKER_SCRIPTS)) -Wl,--script,$(DEFAULT_LINKER_SCRIPT) \
		$(BUILD_DIR)/build_message.o \
		$(OBJS) $(LDPATHS) $(LIBWRAPPER) $(LDLIBS) $(BL_LIBS)
else
	$$(Q)$($(ARCH)-ld) -o $$@ $$(TF_LDFLAGS) $$(LDFLAGS) $(BL_LDFLAGS) -Map=$(MAPFILE) \
		$(addprefix -T ,$(LINKER_SCRIPTS)) --script $(DEFAULT_LINKER_SCRIPT) \
		$(BUILD_DIR)/build_message.o \
		$(OBJS) $(LDPATHS) $(LIBWRAPPER) $(LDLIBS) $(BL_LIBS)
endif
ifeq ($(DISABLE_BIN_GENERATION),1)
	@${ECHO_BLANK_LINE}
	@echo "Built $$@ successfully"
	@${ECHO_BLANK_LINE}
endif

$(DUMP): $(ELF)
	$${ECHO} "  OD      $$@"
	$${Q}$($(ARCH)-od) -dx $$< > $$@

$(BIN): $(ELF)
	$${ECHO} "  BIN     $$@"
	$$(Q)$($(ARCH)-oc) -O binary $$< $$@
	@${ECHO_BLANK_LINE}
	@echo "Built $$@ successfully"
	@${ECHO_BLANK_LINE}

.PHONY: $(1)
ifeq ($(DISABLE_BIN_GENERATION),1)
$(1): $(ELF) $(DUMP)
else
$(1): $(BIN) $(DUMP)
endif

all: $(1)

ifeq ($(4),1)
$(call ENCRYPT_FW,$(BIN),$(ENC_BIN))
$(if $(2),$(call TOOL_ADD_IMG_PAYLOAD,$(1),$(BIN),--$(2),$(ENC_BIN),$(3), \
		$(ENC_BIN)))
else
$(if $(2),$(call TOOL_ADD_IMG_PAYLOAD,$(1),$(BIN),--$(2),$(BIN),$(3)))
endif

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

# List of DTB file(s) to generate, based on DTS file basename list
$(eval DOBJ := $(addprefix $(1)/,$(call SOURCES_TO_DTBS,$(2))))
# List of the pre-compiled DTS file(s)
$(eval DPRE := $(addprefix $(1)/,$(patsubst %.dts,%.pre.dts,$(notdir $(2)))))
# Dependencies of the pre-compiled DTS file(s) on its source and included files
$(eval DTSDEP := $(patsubst %.dtb,%.o.d,$(DOBJ)))
# Dependencies of the DT compilation on its pre-compiled DTS
$(eval DTBDEP := $(patsubst %.dtb,%.d,$(DOBJ)))

$(DOBJ): $(2) $(filter-out %.d,$(MAKEFILE_LIST)) | fdt_dirs
	$${ECHO} "  CPP     $$<"
	$(eval DTBS       := $(addprefix $(1)/,$(call SOURCES_TO_DTBS,$(2))))
	$$(Q)$($(ARCH)-cpp) -E $$(TF_CFLAGS_$(ARCH)) $$(DTC_CPPFLAGS) -MT $(DTBS) -MMD -MF $(DTSDEP) -o $(DPRE) $$<
	$${ECHO} "  DTC     $$<"
	$$(Q)$($(ARCH)-dtc) $$(DTC_FLAGS) -d $(DTBDEP) -o $$@ $(DPRE)

-include $(DTBDEP)
-include $(DTSDEP)

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
