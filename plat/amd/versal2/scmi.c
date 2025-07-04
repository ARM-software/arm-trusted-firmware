/*
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <platform_def.h>
#include <scmi.h>

#include "plat_private.h"

#define HIGH (1)
#define LOW (0)

struct scmi_clk {
	unsigned long clock_id;
	unsigned long rate;
	const char *name;
	bool enabled;
};

#define CLOCK_CELL(_scmi_id, _id, _name, _init_enabled, _rate) \
	[_scmi_id] = { \
		.clock_id = (_id), \
		.name = (_name), \
		.enabled = (_init_enabled), \
		.rate = (_rate), \
	}

static struct scmi_clk scmi0_clock[] = {
	CLOCK_CELL(CLK_GEM0_0, CLK_GEM0_0, "gem0_pclk", true, 100000000),
	CLOCK_CELL(CLK_GEM0_1, CLK_GEM0_1, "gem0_hclk", true, 100000000),
	CLOCK_CELL(CLK_GEM0_2, CLK_GEM0_2, "gem0_tx_clk", true, 125000000),
	CLOCK_CELL(CLK_GEM0_3, CLK_GEM0_3, "gem0_rx_clk", true, 100000000),
	CLOCK_CELL(CLK_GEM0_4, CLK_GEM0_4, "gem0_tsu_clk", true, 100000000),
	CLOCK_CELL(CLK_GEM1_0, CLK_GEM1_0, "gem1_pclk", true, 100000000),
	CLOCK_CELL(CLK_GEM1_1, CLK_GEM1_1, "gem1_hclk", true, 100000000),
	CLOCK_CELL(CLK_GEM1_2, CLK_GEM1_2, "gem1_tx_clk", true, 125000000),
	CLOCK_CELL(CLK_GEM1_3, CLK_GEM1_3, "gem1_rx_clk", true, 100000000),
	CLOCK_CELL(CLK_GEM1_4, CLK_GEM1_4, "gem1_tsu_clk", true, 100000000),
	CLOCK_CELL(CLK_SERIAL0_0, CLK_SERIAL0_0, "uart0_uartclk", true, 100000000),
	CLOCK_CELL(CLK_SERIAL0_1, CLK_SERIAL0_1, "uart0_apb_pclk", true, 100000000),
	CLOCK_CELL(CLK_SERIAL1_0, CLK_SERIAL1_0, "uart1_uartclk", true, 100000000),
	CLOCK_CELL(CLK_SERIAL1_1, CLK_SERIAL1_1, "uart1_apb_pclk", true, 100000000),
	CLOCK_CELL(CLK_UFS0_0, CLK_UFS0_0, "ufs_core_clk", true, 100000000),
	CLOCK_CELL(CLK_UFS0_1, CLK_UFS0_1, "ufs_phy_clk", true, 26000000),
	CLOCK_CELL(CLK_UFS0_2, CLK_UFS0_2, "ufs_ref_pclk", true, 26000000),
	CLOCK_CELL(CLK_USB0_0, CLK_USB0_0, "usb0_bus_clk", true, 100000000),
	CLOCK_CELL(CLK_USB0_1, CLK_USB0_1, "usb0_ref_clk", true, 100000000),
	CLOCK_CELL(CLK_USB0_2, CLK_USB0_2, "usb0_dwc_clk", true, 100000000),
	CLOCK_CELL(CLK_USB1_0, CLK_USB1_0, "usb1_bus_clk", true, 100000000),
	CLOCK_CELL(CLK_USB1_1, CLK_USB1_1, "usb1_ref_clk", true, 100000000),
	CLOCK_CELL(CLK_USB1_2, CLK_USB1_2, "usb1_dwc_clk", true, 100000000),
	CLOCK_CELL(CLK_MMC0_0, CLK_MMC0_0, "mmc0_xin_clk", true, 100000000),
	CLOCK_CELL(CLK_MMC0_1, CLK_MMC0_1, "mmc0_ahb_clk", true, 100000000),
	CLOCK_CELL(CLK_MMC0_2, CLK_MMC0_2, "mmc0_gate_clk", true, 100000000),
	CLOCK_CELL(CLK_MMC1_0, CLK_MMC1_0, "mmc1_xin_clk", true, 100000000),
	CLOCK_CELL(CLK_MMC1_1, CLK_MMC1_1, "mmc1_ahb_clk", true, 100000000),
	CLOCK_CELL(CLK_MMC1_2, CLK_MMC1_2, "mmc1_gate_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC0_0, CLK_TTC0_0, "ttc0_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC1_0, CLK_TTC1_0, "ttc1_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC2_0, CLK_TTC2_0, "ttc2_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC3_0, CLK_TTC3_0, "ttc3_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC4_0, CLK_TTC4_0, "ttc4_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC5_0, CLK_TTC5_0, "ttc5_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC6_0, CLK_TTC6_0, "ttc6_clk", true, 100000000),
	CLOCK_CELL(CLK_TTC7_0, CLK_TTC7_0, "ttc7_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C0_0, CLK_I2C0_0, "i2c0_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C1_0, CLK_I2C1_0, "i2c1_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C2_0, CLK_I2C2_0, "i2c2_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C3_0, CLK_I2C3_0, "i2c3_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C4_0, CLK_I2C4_0, "i2c4_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C5_0, CLK_I2C5_0, "i2c5_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C6_0, CLK_I2C6_0, "i2c6_clk", true, 100000000),
	CLOCK_CELL(CLK_I2C7_0, CLK_I2C7_0, "i2c7_clk", true, 100000000),
	CLOCK_CELL(CLK_OSPI0_0, CLK_OSPI0_0, "ospi0_clk", true, 100000000),
	CLOCK_CELL(CLK_QSPI0_0, CLK_QSPI0_0, "qpsi0_ref_clk", true, 100000000),
	CLOCK_CELL(CLK_QSPI0_1, CLK_QSPI0_1, "qspi0_pclk", true, 100000000),
	CLOCK_CELL(CLK_WWDT0_0, CLK_WWDT0_0, "wwdt0_clk", true, 100000000),
	CLOCK_CELL(CLK_WWDT1_0, CLK_WWDT1_0, "wwdt1_clk", true, 100000000),
	CLOCK_CELL(CLK_WWDT2_0, CLK_WWDT2_0, "wwdt2_clk", true, 100000000),
	CLOCK_CELL(CLK_WWDT3_0, CLK_WWDT3_0, "wwdt3_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA0_0, CLK_ADMA0_0, "adma0_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA0_1, CLK_ADMA0_1, "adma0_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA1_0, CLK_ADMA1_0, "adma1_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA1_1, CLK_ADMA1_1, "adma1_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA2_0, CLK_ADMA2_0, "adma2_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA2_1, CLK_ADMA2_1, "adma2_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA3_0, CLK_ADMA3_0, "adma3_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA3_1, CLK_ADMA3_1, "adma3_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA4_0, CLK_ADMA4_0, "adma4_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA4_1, CLK_ADMA4_1, "adma4_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA5_0, CLK_ADMA5_0, "adma5_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA5_1, CLK_ADMA5_1, "adma5_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA6_0, CLK_ADMA6_0, "adma6_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA6_1, CLK_ADMA6_1, "adma6_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA7_0, CLK_ADMA7_0, "adma7_main_clk", true, 100000000),
	CLOCK_CELL(CLK_ADMA7_1, CLK_ADMA7_1, "adma7_apb_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN0_0, CLK_CAN0_0, "can0_can_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN0_1, CLK_CAN0_1, "can0_axi_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN1_0, CLK_CAN1_0, "can1_can_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN1_1, CLK_CAN1_1, "can1_axi_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN2_0, CLK_CAN2_0, "can2_can_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN2_1, CLK_CAN2_1, "can2_axi_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN3_0, CLK_CAN3_0, "can3_can_clk", true, 100000000),
	CLOCK_CELL(CLK_CAN3_1, CLK_CAN3_1, "can3_axi_clk", true, 100000000),
	CLOCK_CELL(CLK_PS_GPIO_0, CLK_PS_GPIO_0, "ps_gpio_clk", true, 100000000),
	CLOCK_CELL(CLK_PMC_GPIO_0, CLK_PMC_GPIO_0, "pmc_gpio_clk", true, 100000000),
	CLOCK_CELL(CLK_SPI0_0, CLK_SPI0_0, "spi0_ref_clk", true, 100000000),
	CLOCK_CELL(CLK_SPI0_1, CLK_SPI0_1, "spi0_pclk", true, 100000000),
	CLOCK_CELL(CLK_SPI1_0, CLK_SPI1_0, "spi1_ref_clk", true, 100000000),
	CLOCK_CELL(CLK_SPI1_1, CLK_SPI1_1, "spi1_pclk", true, 100000000),
	CLOCK_CELL(CLK_I3C0_0, CLK_I3C0_0, "i3c0_clk", true, 100000000),
	CLOCK_CELL(CLK_I3C1_0, CLK_I3C1_0, "i3c1_clk", true, 100000000),
	CLOCK_CELL(CLK_I3C2_0, CLK_I3C2_0, "i3c2_clk", true, 100000000),
	CLOCK_CELL(CLK_I3C3_0, CLK_I3C3_0, "i3c3_clk", true, 100000000),
	CLOCK_CELL(CLK_I3C4_0, CLK_I3C4_0, "i3c4_clk", true, 100000000),
	CLOCK_CELL(CLK_I3C5_0, CLK_I3C5_0, "i3c5_clk", true, 100000000),
	CLOCK_CELL(CLK_I3C6_0, CLK_I3C6_0, "i3c6_clk", true, 100000000),
	CLOCK_CELL(CLK_I3C7_0, CLK_I3C7_0, "i3c7_clk", true, 100000000),
};

/*
 * struct scmi_reset - Data for the exposed reset controller
 * @reset_id: Reset identifier in RCC reset driver
 * @name: Reset string ID exposed to agent
 */
