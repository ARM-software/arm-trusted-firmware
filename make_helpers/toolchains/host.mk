#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

host-name := host

host-cc-parameter := HOSTCC
host-cc-default := gcc
host-cc-id-default := gnu-gcc

host-cpp-parameter := HOSTCPP
host-cpp-default := gcc
host-cpp-id-default := gnu-gcc

host-as-parameter := HOSTAS
host-as-default := gcc
host-as-id-default := gnu-gcc

host-ld-parameter := HOSTLD
host-ld-default := gcc
host-ld-id-default := gnu-gcc

host-oc-parameter := HOSTOC
host-oc-default := objcopy
host-oc-id-default := gnu-objcopy

host-od-parameter := HOSTOD
host-od-default := objdump
host-od-id-default := gnu-objdump

host-ar-parameter := HOSTAR
host-ar-default := gcc-ar
host-ar-id-default := gnu-ar

host-dtc-parameter := HOSTDTC
host-dtc-default := dtc
host-dtc-id-default := generic-dtc
