#
# Copyright (c) 2026, Texas Instruments Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES	+=	\
				-I${PLAT_PATH}/board/am62lx/lpm			\

BL31_SOURCES	+=	\
				${PLAT_PATH}/board/am62lx/lpm/standby.c		\

