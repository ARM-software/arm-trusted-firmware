#
# Copyright (c) 2025-2026, Advanced Micro Devices, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Custom Package Integration for AMD Platforms
#

# Flag to track if custom package is included
CUSTOM_PKG_INCLUDED := 0

# ============================================================================
# Custom Package Integration (supports multiple packages)
# ============================================================================
# This framework:
# 1. Includes each package's custom_pkg.mk for build rules and flags
# 2. Discovers custom_pkg.ld.S files for linker script consolidation
# 3. Prepares them for preprocessing into the final plat.ld.S
#
# Each package provides:
# - custom_pkg.mk: Build configuration, source files, and package-specific macros
# - custom_pkg.ld.S: Memory layout (MEMORY regions and SECTIONS)
# ============================================================================

ifdef CUSTOM_PKG_PATH
    # Split CUSTOM_PKG_PATH by space, colon, or semicolon to support multiple paths
    _CUSTOM_PKG_PATHS := $(strip $(subst ;, ,$(subst :, ,$(CUSTOM_PKG_PATH))))
    # Normalize to absolute paths to handle both relative and absolute inputs
    _CUSTOM_PKG_PATHS_ABS := $(foreach pkg_path,$(_CUSTOM_PKG_PATHS),$(abspath $(pkg_path)))

    # Include each package's custom_pkg.mk
    $(foreach pkg_path,$(_CUSTOM_PKG_PATHS_ABS), \
        $(eval PKG_PATH_CLEAN := $(strip $(pkg_path))) \
        $(eval PKG_MK := $(PKG_PATH_CLEAN)/custom_pkg.mk) \
        $(if $(wildcard $(PKG_MK)), \
            $(eval include $(PKG_MK)) \
            $(eval CUSTOM_PKG_INCLUDED := 1) \
            $(info Including custom package from: $(PKG_PATH_CLEAN)), \
            $(error CUSTOM_PKG_PATH contains $(PKG_PATH_CLEAN) but custom_pkg.mk not found) \
        ) \
    )

    # Find all custom_pkg.ld.S files in package directories (absolute paths)
    CUSTOM_PKG_LD_SCRIPTS := $(wildcard $(addsuffix /custom_pkg.ld.S,$(_CUSTOM_PKG_PATHS_ABS)))

    # Auto-discover package linker script (prefer first hit) and set TF-A flag
    $(foreach pkg_path,$(_CUSTOM_PKG_PATHS_ABS), \
        $(eval _PKG_LS := $(wildcard $(pkg_path)/plat.ld.S $(pkg_path)/inc/plat.ld.S)) \
        $(if $(_PKG_LS), \
            $(if $(PLAT_EXTRA_LD_SCRIPT_PATH),, \
                $(eval PLAT_EXTRA_LD_SCRIPT_PATH := $(word 1,$(_PKG_LS))) \
                $(info Auto-discovered linker script: $(PLAT_EXTRA_LD_SCRIPT_PATH)) \
            ) \
        ) \
    )

    # If a package linker script is present, export the TF-A define (value = 1)
    ifdef PLAT_EXTRA_LD_SCRIPT_PATH
        PLAT_EXTRA_LD_SCRIPT := 1
        $(eval $(call add_define_val,PLAT_EXTRA_LD_SCRIPT,1))
        $(info Enabling PLAT_EXTRA_LD_SCRIPT=1 (script: $(PLAT_EXTRA_LD_SCRIPT_PATH)))
    endif

    ifneq ($(CUSTOM_PKG_LD_SCRIPTS),)
        $(info Custom Package Linker Scripts Found:)
        $(foreach _LS,$(CUSTOM_PKG_LD_SCRIPTS),$(info  - $(_LS)))

        # Generate paths for preprocessed scripts (.ld.S -> .ld.pp)
        CUSTOM_PKG_LD_SCRIPTS_PP := $(patsubst %.ld.S,%.ld.pp,$(CUSTOM_PKG_LD_SCRIPTS))

        # Collect include directories from all custom packages for linker script preprocessing
        CUSTOM_PKG_LD_INCLUDES := $(foreach pkg_path,$(_CUSTOM_PKG_PATHS_ABS), \
            $(if $(wildcard $(pkg_path)/inc),-I$(pkg_path)/inc) \
            $(if $(wildcard $(pkg_path)/include),-I$(pkg_path)/include))

        # Make them available to linker script generation
        export CUSTOM_PKG_LD_SCRIPTS_PP
        export CUSTOM_PKG_LD_INCLUDES
    else
        $(info No custom_pkg.ld.S files found - using platform defaults)
        CUSTOM_PKG_LD_INCLUDES :=
    endif
