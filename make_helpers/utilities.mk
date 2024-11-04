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
# The grouped-target symbol. Grouped targets are not supported on versions of
# GNU Make <= 4.2, which was most recently packaged with Ubuntu 20.04.
#

& := $(if $(filter grouped-target,$(.FEATURES)),&)

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

#
# Determine the "truthiness" of a value.
#
# Parameters:
#
#   - $(1): The value to determine the truthiness of.
#
# A value is considered to be falsy if it is:
#
#   - empty, or
#   - equal to "0", "N", "NO", "F" or "FALSE" after upper-casing.
#
# If the value is truthy then the value is returned as-is, otherwise no value
# is returned.
#
# Example usage:
#
#     truthy := y
#     truthy-bool := $(call bool,$(truthy)) # "y"
#
#     falsy := n
#     falsy-bool := $(call bool,$(falsy)) # <empty>
#

bool = $(filter-out 0 n no f false,$(call lowercase,$(1)))

#
# Determine the "truthiness" of a value, returning 0 or 1.
#
# Parameters:
#
#   - $(1): The value to determine the truthiness of.
#
# A value is considered to be falsy if it is:
#
#   - empty, or
#   - equal to "0", "N", "NO", "F" or "FALSE" after upper-casing.
#
# If the value is truthy then the value is returned as-is, otherwise no value
# is returned.
#
# Example usage:
#
#     truthy := y
#     truthy-bool := $(call bool,$(truthy)) # "1"
#
#     falsy := n
#     falsy-bool := $(call bool,$(falsy)) # "0"
#

bool-01 = $(if $(call bool,$(1)),1,0)

#
# Determine whether a variable is defined or not.
#
# Parameters:
#
#   - $(1): The variable to check.
#
# Example usage:
#
#     xyz-defined := $(call defined,xyz) # <empty>
#
#     xyz :=
#     xyz-defined := $(call defined,xyz) # <non-empty>
#
#     xyz := hello
#     xyz-defined := $(call defined,xyz) # <non-empty>
#

defined = $(call bool,$(filter-out undefined,$(origin $(1))))
