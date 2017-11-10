#
# Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# platform configs
ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS	:= 0
$(eval $(call add_define,ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS))

RELOCATE_TO_BL31_BASE			:= 1
$(eval $(call add_define,RELOCATE_TO_BL31_BASE))

ENABLE_CHIP_VERIFICATION_HARNESS	:= 0
$(eval $(call add_define,ENABLE_CHIP_VERIFICATION_HARNESS))

ENABLE_SMMU_DEVICE			:= 1
$(eval $(call add_define,ENABLE_SMMU_DEVICE))

RESET_TO_BL31				:= 1

PROGRAMMABLE_RESET_ADDRESS		:= 1

COLD_BOOT_SINGLE_CPU			:= 1

# platform settings
TZDRAM_BASE				:= 0x40000000
$(eval $(call add_define,TZDRAM_BASE))

PLATFORM_CLUSTER_COUNT			:= 4
$(eval $(call add_define,PLATFORM_CLUSTER_COUNT))

PLATFORM_MAX_CPUS_PER_CLUSTER		:= 2
$(eval $(call add_define,PLATFORM_MAX_CPUS_PER_CLUSTER))

MAX_XLAT_TABLES				:= 25
$(eval $(call add_define,MAX_XLAT_TABLES))

MAX_MMAP_REGIONS			:= 30
$(eval $(call add_define,MAX_MMAP_REGIONS))

# platform files
PLAT_INCLUDES		+=	-I${SOC_DIR}/drivers/include

BL31_SOURCES		+=	lib/cpus/aarch64/denver.S		\
				${COMMON_DIR}/drivers/memctrl/memctrl_v2.c	\
				${COMMON_DIR}/drivers/smmu/smmu.c	\
				${SOC_DIR}/drivers/mce/mce.c		\
				${SOC_DIR}/drivers/mce/nvg.c		\
				${SOC_DIR}/drivers/mce/aarch64/nvg_helpers.S \
				${SOC_DIR}/drivers/se/se.c		\
				${SOC_DIR}/plat_memctrl.c		\
				${SOC_DIR}/plat_psci_handlers.c		\
				${SOC_DIR}/plat_setup.c			\
				${SOC_DIR}/plat_secondary.c		\
				${SOC_DIR}/plat_sip_calls.c		\
				${SOC_DIR}/plat_smmu.c			\
				${SOC_DIR}/plat_trampoline.S
