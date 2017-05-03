#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
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
