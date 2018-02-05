#
# Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# This makefile only aims at complying with ARM Trusted Firmware build process so
# that "optee" is a valid ARM Trusted Firmware AArch32 Secure Playload identifier.

ifneq ($(ARCH),aarch32)
$(error This directory targets AArch32 support)
endif

$(eval $(call add_define,AARCH32_SP_OPTEE))

$(info ARM Trusted Firmware built for OP-TEE payload support)
