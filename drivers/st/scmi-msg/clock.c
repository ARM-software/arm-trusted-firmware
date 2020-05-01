// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, Linaro Limited
 */
#include <cdefs.h>
#include <string.h>

#include <drivers/st/scmi-msg.h>
#include <drivers/st/scmi.h>
#include <lib/utils_def.h>

#include "common.h"

#pragma weak plat_scmi_clock_count
#pragma weak plat_scmi_clock_get_name
#pragma weak plat_scmi_clock_rates_array
#pragma weak plat_scmi_clock_rates_by_step
#pragma weak plat_scmi_clock_get_rate
#pragma weak plat_scmi_clock_set_rate
#pragma weak plat_scmi_clock_get_state
#pragma weak plat_scmi_clock_set_state

static bool message_id_is_supported(unsigned int message_id);

size_t plat_scmi_clock_count(unsigned int agent_id __unused)
{
	return 0U;
}

const char *plat_scmi_clock_get_name(unsigned int agent_id __unused,
				     unsigned int scmi_id __unused)
{
	return NULL;
}

int32_t plat_scmi_clock_rates_array(unsigned int agent_id __unused,
				    unsigned int scmi_id __unused,
				    unsigned long *rates __unused,
				    size_t *nb_elts __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_clock_rates_by_step(unsigned int agent_id __unused,
				      unsigned int scmi_id __unused,
				      unsigned long *steps __unused)
{
	return SCMI_NOT_SUPPORTED;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id __unused,
				       unsigned int scmi_id __unused)
{
	return 0U;
}

int32_t plat_scmi_clock_set_rate(unsigned int agent_id __unused,
				 unsigned int scmi_id __unused,
				 unsigned long rate __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id __unused,
				  unsigned int scmi_id __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id __unused,
				  unsigned int scmi_id __unused,
				  bool enable_not_disable __unused)
{
	return SCMI_NOT_SUPPORTED;
}

static void report_version(struct scmi_msg *msg)
{
	struct scmi_protocol_version_p2a return_values = {
		.status = SCMI_SUCCESS,
		.version = SCMI_PROTOCOL_VERSION_CLOCK,
	};

	if (msg->in_size != 0) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_attributes(struct scmi_msg *msg)
{
	size_t agent_count = plat_scmi_clock_count(msg->agent_id);
	struct scmi_protocol_attributes_p2a return_values = {
		.status = SCMI_SUCCESS,
		.attributes = SCMI_CLOCK_PROTOCOL_ATTRIBUTES(1U, agent_count),
	};

	if (msg->in_size != 0) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_message_attributes(struct scmi_msg *msg)
{
	struct scmi_protocol_message_attributes_a2p *in_args = (void *)msg->in;
	struct scmi_protocol_message_attributes_p2a return_values = {
		.status = SCMI_SUCCESS,
		/* For this protocol, attributes shall be zero */
		.attributes = 0U,
	};

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	if (!message_id_is_supported(in_args->message_id)) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_clock_attributes(struct scmi_msg *msg)
{
	const struct scmi_clock_attributes_a2p *in_args = (void *)msg->in;
	struct scmi_clock_attributes_p2a return_values = {
		.status = SCMI_SUCCESS,
	};
	const char *name = NULL;
	unsigned int clock_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	clock_id = SPECULATION_SAFE_VALUE(in_args->clock_id);

	if (clock_id >= plat_scmi_clock_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}


	name = plat_scmi_clock_get_name(msg->agent_id, clock_id);
	if (name == NULL) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}

	COPY_NAME_IDENTIFIER(return_values.clock_name, name);

	return_values.attributes = plat_scmi_clock_get_state(msg->agent_id,
							     clock_id);

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_clock_rate_get(struct scmi_msg *msg)
{
	const struct scmi_clock_rate_get_a2p *in_args = (void *)msg->in;
	unsigned long rate = 0U;
	struct scmi_clock_rate_get_p2a return_values = {
		.status = SCMI_SUCCESS,
	};
	unsigned int clock_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	clock_id = SPECULATION_SAFE_VALUE(in_args->clock_id);

	if (clock_id >= plat_scmi_clock_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	rate = plat_scmi_clock_get_rate(msg->agent_id, clock_id);

	return_values.rate[0] = (uint32_t)rate;
	return_values.rate[1] = (uint32_t)((uint64_t)rate >> 32);

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_clock_rate_set(struct scmi_msg *msg)
{
	const struct scmi_clock_rate_set_a2p *in_args = (void *)msg->in;
	unsigned long rate = 0U;
	int32_t status = 0;
	unsigned int clock_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	clock_id = SPECULATION_SAFE_VALUE(in_args->clock_id);

	if (clock_id >= plat_scmi_clock_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	rate = (unsigned long)(((uint64_t)in_args->rate[1] << 32) |
			       in_args->rate[0]);

	status = plat_scmi_clock_set_rate(msg->agent_id, clock_id, rate);

	scmi_status_response(msg, status);
}

static void scmi_clock_config_set(struct scmi_msg *msg)
{
	const struct scmi_clock_config_set_a2p *in_args = (void *)msg->in;
	int32_t status = SCMI_GENERIC_ERROR;
	bool enable = false;
	unsigned int clock_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	clock_id = SPECULATION_SAFE_VALUE(in_args->clock_id);

	if (clock_id >= plat_scmi_clock_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	enable = in_args->attributes & SCMI_CLOCK_CONFIG_SET_ENABLE_MASK;

	status = plat_scmi_clock_set_state(msg->agent_id, clock_id, enable);

	scmi_status_response(msg, status);
}

#define RATES_ARRAY_SIZE_MAX	(SCMI_PLAYLOAD_MAX - \
				 sizeof(struct scmi_clock_describe_rates_p2a))

#define SCMI_RATES_BY_ARRAY(_nb_rates, _rem_rates) \
	SCMI_CLOCK_DESCRIBE_RATES_NUM_RATES_FLAGS((_nb_rates), \
						SCMI_CLOCK_RATE_FORMAT_LIST, \
						(_rem_rates))
#define SCMI_RATES_BY_STEP \
	SCMI_CLOCK_DESCRIBE_RATES_NUM_RATES_FLAGS(3U, \
						SCMI_CLOCK_RATE_FORMAT_RANGE, \
						0U)

#define RATE_DESC_SIZE		sizeof(struct scmi_clock_rate)

static void write_rate_desc_array_in_buffer(char *dest, unsigned long *rates,
					    size_t nb_elt)
{
	uint32_t *out = (uint32_t *)(uintptr_t)dest;
	size_t n;

	ASSERT_SYM_PTR_ALIGN(out);

	for (n = 0U; n < nb_elt; n++) {
		out[2 * n] = (uint32_t)rates[n];
		out[2 * n + 1] = (uint32_t)((uint64_t)rates[n] >> 32);
	}
}

static void scmi_clock_describe_rates(struct scmi_msg *msg)
{
	const struct scmi_clock_describe_rates_a2p *in_args = (void *)msg->in;
	struct scmi_clock_describe_rates_p2a p2a = {
		.status = SCMI_SUCCESS,
	};
	size_t nb_rates;
	int32_t status;
	unsigned int clock_id;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	clock_id = SPECULATION_SAFE_VALUE(in_args->clock_id);

	if (clock_id >= plat_scmi_clock_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	/* Platform may support array rate description */
	status = plat_scmi_clock_rates_array(msg->agent_id, clock_id, NULL,
					     &nb_rates);
	if (status == SCMI_SUCCESS) {
		/* Currently 12 cells mex, so it's affordable for the stack */
		unsigned long plat_rates[RATES_ARRAY_SIZE_MAX / RATE_DESC_SIZE];
		size_t max_nb = RATES_ARRAY_SIZE_MAX / RATE_DESC_SIZE;
		size_t ret_nb = MIN(nb_rates - in_args->rate_index, max_nb);
		size_t rem_nb = nb_rates - in_args->rate_index - ret_nb;

		status =  plat_scmi_clock_rates_array(msg->agent_id, clock_id,
						      plat_rates, &ret_nb);
		if (status == SCMI_SUCCESS) {
			write_rate_desc_array_in_buffer(msg->out + sizeof(p2a),
							plat_rates, ret_nb);

			p2a.num_rates_flags = SCMI_RATES_BY_ARRAY(ret_nb,
								  rem_nb);
			p2a.status = SCMI_SUCCESS;

			memcpy(msg->out, &p2a, sizeof(p2a));
			msg->out_size_out = sizeof(p2a) +
					    ret_nb * RATE_DESC_SIZE;
		}
	} else if (status == SCMI_NOT_SUPPORTED) {
		unsigned long triplet[3] = { 0U, 0U, 0U };

		/* Platform may support min§max/step triplet description */
		status =  plat_scmi_clock_rates_by_step(msg->agent_id, clock_id,
							triplet);
		if (status == SCMI_SUCCESS) {
			write_rate_desc_array_in_buffer(msg->out + sizeof(p2a),
							triplet, 3U);

			p2a.num_rates_flags = SCMI_RATES_BY_STEP;
			p2a.status = SCMI_SUCCESS;

			memcpy(msg->out, &p2a, sizeof(p2a));
			msg->out_size_out = sizeof(p2a) + (3U * RATE_DESC_SIZE);
		}
	} else {
		/* Fallthrough generic exit sequence below with error status */
	}

	if (status != SCMI_SUCCESS) {
		scmi_status_response(msg, status);
	} else {
		/*
		 * Message payload is already writen to msg->out, and
		 * msg->out_size_out updated.
		 */
	}
}

static const scmi_msg_handler_t scmi_clock_handler_table[] = {
	[SCMI_PROTOCOL_VERSION] = report_version,
	[SCMI_PROTOCOL_ATTRIBUTES] = report_attributes,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = report_message_attributes,
	[SCMI_CLOCK_ATTRIBUTES] = scmi_clock_attributes,
	[SCMI_CLOCK_DESCRIBE_RATES] = scmi_clock_describe_rates,
	[SCMI_CLOCK_RATE_SET] = scmi_clock_rate_set,
	[SCMI_CLOCK_RATE_GET] = scmi_clock_rate_get,
	[SCMI_CLOCK_CONFIG_SET] = scmi_clock_config_set,
};

static bool message_id_is_supported(size_t message_id)
{
	return (message_id < ARRAY_SIZE(scmi_clock_handler_table)) &&
	       (scmi_clock_handler_table[message_id] != NULL);
}

scmi_msg_handler_t scmi_msg_get_clock_handler(struct scmi_msg *msg)
{
	const size_t array_size = ARRAY_SIZE(scmi_clock_handler_table);
	unsigned int message_id = SPECULATION_SAFE_VALUE(msg->message_id);

	if (message_id >= array_size) {
		VERBOSE("Clock handle not found %u", msg->message_id);
		return NULL;
	}

	return scmi_clock_handler_table[message_id];
}
