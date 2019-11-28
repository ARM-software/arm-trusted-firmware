/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited
 */

#ifndef SCMI_MSG_H
#define SCMI_MSG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Minimum size expected for SMT based shared memory message buffers */
#define SMT_BUF_SLOT_SIZE	128U

/* A channel abstract a communication path between agent and server */
struct scmi_msg_channel;

/*
 * struct scmi_msg_channel - Shared memory buffer for a agent-to-server channel
 *
 * @shm_addr: Address of the shared memory for the SCMI channel
 * @shm_size: Byte size of the shared memory for the SCMI channel
 * @busy: True when channel is busy, flase when channel is free
 * @agent_name: Agent name, SCMI protocol exposes 16 bytes max, or NULL
 */
struct scmi_msg_channel {
	uintptr_t shm_addr;
	size_t shm_size;
	bool busy;
	const char *agent_name;
};

/* Platform callback functions */

/*
 * Return the SCMI channel related to an agent
 * @agent_id: SCMI agent ID
 * Return a pointer to channel on success, NULL otherwise
 */
struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id);

/*
 * Return how many SCMI protocols supported by the platform
 * According to the SCMI specification, this function does not target
 * a specific agent ID and shall return all platform known capabilities.
 */
size_t plat_scmi_protocol_count(void);

/*
 * Get the count and list of SCMI protocols (but base) supported for an agent
 *
 * @agent_id: SCMI agent ID
 * Return a pointer to a null terminated array supported protocol IDs.
 */
const uint8_t *plat_scmi_protocol_list(unsigned int agent_id);

/* Get the name of the SCMI vendor for the platform */
const char *plat_scmi_vendor_name(void);

/* Get the name of the SCMI sub-vendor for the platform */
const char *plat_scmi_sub_vendor_name(void);

#endif /* SCMI_MSG_H */
