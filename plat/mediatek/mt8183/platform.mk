#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT      := plat/mediatek
MTK_PLAT_SOC  := ${MTK_PLAT}/${PLAT}

PLAT_INCLUDES := -I${MTK_PLAT}/common/                            \
                 -I${MTK_PLAT_SOC}/drivers/                       \
                 -I${MTK_PLAT_SOC}/drivers/spmc/                  \
                 -I${MTK_PLAT_SOC}/drivers/gpio/                  \
                 -I${MTK_PLAT_SOC}/drivers/pmic/                  \
                 -I${MTK_PLAT_SOC}/drivers/rtc/                   \
                 -I${MTK_PLAT_SOC}/include/

PLAT_BL_COMMON_SOURCES := lib/xlat_tables/aarch64/xlat_tables.c	      \
                          lib/xlat_tables/xlat_tables_common.c        \
                          plat/common/plat_gicv2.c                    \
                          plat/common/aarch64/crash_console_helpers.S

BL31_SOURCES    += drivers/arm/cci/cci.c                                 \
                   drivers/arm/gic/common/gic_common.c                   \
                   drivers/arm/gic/v2/gicv2_main.c                       \
                   drivers/arm/gic/v2/gicv2_helpers.c                    \
                   drivers/delay_timer/delay_timer.c                     \
                   drivers/delay_timer/generic_delay_timer.c             \
                   drivers/gpio/gpio.c                                   \
                   drivers/ti/uart/aarch64/16550_console.S               \
                   lib/cpus/aarch64/aem_generic.S                        \
                   lib/cpus/aarch64/cortex_a53.S                         \
                   lib/cpus/aarch64/cortex_a73.S                         \
                   ${MTK_PLAT}/common/mtk_plat_common.c                  \
                   ${MTK_PLAT}/common/drivers/pmic_wrap/pmic_wrap_init.c \
                   ${MTK_PLAT}/common/params_setup.c                     \
                   ${MTK_PLAT_SOC}/aarch64/plat_helpers.S                \
                   ${MTK_PLAT_SOC}/aarch64/platform_common.c             \
                   ${MTK_PLAT_SOC}/drivers/mcsi/mcsi.c                   \
                   ${MTK_PLAT_SOC}/drivers/pmic/pmic.c                   \
                   ${MTK_PLAT_SOC}/drivers/rtc/rtc.c                     \
                   ${MTK_PLAT_SOC}/drivers/spmc/mtspmc.c                 \
                   ${MTK_PLAT_SOC}/drivers/gpio/mtgpio.c                 \
                   ${MTK_PLAT_SOC}/plat_pm.c                             \
                   ${MTK_PLAT_SOC}/plat_topology.c                       \
                   ${MTK_PLAT_SOC}/plat_mt_gic.c                         \
                   ${MTK_PLAT_SOC}/plat_dcm.c                            \
                   ${MTK_PLAT_SOC}/bl31_plat_setup.c                     \
                   ${MTK_PLAT_SOC}/plat_debug.c                          \
                   ${MTK_PLAT_SOC}/scu.c

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319 := 0
ERRATA_A53_836870 := 1
ERRATA_A53_855873 := 1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS := 1

COLD_BOOT_SINGLE_CPU := 1

MULTI_CONSOLE_API := 1

CONFIG_MACH_MT8183 := 1
$(eval $(call add_define,CONFIG_MACH_MT8183))

