#
# Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
#
# SPDX-License-Identifier: BSD-3-Clause
#

# We need to enable this for robust clocking
CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION := 1
$(eval $(call add_define,CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION))

INCLUDES += -Idrivers/ti/clk/include \
	    -Iinclude/drivers \
	    -Idrivers/ti/common/include \
	    -Idrivers/ti/common/pm/include

BL31_SOURCES += drivers/ti/clk/ti_clk_handler.c  	  		\
		drivers/ti/clk/ti_clk.c  		  		\
		drivers/ti/clk/ti_clk_dev.c 		   		\
		drivers/ti/clk/ti_clk_div.c    				\
		drivers/ti/clk/ti_clk_fixed.c    			\
		drivers/ti/clk/ti_clk_mux.c    				\
		drivers/ti/clk/ti_clk_pll_16fft.c    			\
		drivers/ti/clk/ti_clk_pllctrl.c    			\
		drivers/ti/clk/ti_clk_soc_hfosc0.c   			\
		drivers/ti/clk/ti_clk_soc_lfosc0.c   			\
		drivers/ti/clk/ti_pll.c    				\
		drivers/delay_timer/delay_timer.c

