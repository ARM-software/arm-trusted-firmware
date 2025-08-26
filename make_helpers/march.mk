#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This build helper makefile is used to determine a suitable march build
# option to be used, based on platform provided ARM_ARCH_MAJOR/MINOR
# data and compiler supported march version.

# Set the compiler's target architecture profile based on
# ARM_ARCH_MAJOR and ARM_ARCH_MINOR options.


# This is used to collect available march values from compiler.
# Example on a gcc-12.2 arm64 toolchain it will look like:
# [...]
#./aarch64-none-elf-gcc --target-help -march=foo
# cc1: error: unknown value 'foo' for '-march'
# cc1: note: valid arguments are: armv8-a armv8.1-a armv8.2-a armv8.3-a armv8.4-a armv8.5-a
# armv8.6-a armv8.7-a armv8.8-a armv8-r armv9-a
# [...]
#
GCC_MARCH_OUTPUT := $(if $($(ARCH)-cc),$(shell $($(ARCH)-cc) -march=foo -Q --help=target -v 2>&1))

# This function is used to find the best march value supported by the given compiler.
# We try to use `GCC_MARCH_OUTPUT` which has verbose message with supported march values we filter that
# to find armvX.Y-a or armvX-a values, then filter the best supported arch based on ARM_ARCH_MAJOR.
#
# Example on a gcc-12.2 arm64 toolchain this will return armv9-a if platform requested for armv9.2-a
# Similarly on gcc 11.3 it would return armv8.6-a if platform requested armv8.8-a
define major_best_avail_march
$(1) := $(lastword $(filter armv$(ARM_ARCH_MAJOR)% ,$(filter armv%-a, $(GCC_MARCH_OUTPUT))))
endef

# This function is used to just return latest march value supported by the given compiler.
#
# Example: this would return armv8.6-a on a gcc-11.3 when platform requested for armv9.0-a
#
# Thus this function should be used in conjunction with major_best_avail_march, when best match
# is not found it should be ok to try with lastest known supported march value from the
# compiler.
define latest_match_march
$(1) := $(lastword $(filter armv%-a, $(GCC_MARCH_OUTPUT)))
endef

ifdef MARCH_DIRECTIVE
    march-directive		:= $(MARCH_DIRECTIVE)
else

ifeq (${ARM_ARCH_MINOR},0)
    provided-march = armv${ARM_ARCH_MAJOR}-a
else
    provided-march = armv${ARM_ARCH_MAJOR}.${ARM_ARCH_MINOR}-a
endif

ifeq ($(filter %-clang,$($(ARCH)-cc-id)),)

# We expect from Platform to provide a correct Major/Minor value but expecting something
# from compiler with unsupported march means we shouldn't fail without trying anything,
# so here we try to find best supported march value and use that for compilation.
# If we don't support current march version from gcc compiler, try with lower arch based on
# availability. In TF-A there is no hard rule for need of higher version march for basic
# functionality, denying a build on only this fact doesn't look correct, so try with best
# or latest march values from whats available from compiler.
ifeq (,$(findstring $(provided-march), $(GCC_MARCH_OUTPUT)))
    $(eval $(call major_best_avail_march, available-march))

ifeq (, $(available-march))
    $(eval $(call latest_match_march, available-march))
endif

# If we fail to come up with any available-march value so far, don't update
# provided-march and thus allow the build to fail using the provided-march
# which is derived based on arch_major and arch_minor values.
ifneq (,$(available-march))
    provided-march := ${available-march}
endif

endif # provided-march supported
endif # not clang

march-directive := -march=${provided-march}

################################################################################
# Get Architecture Feature Modifiers
################################################################################
arch-features		=	${ARM_ARCH_FEATURE}

# This section is where we place modifiers for optional arch features. If they
# are enabled with build flags, then we need to explicitly enable them in the
# compiler as well since they are not enabled by default.
ifneq ($(ENABLE_FEAT_PAUTH_LR), 0)
    # Currently, FEAT_PAUTH_LR is only supported by arm/clang compilers
    # TODO implement for GCC when support is added
    ifeq ($($(ARCH)-cc-id),arm-clang)
            arch-features	:= $(arch-features)+pauth-lr
    else
            $(error Error: ENABLE_FEAT_PAUTH_LR not supported for GCC compiler)
    endif
endif

# Set the compiler's architecture feature modifiers
ifneq ($(arch-features), none)
	# Strip "none+" from arch-features
	arch-features	:=	$(subst none+,,$(arch-features))
	march-directive	:=	$(march-directive)+$(arch-features)
# Print features
        $(info Arm Architecture Features specified: $(subst +, ,$(arch-features)))
endif #(arch-features)

endif # MARCH_DIRECTIVE
