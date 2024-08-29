#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

rk3399-m0-name := RK3399 M0

rk3399-m0-cc-default-id := gnu-gcc
rk3399-m0-cc-default := $(or $(M0_CROSS_COMPILE),arm-none-eabi-)gcc

rk3399-m0-cpp-default-id := gnu-gcc
rk3399-m0-cpp-default := $(or $(M0_CROSS_COMPILE),arm-none-eabi-)gcc

rk3399-m0-as-default-id := gnu-gcc
rk3399-m0-as-default := $(or $(M0_CROSS_COMPILE),arm-none-eabi-)gcc

rk3399-m0-ld-default-id := gnu-gcc
rk3399-m0-ld-default := $(or $(M0_CROSS_COMPILE),arm-none-eabi-)gcc

rk3399-m0-oc-default-id := gnu-objcopy
rk3399-m0-oc-default := $(or $(M0_CROSS_COMPILE),arm-none-eabi-)objcopy

rk3399-m0-od-default-id := gnu-objdump
rk3399-m0-od-default := $(or $(M0_CROSS_COMPILE),arm-none-eabi-)objdump

rk3399-m0-ar-default-id := gnu-ar
rk3399-m0-ar-default := $(or $(M0_CROSS_COMPILE),arm-none-eabi-)gcc-ar

rk3399-m0-dtc-default-id := generic-dtc
rk3399-m0-dtc-default := dtc
