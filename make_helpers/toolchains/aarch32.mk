#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

aarch32-cc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-cpp-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-as-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-ld-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-oc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objcopy
aarch32-od-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objdump
aarch32-ar-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc-ar
aarch32-dtc-default := dtc
