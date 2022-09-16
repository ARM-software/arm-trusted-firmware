#
# Copyright (c) 2022-2023, Stephan Gerhold <stephan@gerhold.net>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Cache cannot be enabled early on MSM8939 because the CCI-400 must be
# enabled before the CPUs in both clusters become cache-coherent.
override WARMBOOT_ENABLE_DCACHE_EARLY := 0

include plat/qti/msm8916/platform.mk
