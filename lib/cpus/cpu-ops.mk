#
# Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

# Cortex A57 specific optimisation to skip L1 cache flush when
# cluster is powered down.
SKIP_A57_L1_FLUSH_PWR_DWN	?=0

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
A53_DISABLE_NON_TEMPORAL_HINT	?=1

# Flag to disable the cache non-temporal hint.
# It is enabled by default.
A57_DISABLE_NON_TEMPORAL_HINT	?=1

# Process SKIP_A57_L1_FLUSH_PWR_DWN flag
$(eval $(call assert_boolean,SKIP_A57_L1_FLUSH_PWR_DWN))
$(eval $(call add_define,SKIP_A57_L1_FLUSH_PWR_DWN))

# Process A53_DISABLE_NON_TEMPORAL_HINT flag
$(eval $(call assert_boolean,A53_DISABLE_NON_TEMPORAL_HINT))
$(eval $(call add_define,A53_DISABLE_NON_TEMPORAL_HINT))

# Process A57_DISABLE_NON_TEMPORAL_HINT flag
$(eval $(call assert_boolean,A57_DISABLE_NON_TEMPORAL_HINT))
$(eval $(call add_define,A57_DISABLE_NON_TEMPORAL_HINT))


# CPU Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 826319 workaround during reset. This erratum applies
# only to revision <= r0p2 of the Cortex A53 cpu.
ERRATA_A53_826319	?=0

# Flag to apply erratum 836870 workaround during reset. This erratum applies
# only to revision <= r0p3 of the Cortex A53 cpu. From r0p4 and onwards, this
# erratum workaround is enabled by default.
ERRATA_A53_836870	?=0

# Flag to apply erratum 806969 workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_806969	?=0

# Flag to apply erratum 813420  workaround during reset. This erratum applies
# only to revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_813420	?=0

# Flag to apply erratum 826974 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
ERRATA_A57_826974	?=0

# Flag to apply erratum 826977 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
ERRATA_A57_826977	?=0

# Flag to apply erratum 828024 workaround during reset. This erratum applies
# only to revision <= r1p1 of the Cortex A57 cpu.
ERRATA_A57_828024	?=0

# Flag to apply erratum 829520 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
ERRATA_A57_829520	?=0

# Flag to apply erratum 833471 workaround during reset. This erratum applies
# only to revision <= r1p2 of the Cortex A57 cpu.
ERRATA_A57_833471	?=0

# Process ERRATA_A53_826319 flag
$(eval $(call assert_boolean,ERRATA_A53_826319))
$(eval $(call add_define,ERRATA_A53_826319))

# Process ERRATA_A53_836870 flag
$(eval $(call assert_boolean,ERRATA_A53_836870))
$(eval $(call add_define,ERRATA_A53_836870))

# Process ERRATA_A57_806969 flag
$(eval $(call assert_boolean,ERRATA_A57_806969))
$(eval $(call add_define,ERRATA_A57_806969))

# Process ERRATA_A57_813420 flag
$(eval $(call assert_boolean,ERRATA_A57_813420))
$(eval $(call add_define,ERRATA_A57_813420))

# Process ERRATA_A57_826974 flag
$(eval $(call assert_boolean,ERRATA_A57_826974))
$(eval $(call add_define,ERRATA_A57_826974))

# Process ERRATA_A57_826977 flag
$(eval $(call assert_boolean,ERRATA_A57_826977))
$(eval $(call add_define,ERRATA_A57_826977))

# Process ERRATA_A57_828024 flag
$(eval $(call assert_boolean,ERRATA_A57_828024))
$(eval $(call add_define,ERRATA_A57_828024))

# Process ERRATA_A57_829520 flag
$(eval $(call assert_boolean,ERRATA_A57_829520))
$(eval $(call add_define,ERRATA_A57_829520))

# Process ERRATA_A57_833471 flag
$(eval $(call assert_boolean,ERRATA_A57_833471))
$(eval $(call add_define,ERRATA_A57_833471))
