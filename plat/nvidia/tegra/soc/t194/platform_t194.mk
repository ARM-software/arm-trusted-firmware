#
# Copyright (c) 2019-2021, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

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

MAX_XLAT_TABLES				:= 25
$(eval $(call add_define,MAX_XLAT_TABLES))

MAX_MMAP_REGIONS			:= 30
$(eval $(call add_define,MAX_MMAP_REGIONS))

# enable RAS handling
HANDLE_EA_EL3_FIRST			:= 1
RAS_EXTENSION				:= 1

# platform files
PLAT_INCLUDES		+=	-Iplat/nvidia/tegra/include/t194 \
				-I${SOC_DIR}/drivers/include

BL31_SOURCES		+=	${TEGRA_GICv2_SOURCES}			\
				drivers/ti/uart/aarch64/16550_console.S \
				lib/cpus/aarch64/denver.S		\
				${TEGRA_DRIVERS}/bpmp_ipc/intf.c	\
				${TEGRA_DRIVERS}/bpmp_ipc/ivc.c		\
				${TEGRA_DRIVERS}/memctrl/memctrl_v2.c	\
				${TEGRA_DRIVERS}/smmu/smmu.c		\
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

ifeq (${USE_GPC_DMA}, 1)
BL31_SOURCES		+=	${TEGRA_DRIVERS}/gpcdma/gpcdma.c
endif

ifeq (${ENABLE_CONSOLE_SPE},1)
BL31_SOURCES		+=	${TEGRA_DRIVERS}/spe/shared_console.S
endif

# RAS sources
ifeq (${RAS_EXTENSION},1)
BL31_SOURCES		+=	lib/extensions/ras/std_err_record.c		\
				lib/extensions/ras/ras_common.c			\
				${SOC_DIR}/plat_ras.c
endif

# SPM dispatcher
ifeq (${SPD},spmd)
include lib/libfdt/libfdt.mk
# sources to support spmd
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c	\
				${LIBFDT_SRCS}

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}
endif
