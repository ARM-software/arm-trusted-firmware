/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include "agilex5_pinmux.h"
#include "agilex5_system_manager.h"

const uint32_t sysmgr_pinmux_array_sel[] = {
	0x00000000, 0x00000001, /* usb */
	0x00000004, 0x00000001,
	0x00000008, 0x00000001,
	0x0000000c, 0x00000001,
	0x00000010, 0x00000001,
	0x00000014, 0x00000001,
	0x00000018, 0x00000001,
	0x0000001c, 0x00000001,
	0x00000020, 0x00000001,
	0x00000024, 0x00000001,
	0x00000028, 0x00000001,
	0x0000002c, 0x00000001,
	0x00000030, 0x00000000, /* emac0 */
	0x00000034, 0x00000000,
	0x00000038, 0x00000000,
	0x0000003c, 0x00000000,
	0x00000040, 0x00000000,
	0x00000044, 0x00000000,
	0x00000048, 0x00000000,
	0x0000004c, 0x00000000,
	0x00000050, 0x00000000,
	0x00000054, 0x00000000,
	0x00000058, 0x00000000,
	0x0000005c, 0x00000000,
	0x00000060, 0x00000008, /* gpio1 */
	0x00000064, 0x00000008,
	0x00000068, 0x00000005, /* uart0 tx */
	0x0000006c, 0x00000005, /* uart 0 rx */
	0x00000070, 0x00000008, /* gpio */
	0x00000074, 0x00000008,
	0x00000078, 0x00000004, /* i2c1 */
	0x0000007c, 0x00000004,
	0x00000080, 0x00000007, /* jtag */
	0x00000084, 0x00000007,
	0x00000088, 0x00000007,
	0x0000008c, 0x00000007,
	0x00000090, 0x00000001, /* sdmmc data0 */
	0x00000094, 0x00000001,
	0x00000098, 0x00000001,
	0x0000009c, 0x00000001,
	0x00000100, 0x00000001,
	0x00000104, 0x00000001, /* sdmmc.data3 */
	0x00000108, 0x00000008, /* loan */
	0x0000010c, 0x00000008, /* gpio */
	0x00000110, 0x00000008,
	0x00000114, 0x00000008, /* gpio1.io21 */
	0x00000118, 0x00000005, /* mdio0.mdio */
	0x0000011c, 0x00000005  /* mdio0.mdc */
};

const uint32_t sysmgr_pinmux_array_ctrl[] = {
	0x00000000, 0x00502c38, /* Q1_1 */
	0x00000004, 0x00102c38,
	0x00000008, 0x00502c38,
	0x0000000c, 0x00502c38,
	0x00000010, 0x00502c38,
	0x00000014, 0x00502c38,
	0x00000018, 0x00502c38,
	0x0000001c, 0x00502c38,
	0x00000020, 0x00502c38,
	0x00000024, 0x00502c38,
	0x00000028, 0x00502c38,
	0x0000002c, 0x00502c38,
	0x00000030, 0x00102c38, /* Q2_1 */
	0x00000034, 0x00102c38,
	0x00000038, 0x00502c38,
	0x0000003c, 0x00502c38,
	0x00000040, 0x00102c38,
	0x00000044, 0x00102c38,
	0x00000048, 0x00502c38,
	0x0000004c, 0x00502c38,
	0x00000050, 0x00102c38,
	0x00000054, 0x00102c38,
	0x00000058, 0x00502c38,
	0x0000005c, 0x00502c38,
	0x00000060, 0x00502c38, /* Q3_1 */
	0x00000064, 0x00502c38,
	0x00000068, 0x00102c38,
	0x0000006c, 0x00502c38,
	0x000000d0, 0x00502c38,
	0x000000d4, 0x00502c38,
	0x000000d8, 0x00542c38,
	0x000000dc, 0x00542c38,
	0x000000e0, 0x00502c38,
	0x000000e4, 0x00502c38,
	0x000000e8, 0x00102c38,
	0x000000ec, 0x00502c38,
	0x000000f0, 0x00502c38, /* Q4_1 */
	0x000000f4, 0x00502c38,
	0x000000f8, 0x00102c38,
	0x000000fc, 0x00502c38,
	0x00000100, 0x00502c38,
	0x00000104, 0x00502c38,
	0x00000108, 0x00102c38,
	0x0000010c, 0x00502c38,
	0x00000110, 0x00502c38,
	0x00000114, 0x00502c38,
	0x00000118, 0x00542c38,
	0x0000011c, 0x00102c38
};

const uint32_t sysmgr_pinmux_array_fpga[] = {
	0x00000000, 0x00000000,
	0x00000004, 0x00000000,
	0x00000008, 0x00000000,
	0x0000000c, 0x00000000,
	0x00000010, 0x00000000,
	0x00000014, 0x00000000,
	0x00000018, 0x00000000,
	0x0000001c, 0x00000000,
	0x00000020, 0x00000000,
	0x00000028, 0x00000000,
	0x0000002c, 0x00000000,
	0x00000030, 0x00000000,
	0x00000034, 0x00000000,
	0x00000038, 0x00000000,
	0x0000003c, 0x00000000,
	0x00000040, 0x00000000,
	0x00000044, 0x00000000,
	0x00000048, 0x00000000,
	0x00000050, 0x00000000,
	0x00000054, 0x00000000,
	0x00000058, 0x0000002a
};

