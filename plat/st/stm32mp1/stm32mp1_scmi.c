/*
 * Copyright (c) 2019-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

#include <drivers/st/scmi-msg.h>
#include <drivers/st/scmi.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp_reset.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/reset/stm32mp1-resets.h>

#define TIMEOUT_US_1MS		1000U

#define SCMI_CLOCK_NAME_SIZE	16U
#define SCMI_RSTD_NAME_SIZE	16U

/*
 * struct stm32_scmi_clk - Data for the exposed clock
 * @clock_id: Clock identifier in RCC clock driver
 * @name: Clock string ID exposed to agent
 * @enabled: State of the SCMI clock
 */
struct stm32_scmi_clk {
	unsigned long clock_id;
	const char *name;
	bool enabled;
};

/*
 * struct stm32_scmi_rstd - Data for the exposed reset controller
 * @reset_id: Reset identifier in RCC reset driver
 * @name: Reset string ID exposed to agent
 */
struct stm32_scmi_rstd {
	unsigned long reset_id;
	const char *name;
};

/* Locate all non-secure SMT message buffers in last page of SYSRAM */
#define SMT_BUFFER_BASE		STM32MP_SCMI_NS_SHM_BASE
#define SMT_BUFFER0_BASE	SMT_BUFFER_BASE
#define SMT_BUFFER1_BASE	(SMT_BUFFER_BASE + 0x200)

CASSERT((STM32MP_SCMI_NS_SHM_BASE + STM32MP_SCMI_NS_SHM_SIZE) >=
	(SMT_BUFFER1_BASE + SMT_BUF_SLOT_SIZE),
	assert_scmi_non_secure_shm_fits_scmi_overall_buffer_size);

static struct scmi_msg_channel scmi_channel[] = {
	[0] = {
		.shm_addr = SMT_BUFFER0_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
	[1] = {
		.shm_addr = SMT_BUFFER1_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
};

struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(scmi_channel));

	return &scmi_channel[agent_id];
}

#define CLOCK_CELL(_scmi_id, _id, _name, _init_enabled) \
	[_scmi_id] = { \
		.clock_id = _id, \
		.name = _name, \
		.enabled = _init_enabled, \
	}

static struct stm32_scmi_clk stm32_scmi0_clock[] = {
	CLOCK_CELL(CK_SCMI0_HSE, CK_HSE, "ck_hse", true),
	CLOCK_CELL(CK_SCMI0_HSI, CK_HSI, "ck_hsi", true),
	CLOCK_CELL(CK_SCMI0_CSI, CK_CSI, "ck_csi", true),
	CLOCK_CELL(CK_SCMI0_LSE, CK_LSE, "ck_lse", true),
	CLOCK_CELL(CK_SCMI0_LSI, CK_LSI, "ck_lsi", true),
	CLOCK_CELL(CK_SCMI0_PLL2_Q, PLL2_Q, "pll2_q", true),
	CLOCK_CELL(CK_SCMI0_PLL2_R, PLL2_R, "pll2_r", true),
	CLOCK_CELL(CK_SCMI0_MPU, CK_MPU, "ck_mpu", true),
	CLOCK_CELL(CK_SCMI0_AXI, CK_AXI, "ck_axi", true),
	CLOCK_CELL(CK_SCMI0_BSEC, BSEC, "bsec", true),
	CLOCK_CELL(CK_SCMI0_CRYP1, CRYP1, "cryp1", false),
	CLOCK_CELL(CK_SCMI0_GPIOZ, GPIOZ, "gpioz", false),
	CLOCK_CELL(CK_SCMI0_HASH1, HASH1, "hash1", false),
	CLOCK_CELL(CK_SCMI0_I2C4, I2C4_K, "i2c4_k", false),
	CLOCK_CELL(CK_SCMI0_I2C6, I2C6_K, "i2c6_k", false),
	CLOCK_CELL(CK_SCMI0_IWDG1, IWDG1, "iwdg1", false),
	CLOCK_CELL(CK_SCMI0_RNG1, RNG1_K, "rng1_k", true),
	CLOCK_CELL(CK_SCMI0_RTC, RTC, "ck_rtc", true),
	CLOCK_CELL(CK_SCMI0_RTCAPB, RTCAPB, "rtcapb", true),
	CLOCK_CELL(CK_SCMI0_SPI6, SPI6_K, "spi6_k", false),
	CLOCK_CELL(CK_SCMI0_USART1, USART1_K, "usart1_k", false),
};

