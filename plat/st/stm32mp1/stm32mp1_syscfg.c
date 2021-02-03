/*
 * Copyright (c) 2019-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stpmic1.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>
#include <stm32mp_common.h>
#include <stm32mp_dt.h>
#include <stm32mp1_private.h>

/*
 * SYSCFG REGISTER OFFSET (base relative)
 */
#define SYSCFG_BOOTR				0x00U
#define SYSCFG_BOOTCR				0x0CU
#if STM32MP15
#define SYSCFG_IOCTRLSETR			0x18U
#define SYSCFG_ICNR				0x1CU
#endif
#define SYSCFG_CMPCR				0x20U
#define SYSCFG_CMPENSETR			0x24U
#define SYSCFG_CMPENCLRR			0x28U
#if STM32MP13
#define SYSCFG_CMPSD1CR				0x30U
#define SYSCFG_CMPSD1ENSETR			0x34U
#define SYSCFG_CMPSD1ENCLRR			0x38U
#define SYSCFG_CMPSD2CR				0x40U
#define SYSCFG_CMPSD2ENSETR			0x44U
#define SYSCFG_CMPSD2ENCLRR			0x48U
#define SYSCFG_HSLVEN0R				0x50U
#endif
#define SYSCFG_IDC				0x380U

#define CMPCR_CMPENSETR_OFFSET			0x4U
#define CMPCR_CMPENCLRR_OFFSET			0x8U

/*
 * SYSCFG_BOOTR Register
 */
#define SYSCFG_BOOTR_BOOT_MASK			GENMASK(2, 0)
#if STM32MP15
#define SYSCFG_BOOTR_BOOTPD_MASK		GENMASK(6, 4)
#define SYSCFG_BOOTR_BOOTPD_SHIFT		4
#endif

/*
 * SYSCFG_BOOTCR Register
 */
#define SYSCFG_BOOTCR_BMEN			BIT(0)

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

#define SYSCFG_CMPCR_READY_TIMEOUT_US		10000U

/*
 * SYSCFG_CMPENSETR Register
 */
#define SYSCFG_CMPENSETR_MPU_EN			BIT(0)

/*
 * HSLV definitions
 */
#define HSLV_IDX_TPIU				0U
#define HSLV_IDX_QSPI				1U
#define HSLV_IDX_ETH1				2U
#define HSLV_IDX_ETH2				3U
#define HSLV_IDX_SDMMC1				4U
#define HSLV_IDX_SDMMC2				5U
#define HSLV_IDX_SPI1				6U
#define HSLV_IDX_SPI2				7U
#define HSLV_IDX_SPI3				8U
#define HSLV_IDX_SPI4				9U
#define HSLV_IDX_SPI5				10U
#define HSLV_IDX_LTDC				11U
#define HSLV_NB_IDX				12U

#define HSLV_KEY				0x1018U

/*
 * SYSCFG_IDC Register
 */
#define SYSCFG_IDC_DEV_ID_MASK			GENMASK(11, 0)
#define SYSCFG_IDC_REV_ID_MASK			GENMASK(31, 16)
#define SYSCFG_IDC_REV_ID_SHIFT			16

static void enable_io_comp_cell_finish(uintptr_t cmpcr_off)
{
	uint64_t start;

	start = timeout_init_us(SYSCFG_CMPCR_READY_TIMEOUT_US);

	while ((mmio_read_32(SYSCFG_BASE + cmpcr_off) & SYSCFG_CMPCR_READY) == 0U) {
		if (timeout_elapsed(start)) {
			/* Failure on IO compensation enable is not a issue: warn only. */
			WARN("IO compensation cell not ready\n");
			break;
		}
	}

	mmio_clrbits_32(SYSCFG_BASE + cmpcr_off, SYSCFG_CMPCR_SW_CTRL);
}

