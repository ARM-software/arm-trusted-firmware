#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

aarch32-name := AArch32

aarch32-cc := $(if $(filter-out default,$(origin CC)),$(CC))
aarch32-cc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-cc-id-default := gnu-gcc

aarch32-cpp := $(if $(filter-out default,$(origin CPP)),$(CPP))
aarch32-cpp-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-cpp-id-default := gnu-gcc

aarch32-as := $(if $(filter-out default,$(origin AS)),$(AS))
aarch32-as-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-as-id-default := gnu-gcc

aarch32-ld := $(if $(filter-out default,$(origin LD)),$(LD))
aarch32-ld-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-ld-id-default := gnu-gcc

aarch32-oc := $(if $(filter-out default,$(origin OC)),$(OC))
aarch32-oc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objcopy
aarch32-oc-id-default := gnu-objcopy

aarch32-od := $(if $(filter-out default,$(origin OD)),$(OD))
aarch32-od-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objdump
aarch32-od-id-default := gnu-objdump

aarch32-ar := $(if $(filter-out default,$(origin AR)),$(AR))
aarch32-ar-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc-ar
aarch32-ar-id-default := gnu-ar

aarch32-dtc := $(if $(filter-out default,$(origin DTC)),$(DTC))
aarch32-dtc-default := dtc
aarch32-dtc-id-default := generic-dtc