static struct stm32_scmi_clk stm32_scmi1_clock[] = {
	CLOCK_CELL(CK_SCMI1_PLL3_Q, PLL3_Q, "pll3_q", true),
	CLOCK_CELL(CK_SCMI1_PLL3_R, PLL3_R, "pll3_r", true),
	CLOCK_CELL(CK_SCMI1_MCU, CK_MCU, "ck_mcu", false),
};

#define RESET_CELL(_scmi_id, _id, _name) \
	[_scmi_id] = { \
		.reset_id = _id, \
		.name = _name, \
	}

static struct stm32_scmi_rstd stm32_scmi0_reset_domain[] = {
	RESET_CELL(RST_SCMI0_SPI6, SPI6_R, "spi6"),
	RESET_CELL(RST_SCMI0_I2C4, I2C4_R, "i2c4"),
	RESET_CELL(RST_SCMI0_I2C6, I2C6_R, "i2c6"),
	RESET_CELL(RST_SCMI0_USART1, USART1_R, "usart1"),
	RESET_CELL(RST_SCMI0_STGEN, STGEN_R, "stgen"),
	RESET_CELL(RST_SCMI0_GPIOZ, GPIOZ_R, "gpioz"),
	RESET_CELL(RST_SCMI0_CRYP1, CRYP1_R, "cryp1"),
	RESET_CELL(RST_SCMI0_HASH1, HASH1_R, "hash1"),
	RESET_CELL(RST_SCMI0_RNG1, RNG1_R, "rng1"),
	RESET_CELL(RST_SCMI0_MDMA, MDMA_R, "mdma"),
	RESET_CELL(RST_SCMI0_MCU, MCU_R, "mcu"),
};

struct scmi_agent_resources {
	struct stm32_scmi_clk *clock;
	size_t clock_count;
	struct stm32_scmi_rstd *rstd;
	size_t rstd_count;
};

static const struct scmi_agent_resources agent_resources[] = {
	[0] = {
		.clock = stm32_scmi0_clock,
		.clock_count = ARRAY_SIZE(stm32_scmi0_clock),
		.rstd = stm32_scmi0_reset_domain,
		.rstd_count = ARRAY_SIZE(stm32_scmi0_reset_domain),
	},
	[1] = {
		.clock = stm32_scmi1_clock,
		.clock_count = ARRAY_SIZE(stm32_scmi1_clock),
	},
};

static const struct scmi_agent_resources *find_resource(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(agent_resources));

	return &agent_resources[agent_id];
}

#if ENABLE_ASSERTIONS
static size_t plat_scmi_protocol_count_paranoid(void)
{
	unsigned int n = 0U;
	unsigned int count = 0U;

	for (n = 0U; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].clock_count) {
			count++;
			break;
		}
	}

	for (n = 0U; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].rstd_count) {
			count++;
			break;
		}
	}

	return count;
}
#endif

static const char vendor[] = "ST";
static const char sub_vendor[] = "";

const char *plat_scmi_vendor_name(void)
{
	return vendor;
}

const char *plat_scmi_sub_vendor_name(void)
{
	return sub_vendor;
}

/* Currently supporting Clocks and Reset Domains */
static const uint8_t plat_protocol_list[] = {
	SCMI_PROTOCOL_ID_CLOCK,
	SCMI_PROTOCOL_ID_RESET_DOMAIN,
	0U /* Null termination */
};

size_t plat_scmi_protocol_count(void)
{
	const size_t count = ARRAY_SIZE(plat_protocol_list) - 1U;

	assert(count == plat_scmi_protocol_count_paranoid());

	return count;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id __unused)
{
	assert(plat_scmi_protocol_count_paranoid() ==
	       (ARRAY_SIZE(plat_protocol_list) - 1U));

	return plat_protocol_list;
}

/*
 * Platform SCMI clocks
 */
static struct stm32_scmi_clk *find_clock(unsigned int agent_id,
					 unsigned int scmi_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);
	size_t n = 0U;

	if (resource != NULL) {
		for (n = 0U; n < resource->clock_count; n++) {
			if (n == scmi_id) {
				return &resource->clock[n];
			}
		}
	}

	return NULL;
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);

	if (resource == NULL) {
		return 0U;
	}

	return resource->clock_count;
}

const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if ((clock == NULL) ||
	    !stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return NULL;
	}

	return clock->name;
}

