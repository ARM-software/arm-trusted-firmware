#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

TEGRA_BOOT_UART_BASE 			:= 0x70006000
$(eval $(call add_define,TEGRA_BOOT_UART_BASE))

TZDRAM_BASE				:= 0xFDC00000
$(eval $(call add_define,TZDRAM_BASE))

ERRATA_TEGRA_INVALIDATE_BTB_AT_BOOT	:= 1
$(eval $(call add_define,ERRATA_TEGRA_INVALIDATE_BTB_AT_BOOT))

ENABLE_NS_L2_CPUECTRL_RW_ACCESS		:= 1
$(eval $(call add_define,ENABLE_NS_L2_CPUECTRL_RW_ACCESS))

ENABLE_L2_DYNAMIC_RETENTION		:= 1
$(eval $(call add_define,ENABLE_L2_DYNAMIC_RETENTION))

ENABLE_CPU_DYNAMIC_RETENTION		:= 1
$(eval $(call add_define,ENABLE_CPU_DYNAMIC_RETENTION))

PLATFORM_CLUSTER_COUNT			:= 2
$(eval $(call add_define,PLATFORM_CLUSTER_COUNT))

PLATFORM_MAX_CPUS_PER_CLUSTER		:= 4
$(eval $(call add_define,PLATFORM_MAX_CPUS_PER_CLUSTER))

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				${SOC_DIR}/plat_psci_handlers.c		\
				${SOC_DIR}/plat_setup.c			\
				${SOC_DIR}/plat_secondary.c

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	1

