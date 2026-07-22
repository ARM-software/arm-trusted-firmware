#
# Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable LTO for BL1 only to reduce code size and ensure aggressive dead-code
# elimination. This is necessary for DEBUG builds.
BL1_CFLAGS	+= -flto
BL1_LDFLAGS	+= -flto
