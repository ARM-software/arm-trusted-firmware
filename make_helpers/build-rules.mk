#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef build-rules-mk
        build-rules-mk := $(lastword $(MAKEFILE_LIST))

        include $(dir $(build-rules-mk))common.mk
        include $(dir $(build-rules-mk))utilities.mk

        .SECONDEXPANSION:
        .SUFFIXES:

        %/:
		$(s)echo '  MD      '$(call escape-shell,$(abspath $@))
		$(q)mkdir -p $(call escape-shell,$@)
endif
