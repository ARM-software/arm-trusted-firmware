#
# Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef common-mk
        common-mk := $(lastword $(MAKEFILE_LIST))

        include $(dir $(common-mk))utilities.mk

        silent := $(call bool,$(findstring s,$(firstword ~$(MAKEFLAGS))))
        verbose := $(if $(silent),,$(call bool,$(V)))

        s := @$(if $(or $(verbose),$(silent)),: )
        q := $(if $(verbose),,@)

        .exe := $(if $(filter Windows_NT,$(OS)),.exe)
endif
