#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

aarch32-name := AArch32

aarch32-cc-parameter := CC
aarch32-cc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-cc-id-default := gnu-gcc

aarch32-cpp-parameter := CPP
aarch32-cpp-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-cpp-id-default := gnu-gcc

aarch32-as-parameter := AS
aarch32-as-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-as-id-default := gnu-gcc

aarch32-ld-parameter := LD
aarch32-ld-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-ld-id-default := gnu-gcc

aarch32-oc-parameter := OC
aarch32-oc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objcopy
aarch32-oc-id-default := gnu-objcopy

aarch32-od-parameter := OD
aarch32-od-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objdump
aarch32-od-id-default := gnu-objdump

aarch32-ar-parameter := AR
aarch32-ar-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc-ar
aarch32-ar-id-default := gnu-ar

aarch32-dtc-parameter := DTC
aarch32-dtc-default := dtc
aarch32-dtc-id-default := generic-dtc
