/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RPMh command service for the QTI APSS RSC, TZ DRV.
 *
 * The TZ DRV issues all of its commands as ACTIVE-set AMCs (Active Mode
 * Commands). Unlike the full multi-client RPMh driver, BL31 runs single
 * threaded and issues commands synchronously: a command set is programmed
 * into a TCS, the AMC is triggered, and the issuing call polls the AMC
 * finished interrupt status before returning. Sleep/wake sets are not
 * supported on the TZ DRV.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/rpmh/rpmh_client.h>
#include <lib/spinlock.h>

#include "rsc_regs.h"

/*
 * TZ DRV TCS configuration (matches the RPMh DRV config):
 *   - 4 TCSs total, the first two are AMC TCSs.
 * We use the first AMC TCS for synchronous ACTIVE requests.
 */
#define RPMH_TZ_NUM_TCS		4U
#define RPMH_TZ_AMC_TCS		0U

/* AMC completion poll bound; the AMC normally completes in microseconds. */
#define RPMH_AMC_POLL_COUNT	1000000U

struct rpmh_client {
	uint32_t		drv_id;
	const char		*name;
	uint32_t		next_req_id;
	bool			in_use;
};

static struct rpmh_client rpmh_clients[RSC_DRV_TZ + 1U];
static spinlock_t rpmh_lock;
static bool rpmh_initialized;

void rpmh_client_init(void)
{
	rpmh_initialized = true;
}

struct rpmh_client *rpmh_create_handle(uint32_t drv_id,
				      const char *client_name)
{
	struct rpmh_client *client;

	assert(rpmh_initialized);

	/* Only the TZ DRV is driven from TF-A. */
	if (drv_id != RSC_DRV_TZ) {
		return NULL;
	}

	client = &rpmh_clients[drv_id];
	client->drv_id = drv_id;
	client->name = client_name;
	client->next_req_id = 1U;
	client->in_use = true;

	return client;
}

static bool rpmh_tcs_is_idle(uint32_t tcs)
{
	uint32_t status = mmio_read_32(RSC_TCS_REG(tcs, RSC_TCS_STATUS_OFF));

	return (status & RSC_TCS_STATUS_CONTROLLER_IDLE) != 0U;
}

static void rpmh_setup_cmd(uint32_t tcs, uint32_t cmd,
			   const struct rpmh_command *info)
{
	uint32_t msgid;
	uint32_t addr;

	/* Write request: always set the read/write bit for a write. */
	msgid = RSC_CMD_MSGID_READ_OR_WRITE;
	if (info->completion) {
		msgid |= RSC_CMD_MSGID_RES_REQ;
	}
	msgid |= (RSC_CMD_DATA_LEN & RSC_CMD_MSGID_MSG_LENGTH_MASK) <<
		 RSC_CMD_MSGID_MSG_LENGTH_SHFT;

	addr = (RSC_SLAVE_ID(info->address) << RSC_CMD_ADDR_SLV_ID_SHFT) |
	       (RSC_OFFSET(info->address) << RSC_CMD_ADDR_OFFSET_SHFT);

	mmio_write_32(RSC_CMD_REG(tcs, cmd, RSC_CMD_MSGID_OFF), msgid);
	mmio_write_32(RSC_CMD_REG(tcs, cmd, RSC_CMD_ADDR_OFF), addr);
	mmio_write_32(RSC_CMD_REG(tcs, cmd, RSC_CMD_DATA_OFF), info->data);
}

/*
 * Program the AMC TCS with the given command set, trigger it and poll until
 * the AMC has finished. Returns the request id assigned to the command set.
 */
