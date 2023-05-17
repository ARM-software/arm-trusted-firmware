/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/cadence/cdns_nand.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include "nand.h"

#include "agilex5_pinmux.h"
#include "combophy/combophy.h"

/* Pinmux configuration */
static void nand_pinmux_config(void)
{
	mmio_write_32(SOCFPGA_PINMUX(PIN0SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN1SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN2SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN3SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN4SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN5SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN6SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN7SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN8SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN9SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN10SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN11SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN12SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN13SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN14SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN16SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN17SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN18SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN19SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN20SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN21SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN22SEL), SOCFPGA_PINMUX_SEL_NAND);
	mmio_write_32(SOCFPGA_PINMUX(PIN23SEL), SOCFPGA_PINMUX_SEL_NAND);
}

int nand_init(handoff *hoff_ptr)
{
	/* NAND pin mux configuration */
	nand_pinmux_config();

	return cdns_nand_host_init();
}
