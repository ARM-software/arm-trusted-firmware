#
# Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# OS specific parts for builds in a Windows_NT environment. The
# environment variable OS is set to Windows_NT on all modern Windows platforms

# Include generic windows command definitions.

ifndef WINDOWS_MK
    WINDOWS_MK := $(lastword $(MAKEFILE_LIST))

    ECHO_BLANK_LINE := @cmd /c echo.
    ECHO_QUIET := @rem
    DIR_DELIM := $(strip \)
    BIN_EXT   := .exe
    PATH_SEP  := ;

    # For some Windows native commands there is a problem with the directory delimiter.
    # Make uses / (slash) and the commands expect \ (backslash)
    # We have to provide a means of translating these, so we define local functions.

    # ${1} is the file to be copied.
    # ${2} is the destination file name.
    define SHELL_COPY
	$(eval tmp_from_file:=$(subst /,\,${1}))
	$(eval tmp_to_file:=$(subst /,\,${2}))
	copy "${tmp_from_file}" "${tmp_to_file}"
    endef

    # ${1} is the directory to be copied.
    # ${2} is the destination directory path.
    define SHELL_COPY_TREE
	$(eval tmp_from_dir:=$(subst /,\,${1}))
	$(eval tmp_to_dir:=$(subst /,\,${2}))
	xcopy /HIVE "${tmp_from_dir}" "${tmp_to_dir}"
    endef

    # ${1} is the file to be deleted.
    define SHELL_DELETE
	$(eval tmp_del_file:=$(subst /,\,${*}))
	-@if exist $(tmp_del_file)  del /Q $(tmp_del_file)
    endef

    # ${1} is a space delimited list of files to be deleted.
    define SHELL_DELETE_ALL
	$(eval $(foreach filename,$(wildcard ${1}),$(call DELETE_IF_THERE,${filename})))
    endef

    # ${1} is the directory to be generated.
    # ${2} is optional, and allows prerequisites to be specified.
    # Do nothing if $1 == $2, to ignore self dependencies.
    define MAKE_PREREQ_DIR
        ifneq (${1},${2})

${1} : ${2}
	$(eval tmp_dir:=$(subst /,\,${1}))
	-@if not exist "$(tmp_dir)"  mkdir "${tmp_dir}"

        endif
    endef

    # ${1} is the directory to be removed.
    define SHELL_REMOVE_DIR
	$(eval tmp_dir:=$(subst /,\,${1}))
	-@if exist "$(tmp_dir)"  rd /Q /S "$(tmp_dir)"
    endef

endif

# Because git is not available from CMD.EXE, we need to avoid
# the BUILD_STRING generation which uses git.
# For now we use "development build".
# This can be overridden from the command line or environment.
BUILD_STRING ?= development build

# The DOS echo shell command does not strip ' characters from the command
# parameters before printing. We therefore use an alternative method invoked
# by defining the MAKE_BUILD_STRINGS macro.
BUILT_TIME_DATE_STRING = const char build_message[] = "Built : "${BUILD_MESSAGE_TIMESTAMP};
VERSION_STRING_MESSAGE = const char version_string[] = "${VERSION_STRING}";
define MAKE_BUILD_STRINGS
	@echo $$(BUILT_TIME_DATE_STRING) $$(VERSION_STRING_MESSAGE) | \
		$$(CC) $$(TF_CFLAGS) $$(CFLAGS) -x c -c - -o $1
endef

MSVC_NMAKE := nmake.exe

