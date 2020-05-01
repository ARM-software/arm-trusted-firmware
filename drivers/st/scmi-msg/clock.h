/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited
 */

#ifndef SCMI_MSG_CLOCK_H
#define SCMI_MSG_CLOCK_H

#include <stdint.h>

#include <lib/utils_def.h>

#define SCMI_PROTOCOL_VERSION_CLOCK	0x20000U

/*
 * Identifiers of the SCMI Clock Management Protocol commands
 */
enum scmi_clock_command_id {
	SCMI_CLOCK_ATTRIBUTES = 0x003,
	SCMI_CLOCK_DESCRIBE_RATES = 0x004,
	SCMI_CLOCK_RATE_SET = 0x005,
	SCMI_CLOCK_RATE_GET = 0x006,
	SCMI_CLOCK_CONFIG_SET = 0x007,
};

/* Protocol attributes */
#define SCMI_CLOCK_CLOCK_COUNT_MASK			GENMASK(15, 0)
#define SCMI_CLOCK_MAX_PENDING_TRANSITIONS_MASK		GENMASK(23, 16)

#define SCMI_CLOCK_PROTOCOL_ATTRIBUTES(_max_pending, _clk_count) \
	((((_max_pending) << 16) & SCMI_CLOCK_MAX_PENDING_TRANSITIONS_MASK) | \
	 (((_clk_count) & SCMI_CLOCK_CLOCK_COUNT_MASK)))

struct scmi_clock_attributes_a2p {
	uint32_t clock_id;
};

#define SCMI_CLOCK_NAME_LENGTH_MAX	16U

struct scmi_clock_attributes_p2a {
	int32_t status;
	uint32_t attributes;
	char clock_name[SCMI_CLOCK_NAME_LENGTH_MAX];
};

/*
 * Clock Rate Get
 */

struct scmi_clock_rate_get_a2p {
	uint32_t clock_id;
};

struct scmi_clock_rate_get_p2a {
	int32_t status;
	uint32_t rate[2];
};

/*
 * Clock Rate Set
 */

/* If set, set the new clock rate asynchronously */
#define SCMI_CLOCK_RATE_SET_ASYNC_POS			0
/* If set, do not send a delayed asynchronous response */
#define SCMI_CLOCK_RATE_SET_NO_DELAYED_RESPONSE_POS	1
/* Round up, if set, otherwise round down */
#define SCMI_CLOCK_RATE_SET_ROUND_UP_POS		2
/* If set, the platform chooses the appropriate rounding mode */
#define SCMI_CLOCK_RATE_SET_ROUND_AUTO_POS		3

#define SCMI_CLOCK_RATE_SET_ASYNC_MASK \
		BIT(SCMI_CLOCK_RATE_SET_ASYNC_POS)
#define SCMI_CLOCK_RATE_SET_NO_DELAYED_RESPONSE_MASK \
		BIT(SCMI_CLOCK_RATE_SET_NO_DELAYED_RESPONSE_POS)
#define SCMI_CLOCK_RATE_SET_ROUND_UP_MASK \
		BIT(SCMI_CLOCK_RATE_SET_ROUND_UP_POS)
#define SCMI_CLOCK_RATE_SET_ROUND_AUTO_MASK \
		BIT(SCMI_CLOCK_RATE_SET_ROUND_AUTO_POS)

struct scmi_clock_rate_set_a2p {
	uint32_t flags;
	uint32_t clock_id;
	uint32_t rate[2];
};

struct scmi_clock_rate_set_p2a {
	int32_t status;
};

/*
 * Clock Config Set
 */

#define SCMI_CLOCK_CONFIG_SET_ENABLE_POS	0

#define SCMI_CLOCK_CONFIG_SET_ENABLE_MASK \
	BIT(SCMI_CLOCK_CONFIG_SET_ENABLE_POS)

struct scmi_clock_config_set_a2p {
	uint32_t clock_id;
	uint32_t attributes;
};

struct scmi_clock_config_set_p2a {
	int32_t status;
};

/*
 * Clock Describe Rates
 */

#define SCMI_CLOCK_RATE_FORMAT_RANGE			1U
#define SCMI_CLOCK_RATE_FORMAT_LIST			0U

#define SCMI_CLOCK_DESCRIBE_RATES_REMAINING_MASK	GENMASK_32(31, 16)
#define SCMI_CLOCK_DESCRIBE_RATES_REMAINING_POS		16

#define SCMI_CLOCK_DESCRIBE_RATES_FORMAT_MASK		BIT(12)
#define SCMI_CLOCK_DESCRIBE_RATES_FORMAT_POS		12

#define SCMI_CLOCK_DESCRIBE_RATES_COUNT_MASK		GENMASK_32(11, 0)

#define SCMI_CLOCK_DESCRIBE_RATES_NUM_RATES_FLAGS(_count, _fmt, _rem_rates) \
	( \
		((_count) & SCMI_CLOCK_DESCRIBE_RATES_COUNT_MASK) | \
		(((_rem_rates) << SCMI_CLOCK_DESCRIBE_RATES_REMAINING_POS) & \
		 SCMI_CLOCK_DESCRIBE_RATES_REMAINING_MASK) | \
		(((_fmt) << SCMI_CLOCK_DESCRIBE_RATES_FORMAT_POS) & \
		 SCMI_CLOCK_DESCRIBE_RATES_FORMAT_MASK) \
	)

struct scmi_clock_rate {
	uint32_t low;
	uint32_t high;
};

struct scmi_clock_describe_rates_a2p {
	uint32_t clock_id;
	uint32_t rate_index;
};

struct scmi_clock_describe_rates_p2a {
	int32_t status;
	uint32_t num_rates_flags;
	struct scmi_clock_rate rates[];
};

#endif /* SCMI_MSG_CLOCK_H */