int32_t plat_scmi_clock_rates_array(unsigned int agent_id, unsigned int scmi_id,
				    unsigned long *array, size_t *nb_elts)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (clock == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return SCMI_DENIED;
	}

	if (array == NULL) {
		*nb_elts = 1U;
	} else if (*nb_elts == 1U) {
		*array = stm32mp_clk_get_rate(clock->clock_id);
	} else {
		return SCMI_GENERIC_ERROR;
	}

	return SCMI_SUCCESS;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id,
				       unsigned int scmi_id)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if ((clock == NULL) ||
	    !stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return 0U;
	}

	return stm32mp_clk_get_rate(clock->clock_id);
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id, unsigned int scmi_id)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if ((clock == NULL) ||
	    !stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return 0U;
	}

	return (int32_t)clock->enabled;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id, unsigned int scmi_id,
				  bool enable_not_disable)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (clock == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return SCMI_DENIED;
	}

	if (enable_not_disable) {
		if (!clock->enabled) {
			VERBOSE("SCMI clock %u enable\n", scmi_id);
			stm32mp_clk_enable(clock->clock_id);
			clock->enabled = true;
		}
	} else {
		if (clock->enabled) {
			VERBOSE("SCMI clock %u disable\n", scmi_id);
			stm32mp_clk_disable(clock->clock_id);
			clock->enabled = false;
		}
	}

	return SCMI_SUCCESS;
}

/*
 * Platform SCMI reset domains
 */
static struct stm32_scmi_rstd *find_rstd(unsigned int agent_id,
					 unsigned int scmi_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);
	size_t n;

	if (resource != NULL) {
		for (n = 0U; n < resource->rstd_count; n++) {
			if (n == scmi_id) {
				return &resource->rstd[n];
			}
		}
	}

	return NULL;
}

const char *plat_scmi_rstd_get_name(unsigned int agent_id, unsigned int scmi_id)
{
	const struct stm32_scmi_rstd *rstd = find_rstd(agent_id, scmi_id);

	if (rstd == NULL) {
		return NULL;
	}

	return rstd->name;
}

size_t plat_scmi_rstd_count(unsigned int agent_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);

	if (resource == NULL) {
		return 0U;
	}

	return resource->rstd_count;
}

int32_t plat_scmi_rstd_autonomous(unsigned int agent_id, unsigned int scmi_id,
				uint32_t state)
{
	const struct stm32_scmi_rstd *rstd = find_rstd(agent_id, scmi_id);

	if (rstd == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_reset(rstd->reset_id)) {
		return SCMI_DENIED;
	}

	/* Supports only reset with context loss */
	if (state != 0U) {
		return SCMI_NOT_SUPPORTED;
	}

	VERBOSE("SCMI reset %lu cycle\n", rstd->reset_id);

	if (stm32mp_reset_assert(rstd->reset_id, TIMEOUT_US_1MS)) {
		return SCMI_HARDWARE_ERROR;
	}

	if (stm32mp_reset_deassert(rstd->reset_id, TIMEOUT_US_1MS)) {
		return SCMI_HARDWARE_ERROR;
	}

	return SCMI_SUCCESS;
}

int32_t plat_scmi_rstd_set_state(unsigned int agent_id, unsigned int scmi_id,
				 bool assert_not_deassert)
{
	const struct stm32_scmi_rstd *rstd = find_rstd(agent_id, scmi_id);

	if (rstd == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_reset(rstd->reset_id)) {
		return SCMI_DENIED;
	}

	if (assert_not_deassert) {
		VERBOSE("SCMI reset %lu set\n", rstd->reset_id);
		stm32mp_reset_set(rstd->reset_id);
	} else {
		VERBOSE("SCMI reset %lu release\n", rstd->reset_id);
		stm32mp_reset_release(rstd->reset_id);
	}

	return SCMI_SUCCESS;
}

/*
 * Initialize platform SCMI resources
 */
void stm32mp1_init_scmi_server(void)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(scmi_channel); i++) {
		scmi_smt_init_agent_channel(&scmi_channel[i]);
	}

	for (i = 0U; i < ARRAY_SIZE(agent_resources); i++) {
		const struct scmi_agent_resources *res = &agent_resources[i];
		size_t j;

		for (j = 0U; j < res->clock_count; j++) {
			struct stm32_scmi_clk *clk = &res->clock[j];

			if ((clk->name == NULL) ||
			    (strlen(clk->name) >= SCMI_CLOCK_NAME_SIZE)) {
				ERROR("Invalid SCMI clock name\n");
				panic();
			}

			/* Sync SCMI clocks with their targeted initial state */
			if (clk->enabled &&
			    stm32mp_nsec_can_access_clock(clk->clock_id)) {
				stm32mp_clk_enable(clk->clock_id);
			}
		}

		for (j = 0U; j < res->rstd_count; j++) {
			struct stm32_scmi_rstd *rstd = &res->rstd[j];

			if ((rstd->name == NULL) ||
			    (strlen(rstd->name) >= SCMI_RSTD_NAME_SIZE)) {
				ERROR("Invalid SCMI reset domain name\n");
				panic();
			}
		}
	}
}
