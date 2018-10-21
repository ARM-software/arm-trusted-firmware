/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu.h>
#include <drivers/arm/css/css_scpi.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#define SCPI_SHARED_MEM_SCP_TO_AP	PLAT_CSS_SCP_COM_SHARED_MEM_BASE
#define SCPI_SHARED_MEM_AP_TO_SCP	(PLAT_CSS_SCP_COM_SHARED_MEM_BASE \
								 + 0x100)

/* Header and payload addresses for commands from AP to SCP */
#define SCPI_CMD_HEADER_AP_TO_SCP		\
	((scpi_cmd_t *) SCPI_SHARED_MEM_AP_TO_SCP)
#define SCPI_CMD_PAYLOAD_AP_TO_SCP		\
	((void *) (SCPI_SHARED_MEM_AP_TO_SCP + sizeof(scpi_cmd_t)))

/* Header and payload addresses for responses from SCP to AP */
#define SCPI_RES_HEADER_SCP_TO_AP \
	((scpi_cmd_t *) SCPI_SHARED_MEM_SCP_TO_AP)
#define SCPI_RES_PAYLOAD_SCP_TO_AP \
	((void *) (SCPI_SHARED_MEM_SCP_TO_AP + sizeof(scpi_cmd_t)))

/* ID of the MHU slot used for the SCPI protocol */
#define SCPI_MHU_SLOT_ID		0

static void scpi_secure_message_start(void)
{
	mhu_secure_message_start(SCPI_MHU_SLOT_ID);
}

static void scpi_secure_message_send(size_t payload_size)
{
	/*
	 * Ensure that any write to the SCPI payload area is seen by SCP before
	 * we write to the MHU register. If these 2 writes were reordered by
	 * the CPU then SCP would read stale payload data
	 */
	dmbst();

	mhu_secure_message_send(SCPI_MHU_SLOT_ID);
}

static int scpi_secure_message_receive(scpi_cmd_t *cmd)
{
	uint32_t mhu_status;

	assert(cmd != NULL);

	mhu_status = mhu_secure_message_wait();

	/* Expect an SCPI message, reject any other protocol */
	if (mhu_status != (1 << SCPI_MHU_SLOT_ID)) {
		ERROR("MHU: Unexpected protocol (MHU status: 0x%x)\n",
			mhu_status);
		return -1;
	}

	/*
	 * Ensure that any read to the SCPI payload area is done after reading
	 * the MHU register. If these 2 reads were reordered then the CPU would
	 * read invalid payload data
	 */
	dmbld();

	memcpy(cmd, (void *) SCPI_SHARED_MEM_SCP_TO_AP, sizeof(*cmd));

	return 0;
}

static void scpi_secure_message_end(void)
{
	mhu_secure_message_end(SCPI_MHU_SLOT_ID);
}

int scpi_wait_ready(void)
{
	scpi_cmd_t scpi_cmd;
	int rc;

	VERBOSE("Waiting for SCP_READY command...\n");

	/* Get a message from the SCP */
	scpi_secure_message_start();
	rc = scpi_secure_message_receive(&scpi_cmd);
	scpi_secure_message_end();

	/* If no message was received, don't send a response */
	if (rc != 0)
		return rc;

	/* We are expecting 'SCP Ready', produce correct error if it's not */
	scpi_status_t status = SCP_OK;
	if (scpi_cmd.id != SCPI_CMD_SCP_READY) {
		ERROR("Unexpected SCP command: expected command #%u, got command #%u\n",
		      SCPI_CMD_SCP_READY, scpi_cmd.id);
		status = SCP_E_SUPPORT;
	} else if (scpi_cmd.size != 0) {
		ERROR("SCP_READY command has incorrect size: expected 0, got %u\n",
		      scpi_cmd.size);
		status = SCP_E_SIZE;
	}

	VERBOSE("Sending response for SCP_READY command\n");

	/*
	 * Send our response back to SCP.
	 * We are using the same SCPI header, just update the status field.
	 */
	scpi_cmd.status = status;
	scpi_secure_message_start();
	memcpy((void *) SCPI_SHARED_MEM_AP_TO_SCP, &scpi_cmd, sizeof(scpi_cmd));
	scpi_secure_message_send(0);
	scpi_secure_message_end();

	return status == SCP_OK ? 0 : -1;
}

