/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk.h>
#include <clk_mux.h>
#include <clk_div.h>
#include <device.h>
#include <device_clk.h>
#include <types/errno.h>
#include <tisci/tisci_protocol.h>
#include <pm.h>

#include <tisci/pm/tisci_pm_clock.h>
#include <limits.h>
#include <lib/trace.h>
#include <lib/mmr_lock.h>
#include <device_prepare.h>

int32_t set_clock_handler(struct tisci_msg_set_clock_req *msg_recv)
{
	struct tisci_msg_set_clock_req *req =
		(struct tisci_msg_set_clock_req *) msg_recv;
	struct tisci_msg_set_clock_resp *resp =
		(struct tisci_msg_set_clock_resp *) msg_recv;
	struct device *dev = NULL;
	bool gated = false;
	uint32_t id = req->device;
	dev_clk_idx_t clkidx;
	uint8_t state = req->state;
	int32_t ret = SUCCESS;
	uint32_t flags = req->hdr.flags;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_SET_CLOCK);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_VAL, state);

	resp->hdr.flags = 0U;

	ret = device_prepare_exclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		switch (state) {
		case TISCI_MSG_VALUE_CLOCK_SW_STATE_UNREQ:
			gated = true;
			break;
		case TISCI_MSG_VALUE_CLOCK_SW_STATE_AUTO:
		case TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ:
			gated = false;
			break;
		default:
			pm_trace(TRACE_PM_ACTION_INVALID_STATE, state);
			ret = -EINVAL;
			break;
		}
	}

	if (ret == SUCCESS) {
		if ((flags & TISCI_MSG_FLAG_CLOCK_ALLOW_SSC) != 0UL) {
			device_clk_set_ssc(dev, clkidx, true);
		} else {
			device_clk_set_ssc(dev, clkidx, false);
		}

		if ((flags & TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE) != 0UL) {
			device_clk_set_freq_change(dev, clkidx, true);
		} else {
			device_clk_set_freq_change(dev, clkidx, false);
		}

		if ((flags & TISCI_MSG_FLAG_CLOCK_INPUT_TERM) != 0UL) {
			device_clk_set_input_term(dev, clkidx, true);
		} else {
			device_clk_set_input_term(dev, clkidx, false);
		}

		if (!device_clk_set_gated(dev, clkidx, gated)) {
			ret = -EINVAL;
		}
	}

	mmr_lock_all();

	return ret;
}

int32_t get_clock_handler(struct tisci_msg_get_clock_req *msg_recv)
{
	struct tisci_msg_get_clock_req *req =
		(struct tisci_msg_get_clock_req *) msg_recv;
	struct tisci_msg_get_clock_resp *resp =
		(struct tisci_msg_get_clock_resp *) msg_recv;
	struct device *dev = NULL;
	uint32_t id = req->device;
	dev_clk_idx_t clkidx;
	int32_t ret = SUCCESS;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_GET_CLOCK);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));

	resp->hdr.flags = 0U;

	ret = device_prepare_nonexclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		uint8_t prog;
		uint8_t state;

		prog = (uint8_t) (device_clk_get_sw_gated(dev, clkidx) ?
				  TISCI_MSG_VALUE_CLOCK_SW_STATE_UNREQ :
				  TISCI_MSG_VALUE_CLOCK_SW_STATE_AUTO);

		state = (uint8_t) (device_clk_get_hw_ready(dev, clkidx) ?
				   TISCI_MSG_VALUE_CLOCK_HW_STATE_READY :
				   TISCI_MSG_VALUE_CLOCK_HW_STATE_NOT_READY);

		if (device_clk_get_ssc(dev, clkidx)) {
			resp->hdr.flags |= TISCI_MSG_FLAG_CLOCK_ALLOW_SSC;
		}

		if (device_clk_get_hw_ssc(dev, clkidx)) {
			resp->hdr.flags |= TISCI_MSG_FLAG_CLOCK_SSC_ACTIVE;
		}

		if (device_clk_get_freq_change(dev, clkidx)) {
			resp->hdr.flags |= TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE;
		}

		if (device_clk_get_input_term(dev, clkidx)) {
			resp->hdr.flags |= TISCI_MSG_FLAG_CLOCK_INPUT_TERM;
		}

		resp->programmed_state	= prog;
		resp->current_state	= state;
	}

	mmr_lock_all();

	return ret;
}

