#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

aarch64-cc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-cpp-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-as-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-ld-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-oc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objcopy
aarch64-od-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objdump
aarch64-ar-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc-ar
aarch64-dtc-default := dtc