static void disable_io_comp_cell(uintptr_t cmpcr_off)
{
	uint32_t value;

	if (((mmio_read_32(SYSCFG_BASE + cmpcr_off) & SYSCFG_CMPCR_READY) == 0U) ||
	    ((mmio_read_32(SYSCFG_BASE + cmpcr_off + CMPCR_CMPENSETR_OFFSET) &
	     SYSCFG_CMPENSETR_MPU_EN) == 0U)) {
		return;
	}

	value = mmio_read_32(SYSCFG_BASE + cmpcr_off) >> SYSCFG_CMPCR_ANSRC_SHIFT;

	mmio_clrbits_32(SYSCFG_BASE + cmpcr_off, SYSCFG_CMPCR_RANSRC | SYSCFG_CMPCR_RAPSRC);

	value <<= SYSCFG_CMPCR_RANSRC_SHIFT;
	value |= mmio_read_32(SYSCFG_BASE + cmpcr_off);

	mmio_write_32(SYSCFG_BASE + cmpcr_off, value | SYSCFG_CMPCR_SW_CTRL);

	mmio_setbits_32(SYSCFG_BASE + cmpcr_off + CMPCR_CMPENCLRR_OFFSET, SYSCFG_CMPENSETR_MPU_EN);
}

#if STM32MP13
static int get_regu_max_voltage(void *fdt, int sdmmc_node,
				const char *regu_name, uint32_t *regu_val)
{
	int node;
	const fdt32_t *cuint;

	cuint = fdt_getprop(fdt, sdmmc_node, regu_name, NULL);
	if (cuint == NULL) {
		return -ENODEV;
	}

	node = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*cuint));
	if (node < 0) {
		return -ENODEV;
	}

	cuint = fdt_getprop(fdt, node, "regulator-max-microvolt", NULL);
	if (cuint == NULL) {
		return -ENODEV;
	}

	*regu_val = fdt32_to_cpu(*cuint);

	return 0;
}

static bool sdmmc_is_low_voltage(uintptr_t sdmmc_base)
{
	int ret;
	int node;
	void *fdt = NULL;
	uint32_t regu_max_val;

	if (fdt_get_address(&fdt) == 0) {
		return false;
	}

	if (fdt == NULL) {
		return false;
	}

	node = dt_match_instance_by_compatible(DT_SDMMC2_COMPAT, sdmmc_base);
	if (node < 0) {
		/* No SD or eMMC device on this instance, enable HSLV */
		return true;
	}

	ret = get_regu_max_voltage(fdt, node, "vqmmc-supply", &regu_max_val);
	if ((ret < 0) || (regu_max_val > 1800000U)) {
		/*
		 * The vqmmc-supply property should always be present for eMMC.
		 * For SD-card, if it is not, then the card only supports 3.3V.
		 */
		return false;
	}

	return true;
}

static void enable_hslv_by_index(uint32_t index)
{
	bool apply_hslv;

	assert(index < HSLV_NB_IDX);

	switch (index) {
	case HSLV_IDX_SDMMC1:
		apply_hslv = sdmmc_is_low_voltage(STM32MP_SDMMC1_BASE);
		break;
	case HSLV_IDX_SDMMC2:
		apply_hslv = sdmmc_is_low_voltage(STM32MP_SDMMC2_BASE);
		break;
	default:
		apply_hslv = true;
		break;
	}

	if (apply_hslv) {
		mmio_write_32(SYSCFG_BASE + SYSCFG_HSLVEN0R + index * sizeof(uint32_t), HSLV_KEY);
	}
}
#endif

static void enable_high_speed_mode_low_voltage(void)
{
#if STM32MP13
	uint32_t idx;

	for (idx = 0U; idx < HSLV_NB_IDX; idx++) {
		enable_hslv_by_index(idx);
	}
#endif
#if STM32MP15
	mmio_write_32(SYSCFG_BASE + SYSCFG_IOCTRLSETR,
		      SYSCFG_IOCTRLSETR_HSLVEN_TRACE |
		      SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI |
		      SYSCFG_IOCTRLSETR_HSLVEN_ETH |
		      SYSCFG_IOCTRLSETR_HSLVEN_SDMMC |
		      SYSCFG_IOCTRLSETR_HSLVEN_SPI);
#endif
}

