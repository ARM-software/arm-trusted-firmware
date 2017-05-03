#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

TZDRAM_BASE				:= 0xFF800000
$(eval $(call add_define,TZDRAM_BASE))

ERRATA_TEGRA_INVALIDATE_BTB_AT_BOOT	:= 1
$(eval $(call add_define,ERRATA_TEGRA_INVALIDATE_BTB_AT_BOOT))

PLATFORM_CLUSTER_COUNT			:= 2
$(eval $(call add_define,PLATFORM_CLUSTER_COUNT))

PLATFORM_MAX_CPUS_PER_CLUSTER		:= 4
$(eval $(call add_define,PLATFORM_MAX_CPUS_PER_CLUSTER))

MAX_XLAT_TABLES				:= 4
$(eval $(call add_define,MAX_XLAT_TABLES))

MAX_MMAP_REGIONS			:= 8
$(eval $(call add_define,MAX_MMAP_REGIONS))

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				${COMMON_DIR}/drivers/flowctrl/flowctrl.c	\
				${COMMON_DIR}/drivers/memctrl/memctrl_v1.c	\
				${SOC_DIR}/plat_psci_handlers.c		\
				${SOC_DIR}/plat_setup.c			\
				${SOC_DIR}/plat_secondary.c

# Enable workarounds for selected Cortex-A57 erratas.
A57_DISABLE_NON_TEMPORAL_HINT	:=	1
ERRATA_A57_826974		:=	1
ERRATA_A57_826977		:=	1
ERRATA_A57_828024		:=	1
ERRATA_A57_829520		:=	1
ERRATA_A57_833471		:=	1

# Enable workarounds for selected Cortex-A53 erratas.
A53_DISABLE_NON_TEMPORAL_HINT	:=	1
ERRATA_A53_826319		:=	1
ERRATA_A53_836870		:=	1
ERRATA_A53_855873		:=	1