endif

# ============================================================================
# Note: Package-specific macros are defined in each package's custom_pkg.mk
# and passed to preprocessor via ASFLAGS
# ============================================================================

# ============================================================================
# Custom Package Linker Script Preprocessing
# ============================================================================
# This handles preprocessing of custom package linker scripts
# and generation of the final consolidated plat.ld.S
#
# Two-stage preprocessing using TF-A's standard C preprocessor:
# Stage 1: Preprocess custom_pkg.ld.S files
#   Input: custom_pkg.ld.S (contains package-specific macros)
#   Process: $($(ARCH)-cpp) -E -P with ASFLAGS containing package macro definitions
#   Output: custom_pkg.ld.pp (macros expanded)
#
# Stage 2: Preprocess template with all preprocessed scripts included
#   Input: plat.ld.S.tpl (contains #include CUSTOM_PKG_LD_SCRIPTS_PP)
#   Process: $($(ARCH)-cpp) -E -P with all macro definitions
#   Output: build/versal2/release/bl31/plat.ld.S (final linker script)
# ============================================================================

# Directories
BUILD_DIR := $(shell pwd)/build/$(PLAT)
BL31_BUILD_DIR := $(BUILD_DIR)/bl31

# Linker script template and output
PLAT_LD_TEMPLATE := plat/amd/versal2/plat.ld.S.tpl
PLAT_LD_SCRIPT := $(BL31_BUILD_DIR)/plat.ld.S

# Rule 1: Preprocess individual custom_pkg.ld.S files
# Uses TF-A's standard architecture-specific C preprocessor
# Includes custom package include directories for proper header resolution
%.ld.pp: %.ld.S
	@echo "  PP      $<"
	$(q)$($(ARCH)-cpp) -E -P $(CUSTOM_PKG_LD_INCLUDES) $(ASFLAGS) -o $@ $<

# Rule 2: Generate final plat.ld.S from template + preprocessed scripts
# Uses TF-A's standard architecture-specific C preprocessor
# Includes custom package include directories for proper header resolution
$(PLAT_LD_SCRIPT): $(PLAT_LD_TEMPLATE) $(CUSTOM_PKG_LD_SCRIPTS_PP)
	@echo "  GEN     $@"
	@mkdir -p $(BL31_BUILD_DIR)
	$(q)$($(ARCH)-cpp) -E -P \
		-DCUSTOM_PKG_LD_SCRIPTS_PP="$(CUSTOM_PKG_LD_SCRIPTS_PP)" \
		$(CUSTOM_PKG_LD_INCLUDES) \
		$(ASFLAGS) \
		-o $@ \
		$(PLAT_LD_TEMPLATE)

# Ensure final linker script exists before linking BL31
bl31: $(PLAT_LD_SCRIPT)

# ============================================================================
# Build Summary
# ============================================================================
$(info ============================================================)
$(info Custom Package Configuration:)
$(info   Custom Package: $(if $(filter 1,$(CUSTOM_PKG_INCLUDED)),ENABLED,DISABLED))
$(if $(filter 1,$(CUSTOM_PKG_INCLUDED)), \
    $(foreach pkg_path,$(_CUSTOM_PKG_PATHS_ABS), \
        $(info   - $(strip $(pkg_path))) \
    ) \
)
$(info ============================================================)
