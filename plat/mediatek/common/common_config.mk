#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS := 1
MULTI_CONSOLE_API := 1
COLD_BOOT_SINGLE_CPU := 1
# Build flag to include AArch32 registers in cpu context save and restore during
# world switch. This flag must be set to 0 for AArch64-only platforms.
CTX_INCLUDE_AARCH32_REGS := 0
PLAT_XLAT_TABLES_DYNAMIC := 1
# enable this definition to print irq dump status in tf-a
GIC_DEBUG := 0
# Enable stack protector.
# Allowed values are "all", "strong", "default" and "none"
ENABLE_STACK_PROTECTOR := strong
# AMU, Kernel will access amuserenr_el0 if PE supported
# Firmware _must_ implement AMU support
ENABLE_AMU := 1
VENDOR_EXTEND_PUBEVENT_ENABLE := 1

# MTK define options
MTK_BL33_IS_64BIT := 0
MTK_ADAPTED := 1

# MTK module config
CONFIG_MTK_INTERRUPT := y
CONFIG_MTK_UART := y

# UART baudrate
UART_BAUDRATE := 921600