int32_t set_clock_parent_handler(struct tisci_msg_set_clock_parent_req *msg_recv)
{
	struct tisci_msg_set_clock_parent_req *req =
		(struct tisci_msg_set_clock_parent_req *) msg_recv;
	struct tisci_msg_set_clock_parent_resp *resp =
		(struct tisci_msg_set_clock_parent_resp *) msg_recv;
	struct device *dev = NULL;
	uint32_t id = req->device;
	dev_clk_idx_t clkidx;
	dev_clk_idx_t parent;
	int32_t ret = SUCCESS;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	if (req->parent == 255UL) {
		parent = (dev_clk_idx_t) req->parent32;
	} else {
		parent = (dev_clk_idx_t) req->parent;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_SET_CLOCK_PARENT);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_VAL, parent);

	resp->hdr.flags = 0U;

	ret = device_prepare_exclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		if (!device_clk_set_parent(dev, clkidx, parent)) {
			ret = -EINVAL;
		}
	}

	mmr_lock_all();

	return ret;
}

int32_t get_clock_parent_handler(struct tisci_msg_get_clock_parent_req *msg_recv)
{
	struct tisci_msg_get_clock_parent_req *req =
		(struct tisci_msg_get_clock_parent_req *) msg_recv;
	struct tisci_msg_get_clock_parent_resp *resp =
		(struct tisci_msg_get_clock_parent_resp *) msg_recv;
	struct device *dev = NULL;
	uint32_t id = req->device;
	dev_clk_idx_t clkidx;
	int32_t ret = SUCCESS;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_GET_CLOCK_PARENT);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));

	resp->hdr.flags = 0U;

	ret = device_prepare_nonexclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		dev_clk_idx_t parent;

		parent = device_clk_get_parent(dev, clkidx);

		if (parent == DEV_CLK_ID_NONE) {
			ret = -EINVAL;
		} else {
			/* Use extended API for clock indexes >= 255 */
			if (parent >= 255UL) {
				resp->parent = 255U;
				resp->parent32 = parent;
			} else {
				resp->parent = (uint8_t) parent;
				resp->parent32 = 0xffffffffU;
			}
		}
	}

	mmr_lock_all();

	return ret;
}

int32_t get_num_clock_parents_handler(struct tisci_msg_get_num_clock_parents_req *msg_recv)
{
	struct tisci_msg_get_num_clock_parents_req *req =
		(struct tisci_msg_get_num_clock_parents_req *) msg_recv;
	struct tisci_msg_get_num_clock_parents_resp *resp =
		(struct tisci_msg_get_num_clock_parents_resp *) msg_recv;
	struct device *dev = NULL;
	uint32_t id = req->device;
	dev_clk_idx_t clkidx;
	int32_t ret = SUCCESS;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED,
		 TISCI_MSG_GET_NUM_CLOCK_PARENTS);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));

	resp->hdr.flags = 0U;

	ret = device_prepare_nonexclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		dev_clk_idx_t num_parents;

		num_parents = device_clk_get_num_parents(dev, clkidx);

		if (num_parents == DEV_CLK_ID_NONE) {
			ret = -EINVAL;
		} else {
			/* Use extended API for clock indexes >= 255 */
			if (num_parents >= 255UL) {
				resp->num_parents = 255U;
				resp->num_parentint32_t = num_parents;
			} else {
				resp->num_parents = (uint8_t) num_parents;
				resp->num_parentint32_t = 0xffffffffU;
			}
		}
	}

	mmr_lock_all();

	return ret;
}