struct scmi_reset {
	unsigned long reset_id;
	const char *name;
};

#define RESET_CELL(_scmi_id, _id, _name) \
	[_scmi_id] = { \
		.reset_id = (_id), \
		.name = (_name), \
	}

static struct scmi_reset scmi0_reset[] = {
	RESET_CELL(RESET_GEM0_0, RESET_GEM0_0, "gem0"),
	RESET_CELL(RESET_GEM1_0, RESET_GEM1_0, "gem1"),
	RESET_CELL(RESET_SERIAL0_0, RESET_SERIAL0_0, "serial0"),
	RESET_CELL(RESET_SERIAL1_0, RESET_SERIAL1_0, "serial1"),
	RESET_CELL(RESET_UFS0_0, RESET_UFS0_0, "ufs0"),
	RESET_CELL(RESET_I2C0_0, RESET_I2C0_0, "i2c0"),
	RESET_CELL(RESET_I2C1_0, RESET_I2C1_0, "i2c1"),
	RESET_CELL(RESET_I2C2_0, RESET_I2C2_0, "i2c2"),
	RESET_CELL(RESET_I2C3_0, RESET_I2C3_0, "i2c3"),
	RESET_CELL(RESET_I2C4_0, RESET_I2C4_0, "i2c4"),
	RESET_CELL(RESET_I2C5_0, RESET_I2C5_0, "i2c5"),
	RESET_CELL(RESET_I2C6_0, RESET_I2C6_0, "i2c6"),
	RESET_CELL(RESET_I2C7_0, RESET_I2C7_0, "i2c7"),
	RESET_CELL(RESET_I2C8_0, RESET_I2C8_0, "i2c8"),
	RESET_CELL(RESET_OSPI0_0, RESET_OSPI0_0, "ospi"),
	RESET_CELL(RESET_USB0_0, RESET_USB0_0, "usb0_0"),
	RESET_CELL(RESET_USB0_1, RESET_USB0_1, "usb0_1"),
	RESET_CELL(RESET_USB0_2, RESET_USB0_2, "usb0_2"),
	RESET_CELL(RESET_USB1_0, RESET_USB1_0, "usb1_0"),
	RESET_CELL(RESET_USB1_1, RESET_USB1_1, "usb1_1"),
	RESET_CELL(RESET_USB1_2, RESET_USB1_2, "usb1_2"),
	RESET_CELL(RESET_MMC0_0, RESET_MMC0_0, "mmc0"),
	RESET_CELL(RESET_MMC1_0, RESET_MMC1_0, "mmc1"),
	RESET_CELL(RESET_SPI0_0, RESET_SPI0_0, "spi0"),
	RESET_CELL(RESET_SPI1_0, RESET_SPI1_0, "spi1"),
	RESET_CELL(RESET_QSPI0_0, RESET_QSPI0_0, "qspi"),
	RESET_CELL(RESET_I3C0_0, RESET_I3C0_0, "i3c0"),
	RESET_CELL(RESET_I3C1_0, RESET_I3C1_0, "i3c1"),
	RESET_CELL(RESET_I3C2_0, RESET_I3C2_0, "i3c2"),
	RESET_CELL(RESET_I3C3_0, RESET_I3C3_0, "i3c3"),
	RESET_CELL(RESET_I3C4_0, RESET_I3C4_0, "i3c4"),
	RESET_CELL(RESET_I3C5_0, RESET_I3C5_0, "i3c5"),
	RESET_CELL(RESET_I3C6_0, RESET_I3C6_0, "i3c6"),
	RESET_CELL(RESET_I3C7_0, RESET_I3C7_0, "i3c7"),
	RESET_CELL(RESET_I3C8_0, RESET_I3C8_0, "i3c8"),
	RESET_CELL(RESET_UFSPHY_0, RESET_UFSPHY_0, "ufsphy0"),
};

