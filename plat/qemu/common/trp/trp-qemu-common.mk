#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TRP source files common to QEMU platforms
RMM_SOURCES		+=	plat/qemu/common/trp/qemu_trp_setup.c	\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/qemu/common/aarch64/plat_helpers.S

INCLUDES		+=	-Iinclude/services/trp