static uint32_t rpmh_send_amc(struct rpmh_client *client,
			      const struct rpmh_command_set *cmd_set)
{
	uint32_t tcs = RPMH_TZ_AMC_TCS;
	uint32_t cmd_en_mask = 0U;
	uint32_t wait_mask = 0U;
	uint32_t req_id;
	uint32_t poll;
	uint32_t i;

	assert(cmd_set->num_commands > 0U);
	assert(cmd_set->num_commands <= IMAGE_TCS_SIZE);

	/* TZ DRV only supports active requests. */
	assert(cmd_set->set == RPMH_SET_ACTIVE);

	spin_lock(&rpmh_lock);

	/* The TCS must be idle before reprogramming it. */
	for (poll = 0U; poll < RPMH_AMC_POLL_COUNT; poll++) {
		if (rpmh_tcs_is_idle(tcs)) {
			break;
		}
	}
	if (poll == RPMH_AMC_POLL_COUNT) {
		ERROR("RPMh: TCS %u not idle\n", tcs);
		panic();
	}

	for (i = 0U; i < cmd_set->num_commands; i++) {
		rpmh_setup_cmd(tcs, i, &cmd_set->commands[i]);
		cmd_en_mask |= BIT(i);
		if (cmd_set->commands[i].completion) {
			wait_mask |= BIT(i);
		}
	}

	/* Program completion dependencies and enabled commands. */
	mmio_write_32(RSC_TCS_REG(tcs, RSC_TCS_CMD_WAIT_FOR_CMPL_OFF),
		      wait_mask);
	mmio_write_32(RSC_TCS_REG(tcs, RSC_TCS_CMD_ENABLE_OFF), cmd_en_mask);

	/* Clear and enable the AMC finished interrupt for this TCS. */
	mmio_write_32(RSC_DRV0_REG(RSC_AMC_IRQ_CLEAR_OFF), BIT(tcs));
	mmio_setbits_32(RSC_DRV0_REG(RSC_AMC_IRQ_ENABLE_OFF), BIT(tcs));

	/* Trigger the AMC: set AMC mode then pulse the trigger bit. */
	mmio_clrsetbits_32(RSC_TCS_REG(tcs, RSC_TCS_CONTROL_OFF),
			   RSC_TCS_CONTROL_AMC_MODE_TRIGGER,
			   RSC_TCS_CONTROL_AMC_MODE_EN);
	mmio_setbits_32(RSC_TCS_REG(tcs, RSC_TCS_CONTROL_OFF),
			RSC_TCS_CONTROL_AMC_MODE_TRIGGER);

	/* Poll for AMC completion. */
	for (poll = 0U; poll < RPMH_AMC_POLL_COUNT; poll++) {
		uint32_t irq = mmio_read_32(RSC_DRV0_REG(RSC_AMC_IRQ_STATUS_OFF));

		if ((irq & BIT(tcs)) != 0U) {
			break;
		}
	}
	if (poll == RPMH_AMC_POLL_COUNT) {
		ERROR("RPMh: AMC on TCS %u did not complete\n", tcs);
		panic();
	}

	/* Clear the AMC finished interrupt and disable AMC mode. */
	mmio_write_32(RSC_DRV0_REG(RSC_AMC_IRQ_CLEAR_OFF), BIT(tcs));
	mmio_clrbits_32(RSC_TCS_REG(tcs, RSC_TCS_CONTROL_OFF),
			RSC_TCS_CONTROL_AMC_MODE_EN);

	req_id = client->next_req_id++;

	spin_unlock(&rpmh_lock);

	return req_id;
}

uint32_t rpmh_issue_command_set(struct rpmh_client *handle,
				struct rpmh_command_set *command_set)
{
	assert(handle != NULL);
	assert(command_set != NULL);

	return rpmh_send_amc(handle, command_set);
}

uint32_t rpmh_issue_command(struct rpmh_client *handle, enum rpmh_set set,
			    bool completion, uint32_t address, uint32_t data)
{
	struct rpmh_command_set cmd_set = {
		.set = set,
		.num_commands = 1U,
		.commands[0] = {
			.address = address,
			.data = data,
			.completion = completion,
		},
	};

	assert(handle != NULL);

	return rpmh_send_amc(handle, &cmd_set);
}

/*
 * Commands are issued synchronously, so by the time a request id has been
 * returned the corresponding AMC has already finished. The barrier APIs are
 * therefore no-ops kept for interface compatibility.
 */
void rpmh_barrier_single(struct rpmh_client *handle, uint32_t req_id)
{
	assert(handle != NULL);
	(void)req_id;
}

void rpmh_barrier_all(struct rpmh_client *handle, uint32_t req_id)
{
	assert(handle != NULL);
	(void)req_id;
}
