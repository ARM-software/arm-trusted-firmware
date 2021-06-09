/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2021 NXP
 */

#ifndef SCMI_MSG_PD_H
#define SCMI_MSG_PD_H

#include <stdint.h>

#include <lib/utils_def.h>

#define SCMI_PROTOCOL_VERSION_PD	0x21000U

/*
 * Identifiers of the SCMI POWER DOMAIN Protocol commands
 */
enum scmi_pd_command_id {
	SCMI_PD_ATTRIBUTES = 0x003,
	SCMI_PD_STATE_SET = 0x004,
	SCMI_PD_STATE_GET = 0x005,
};

/* Protocol attributes */
struct scmi_pd_attributes_a2p {
	uint32_t pd_id;
};

struct scmi_protocol_attributes_p2a_pd {
	int32_t status;
	uint32_t attributes;
	uint32_t statistics_addr_low;
	uint32_t statistics_addr_high;
	uint32_t statistics_len;
};

#define SCMI_PD_NAME_LENGTH_MAX	16U

struct scmi_pd_attributes_p2a {
	int32_t status;
	uint32_t attributes;
	char pd_name[SCMI_PD_NAME_LENGTH_MAX];
};

/*
 * Power Domain State Get
 */

struct scmi_pd_state_get_a2p {
	uint32_t pd_id;
};

struct scmi_pd_state_get_p2a {
	int32_t status;
	uint32_t power_state;
};

/*
 * Power domain State Set
 */

struct scmi_pd_state_set_a2p {
	uint32_t flags;
	uint32_t pd_id;
	uint32_t power_state;
};

struct scmi_pd_state_set_p2a {
	int32_t status;
};

#endif /* SCMI_MSG_PD_H */