void scpi_set_css_power_state(unsigned int mpidr,
		scpi_power_state_t cpu_state, scpi_power_state_t cluster_state,
		scpi_power_state_t css_state)
{
	scpi_cmd_t *cmd;
	uint32_t state = 0;
	uint32_t *payload_addr;

#if ARM_PLAT_MT
	/*
	 * The current SCPI driver only caters for single-threaded platforms.
	 * Hence we ignore the thread ID (which is always 0) for such platforms.
	 */
	state |= (mpidr >> MPIDR_AFF1_SHIFT) & 0x0f;	/* CPU ID */
	state |= ((mpidr >> MPIDR_AFF2_SHIFT) & 0x0f) << 4;	/* Cluster ID */
#else
	state |= mpidr & 0x0f;	/* CPU ID */
	state |= (mpidr & 0xf00) >> 4;	/* Cluster ID */
#endif /* ARM_PLAT_MT */

	state |= cpu_state << 8;
	state |= cluster_state << 12;
	state |= css_state << 16;

	scpi_secure_message_start();

	/* Populate the command header */
	cmd = SCPI_CMD_HEADER_AP_TO_SCP;
	cmd->id = SCPI_CMD_SET_CSS_POWER_STATE;
	cmd->set = SCPI_SET_NORMAL;
	cmd->sender = 0;
	cmd->size = sizeof(state);
	/* Populate the command payload */
	payload_addr = SCPI_CMD_PAYLOAD_AP_TO_SCP;
	*payload_addr = state;
	scpi_secure_message_send(sizeof(state));
	/*
	 * SCP does not reply to this command in order to avoid MHU interrupts
	 * from the sender, which could interfere with its power state request.
	 */

	scpi_secure_message_end();
}

/*
 * Query and obtain CSS power state from SCP.
 *
 * In response to the query, SCP returns power states of all CPUs in all
 * clusters of the system. The returned response is then filtered based on the
 * supplied MPIDR. Power states of requested cluster and CPUs within are updated
 * via supplied non-NULL pointer arguments.
 *
 * Returns 0 on success, or -1 on errors.
 */
int scpi_get_css_power_state(unsigned int mpidr, unsigned int *cpu_state_p,
		unsigned int *cluster_state_p)
{
	scpi_cmd_t *cmd;
	scpi_cmd_t response;
	int power_state, cpu, cluster, rc = -1;

	/*
	 * Extract CPU and cluster membership of the given MPIDR. SCPI caters
	 * for only up to 0xf clusters, and 8 CPUs per cluster
	 */
#if ARM_PLAT_MT
	/*
	 * The current SCPI driver only caters for single-threaded platforms.
	 * Hence we ignore the thread ID (which is always 0) for such platforms.
	 */
	cpu = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cluster = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
#else
	cpu = mpidr & MPIDR_AFFLVL_MASK;
	cluster = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
#endif  /* ARM_PLAT_MT */
	if (cpu >= 8 || cluster >= 0xf)
		return -1;

	scpi_secure_message_start();

	/* Populate request headers */
	zeromem(SCPI_CMD_HEADER_AP_TO_SCP, sizeof(*cmd));
	cmd = SCPI_CMD_HEADER_AP_TO_SCP;
	cmd->id = SCPI_CMD_GET_CSS_POWER_STATE;

	/*
	 * Send message and wait for SCP's response
	 */
	scpi_secure_message_send(0);
	if (scpi_secure_message_receive(&response) != 0)
		goto exit;

	if (response.status != SCP_OK)
		goto exit;

	/* Validate SCP response */
	if (!CHECK_RESPONSE(response, cluster))
		goto exit;

	/* Extract power states for required cluster */
	power_state = *(((uint16_t *) SCPI_RES_PAYLOAD_SCP_TO_AP) + cluster);
	if (CLUSTER_ID(power_state) != cluster)
		goto exit;

	/* Update power state via pointers */
	if (cluster_state_p)
		*cluster_state_p = CLUSTER_POWER_STATE(power_state);
	if (cpu_state_p)
		*cpu_state_p = CPU_POWER_STATE(power_state);
	rc = 0;

exit:
	scpi_secure_message_end();
	return rc;
}

uint32_t scpi_sys_power_state(scpi_system_state_t system_state)
{
	scpi_cmd_t *cmd;
	uint8_t *payload_addr;
	scpi_cmd_t response;

	scpi_secure_message_start();

	/* Populate the command header */
	cmd = SCPI_CMD_HEADER_AP_TO_SCP;
	cmd->id = SCPI_CMD_SYS_POWER_STATE;
	cmd->set = 0;
	cmd->sender = 0;
	cmd->size = sizeof(*payload_addr);
	/* Populate the command payload */
	payload_addr = SCPI_CMD_PAYLOAD_AP_TO_SCP;
	*payload_addr = system_state & 0xff;
	scpi_secure_message_send(sizeof(*payload_addr));

	/* If no response is received, fill in an error status */
	if (scpi_secure_message_receive(&response) != 0)
		response.status = SCP_E_TIMEOUT;

	scpi_secure_message_end();

	return response.status;
}
