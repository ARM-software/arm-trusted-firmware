#
# Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
# Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

TZDRAM_BASE			:= 0xF5C00000
$(eval $(call add_define,TZDRAM_BASE))

PLATFORM_CLUSTER_COUNT		:= 1
$(eval $(call add_define,PLATFORM_CLUSTER_COUNT))

PLATFORM_MAX_CPUS_PER_CLUSTER	:= 2
$(eval $(call add_define,PLATFORM_MAX_CPUS_PER_CLUSTER))

MAX_XLAT_TABLES			:= 3
$(eval $(call add_define,MAX_XLAT_TABLES))

MAX_MMAP_REGIONS		:= 8
$(eval $(call add_define,MAX_MMAP_REGIONS))

# platform files
PLAT_INCLUDES		+=	-Iplat/nvidia/tegra/include/t132

BL31_SOURCES		+=	${TEGRA_GICv2_SOURCES}			\
				drivers/ti/uart/aarch64/16550_console.S	\
				lib/cpus/aarch64/denver.S		\
				${TEGRA_DRIVERS}/flowctrl/flowctrl.c	\
				${TEGRA_DRIVERS}/memctrl/memctrl_v1.c	\
				${TEGRA_DRIVERS}/pmc/pmc.c		\
				${SOC_DIR}/plat_psci_handlers.c		\
				${SOC_DIR}/plat_sip_calls.c		\
				${SOC_DIR}/plat_setup.c			\
				${SOC_DIR}/plat_secondary.c