int32_t set_freq_handler(struct tisci_msg_set_freq_req *msg_recv)
{
	struct tisci_msg_set_freq_req *req =
		(struct tisci_msg_set_freq_req *) msg_recv;
	struct tisci_msg_set_freq_resp *resp =
		(struct tisci_msg_set_freq_resp *) msg_recv;
	struct device *dev = NULL;
	uint32_t id = req->device;
	uint64_t min_freq_hz = req->min_freq_hz;
	uint64_t target_freq_hz = req->target_freq_hz;
	uint64_t max_freq_hz = req->max_freq_hz;
	dev_clk_idx_t clkidx;
	int32_t ret = SUCCESS;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_SET_FREQ);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));

	resp->hdr.flags = 0U;

	ret = device_prepare_exclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		if ((min_freq_hz > (uint64_t) ULONG_MAX) || (min_freq_hz > target_freq_hz) ||
		    (target_freq_hz > max_freq_hz)) {
			ret = -EINVAL;
		}
	}

	if (ret == SUCCESS) {
		if (max_freq_hz > (uint64_t) ULONG_MAX) {
			max_freq_hz = ULONG_MAX;
		}

		if (target_freq_hz > (uint64_t) ULONG_MAX) {
			target_freq_hz = ULONG_MAX;
		}

		if (!device_clk_set_freq(dev, clkidx, (uint32_t) min_freq_hz,
					 (uint32_t) target_freq_hz, (uint32_t) max_freq_hz)) {
			ret = -EINVAL;
		}
	}

	mmr_lock_all();

	return ret;
}

int32_t query_freq_handler(uint32_t *msg_recv)
{
	struct tisci_msg_query_freq_req *req =
		(struct tisci_msg_query_freq_req *) msg_recv;
	struct tisci_msg_query_freq_resp *resp =
		(struct tisci_msg_query_freq_resp *) msg_recv;
	struct device *dev = NULL;
	uint32_t id = req->device;
	uint64_t min_freq_hz = req->min_freq_hz;
	uint64_t target_freq_hz = req->target_freq_hz;
	uint64_t max_freq_hz = req->max_freq_hz;
	dev_clk_idx_t clkidx;
	uint64_t freq_hz;
	int32_t ret = SUCCESS;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_QUERY_FREQ);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));

	resp->hdr.flags = 0U;

	ret = device_prepare_exclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		if ((min_freq_hz > (uint64_t) ULONG_MAX) || (min_freq_hz > target_freq_hz) ||
		    (target_freq_hz > max_freq_hz)) {
			ret = -EINVAL;
		}
	}

	if (ret == SUCCESS) {
		if (max_freq_hz > (uint64_t) ULONG_MAX) {
			max_freq_hz = ULONG_MAX;
		}

		if (target_freq_hz > (uint64_t) ULONG_MAX) {
			target_freq_hz = ULONG_MAX;
		}

		freq_hz = device_clk_query_freq(dev, clkidx, (uint32_t) min_freq_hz,
						(uint32_t) target_freq_hz,
						(uint32_t) max_freq_hz);

		resp->freq_hz = freq_hz;
	}

	mmr_lock_all();

	return ret;
}

int32_t get_freq_handler(struct tisci_msg_get_freq_req *msg_recv)
{
	struct tisci_msg_get_freq_req *req =
		(struct tisci_msg_get_freq_req *) msg_recv;
	struct tisci_msg_get_freq_resp *resp =
		(struct tisci_msg_get_freq_resp *) msg_recv;
	struct device *dev = NULL;
	uint32_t id = req->device;
	dev_clk_idx_t clkidx;
	int32_t ret = SUCCESS;

	mmr_unlock_all();

	/* Handle higher clock IDs */
	if (req->clk == 255UL) {
		clkidx = (dev_clk_idx_t) req->clk32;
	} else {
		clkidx = (dev_clk_idx_t) req->clk;
	}

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_GET_FREQ);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID,
		 id | ((uint32_t) clkidx << TRACE_PM_MSG_CLK_ID_SHIFT));

	resp->hdr.flags = 0U;

	ret = device_prepare_nonexclusive(req->hdr.host, id, NULL, &dev);
	if (ret == SUCCESS) {
		resp->freq_hz = device_clk_get_freq(dev, clkidx);
	}

	mmr_lock_all();

	return ret;
}