/**
 * struct scmi_pd - Data for the exposed power domain controller
 * @pd_id: pd identifier in RCC reset driver
 * @name: pd string ID exposed to agent
 * @state: keep state setting
 */
struct scmi_pd {
	unsigned long pd_id;
	const char *name;
	unsigned int state;
};

#define PD_CELL(_scmi_id, _id, _name, _state) \
	[_scmi_id] = { \
		.pd_id = _id, \
		.name = _name, \
		.state = _state, \
	}

static struct scmi_pd scmi0_pd[] = {
	PD_CELL(PD_USB0, PD_USB0, "usb0", 0),
	PD_CELL(PD_USB1, PD_USB1, "usb1", 0),
};

struct scmi_resources {
	struct scmi_clk *clock;
	size_t clock_count;
	struct scmi_reset *reset;
	size_t reset_count;
	struct scmi_pd *pd;
	size_t pd_count;
};

static const struct scmi_resources resources[] = {
	[0] = {
		.clock = scmi0_clock,
		.clock_count = ARRAY_SIZE(scmi0_clock),
		.reset = scmi0_reset,
		.reset_count = ARRAY_SIZE(scmi0_reset),
		.pd = scmi0_pd,
		.pd_count = ARRAY_SIZE(scmi0_pd),
	},
};

