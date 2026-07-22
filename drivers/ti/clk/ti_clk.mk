#
# Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier: BSD-3-Clause
#

INCLUDES += -Idrivers/ti/common/include \
	    -Idrivers/ti/common/pm/include \
	    -Idrivers/ti/clk/include

BL31_SOURCES += drivers/ti/clk/ti_clk_mux.c \
		drivers/ti/clk/ti_clk_div.c \
		drivers/ti/clk/ti_clk_fixed.c \
		drivers/ti/clk/ti_clk_pllctrl.c \
		drivers/ti/clk/ti_pll.c \
		drivers/ti/clk/ti_clk_pll_16fft.c \
		drivers/ti/clk/ti_clk_soc_hfosc0.c \
		drivers/ti/clk/ti_clk_soc_lfosc0.c \
		drivers/ti/clk/ti_clk.c \
		drivers/ti/clk/ti_clk_dev.c \
		drivers/ti/clk/ti_clk_handler.c
