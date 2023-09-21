/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2023-2024 NXP
 */

#ifndef SCMI_MSG_SENSOR_H
#define SCMI_MSG_SENSOR_H

#include <stdint.h>

#include <lib/utils_def.h>

#define SCMI_PROTOCOL_VERSION_SENSOR	0x20000U

/*
 * Identifiers of the SCMI SENSOR Protocol commands
 */
enum scmi_sensor_command_id {
	SCMI_SENSOR_DESCRIPTION_GET = 0x003,
	SCMI_SENSOR_TRIP_POINT_NOTIFY = 0x004,
	SCMI_SENSOR_TRIP_POINT_CONFIG = 0x005,
	SCMI_SENSOR_READING_GET = 0x006,
	SCMI_SENSOR_AXIS_DESCRIPTION_GET = 0x007,
	SCMI_SENSOR_LIST_UPDATE_INTERVALS = 0x008,
	SCMI_SENSOR_CONFIG_GET = 0x009,
	SCMI_SENSOR_CONFIG_SET = 0x00A,
	SCMI_SENSOR_CONTINUOUS_UPDATE_NOTIFY = 0x00B,
	SCMI_SENSOR_MAX = 0x00C,
};

/* Protocol attributes */
struct scmi_protocol_attributes_p2a_sensor {
	int32_t status;
	int16_t num_sensors;
	uint8_t max_reqs;
	uint8_t res;
	uint32_t sensor_reg_low;
	uint32_t sensor_reg_high;
	uint32_t sensor_reg_len;
};

#define SCMI_SENSOR_NAME_LENGTH_MAX	16U

struct scmi_sensor_desc {
	uint32_t id;
	uint32_t attr_low;
	uint32_t attr_high;
	uint8_t name[SCMI_SENSOR_NAME_LENGTH_MAX];
	uint32_t power;
	uint32_t resolution;
	int32_t min_range_low;
	int32_t min_range_high;
	int32_t max_range_low;
	int32_t max_range_high;
};

struct scmi_sensor_description_get_a2p {
	uint32_t desc_index;
};

struct scmi_sensor_description_get_p2a {
	int32_t status;
	uint32_t num_sensor_flags;
};

struct scmi_sensor_config_get_a2p {
	uint32_t sensor_id;
};

struct scmi_sensor_config_get_p2a {
	int32_t status;
	uint32_t sensor_config;
};

/*
 * Sensor Reading Get
 */
struct scmi_sensor_reading_get_a2p {
	uint32_t sensor_id;
	uint32_t flags;
};

struct scmi_sensor_val {
	uint32_t value_low;
	uint32_t value_high;
	uint32_t timestap_low;
	uint32_t timestap_high;
};

struct scmi_sensor_reading_get_p2a {
	int32_t status;
	struct scmi_sensor_val val;
};

typedef struct {
	uint16_t (*sensor_count)(unsigned int agent_id);
	uint8_t (*sensor_max_request)(unsigned int agent_id);
	uint32_t (*get_sensor_req)(unsigned int agent_id, unsigned int *addr);
	int32_t (*sensor_reading_get)(uint32_t agent_id, uint16_t sensor_id,
				      uint32_t *val);
	uint32_t (*sensor_description_get)(unsigned int agent_id, uint16_t sensor_id,
					  struct scmi_sensor_desc *desc);
	uint32_t (*sensor_update_interval)(uint32_t agent_id, uint16_t sensor_id);
	uint32_t (*sensor_state)(uint32_t agent_id, uint16_t sensor_id);
	uint16_t (*sensor_timestamped)(uint32_t agent_id, uint16_t sensor_id);
} plat_scmi_sensor_ops_t;

#define REGISTER_SCMI_SENSOR_OPS(_sensor_count, _sensor_max_request, \
				 _get_sensor_req, _sensor_reading_get, \
				 _sensor_description_get, _sensor_update_interval, \
				 _sensor_state, _sensor_timestamped) \
	const plat_scmi_sensor_ops_t sensor_ops = { \
		.sensor_count = _sensor_count, \
		.sensor_max_request = _sensor_max_request, \
		.get_sensor_req = _get_sensor_req, \
		.sensor_reading_get = _sensor_reading_get, \
		.sensor_description_get = _sensor_description_get, \
		.sensor_update_interval = _sensor_update_interval, \
		.sensor_state = _sensor_state, \
		.sensor_timestamped = _sensor_timestamped, \
	}

extern const plat_scmi_sensor_ops_t sensor_ops;

#endif /* SCMI_MSG_SENSOR_H */