static const struct scmi_resources *find_resource(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(resources));

	return &resources[agent_id];
}

static struct scmi_clk *clk_find(unsigned int agent_id, unsigned int scmi_id)
{
	const struct scmi_resources *resource = find_resource(agent_id);
	size_t n = 0U;
	struct scmi_clk *ret = NULL;

	if (resource != NULL) {
		for (n = 0U; n < resource->clock_count; n++) {
			if (n == scmi_id) {
				ret = &resource->clock[n];
				break;
			}
		}
	}

	return ret;
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	const struct scmi_resources *resource = find_resource(agent_id);
	size_t ret;

	if (resource == NULL) {
		ret = 0U;
	} else {
		VERBOSE("SCMI: CLK: %d clocks\n", (unsigned int)resource->clock_count);

		ret = resource->clock_count;
	}
	return ret;
}

const char *plat_scmi_clock_get_name(unsigned int agent_id, unsigned int scmi_id)
{
	const struct scmi_clk *clock = clk_find(agent_id, scmi_id);
	const char *ret;

	if (clock == NULL) {
		ret = NULL;
	} else {
		VERBOSE("SCMI: CLK: id: %d, get_name: %s\n", scmi_id, clock->name);

		ret = clock->name;
	}
	return ret;
};

/* Called by Linux */
int32_t plat_scmi_clock_rates_array(unsigned int agent_id, unsigned int scmi_id,
				    unsigned long *array, size_t *nb_elts,
				    uint32_t start_idx)
{
	const struct scmi_clk *clock = clk_find(agent_id, scmi_id);
	int32_t ret = SCMI_SUCCESS;

	if (clock == NULL) {
		ret = SCMI_NOT_FOUND;
		goto exit_label;
	}

	if (start_idx > 0U) {
		ret = SCMI_OUT_OF_RANGE;
		goto exit_label;
	}

	if (array == NULL) {
		*nb_elts = 1U;
	} else if (*nb_elts == 1U) {
		*array = clock->rate;
		VERBOSE("SCMI: CLK: id: %d, clk_name: %s, get_rate %lu\n",
		     scmi_id, clock->name, *array);
	} else {
		ret = SCMI_GENERIC_ERROR;
	}

exit_label:
	return ret;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id, unsigned int scmi_id)
{
	const struct scmi_clk *clock = clk_find(agent_id, scmi_id);
	unsigned long ret;

	if (clock == NULL) {
		ret = SCMI_NOT_FOUND;
	} else {
		VERBOSE("SCMI: CLK: id: %d, get_rate: %lu\n", scmi_id, clock->rate);
		ret = clock->rate;
	}
	return ret;
}

