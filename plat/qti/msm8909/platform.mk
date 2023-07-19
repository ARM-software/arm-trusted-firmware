#
# Copyright (c) 2022-2023, Stephan Gerhold <stephan@gerhold.net>
#
# SPDX-License-Identifier: BSD-3-Clause
#

ARM_ARCH_MAJOR	:= 7
ARM_CORTEX_A7	:= yes

BL31_BASE	?= 0x87e80000
QTI_UART_NUM	?= 1

include plat/qti/msm8916/platform.mk
