#
# Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Trusted Firmware shell command definitions for a Unix style environment.

ifndef UNIX_MK
    UNIX_MK := $(lastword $(MAKEFILE_LIST))

    DIR_DELIM := /
    PATH_SEP := :

    # These defines provide Unix style equivalents of the shell commands
    # required by the Trusted Firmware build environment.

    # ${1} is the file to be copied.
    # ${2} is the destination file name.
    define SHELL_COPY
	$(q)cp -f  "${1}"  "${2}"
    endef

    # ${1} is the directory to be copied.
    # ${2} is the destination directory path.
    define SHELL_COPY_TREE
	$(q)cp -rf  "${1}"  "${2}"
    endef

    # ${1} is the file to be deleted.
    define SHELL_DELETE
	-$(q)rm -f  "${1}"
    endef

    # ${1} is a space delimited list of files to be deleted.
    # Note that we do not quote ${1}, as multiple parameters may be passed.
    define SHELL_DELETE_ALL
	-$(q)rm -rf  ${1}
    endef

    define SHELL_REMOVE_DIR
	-$(q)rm -rf  "${1}"
    endef

    nul := /dev/null

    which = $(shell command -v $(call escape-shell,$(1)) 2>$(nul))
endif