int32_t plat_scmi_clock_set_rate(unsigned int agent_id, unsigned int scmi_id,
				 unsigned long rate)
{
	struct scmi_clk *clock = clk_find(agent_id, scmi_id);
	int32_t ret = SCMI_SUCCESS;

	if (clock == NULL) {
		ret = SCMI_NOT_FOUND;
	} else {
		VERBOSE("SCMI: CLK: id: %d, set_rate: %lu\n", scmi_id, rate);
		clock->rate = rate;
	}
	return ret;
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id, unsigned int scmi_id)
{
	const struct scmi_clk *clock = clk_find(agent_id, scmi_id);
	int32_t ret;

	if (clock == NULL) {
		ret = SCMI_NOT_FOUND;
	} else {
		VERBOSE("SCMI: CLK: id: %d, get_state: %d\n", scmi_id, clock->enabled);

		if (clock->enabled) {
			ret = HIGH;
		} else {
			ret = LOW;
		}
	}
	return ret;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id, unsigned int scmi_id,
				  bool enable_not_disable)
{
	struct scmi_clk *clock = clk_find(agent_id, scmi_id);
	int32_t ret;

	if (clock == NULL) {
		ret = SCMI_NOT_FOUND;
	} else {
		if (enable_not_disable) {
			if (!clock->enabled) {
				VERBOSE("SCMI: clock: %u enable\n", scmi_id);
				clock->enabled = true;
			}
		} else {
			if (clock->enabled) {
				VERBOSE("SCMI: clock: %u disable\n", scmi_id);
				clock->enabled = false;
			}
		}

		VERBOSE("SCMI: CLK: id: %d, set_state: %d\n", scmi_id, clock->enabled);

		ret = SCMI_SUCCESS;
	}

	return ret;
}


