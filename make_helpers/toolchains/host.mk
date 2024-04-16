#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

host-name := host

host-cc := $(HOSTCC)
host-cc-default := gcc
host-cc-id-default := gnu-gcc

host-cpp := $(HOSTCPP)
host-cpp-default := gcc
host-cpp-id-default := gnu-gcc

host-as := $(HOSTAS)
host-as-default := gcc
host-as-id-default := gnu-gcc

host-ld := $(HOSTLD)
host-ld-default := gcc
host-ld-id-default := gnu-gcc

host-oc := $(HOSTOC)
host-oc-default := objcopy
host-oc-id-default := gnu-objcopy

host-od := $(HOSTOD)
host-od-default := objdump
host-od-id-default := gnu-objdump

host-ar := $(HOSTAR)
host-ar-default := gcc-ar
host-ar-id-default := gnu-ar

host-dtc := $(HOSTDTC)
host-dtc-default := dtc
host-dtc-id-default := generic-dtc
