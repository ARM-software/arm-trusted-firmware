#
# Copyright (c) 2021, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT     := plat/mediatek
MTK_PLAT_SOC := ${MTK_PLAT}/${PLAT}

PLAT_INCLUDES := -I${MTK_PLAT}/common/                            \
                 -I${MTK_PLAT}/common/drivers/gic600/             \
                 -I${MTK_PLAT}/common/drivers/gpio/               \
                 -I${MTK_PLAT}/common/drivers/rtc/                \
                 -I${MTK_PLAT}/common/drivers/timer/              \
                 -I${MTK_PLAT}/common/drivers/uart/               \
                 -I${MTK_PLAT}/common/lpm/                        \
                 -I${MTK_PLAT_SOC}/drivers/apusys/                \
                 -I${MTK_PLAT_SOC}/drivers/dcm                    \
                 -I${MTK_PLAT_SOC}/drivers/dfd                    \
                 -I${MTK_PLAT_SOC}/drivers/dp/                    \
                 -I${MTK_PLAT_SOC}/drivers/emi_mpu/               \
                 -I${MTK_PLAT_SOC}/drivers/gpio/                  \
                 -I${MTK_PLAT_SOC}/drivers/mcdi/                  \
                 -I${MTK_PLAT_SOC}/drivers/pmic/                  \
                 -I${MTK_PLAT_SOC}/drivers/spmc/                  \
                 -I${MTK_PLAT_SOC}/drivers/ptp3/                   \
                 -I${MTK_PLAT_SOC}/include/

GICV3_SUPPORT_GIC600        :=      1
include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES := ${GICV3_SOURCES}                              \
                          ${XLAT_TABLES_LIB_SRCS}                       \
                          plat/common/aarch64/crash_console_helpers.S   \
                          plat/common/plat_psci_common.c


BL31_SOURCES += common/desc_image_load.c                              \
                drivers/delay_timer/delay_timer.c                     \
                drivers/gpio/gpio.c                                   \
                drivers/delay_timer/generic_delay_timer.c             \
                drivers/ti/uart/aarch64/16550_console.S               \
                lib/bl_aux_params/bl_aux_params.c                     \
                lib/cpus/aarch64/cortex_a55.S                         \
                lib/cpus/aarch64/cortex_a78.S                         \
                plat/common/plat_gicv3.c                              \
                ${MTK_PLAT}/common/drivers/gic600/mt_gic_v3.c         \
                ${MTK_PLAT}/common/drivers/gpio/mtgpio_common.c       \
                ${MTK_PLAT}/common/drivers/pmic_wrap/pmic_wrap_init_v2.c \
                ${MTK_PLAT}/common/drivers/rtc/rtc_common.c           \
                ${MTK_PLAT}/common/drivers/rtc/rtc_mt6359p.c          \
                ${MTK_PLAT}/common/drivers/timer/mt_timer.c           \
		${MTK_PLAT}/common/drivers/uart/uart.c                \
		${MTK_PLAT}/common/lpm/mt_lp_rm.c                     \
                ${MTK_PLAT}/common/mtk_cirq.c                         \
                ${MTK_PLAT}/common/mtk_plat_common.c                  \
                ${MTK_PLAT}/common/mtk_sip_svc.c                      \
                ${MTK_PLAT}/common/params_setup.c                     \
                ${MTK_PLAT_SOC}/aarch64/platform_common.c             \
                ${MTK_PLAT_SOC}/aarch64/plat_helpers.S                \
                ${MTK_PLAT_SOC}/bl31_plat_setup.c                     \
                ${MTK_PLAT_SOC}/drivers/apusys/apupll.c               \
                ${MTK_PLAT_SOC}/drivers/apusys/apupwr_clkctl.c        \
		${MTK_PLAT_SOC}/drivers/apusys/mtk_apusys.c           \
                ${MTK_PLAT_SOC}/drivers/dcm/mtk_dcm.c                 \
                ${MTK_PLAT_SOC}/drivers/dcm/mtk_dcm_utils.c           \
                ${MTK_PLAT_SOC}/drivers/dfd/plat_dfd.c                \
                ${MTK_PLAT_SOC}/drivers/dp/mt_dp.c                    \
                ${MTK_PLAT_SOC}/drivers/emi_mpu/emi_mpu.c             \
                ${MTK_PLAT_SOC}/drivers/gpio/mtgpio.c                 \
                ${MTK_PLAT_SOC}/drivers/mcdi/mt_cpu_pm.c              \
                ${MTK_PLAT_SOC}/drivers/mcdi/mt_cpu_pm_cpc.c          \
                ${MTK_PLAT_SOC}/drivers/mcdi/mt_mcdi.c                \
		${MTK_PLAT_SOC}/drivers/mcdi/mt_lp_irqremain.c        \
                ${MTK_PLAT_SOC}/drivers/gpio/mtgpio.c                 \
                ${MTK_PLAT_SOC}/drivers/pmic/pmic.c                   \
                ${MTK_PLAT_SOC}/drivers/ptp3/mtk_ptp3_main.c          \
                ${MTK_PLAT_SOC}/drivers/spmc/mtspmc.c                 \
                ${MTK_PLAT_SOC}/plat_pm.c                             \
                ${MTK_PLAT_SOC}/plat_sip_calls.c                      \
                ${MTK_PLAT_SOC}/plat_topology.c

# Build SPM drivers
include ${MTK_PLAT_SOC}/drivers/spm/build.mk

# Configs for A78 and A55
HW_ASSISTED_COHERENCY := 1
USE_COHERENT_MEM := 0
CTX_INCLUDE_AARCH32_REGS := 0
ERRATA_A55_1530923 := 1

ERRATA_A78_1688305 := 1
ERRATA_A78_1941498 := 1
ERRATA_A78_1951500 := 1
ERRATA_A78_1821534 := 1
ERRATA_A78_2132060 := 1
ERRATA_A78_2242635 := 1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS := 1

COLD_BOOT_SINGLE_CPU := 1

MACH_MT8195 := 1
$(eval $(call add_define,MACH_MT8195))

include lib/coreboot/coreboot.mk
