#
# Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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

# platform configs
ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS	:= 1
$(eval $(call add_define,ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS))

RELOCATE_TO_BL31_BASE			:= 1
$(eval $(call add_define,RELOCATE_TO_BL31_BASE))

ENABLE_CHIP_VERIFICATION_HARNESS	:= 0
$(eval $(call add_define,ENABLE_CHIP_VERIFICATION_HARNESS))

RESET_TO_BL31				:= 1

PROGRAMMABLE_RESET_ADDRESS		:= 1

COLD_BOOT_SINGLE_CPU			:= 1

# platform settings
TZDRAM_BASE				:= 0x30000000
$(eval $(call add_define,TZDRAM_BASE))

PLATFORM_CLUSTER_COUNT			:= 2
$(eval $(call add_define,PLATFORM_CLUSTER_COUNT))

PLATFORM_MAX_CPUS_PER_CLUSTER		:= 4
$(eval $(call add_define,PLATFORM_MAX_CPUS_PER_CLUSTER))

MAX_XLAT_TABLES				:= 20
$(eval $(call add_define,MAX_XLAT_TABLES))

MAX_MMAP_REGIONS			:= 20
$(eval $(call add_define,MAX_MMAP_REGIONS))

# platform files
PLAT_INCLUDES		+=	-I${SOC_DIR}/drivers/include

BL31_SOURCES		+=	lib/cpus/aarch64/denver.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				${COMMON_DIR}/drivers/memctrl/memctrl_v2.c	\
				${SOC_DIR}/drivers/mce/mce.c		\
				${SOC_DIR}/drivers/mce/ari.c		\
				${SOC_DIR}/drivers/mce/nvg.c		\
				${SOC_DIR}/drivers/mce/aarch64/nvg_helpers.S \
				${SOC_DIR}/drivers/smmu/smmu.c		\
				${SOC_DIR}/plat_psci_handlers.c		\
				${SOC_DIR}/plat_setup.c			\
				${SOC_DIR}/plat_secondary.c		\
				${SOC_DIR}/plat_sip_calls.c		\
				${SOC_DIR}/plat_trampoline.S
