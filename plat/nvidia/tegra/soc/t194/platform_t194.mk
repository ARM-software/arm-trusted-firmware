#
# Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# platform configs
ENABLE_CONSOLE_SPE			:= 1
$(eval $(call add_define,ENABLE_CONSOLE_SPE))

ENABLE_STRICT_CHECKING_MODE		:= 1
$(eval $(call add_define,ENABLE_STRICT_CHECKING_MODE))

USE_GPC_DMA				:= 1
$(eval $(call add_define,USE_GPC_DMA))

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

BL31_SOURCES		+=	drivers/ti/uart/aarch64/16550_console.S \
				lib/cpus/aarch64/denver.S		\
				${COMMON_DIR}/drivers/bpmp_ipc/intf.c	\
				${COMMON_DIR}/drivers/bpmp_ipc/ivc.c	\
				${COMMON_DIR}/drivers/gpcdma/gpcdma.c	\
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

ifeq (${ENABLE_CONSOLE_SPE},1)
BL31_SOURCES		+=	${COMMON_DIR}/drivers/spe/shared_console.S
endif
