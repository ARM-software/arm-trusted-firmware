#
# Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		:=	-Iplat/nvidia/tegra/include/drivers \
				-Iplat/nvidia/tegra/include/lib \
				-Iplat/nvidia/tegra/include \
				-Iplat/nvidia/tegra/include/${TARGET_SOC}

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

COMMON_DIR		:=	plat/nvidia/tegra/common

TEGRA_GICv2_SOURCES	:=	drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				plat/common/plat_gicv2.c			\
				${COMMON_DIR}/tegra_gicv2.c

BL31_SOURCES		+=	drivers/console/aarch64/console.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/io/io_storage.c				\
				${TEGRA_GICv2_SOURCES}				\
				${COMMON_DIR}/aarch64/tegra_helpers.S		\
				${COMMON_DIR}/drivers/pmc/pmc.c			\
				${COMMON_DIR}/lib/debug/profiler.c		\
				${COMMON_DIR}/tegra_bl31_setup.c		\
				${COMMON_DIR}/tegra_delay_timer.c		\
				${COMMON_DIR}/tegra_fiq_glue.c			\
				${COMMON_DIR}/tegra_io_storage.c		\
				${COMMON_DIR}/tegra_platform.c			\
				${COMMON_DIR}/tegra_pm.c			\
				${COMMON_DIR}/tegra_sip_calls.c			\
				${COMMON_DIR}/tegra_topology.c
