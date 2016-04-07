#
#  Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  Redistributions of source code must retain the above copyright notice, this
#  list of conditions and the following disclaimer.
#
#  Redistributions in binary form must reproduce the above copyright notice,
#  this list of conditions and the following disclaimer in the documentation
#  and/or other materials provided with the distribution.
#
#  Neither the name of ARM nor the names of its contributors may be used
#  to endorse or promote products derived from this software without specific
#  prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

# This file contains the logic to identify and include any relevant
# build environment specific make include files.

ifndef BUILD_ENV_MK
    BUILD_ENV_MK        :=      $(lastword $(MAKEFILE_LIST))

    # Block possible built-in command definitions that are not fully portable.
    # This traps occurences that need replacing with our OS portable macros
    COPY                :=      $$(error "Replace COPY with call to SHELL_COPY or SHELL_COPY_TREE.")
    CP                  :=      $$(error "Replace CP with call to SHELL_COPY or SHELL_COPY_TREE.")
    DEL                 :=      $$(error "Replace DEL with call to SHELL_DELETE.")
    MD                  :=      $$(error "Replace MD with call to MAKE_PREREQ_DIR.")
    MKDIR               :=      $$(error "Replace MKDIR with call to MAKE_PREREQ_DIR.")
    RD                  :=      $$(error "Replace RD with call to SHELL_REMOVE_DIR.")
    RM                  :=      $$(error "Replace RM with call to SHELL_DELETE.")
    RMDIR               :=      $$(error "Replace RMDIR with call to SHELL_REMOVE_DIR.")

    ENV_FILE_TO_INCLUDE := unix.mk
    ifdef OSTYPE
        ifneq ($(findstring ${OSTYPE}, cygwin),)
            ENV_FILE_TO_INCLUDE := cygwin.mk
        else
            ifneq ($(findstring ${OSTYPE}, MINGW32 mingw msys),)
                ENV_FILE_TO_INCLUDE := msys.mk
            endif
        endif
    else
        ifdef MSYSTEM
            # Although the MINGW MSYS shell sets OSTYPE as msys in its environment,
            # it does not appear in the GNU make view of environment variables.
            # We use MSYSTEM as an alternative, as that is seen by make
            ifneq ($(findstring ${MSYSTEM}, MINGW32 mingw msys),)
                OSTYPE ?= msys
                ENV_FILE_TO_INCLUDE := msys.mk
            endif
        else
            ifdef OS
                ifneq ($(findstring ${OS}, Windows_NT),)
                    ENV_FILE_TO_INCLUDE := windows.mk
                endif
            endif
        endif
    endif
    include ${MAKE_HELPERS_DIRECTORY}${ENV_FILE_TO_INCLUDE}
    ENV_FILE_TO_INCLUDE :=

    ifndef SHELL_COPY
        $(error "SHELL_COPY not defined for build environment.")
    endif
    ifndef SHELL_COPY_TREE
        $(error "SHELL_COPY_TREE not defined for build environment.")
    endif
    ifndef SHELL_DELETE_ALL
        $(error "SHELL_DELETE_ALL not defined for build environment.")
    endif
    ifndef SHELL_DELETE
        $(error "SHELL_DELETE not defined for build environment.")
    endif
    ifndef MAKE_PREREQ_DIR
        $(error "MAKE_PREREQ_DIR not defined for build environment.")
    endif
    ifndef SHELL_REMOVE_DIR
        $(error "SHELL_REMOVE_DIR not defined for build environment.")
    endif

endif