/*
 * Platform SCMI reset domains
 */
static struct scmi_reset *find_reset(unsigned int agent_id,
					 unsigned int scmi_id)
{
	const struct scmi_resources *resource = find_resource(agent_id);
	size_t n;

	if (resource != NULL) {
		for (n = 0U; n < resource->reset_count; n++) {
			if (n == scmi_id) {
				return &resource->reset[n];
			}
		}
	}

	return NULL;
}

const char *plat_scmi_rstd_get_name(unsigned int agent_id, unsigned int scmi_id)
{
	const struct scmi_reset *reset = find_reset(agent_id, scmi_id);

	if (reset == NULL) {
		return NULL;
	}

	return reset->name;
}

size_t plat_scmi_rstd_count(unsigned int agent_id)
{
	const struct scmi_resources *resource = find_resource(agent_id);

	if (resource == NULL) {
		return 0U;
	}

	return resource->reset_count;
}

int32_t plat_scmi_rstd_autonomous(unsigned int agent_id, unsigned int scmi_id,
				uint32_t state)
{
	const struct scmi_reset *reset = find_reset(agent_id, scmi_id);

	if (reset == NULL) {
		return SCMI_NOT_FOUND;
	}

	/* Supports only reset with context loss */
	if (state != 0U) {
		return SCMI_NOT_SUPPORTED;
	}

	NOTICE("SCMI reset on ID %lu/%s\n",
	       reset->reset_id, plat_scmi_rstd_get_name(agent_id, scmi_id));

	return SCMI_SUCCESS;
}

int32_t plat_scmi_rstd_set_state(unsigned int agent_id, unsigned int scmi_id,
				 bool assert_not_deassert)
{
	const struct scmi_reset *reset = find_reset(agent_id, scmi_id);

	if (reset == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (assert_not_deassert) {
		NOTICE("SCMI reset %lu/%s set\n",
		       reset->reset_id, plat_scmi_rstd_get_name(agent_id, scmi_id));

		switch (scmi_id) {
		case RESET_UFS0_0:
			mmio_write_32(PMXC_CRP_RST_UFS, 1);
			break;
		case RESET_UFSPHY_0:
			mmio_write_32(PMXC_IOU_SLCR_PHY_RESET, 1);
			break;
		default:
			break;
		}
	} else {
		NOTICE("SCMI reset %lu/%s release\n",
		       reset->reset_id, plat_scmi_rstd_get_name(agent_id, scmi_id));

		switch (scmi_id) {
		case RESET_UFS0_0:
			mmio_write_32(PMXC_CRP_RST_UFS, 0);
			break;
		case RESET_UFSPHY_0:
			mmio_write_32(PMXC_IOU_SLCR_PHY_RESET, 0);
			break;
		default:
			break;
		}
	}

	return SCMI_SUCCESS;
}

/*
 * Platform SCMI reset domains
 */
static struct scmi_pd *find_pd(unsigned int agent_id, unsigned int pd_id)
{
	const struct scmi_resources *resource = find_resource(agent_id);
	size_t n;

	if (resource != NULL) {
		for (n = 0U; n < resource->pd_count; n++) {
			if (n == pd_id) {
				return &resource->pd[n];
			}
		}
	}

	return NULL;
}

size_t plat_scmi_pd_count(unsigned int agent_id)
{
	const struct scmi_resources *resource = find_resource(agent_id);
	size_t ret;

	if (resource == NULL) {
		ret = 0U;
	} else {
		ret = resource->pd_count;

		NOTICE("SCMI: PD: %d\n", (unsigned int)ret);
	}
	return ret;
}

const char *plat_scmi_pd_get_name(unsigned int agent_id, unsigned int pd_id)
{
	const struct scmi_pd *pd = find_pd(agent_id, pd_id);
	const char *ret = NULL;

	if (pd != NULL) {
		ret = pd->name;
	}

	return ret;
}

unsigned int plat_scmi_pd_statistics(unsigned int agent_id, unsigned long *pd_id)
{
	return 0U;
}

unsigned int plat_scmi_pd_get_attributes(unsigned int agent_id, unsigned int pd_id)
{
	return 0U;
}

unsigned int plat_scmi_pd_get_state(unsigned int agent_id, unsigned int pd_id)
{
	const struct scmi_pd *pd = find_pd(agent_id, pd_id);
	uint32_t ret = SCMI_NOT_SUPPORTED;

	if (pd != NULL) {
		NOTICE("SCMI: PD: get id: %d, state: %x\n", pd_id, pd->state);

		ret = pd->state;
	}

	return ret;
}

int32_t plat_scmi_pd_set_state(unsigned int agent_id, unsigned int flags, unsigned int pd_id,
			       unsigned int state)
{
	struct scmi_pd *pd = find_pd(agent_id, pd_id);
	int32_t ret = SCMI_SUCCESS;

	if (pd == NULL) {
		ret = SCMI_NOT_SUPPORTED;
		goto exit_label;
	}

	NOTICE("SCMI: PD: set id: %d, orig state: %x, new state: %x,  flags: %x\n",
			pd_id, pd->state, state, flags);

	pd->state = state;

exit_label:
	return ret;
}


/* Currently only one channel is supported. Expectation is that channel 0 is used by NS SW */
static struct scmi_msg_channel scmi_channel[] = {
	[0] = {
		.shm_addr = SMT_BUFFER_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
};

struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(scmi_channel));

	VERBOSE("%d: SCMI asking for channel\n", agent_id);

	/* Just in case that code is reused */
	return &scmi_channel[agent_id];
}

