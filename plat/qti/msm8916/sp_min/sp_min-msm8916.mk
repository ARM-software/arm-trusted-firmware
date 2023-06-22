#
# Copyright (c) 2022-2023, Stephan Gerhold <stephan@gerhold.net>
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES	+=	${MSM8916_PM_SOURCES}				\
			plat/common/${ARCH}/platform_mp_stack.S		\
			plat/qti/msm8916/sp_min/msm8916_sp_min_setup.c

override RESET_TO_SP_MIN := 1
