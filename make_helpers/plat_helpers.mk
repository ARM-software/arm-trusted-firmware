#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

################################################################################
# Helpers for finding and referencing platform directories
################################################################################

ifndef PLAT_HELPERS_MK
    PLAT_HELPERS_MK := $(lastword $(MAKEFILE_LIST))

    ifeq (${PLAT},)
        $(error "Error: Unknown platform. Please use PLAT=<platform name> to specify the platform")
    endif

    # PLATFORM_ROOT can be overridden for when building tools directly
    PLATFORM_ROOT               ?= plat/
    PLAT_MAKEFILE               := platform.mk

    # Generate the platforms list by recursively searching for all directories
    # under /plat containing a PLAT_MAKEFILE. Append each platform with a `|`
    # char and strip out the final '|'.
    ALL_PLATFORM_MK_FILES       := $(call rwildcard,${PLATFORM_ROOT},${PLAT_MAKEFILE})
    ALL_PLATFORM_DIRS           := $(patsubst %/,%,$(dir ${ALL_PLATFORM_MK_FILES}))
    ALL_PLATFORMS               := $(sort $(notdir ${ALL_PLATFORM_DIRS}))

    PLAT_MAKEFILE_FULL          := $(filter %/${PLAT}/${PLAT_MAKEFILE},${ALL_PLATFORM_MK_FILES})
    PLATFORM_LIST               := $(subst ${space},|,${ALL_PLATFORMS})
    ifeq ($(PLAT_MAKEFILE_FULL),)
        $(error "Error: Invalid platform. The following platforms are available: ${PLATFORM_LIST}")
    endif

    # Record the directory where the platform make file was found.
    PLAT_DIR                    := $(dir ${PLAT_MAKEFILE_FULL})

endif