/* Base protocol implementations */
const char *plat_scmi_vendor_name(void)
{
	return SCMI_VENDOR;
}

const char *plat_scmi_sub_vendor_name(void)
{
	return SCMI_PRODUCT;
}

/* Currently supporting Clocks and Reset Domains */
static const uint8_t plat_protocol_list[] = {
	SCMI_PROTOCOL_ID_BASE,
	SCMI_PROTOCOL_ID_CLOCK,
	SCMI_PROTOCOL_ID_RESET_DOMAIN,
	SCMI_PROTOCOL_ID_POWER_DOMAIN,
	/* SCMI_PROTOCOL_ID_SENSOR, */
	0U /* Null termination */
};

size_t plat_scmi_protocol_count(void)
{
	const size_t count = ARRAY_SIZE(plat_protocol_list) - 1U;

	VERBOSE("SCMI: Protocol count: %d\n", (int32_t)count);

	return count;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id __unused)
{
	return plat_protocol_list;
}

void init_scmi_server(void)
{
	size_t i;
	int32_t ret;

	for (i = 0U; i < ARRAY_SIZE(scmi_channel); i++) {
		scmi_smt_init_agent_channel(&scmi_channel[i]);
	}

	INFO("SCMI: Server initialized\n");

	if (platform_id == QEMU) {
		/* default setting is for QEMU */
	} else if (platform_id == SPP) {
		for (i = 0U; i < ARRAY_SIZE(scmi0_clock); i++) {

			/* Keep i2c on 100MHz to calculate rates properly */
			if ((i >= CLK_I2C0_0) && (i <= CLK_I2C7_0)) {
				continue;
			}

			/* Keep UFS clocks to default values to get the expected rates */
			if ((i >= CLK_UFS0_0) && (i <= CLK_UFS0_2)) {
				continue;
			}

			/*
			 * SPP supports multiple versions.
			 * The cpu_clock value is set to corresponding SPP
			 * version in early platform setup, resuse the same
			 * value here.
			 */
			ret = plat_scmi_clock_set_rate(0, i, cpu_clock);
			if (ret < 0) {
				NOTICE("Failed to set clock rate for SPP scmi_id=%ld\n", i);
			}
		}
	} else {
		 /* Making MISRA C 2012 15.7 compliant */
	}
}
