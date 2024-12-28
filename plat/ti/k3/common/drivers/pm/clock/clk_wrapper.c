/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tisci/pm/tisci_pm_clock.h>
#include <hosts.h>
#include <pm.h>
#include <common/debug.h>
#include <clk_wrapper.h>

void mmr_lock_all(void)
{
	/* Don't lock MMRs */
}

void mmr_unlock_all(void)
{
	/* Don't lock MMRs */
}

int32_t scmi_handler_clock_prepare(uint32_t dev_id, uint32_t clk_id)
{
	struct tisci_msg_set_clock_req req;
	uint32_t flags = 0;

	flags |= TISCI_MSG_FLAG_CLOCK_ALLOW_SSC;
	flags |= TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE;
	flags |= TISCI_MSG_FLAG_CLOCK_INPUT_TERM;


	req.device = dev_id;
	req.clk = clk_id;
	req.hdr.flags = flags;
	req.hdr.host = HOST_ID_TIFS;
	req.state = TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ;

	return set_clock_handler(&req);
}

int32_t scmi_handler_clock_unprepare(uint32_t dev_id, uint32_t clk_id)
{
	struct tisci_msg_set_clock_req req;
	uint32_t flags = 0;

	flags |= TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE;


	req.device = dev_id;
	req.clk = clk_id;
	req.hdr.flags = flags;
	req.hdr.host = HOST_ID_TIFS;
	req.state = TISCI_MSG_VALUE_CLOCK_SW_STATE_AUTO;

	return set_clock_handler(&req);
}

int32_t scmi_handler_clock_get_state(uint32_t dev_id, uint32_t clk_id)
{
	struct tisci_msg_get_clock_req req;
	struct tisci_msg_get_clock_resp *resp =
		(struct tisci_msg_get_clock_resp *)&req;

	req.device = dev_id;
	req.clk = clk_id;
	req.hdr.host = HOST_ID_TIFS;

	return get_clock_handler(&req) ? 0 : resp->programmed_state;
}

int32_t scmi_handler_clock_set_rate(uint32_t dev_id, uint32_t clk_id, uint64_t target_freq)
{
	struct tisci_msg_set_freq_req req;

	req.device = dev_id;
	req.clk = clk_id;
	req.min_freq_hz = target_freq / 10 * 9;
	req.target_freq_hz = target_freq;
	req.max_freq_hz = target_freq / 10 * 11;
	req.hdr.host = HOST_ID_TIFS;

	return set_freq_handler(&req);
}
uint64_t scmi_handler_clock_get_rate(uint32_t dev_id, uint32_t clk_id)
{
	struct tisci_msg_get_freq_req req;
	struct tisci_msg_get_freq_resp *resp = (struct tisci_msg_get_freq_resp *)&req;

	req.device = dev_id;
	req.clk = clk_id;
	req.hdr.host = HOST_ID_TIFS;

	return get_freq_handler(&req) ? 0 : resp->freq_hz;
}

int32_t scmi_handler_clock_get_num_clock_parents(uint32_t dev_id, uint32_t clk_id)
{
	struct tisci_msg_get_num_clock_parents_req req;
	struct tisci_msg_get_num_clock_parents_resp *resp =
		(struct tisci_msg_get_num_clock_parents_resp *) &req;

	req.device = dev_id;
	req.clk = clk_id;
	req.hdr.host = HOST_ID_TIFS;

	return get_num_clock_parents_handler(&req) ? 0 : resp->num_parents;
}

int32_t scmi_handler_clock_set_clock_parent(uint32_t dev_id, uint32_t clk_id, uint32_t parent_id)
{
	struct tisci_msg_set_clock_parent_req req;

	req.device = dev_id;
	req.clk = clk_id;
	req.hdr.host = HOST_ID_TIFS;
	req.parent = parent_id;

	return set_clock_parent_handler(&req);
}

int32_t scmi_handler_clock_get_clock_parent(uint32_t dev_id, uint32_t clk_id, uint32_t *parent_id)
{
	struct tisci_msg_get_clock_parent_req req;
	struct tisci_msg_get_clock_parent_resp *resp =
		(struct tisci_msg_get_clock_parent_resp *) &req;
	int32_t status = 0;

	req.device = dev_id;
	req.clk = clk_id;
	req.hdr.host = HOST_ID_TIFS;

	status = get_clock_parent_handler(&req);
	if (!status)
		*parent_id = resp->parent;
	else
		*parent_id = 0;

	return status;
}
