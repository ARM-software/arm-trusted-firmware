#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

aarch64-name := AArch64

aarch64-cc := $(if $(filter-out default,$(origin CC)),$(CC))
aarch64-cc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-cc-id-default := gnu-gcc

aarch64-cpp := $(if $(filter-out default,$(origin CPP)),$(CPP))
aarch64-cpp-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-cpp-id-default := gnu-gcc

aarch64-as := $(if $(filter-out default,$(origin AS)),$(AS))
aarch64-as-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-as-id-default := gnu-gcc

aarch64-ld := $(if $(filter-out default,$(origin LD)),$(LD))
aarch64-ld-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-ld-id-default := gnu-gcc

aarch64-oc := $(if $(filter-out default,$(origin OC)),$(OC))
aarch64-oc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objcopy
aarch64-oc-id-default := gnu-objcopy

aarch64-od := $(if $(filter-out default,$(origin OD)),$(OD))
aarch64-od-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objdump
aarch64-od-id-default := gnu-objdump

aarch64-ar := $(if $(filter-out default,$(origin AR)),$(AR))
aarch64-ar-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc-ar
aarch64-ar-id-default := gnu-ar

aarch64-dtc := $(if $(filter-out default,$(origin DTC)),$(DTC))
aarch64-dtc-default := dtc
aarch64-dtc-id-default := generic-dtc
