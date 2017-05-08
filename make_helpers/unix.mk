#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

# Trusted Firmware shell command definitions for a Unix style environment.

ifndef UNIX_MK
    UNIX_MK := $(lastword $(MAKEFILE_LIST))

    ECHO_BLANK_LINE := echo

    DIR_DELIM := /
    PATH_SEP := :

    # These defines provide Unix style equivalents of the shell commands
    # required by the Trusted Firmware build environment.

    # ${1} is the file to be copied.
    # ${2} is the destination file name.
    define SHELL_COPY
	${Q}cp -f  "${1}"  "${2}"
    endef

    # ${1} is the directory to be copied.
    # ${2} is the destination directory path.
    define SHELL_COPY_TREE
	${Q}cp -rf  "${1}"  "${2}"
    endef

    # ${1} is the file to be deleted.
    define SHELL_DELETE
	-${Q}rm -f  "${1}"
    endef

    # ${1} is a space delimited list of files to be deleted.
    # Note that we do not quote ${1}, as multiple parameters may be passed.
    define SHELL_DELETE_ALL
	-${Q}rm -rf  ${1}
    endef

    # ${1} is the directory to be generated.
    # ${2} is optional, and allows a prerequisite to be specified.
    # Do nothing if $1 == $2, to ignore self dependencies.
    define MAKE_PREREQ_DIR
        ifneq (${1},${2})

${1} : ${2}
	${Q}mkdir -p  "${1}"

        endif
    endef

    define SHELL_REMOVE_DIR
	-${Q}rm -rf  "${1}"
    endef

endif
