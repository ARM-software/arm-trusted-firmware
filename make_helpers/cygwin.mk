#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

# OS specific definitions for builds in a Cygwin environment.
# Cygwin allows us to use unix style commands on a windows platform.

ifndef CYGWIN_MK
    CYGWIN_MK := $(lastword $(MAKEFILE_LIST))

    include ${MAKE_HELPERS_DIRECTORY}unix.mk

    # In cygwin executable files have the Windows .exe extension type.
    BIN_EXT := .exe

endif
