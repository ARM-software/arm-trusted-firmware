// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, Linaro Limited
 */
#include <cdefs.h>
#include <string.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils_def.h>

#include "common.h"

#pragma weak plat_scmi_pd_count
#pragma weak plat_scmi_pd_get_name
#pragma weak plat_scmi_pd_get_state
#pragma weak plat_scmi_pd_set_state
#pragma weak plat_scmi_pd_statistics
#pragma weak plat_scmi_pd_get_attributes

static bool message_id_is_supported(unsigned int message_id);

size_t plat_scmi_pd_count(unsigned int agent_id __unused)
{
	return 0U;
}

const char *plat_scmi_pd_get_name(unsigned int agent_id __unused,
				  unsigned int pd_id __unused)
{
	return NULL;
}

unsigned int plat_scmi_pd_statistics(unsigned int agent_id __unused,
				     unsigned long *pd_id __unused)
{
	return 0U;
}

unsigned int plat_scmi_pd_get_attributes(unsigned int agent_id __unused,
					 unsigned int pd_id __unused)
{
	return 0U;
}

unsigned int plat_scmi_pd_get_state(unsigned int agent_id __unused,
				    unsigned int pd_id __unused)
{
	return 0U;
}

int32_t plat_scmi_pd_set_state(unsigned int agent_id __unused,
			       unsigned int flags __unused,
			       unsigned int pd_id __unused,
			       unsigned int state __unused)
{
	return SCMI_NOT_SUPPORTED;
}

static void report_version(struct scmi_msg *msg)
{
	struct scmi_protocol_version_p2a return_values = {
		.status = SCMI_SUCCESS,
		.version = SCMI_PROTOCOL_VERSION_PD,
	};

	if (msg->in_size != 0) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_attributes(struct scmi_msg *msg)
{
	unsigned long addr = 0UL;
	unsigned int len;

	struct scmi_protocol_attributes_p2a_pd return_values = {
		.status = SCMI_SUCCESS,
	};

	if (msg->in_size != 0) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	return_values.attributes = plat_scmi_pd_count(msg->agent_id);
	len = plat_scmi_pd_statistics(msg->agent_id, &addr);
	if (len != 0U) {
		return_values.statistics_addr_low = (unsigned int)addr;
		return_values.statistics_addr_high = (uint32_t)(addr >> 32);
		return_values.statistics_len = len;
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

static void scmi_pd_attributes(struct scmi_msg *msg)
{
	const struct scmi_pd_attributes_a2p *in_args = (void *)msg->in;
	struct scmi_pd_attributes_p2a return_values = {
		.status = SCMI_SUCCESS,
	};
	const char *name = NULL;
	unsigned int pd_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	pd_id = SPECULATION_SAFE_VALUE(in_args->pd_id);

	if (pd_id >= plat_scmi_pd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	name = plat_scmi_pd_get_name(msg->agent_id, pd_id);
	if (name == NULL) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}

	COPY_NAME_IDENTIFIER(return_values.pd_name, name);

	return_values.attributes = plat_scmi_pd_get_attributes(msg->agent_id, pd_id);

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_pd_state_get(struct scmi_msg *msg)
{
	const struct scmi_pd_state_get_a2p *in_args = (void *)msg->in;
	unsigned int state = 0U;
	struct scmi_pd_state_get_p2a return_values = {
		.status = SCMI_SUCCESS,
	};
	unsigned int pd_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	pd_id = SPECULATION_SAFE_VALUE(in_args->pd_id);

	if (pd_id >= plat_scmi_pd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	state = plat_scmi_pd_get_state(msg->agent_id, pd_id);

	return_values.power_state = state;

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_pd_state_set(struct scmi_msg *msg)
{
	const struct scmi_pd_state_set_a2p *in_args = (void *)msg->in;
	unsigned int flags = 0U;
	int32_t status = 0;
	unsigned int pd_id = 0U;
	unsigned int state = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	pd_id = SPECULATION_SAFE_VALUE(in_args->pd_id);

	if (pd_id >= plat_scmi_pd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	flags = SPECULATION_SAFE_VALUE(in_args->flags);
	state = SPECULATION_SAFE_VALUE(in_args->power_state);

	status = plat_scmi_pd_set_state(msg->agent_id, flags, pd_id, state);

	scmi_status_response(msg, status);
}

static const scmi_msg_handler_t scmi_pd_handler_table[] = {
	[SCMI_PROTOCOL_VERSION] = report_version,
	[SCMI_PROTOCOL_ATTRIBUTES] = report_attributes,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = report_message_attributes,
	[SCMI_PD_ATTRIBUTES] = scmi_pd_attributes,
	[SCMI_PD_STATE_SET] = scmi_pd_state_set,
	[SCMI_PD_STATE_GET] = scmi_pd_state_get,
};

static bool message_id_is_supported(unsigned int message_id)
{
	return (message_id < ARRAY_SIZE(scmi_pd_handler_table)) &&
	       (scmi_pd_handler_table[message_id] != NULL);
}

scmi_msg_handler_t scmi_msg_get_pd_handler(struct scmi_msg *msg)
{
	const size_t array_size = ARRAY_SIZE(scmi_pd_handler_table);
	unsigned int message_id = SPECULATION_SAFE_VALUE(msg->message_id);

	if (message_id >= array_size) {
		VERBOSE("pd handle not found %u", msg->message_id);
		return NULL;
	}

	return scmi_pd_handler_table[message_id];
}
