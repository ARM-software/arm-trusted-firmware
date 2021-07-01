/*
 * Copyright (c) 2019-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/st/bsec.h>
#include <drivers/st/stpmic1.h>
#include <lib/mmio.h>

#include <stm32mp_dt.h>
#include <stm32mp1_private.h>

/*
 * SYSCFG REGISTER OFFSET (base relative)
 */
#define SYSCFG_BOOTR				0x00U
#define SYSCFG_IOCTRLSETR			0x18U
#define SYSCFG_ICNR				0x1CU
#define SYSCFG_CMPCR				0x20U
#define SYSCFG_CMPENSETR			0x24U
#define SYSCFG_CMPENCLRR			0x28U

/*
 * SYSCFG_BOOTR Register
 */
#define SYSCFG_BOOTR_BOOT_MASK			GENMASK(2, 0)
#define SYSCFG_BOOTR_BOOTPD_MASK		GENMASK(6, 4)
#define SYSCFG_BOOTR_BOOTPD_SHIFT		4
/*
 * SYSCFG_IOCTRLSETR Register
 */
#define SYSCFG_IOCTRLSETR_HSLVEN_TRACE		BIT(0)
#define SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI	BIT(1)
#define SYSCFG_IOCTRLSETR_HSLVEN_ETH		BIT(2)
#define SYSCFG_IOCTRLSETR_HSLVEN_SDMMC		BIT(3)
#define SYSCFG_IOCTRLSETR_HSLVEN_SPI		BIT(4)

/*
 * SYSCFG_ICNR Register
 */
#define SYSCFG_ICNR_AXI_M9			BIT(9)

/*
 * SYSCFG_CMPCR Register
 */
#define SYSCFG_CMPCR_SW_CTRL			BIT(1)
#define SYSCFG_CMPCR_READY			BIT(8)
#define SYSCFG_CMPCR_RANSRC			GENMASK(19, 16)
#define SYSCFG_CMPCR_RANSRC_SHIFT		16
#define SYSCFG_CMPCR_RAPSRC			GENMASK(23, 20)
#define SYSCFG_CMPCR_ANSRC_SHIFT		24

/*
 * SYSCFG_CMPENSETR Register
 */
#define SYSCFG_CMPENSETR_MPU_EN			BIT(0)

void stm32mp1_syscfg_init(void)
{
	uint32_t bootr;
	uint32_t otp = 0;
	uint32_t vdd_voltage;

	/*
	 * Interconnect update : select master using the port 1.
	 * LTDC = AXI_M9.
	 */
	mmio_write_32(SYSCFG_BASE + SYSCFG_ICNR, SYSCFG_ICNR_AXI_M9);

	/* Disable Pull-Down for boot pin connected to VDD */
	bootr = mmio_read_32(SYSCFG_BASE + SYSCFG_BOOTR) &
		SYSCFG_BOOTR_BOOT_MASK;
	mmio_clrsetbits_32(SYSCFG_BASE + SYSCFG_BOOTR, SYSCFG_BOOTR_BOOTPD_MASK,
			   bootr << SYSCFG_BOOTR_BOOTPD_SHIFT);

	/*
	 * High Speed Low Voltage Pad mode Enable for SPI, SDMMC, ETH, QSPI
	 * and TRACE. Needed above ~50MHz and conditioned by AFMUX selection.
	 * It could be disabled for low frequencies or if AFMUX is selected
	 * but the function is not used, typically for TRACE.
	 * If high speed low voltage pad mode is node enable, platform will
	 * over consume.
	 *
	 * WARNING:
	 *   Enabling High Speed mode while VDD > 2.7V
	 *   with the OTP product_below_2v5 (OTP 18, BIT 13)
	 *   erroneously set to 1 can damage the SoC!
	 *   => TF-A enables the low power mode only if VDD < 2.7V (in DT)
	 *      but this value needs to be consistent with board design.
	 */
	if (bsec_read_otp(&otp, HW2_OTP) != BSEC_OK) {
		panic();
	}

	otp = otp & HW2_OTP_PRODUCT_BELOW_2V5;

	/* Get VDD supply */
	vdd_voltage = dt_get_pwr_vdd_voltage();

	/* Check if VDD is Low Voltage */
	if (vdd_voltage == 0U) {
		WARN("VDD unknown");
	} else if (vdd_voltage < 2700000U) {
		mmio_write_32(SYSCFG_BASE + SYSCFG_IOCTRLSETR,
			      SYSCFG_IOCTRLSETR_HSLVEN_TRACE |
			      SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI |
			      SYSCFG_IOCTRLSETR_HSLVEN_ETH |
			      SYSCFG_IOCTRLSETR_HSLVEN_SDMMC |
			      SYSCFG_IOCTRLSETR_HSLVEN_SPI);

		if (otp == 0U) {
			INFO("Product_below_2v5=0: HSLVEN protected by HW\n");
		}
	} else {
		if (otp != 0U) {
			ERROR("Product_below_2v5=1:\n");
			ERROR("\tHSLVEN update is destructive,\n");
			ERROR("\tno update as VDD > 2.7V\n");
			panic();
		}
	}

	stm32mp1_syscfg_enable_io_compensation();
}

void stm32mp1_syscfg_enable_io_compensation(void)
{
	/*
	 * Activate automatic I/O compensation.
	 * Warning: need to ensure CSI enabled and ready in clock driver.
	 * Enable non-secure clock, we assume non-secure is suspended.
	 */
	stm32mp1_clk_enable_non_secure(SYSCFG);

	mmio_setbits_32(SYSCFG_BASE + SYSCFG_CMPENSETR,
			SYSCFG_CMPENSETR_MPU_EN);

	while ((mmio_read_32(SYSCFG_BASE + SYSCFG_CMPCR) &
		SYSCFG_CMPCR_READY) == 0U) {
		;
	}

	mmio_clrbits_32(SYSCFG_BASE + SYSCFG_CMPCR, SYSCFG_CMPCR_SW_CTRL);
}

void stm32mp1_syscfg_disable_io_compensation(void)
{
	uint32_t value;

	/*
	 * Deactivate automatic I/O compensation.
	 * Warning: CSI is disabled automatically in STOP if not
	 * requested for other usages and always OFF in STANDBY.
	 * Disable non-secure SYSCFG clock, we assume non-secure is suspended.
	 */
	value = mmio_read_32(SYSCFG_BASE + SYSCFG_CMPCR) >>
	      SYSCFG_CMPCR_ANSRC_SHIFT;

	mmio_clrbits_32(SYSCFG_BASE + SYSCFG_CMPCR,
			SYSCFG_CMPCR_RANSRC | SYSCFG_CMPCR_RAPSRC);

	value = mmio_read_32(SYSCFG_BASE + SYSCFG_CMPCR) |
		(value << SYSCFG_CMPCR_RANSRC_SHIFT);

	mmio_write_32(SYSCFG_BASE + SYSCFG_CMPCR, value | SYSCFG_CMPCR_SW_CTRL);

	mmio_setbits_32(SYSCFG_BASE + SYSCFG_CMPENCLRR, SYSCFG_CMPENSETR_MPU_EN);

	stm32mp1_clk_disable_non_secure(SYSCFG);
}
