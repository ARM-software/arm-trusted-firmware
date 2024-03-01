// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021-2024 NXP
 */

#include <cdefs.h>
#include <string.h>

#include "common.h"

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils_def.h>

static bool message_id_is_supported(size_t message_id);

uint16_t plat_scmi_sensor_count(unsigned int agent_id __unused)
{
	if (sensor_ops.sensor_count != NULL) {
		return sensor_ops.sensor_count(agent_id);
	}

	return 0U;
}

uint8_t plat_scmi_sensor_max_requests(unsigned int agent_id __unused)
{
	if (sensor_ops.sensor_max_request != NULL) {
		return sensor_ops.sensor_max_request(agent_id);
	}

	return 0U;
}

uint32_t plat_scmi_sensor_reg(unsigned int agent_id __unused,
			      unsigned int *addr)
{
	if (sensor_ops.get_sensor_req != NULL) {
		return sensor_ops.get_sensor_req(agent_id, addr);
	}

	return 0U;
}

int32_t plat_scmi_sensor_reading_get(uint32_t agent_id __unused,
				     uint16_t sensor_id __unused,
				     uint32_t *val __unused)
{
	if (sensor_ops.sensor_reading_get != NULL) {
		return sensor_ops.sensor_reading_get(agent_id, sensor_id, val);
	}

	return 0;
}

uint32_t plat_scmi_sensor_description_get(uint32_t agent_id __unused,
					  uint16_t desc_index __unused,
					  struct scmi_sensor_desc *desc __unused)
{
	if (sensor_ops.sensor_description_get != NULL) {
		return sensor_ops.sensor_description_get(agent_id, desc_index, desc);
	}

	return 0U;
}

uint32_t plat_scmi_sensor_update_interval(uint32_t agent_id __unused,
					  uint16_t sensor_id __unused)
{
	if (sensor_ops.sensor_update_interval != NULL) {
		return sensor_ops.sensor_update_interval(agent_id, sensor_id);
	}

	return 0U;
}

uint32_t plat_scmi_sensor_state(uint32_t agent_id __unused,
				uint16_t sensor_id __unused)
{
	if (sensor_ops.sensor_state != NULL) {
		return sensor_ops.sensor_state(agent_id, sensor_id);
	}

	return 0U;
}

uint32_t plat_scmi_sensor_timestamped(uint32_t agent_id __unused,
				      uint16_t sensor_id __unused)
{
	if (sensor_ops.sensor_timestamped != NULL) {
		return sensor_ops.sensor_timestamped(agent_id, sensor_id);
	}

	return 0U;
}

static void report_version(struct scmi_msg *msg)
{
	struct scmi_protocol_version_p2a return_values = {
		.status = SCMI_SUCCESS,
		.version = SCMI_PROTOCOL_VERSION_SENSOR,
	};

	if (msg->in_size != 0U) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_attributes(struct scmi_msg *msg)
{
	unsigned int addr[2];
	unsigned int len;

	struct scmi_protocol_attributes_p2a_sensor return_values = {
		.status = SCMI_SUCCESS,
	};

	if (msg->in_size != 0U) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	return_values.num_sensors = plat_scmi_sensor_count(msg->agent_id);
	return_values.max_reqs = plat_scmi_sensor_max_requests(msg->agent_id);
	len = plat_scmi_sensor_reg(msg->agent_id, addr);
	if (len != 0U) {
		return_values.sensor_reg_low = addr[0];
		return_values.sensor_reg_high = addr[1];
		return_values.sensor_reg_len = len;
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

static void scmi_sensor_description_get(struct scmi_msg *msg)
{
	const struct scmi_sensor_description_get_a2p *in_args = (void *)msg->in;
	struct scmi_sensor_description_get_p2a return_values = {
		.status = SCMI_SUCCESS,
	};
	struct scmi_sensor_desc desc;
	unsigned int desc_index = 0U;
	unsigned int num_sensor_flags;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	desc_index = SPECULATION_SAFE_VALUE(in_args->desc_index);

	num_sensor_flags = plat_scmi_sensor_description_get(msg->agent_id, desc_index,
							    &desc);
	return_values.num_sensor_flags = num_sensor_flags;

	memcpy(msg->out, &return_values, sizeof(return_values));
	memcpy(msg->out + sizeof(return_values), &desc, sizeof(desc));
	msg->out_size_out = sizeof(return_values) + sizeof(struct scmi_sensor_desc);
}

static void scmi_sensor_config_get(struct scmi_msg *msg)
{
	const struct scmi_sensor_config_get_a2p *in_args = (void *)msg->in;
	struct scmi_sensor_config_get_p2a return_values = {
		.status = SCMI_SUCCESS,
	};
	unsigned int sensor_id = 0U;
	uint32_t update_interval, state, timestamped;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	sensor_id = SPECULATION_SAFE_VALUE(in_args->sensor_id);

	if (sensor_id >= plat_scmi_sensor_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	update_interval = plat_scmi_sensor_update_interval(msg->agent_id, sensor_id);
	state = plat_scmi_sensor_state(msg->agent_id, sensor_id);
	timestamped = plat_scmi_sensor_timestamped(msg->agent_id, sensor_id);
	return_values.sensor_config = (update_interval << 11) | (timestamped << 1) | state;

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_sensor_reading_get(struct scmi_msg *msg)
{
	const struct scmi_sensor_reading_get_a2p *in_args = (void *)msg->in;
	struct scmi_sensor_reading_get_p2a return_values = {
		.status = SCMI_SUCCESS,
	};
	unsigned int sensor_id = 0U;
	int32_t ret;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	sensor_id = SPECULATION_SAFE_VALUE(in_args->sensor_id);

	if (sensor_id >= plat_scmi_sensor_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	ret = plat_scmi_sensor_reading_get(msg->agent_id, sensor_id,
					  (uint32_t *)&return_values.val);
	if (ret) {
		scmi_status_response(msg, SCMI_HARDWARE_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_sensor_list_update_intervals(struct scmi_msg *msg)
{
	/* TODO */
	scmi_status_response(msg, SCMI_NOT_SUPPORTED);
}

static const scmi_msg_handler_t scmi_sensor_handler_table[SCMI_SENSOR_MAX] = {
	[SCMI_PROTOCOL_VERSION] = report_version,
	[SCMI_PROTOCOL_ATTRIBUTES] = report_attributes,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = report_message_attributes,
	[SCMI_SENSOR_DESCRIPTION_GET] = scmi_sensor_description_get,
	[SCMI_SENSOR_CONFIG_GET] = scmi_sensor_config_get,
	[SCMI_SENSOR_LIST_UPDATE_INTERVALS] = scmi_sensor_list_update_intervals,
	[SCMI_SENSOR_READING_GET] = scmi_sensor_reading_get,
};

static bool message_id_is_supported(size_t message_id)
{
	return scmi_sensor_handler_table[message_id] != NULL;
}

scmi_msg_handler_t scmi_msg_get_sensor_handler(struct scmi_msg *msg)
{
	unsigned int message_id = SPECULATION_SAFE_VALUE(msg->message_id);

	if (!message_id_is_supported(message_id)) {
		VERBOSE("pd handle not found %u\n", msg->message_id);
		return NULL;
	}

	return scmi_sensor_handler_table[message_id];
}
