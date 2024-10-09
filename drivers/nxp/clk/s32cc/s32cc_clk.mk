#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		+= \
	-I${PLAT_DRIVERS_INCLUDE_PATH}/clk/s32cc \
	-I${PLAT_DRIVERS_PATH}/clk/s32cc/include \

CLK_SOURCES		:= \
	${PLAT_DRIVERS_PATH}/clk/s32cc/mc_rgm.c \
	${PLAT_DRIVERS_PATH}/clk/s32cc/mc_me.c \
	${PLAT_DRIVERS_PATH}/clk/s32cc/s32cc_clk_drv.c \
	${PLAT_DRIVERS_PATH}/clk/s32cc/s32cc_clk_modules.c \
	${PLAT_DRIVERS_PATH}/clk/s32cc/s32cc_clk_utils.c \
	${PLAT_DRIVERS_PATH}/clk/s32cc/s32cc_early_clks.c \
	drivers/clk/clk.c \

ifeq (${BL_COMM_CLK_NEEDED},yes)
BL2_SOURCES		+= ${CLK_SOURCES}
endif
