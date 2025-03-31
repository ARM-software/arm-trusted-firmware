#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TRP source files specific to FVP platform

RMM_SOURCES		+=	plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/board/fvp/trp/test_ide_km_interface.c

include plat/arm/common/trp/arm_trp.mk

