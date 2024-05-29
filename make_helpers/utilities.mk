#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

space :=
space := $(space) $(space)
comma := ,

null := �

compat-path = $(subst $(space),$(null),$(1))
decompat-path = $(subst $(null), ,$(1))

absolute-path = $(call decompat-path,$(abspath $(call compat-path,$(1))))
real-path = $(call decompat-path,$(realpath $(call compat-path,$(1))))

file-name = $(call decompat-path,$(notdir $(call compat-path,$(1))))
directory-name = $(call decompat-path,$(dir $(call compat-path,$(1))))

escape-shell = '$(subst ','\'',$(1))'

#
# Upper-case a string value.
#
# Parameters:
#
#   - $(1): The string to upper-case.
#
# Example usage:
#
#     $(call uppercase,HeLlO wOrLd) # "HELLO WORLD"
#

uppercase = $(shell echo $(call escape-shell,$(1)) | tr '[:lower:]' '[:upper:]')

#
# Lower-case a string value.
#
# Parameters:
#
#   - $(1): The string to lower-case.
#
# Example usage:
#
#     $(call lowercase,HeLlO wOrLd) # "hello world"
#

lowercase = $(shell echo $(call escape-shell,$(1)) | tr '[:upper:]' '[:lower:]')
