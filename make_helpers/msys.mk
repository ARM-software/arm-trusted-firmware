#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

# OS specific definitions for builds in a Mingw32 MSYS environment.
# Mingw32 allows us to use some unix style commands on a windows platform.

ifndef MSYS_MK
    MSYS_MK := $(lastword $(MAKEFILE_LIST))

    include ${MAKE_HELPERS_DIRECTORY}unix.mk

    # In MSYS executable files have the Windows .exe extension type.
    BIN_EXT := .exe

endif

