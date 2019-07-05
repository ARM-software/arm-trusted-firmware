#
# Copyright (c) 2016-2019, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# This makefile only aims at complying with Trusted Firmware-A build process so
# that "optee" is a valid TF-A AArch32 Secure Playload identifier.

ifneq ($(ARCH),aarch32)
$(error This directory targets AArch32 support)
endif

$(eval $(call add_define,AARCH32_SP_OPTEE))

$(info Trusted Firmware-A built for OP-TEE payload support)
