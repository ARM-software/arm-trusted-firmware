#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS := 1
COLD_BOOT_SINGLE_CPU := 1
# Build flag to include AArch32 registers in cpu context save and restore during
# world switch. This flag must be set to 0 for AArch64-only platforms.
CTX_INCLUDE_AARCH32_REGS := 0
PLAT_XLAT_TABLES_DYNAMIC := 1

VENDOR_EXTEND_PUBEVENT_ENABLE := 1