const uint32_t sysmgr_pinmux_array_iodelay[] = {
	0x00000000, 0x00000000,
	0x00000004, 0x00000000,
	0x00000008, 0x00000000,
	0x0000000c, 0x00000000,
	0x00000010, 0x00000000,
	0x00000014, 0x00000000,
	0x00000018, 0x00000000,
	0x0000001c, 0x00000000,
	0x00000020, 0x00000000,
	0x00000024, 0x00000000,
	0x00000028, 0x00000000,
	0x0000002c, 0x00000000,
	0x00000030, 0x00000000,
	0x00000034, 0x00000000,
	0x00000038, 0x00000000,
	0x0000003c, 0x00000000,
	0x00000040, 0x00000000,
	0x00000044, 0x00000000,
	0x00000048, 0x00000000,
	0x0000004c, 0x00000000,
	0x00000050, 0x00000000,
	0x00000054, 0x00000000,
	0x00000058, 0x00000000,
	0x0000005c, 0x00000000,
	0x00000060, 0x00000000,
	0x00000064, 0x00000000,
	0x00000068, 0x00000000,
	0x0000006c, 0x00000000,
	0x00000070, 0x00000000,
	0x00000074, 0x00000000,
	0x00000078, 0x00000000,
	0x0000007c, 0x00000000,
	0x00000080, 0x00000000,
	0x00000084, 0x00000000,
	0x00000088, 0x00000000,
	0x0000008c, 0x00000000,
	0x00000090, 0x00000000,
	0x00000094, 0x00000000,
	0x00000098, 0x00000000,
	0x0000009c, 0x00000000,
	0x00000100, 0x00000000,
	0x00000104, 0x00000000,
	0x00000108, 0x00000000,
	0x0000010c, 0x00000000,
	0x00000110, 0x00000000,
	0x00000114, 0x00000000,
	0x00000118, 0x00000000,
	0x0000011c, 0x00000000
};

static void config_fpgaintf_mod(void)
{
	uint32_t fpgaintf_en_val;

	/*
	 * System manager FPGA interface enable2 register, disable individual
	 * interfaces between the FPGA and HPS.
	 */
	fpgaintf_en_val = 0U;
	if ((mmio_read_32(SOCFPGA_PINUMX_USEFPGA(NAND_USEFPGA)) & 0x01) != 0)
		fpgaintf_en_val |= BIT(4);
	if ((mmio_read_32(SOCFPGA_PINUMX_USEFPGA(SDMMC_USEFPGA)) & 0x01) != 0)
		fpgaintf_en_val |= BIT(8);
	if ((mmio_read_32(SOCFPGA_PINUMX_USEFPGA(SPIM0_USEFPGA)) & 0x01) != 0)
		fpgaintf_en_val |= BIT(16);
	if ((mmio_read_32(SOCFPGA_PINUMX_USEFPGA(SPIM1_USEFPGA)) & 0x01) != 0)
		fpgaintf_en_val |= BIT(24);
	mmio_write_32(SOCFPGA_SYSMGR(FPGAINTF_EN_2), fpgaintf_en_val);

	/*
	 * System manager FPGA interface enable3 register, disable individual
	 * interfaces between the FPGA and HPS.
	 */
	fpgaintf_en_val = 0U;
	if ((mmio_read_32(SOCFPGA_PINUMX_USEFPGA(EMAC0_USEFPGA)) & 0x01) != 0)
		fpgaintf_en_val |= BIT(0);
	if ((mmio_read_32(SOCFPGA_PINUMX_USEFPGA(EMAC1_USEFPGA)) & 0x01) != 0)
		fpgaintf_en_val |= BIT(8);
	if ((mmio_read_32(SOCFPGA_PINUMX_USEFPGA(EMAC2_USEFPGA)) & 0x01) != 0)
		fpgaintf_en_val |= BIT(16);
	mmio_write_32(SOCFPGA_SYSMGR(FPGAINTF_EN_3), fpgaintf_en_val);
}

void config_pinmux(handoff *hoff_ptr)
{
	uint32_t i;

	/* Configure the pin selection */
	for (i = 0; i < ARRAY_SIZE(hoff_ptr->pinmux_sel_array); i += 2) {
		mmio_write_32(AGX5_PINMUX_PIN0SEL + hoff_ptr->pinmux_sel_array[i],
			      hoff_ptr->pinmux_sel_array[i+1]);
	}

	/* Configure the pin control */
	for (i = 0; i < ARRAY_SIZE(hoff_ptr->pinmux_io_array); i += 2) {
		mmio_write_32(AGX5_PINMUX_IO0CTRL + hoff_ptr->pinmux_io_array[i],
			      hoff_ptr->pinmux_io_array[i+1]);
	}

	/*
	 * Configure the FPGA use.
	 * The actual generic handoff contains extra 4 elements, and these 4 elements
	 * are not applicable to the Agilex5 platform. Writing these extra 4 elements
	 * will cause the system to crash, so let's avoid writing them here.
	 */
	for (i = 0; i < (ARRAY_SIZE(hoff_ptr->pinmux_fpga_array) - 4); i += 2) {
		mmio_write_32(AGX5_PINMUX_EMAC0_USEFPGA + hoff_ptr->pinmux_fpga_array[i],
			      hoff_ptr->pinmux_fpga_array[i+1]);
	}

	/* Configure the IO delay */
	for (i = 0; i < ARRAY_SIZE(hoff_ptr->pinmux_iodelay_array); i += 2) {
		mmio_write_32(AGX5_PINMUX_IO0_DELAY + hoff_ptr->pinmux_iodelay_array[i],
			      hoff_ptr->pinmux_iodelay_array[i+1]);
	}

	/* Enable/Disable individual interfaces between the FPGA and HPS */
	config_fpgaintf_mod();
}
