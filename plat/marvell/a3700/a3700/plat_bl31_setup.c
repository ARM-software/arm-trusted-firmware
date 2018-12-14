/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <lib/mmio.h>

#include <armada_common.h>
#include <dram_win.h>
#include <io_addr_dec.h>
#include <marvell_plat_priv.h>
#include <plat_marvell.h>

/* This routine does MPP initialization */
static void marvell_bl31_mpp_init(void)
{
	mmio_clrbits_32(MVEBU_NB_GPIO_SEL_REG, 1 << MVEBU_GPIO_TW1_GPIO_EN_OFF);

	/* Set hidden GPIO setting for SPI.
	 * In north_bridge_pin_out_en_high register 13804,
	 * bit 28 is the one which enables CS, CLK pins to be
	 * output, need to set it to 1.
	 * The initial value of this bit is 1, but in UART boot mode
	 * initialization, this bit is disabled and the SPI CS and CLK pins
	 * are used for downloading image purpose; so after downloading,
	 * we should set this bit to 1 again to enable SPI CS and CLK pins.
	 * And anyway, this bit value should be 1 in all modes,
	 * so here we does not judge boot mode and set this bit to 1 always.
	 */
	mmio_setbits_32(MVEBU_NB_GPIO_OUTPUT_EN_HIGH_REG,
			1 << MVEBU_GPIO_NB_SPI_PIN_MODE_OFF);
}

/* This function overruns the same function in marvell_bl31_setup.c */
void bl31_plat_arch_setup(void)
{
	struct dec_win_config *io_dec_map;
	uint32_t dec_win_num;
	struct dram_win_map dram_wins_map;

	marvell_bl31_plat_arch_setup();

	/* MPP init */
	marvell_bl31_mpp_init();

	/* initialize the timer for delay functionality */
	plat_delay_timer_init();

	/* CPU address decoder windows initialization. */
	cpu_wins_init();

	/* fetch CPU-DRAM window mapping information by reading
	 * CPU-DRAM decode windows (only the enabled ones)
	 */
	dram_win_map_build(&dram_wins_map);

	/* Get IO address decoder windows */
	if (marvell_get_io_dec_win_conf(&io_dec_map, &dec_win_num)) {
		printf("No IO address decoder windows configurations found!\n");
		return;
	}

	/* IO address decoder init */
	if (init_io_addr_dec(&dram_wins_map, io_dec_map, dec_win_num)) {
		printf("IO address decoder windows initialization failed!\n");
		return;
	}
}