static void stm32mp1_syscfg_set_hslv(void)
{
	uint32_t otp_value;
	uint32_t vdd_voltage;
	bool product_below_2v5;

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
	if (stm32_get_otp_value(HW2_OTP, &otp_value) != 0) {
		panic();
	}

	product_below_2v5 = (otp_value & HW2_OTP_PRODUCT_BELOW_2V5) != 0U;

	/* Get VDD supply */
	vdd_voltage = dt_get_pwr_vdd_voltage();

	/* Check if VDD is Low Voltage */
	if (vdd_voltage == 0U) {
		WARN("VDD unknown\n");
	} else if (vdd_voltage < 2700000U) {
		enable_high_speed_mode_low_voltage();

		if (!product_below_2v5) {
			INFO("Product_below_2v5=0: HSLVEN protected by HW\n");
		}
	} else {
		if (product_below_2v5) {
			ERROR("Product_below_2v5=1:\n");
			ERROR("\tHSLVEN update is destructive,\n");
			ERROR("\tno update as VDD > 2.7V\n");
			panic();
		}
	}
}

void stm32mp1_syscfg_init(void)
{
#if STM32MP15
	uint32_t bootr;

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
#endif

	stm32mp1_syscfg_set_hslv();

	stm32mp1_syscfg_enable_io_compensation_start();
}

void stm32mp1_syscfg_enable_io_compensation_start(void)
{
	/*
	 * Activate automatic I/O compensation.
	 * Warning: need to ensure CSI enabled and ready in clock driver.
	 * Enable non-secure clock, we assume non-secure is suspended.
	 */
	clk_enable(SYSCFG);

	mmio_setbits_32(SYSCFG_BASE + CMPCR_CMPENSETR_OFFSET + SYSCFG_CMPCR,
			SYSCFG_CMPENSETR_MPU_EN);
#if STM32MP13
	mmio_setbits_32(SYSCFG_BASE + CMPCR_CMPENSETR_OFFSET + SYSCFG_CMPSD1CR,
			SYSCFG_CMPENSETR_MPU_EN);
	mmio_setbits_32(SYSCFG_BASE + CMPCR_CMPENSETR_OFFSET + SYSCFG_CMPSD2CR,
			SYSCFG_CMPENSETR_MPU_EN);

#endif
}

void stm32mp1_syscfg_enable_io_compensation_finish(void)
{
	enable_io_comp_cell_finish(SYSCFG_CMPCR);
#if STM32MP13
	enable_io_comp_cell_finish(SYSCFG_CMPSD1CR);
	enable_io_comp_cell_finish(SYSCFG_CMPSD2CR);
#endif
}

void stm32mp1_syscfg_disable_io_compensation(void)
{
	clk_enable(SYSCFG);

	/*
	 * Deactivate automatic I/O compensation.
	 * Warning: CSI is disabled automatically in STOP if not
	 * requested for other usages and always OFF in STANDBY.
	 * Disable non-secure SYSCFG clock, we assume non-secure is suspended.
	 */
	disable_io_comp_cell(SYSCFG_CMPCR);
#if STM32MP13
	disable_io_comp_cell(SYSCFG_CMPSD1CR);
	disable_io_comp_cell(SYSCFG_CMPSD2CR);
#endif

	clk_disable(SYSCFG);
}

/*
 * @brief  Get silicon revision from SYSCFG registers.
 * @retval chip version (REV_ID).
 */
uint32_t stm32mp1_syscfg_get_chip_version(void)
{
	return (mmio_read_32(SYSCFG_BASE + SYSCFG_IDC) &
		SYSCFG_IDC_REV_ID_MASK) >> SYSCFG_IDC_REV_ID_SHIFT;
}

/*
 * @brief  Get device ID from SYSCFG registers.
 * @retval device ID (DEV_ID).
 */
uint32_t stm32mp1_syscfg_get_chip_dev_id(void)
{
	return mmio_read_32(SYSCFG_BASE + SYSCFG_IDC) & SYSCFG_IDC_DEV_ID_MASK;
}

#if STM32MP13
void stm32mp1_syscfg_boot_mode_enable(void)
{
	mmio_setbits_32(SYSCFG_BASE + SYSCFG_BOOTCR, SYSCFG_BOOTCR_BMEN);
}

void stm32mp1_syscfg_boot_mode_disable(void)
{
	mmio_clrbits_32(SYSCFG_BASE + SYSCFG_BOOTCR, SYSCFG_BOOTCR_BMEN);
}
#endif
