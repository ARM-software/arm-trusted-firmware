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
    define MAKE_PREREQ_DIR

${1} : ${2}
	${Q}mkdir -p  "${1}"

    endef

    define SHELL_REMOVE_DIR
	-${Q}rm -rf  "${1}"
    endef

endif
