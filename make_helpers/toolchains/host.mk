#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

host-name := host

host-cc-parameter := HOSTCC
host-cc-default-id := gnu-gcc
host-cc-default := gcc

host-cpp-parameter := HOSTCPP
host-cpp-default-id := gnu-gcc
host-cpp-default := gcc

host-as-parameter := HOSTAS
host-as-default-id := gnu-gcc
host-as-default := gcc

host-ld-parameter := HOSTLD
host-ld-default-id := gnu-gcc
host-ld-default := gcc

host-oc-parameter := HOSTOC
host-oc-default-id := gnu-objcopy
host-oc-default := objcopy

host-od-parameter := HOSTOD
host-od-default-id := gnu-objdump
host-od-default := objdump

host-ar-parameter := HOSTAR
host-ar-default-id := gnu-ar
host-ar-default := gcc-ar

host-dtc-parameter := HOSTDTC
host-dtc-default-id := generic-dtc
host-dtc-default := dtc

host-poetry-parameter := POETRY
host-poetry-optional := yes
host-poetry-default-id := generic-poetry
host-poetry-default := poetry
