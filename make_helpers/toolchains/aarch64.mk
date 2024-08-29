#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

aarch64-name := AArch64

aarch64-cc-parameter := CC
aarch64-cc-default-id := gnu-gcc
aarch64-cc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-cc-default += $(if $(CROSS_COMPILE),,aarch64-linux-gnu-gcc)

aarch64-cpp-parameter := CPP
aarch64-cpp-default-id := gnu-gcc
aarch64-cpp-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-cpp-default += $(if $(CROSS_COMPILE),,aarch64-linux-gnu-gcc)

aarch64-as-parameter := AS
aarch64-as-default-id := gnu-gcc
aarch64-as-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-as-default += $(if $(CROSS_COMPILE),,aarch64-linux-gnu-gcc)

aarch64-ld-parameter := LD
aarch64-ld-default-id := gnu-gcc
aarch64-ld-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-ld-default += $(if $(CROSS_COMPILE),,aarch64-linux-gnu-gcc)

aarch64-oc-parameter := OC
aarch64-oc-default-id := gnu-objcopy
aarch64-oc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objcopy
aarch64-oc-default += $(if $(CROSS_COMPILE),,aarch64-linux-gnu-objcopy)

aarch64-od-parameter := OD
aarch64-od-default-id := gnu-objdump
aarch64-od-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objdump
aarch64-od-default += $(if $(CROSS_COMPILE),,aarch64-linux-gnu-objdump)

aarch64-ar-parameter := AR
aarch64-ar-default-id := gnu-ar
aarch64-ar-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc-ar
aarch64-ar-default += $(if $(CROSS_COMPILE),,aarch64-linux-gnu-gcc-ar)

aarch64-dtc-parameter := DTC
aarch64-dtc-default-id := generic-dtc
aarch64-dtc-default := dtc
