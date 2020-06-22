// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, Linaro Limited
 */
#include <cdefs.h>
#include <string.h>

#include <drivers/st/scmi-msg.h>
#include <drivers/st/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include "common.h"

static bool message_id_is_supported(unsigned int message_id);

#pragma weak plat_scmi_rstd_count
#pragma weak plat_scmi_rstd_get_name
#pragma weak plat_scmi_rstd_autonomous
#pragma weak plat_scmi_rstd_set_state

size_t plat_scmi_rstd_count(unsigned int agent_id __unused)
{
	return 0U;
}

const char *plat_scmi_rstd_get_name(unsigned int agent_id __unused,
				  unsigned int scmi_id __unused)
{
	return NULL;
}

int32_t plat_scmi_rstd_autonomous(unsigned int agent_id __unused,
				unsigned int scmi_id __unused,
				unsigned int state __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_rstd_set_state(unsigned int agent_id __unused,
			       unsigned int scmi_id __unused,
			       bool assert_not_deassert __unused)
{
	return SCMI_NOT_SUPPORTED;
}

static void report_version(struct scmi_msg *msg)
{
	struct scmi_protocol_version_p2a return_values = {
		.status = SCMI_SUCCESS,
		.version = SCMI_PROTOCOL_VERSION_RESET_DOMAIN,
	};

	if (msg->in_size != 0U) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_attributes(struct scmi_msg *msg)
{
	struct scmi_protocol_attributes_p2a return_values = {
		.status = SCMI_SUCCESS,
		.attributes = plat_scmi_rstd_count(msg->agent_id),
	};

	if (msg->in_size != 0U) {
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

static void reset_domain_attributes(struct scmi_msg *msg)
{
	struct scmi_reset_domain_attributes_a2p *in_args = (void *)msg->in;
	struct scmi_reset_domain_attributes_p2a return_values;
	const char *name = NULL;
	unsigned int domain_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_rstd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	name = plat_scmi_rstd_get_name(msg->agent_id, domain_id);
	if (name == NULL) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}

	zeromem(&return_values, sizeof(return_values));
	COPY_NAME_IDENTIFIER(return_values.name, name);
	return_values.status = SCMI_SUCCESS;
	return_values.flags = 0U; /* Async and Notif are not supported */
	return_values.latency = SCMI_RESET_DOMAIN_ATTR_UNK_LAT;

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void reset_request(struct scmi_msg *msg)
{
	struct scmi_reset_domain_request_a2p *in_args = (void *)msg->in;
	struct scmi_reset_domain_request_p2a out_args = {
		.status = SCMI_SUCCESS,
	};
	unsigned int domain_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_rstd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}

	if ((in_args->flags & SCMI_RESET_DOMAIN_AUTO) != 0U) {
		out_args.status = plat_scmi_rstd_autonomous(msg->agent_id,
							  domain_id,
							  in_args->reset_state);
	} else if ((in_args->flags & SCMI_RESET_DOMAIN_EXPLICIT) != 0U) {
		out_args.status = plat_scmi_rstd_set_state(msg->agent_id,
							 domain_id, true);
	} else {
		out_args.status = plat_scmi_rstd_set_state(msg->agent_id,
							 domain_id, false);
	}

	if (out_args.status != SCMI_SUCCESS) {
		scmi_status_response(msg, out_args.status);
	} else {
		scmi_write_response(msg, &out_args, sizeof(out_args));
	}
}

static const scmi_msg_handler_t scmi_rstd_handler_table[] = {
	[SCMI_PROTOCOL_VERSION] = report_version,
	[SCMI_PROTOCOL_ATTRIBUTES] = report_attributes,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = report_message_attributes,
	[SCMI_RESET_DOMAIN_ATTRIBUTES] = reset_domain_attributes,
	[SCMI_RESET_DOMAIN_REQUEST] = reset_request,
};

static bool message_id_is_supported(unsigned int message_id)
{
	return (message_id < ARRAY_SIZE(scmi_rstd_handler_table)) &&
	       (scmi_rstd_handler_table[message_id] != NULL);
}

scmi_msg_handler_t scmi_msg_get_rstd_handler(struct scmi_msg *msg)
{
	unsigned int message_id = SPECULATION_SAFE_VALUE(msg->message_id);

	if (message_id >= ARRAY_SIZE(scmi_rstd_handler_table)) {
		VERBOSE("Reset domain handle not found %u\n", msg->message_id);
		return NULL;
	}

	return scmi_rstd_handler_table[message_id];
}
