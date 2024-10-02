#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

################################################################################
# Helpers for finding and referencing platform directories
################################################################################

ifndef PLAT_HELPERS_MK
    PLAT_HELPERS_MK := $(lastword $(MAKEFILE_LIST))

    PLAT:= ${DEFAULT_PLAT}
    ifeq (${PLAT},)
        $(error "Error: Unknown platform. Please use PLAT=<platform name> to specify the platform")
    endif

    # TF_PLATFORM_ROOT can be overridden for when building tools directly
    TF_PLATFORM_ROOT               ?= plat/
    PLAT_MAKEFILE               := platform.mk
    PLAT_DEFAULTS_MAKEFILE      := platform_defaults.mk

    # Generate the platforms list by recursively searching for all directories
    # under /plat containing a PLAT_MAKEFILE. Append each platform with a `|`
    # char and strip out the final '|'.
    ALL_PLATFORM_MK_FILES       := $(call rwildcard,${TF_PLATFORM_ROOT},${PLAT_MAKEFILE})
    ALL_PLATFORM_MK_DEF_FILES   := $(call rwildcard,${TF_PLATFORM_ROOT},${PLAT_DEFAULTS_MAKEFILE})
    ALL_PLATFORM_DIRS           := $(patsubst %/,%,$(dir ${ALL_PLATFORM_MK_FILES}))
    ALL_PLATFORMS               := $(sort $(notdir ${ALL_PLATFORM_DIRS}))

    PLAT_MAKEFILE_FULL          := $(filter %/${PLAT}/${PLAT_MAKEFILE},${ALL_PLATFORM_MK_FILES})
    PLAT_DEFAULTS_MAKEFILE_FULL := $(filter %/${PLAT}/${PLAT_DEFAULTS_MAKEFILE},${ALL_PLATFORM_MK_DEF_FILES})
    PLATFORM_LIST               := $(subst ${space},|,${ALL_PLATFORMS})
    ifeq ($(PLAT_MAKEFILE_FULL),)
        $(error "Error: Invalid platform. The following platforms are available: ${PLATFORM_LIST}")
    endif

    # Record the directory where the platform make file was found.
    PLAT_DIR                    := $(dir ${PLAT_MAKEFILE_FULL})

endif
