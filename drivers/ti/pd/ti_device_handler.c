/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device State Request Handler
 *
 * This module processes device state requests from the host, managing device
 * enable/disable, state transitions, and handling exclusive device access controls.
 */

#include <errno.h>

#include <common/debug.h>

#include <ti_device.h>
#include <ti_device_handler.h>
#include <ti_device_pm.h>
#include <ti_device_prepare.h>
#include <ti_host_idx_mapping.h>
#include <ti_psc.h>

#define TI_DEVICE_HW_STATE_OFF	    0U
#define TI_DEVICE_HW_STATE_TRANS    2U

int32_t ti_set_device_handler(uint32_t dev_id, bool enable)
{
	struct ti_device *dev = NULL;
	uint8_t host_id = TI_HOST_ID_TIFS;
	bool retention;
	uint8_t host_idx;
	uint32_t current_device_state;
	uint8_t state;
	int32_t ret;

	if (enable) {
		state = TI_DEVICE_SW_STATE_ON;
	} else {
		state = TI_DEVICE_SW_STATE_AUTO_OFF;
	}

	VERBOSE("SET_DEVICE: dev_id=%u state=%u\n", dev_id, state);

	ret = ti_device_prepare_exclusive(host_id, dev_id, &host_idx, &dev);
	if (ret != 0) {
		return ret;
	}

	switch (state) {
	case TI_DEVICE_SW_STATE_AUTO_OFF:
		enable = false;
		retention = false;
		break;
	case TI_DEVICE_SW_STATE_ON:
		enable = true;
		retention = true;
		break;
	default:
		ERROR("INVALID_STATE: state=%d\n", state);
		return -EINVAL;
	}

	/* Allow any host to modify device */
	dev->exclusive = 0U;

	/* Ordering to avoid unnecessary PD transitions */
	if (retention == true) {
		ti_device_set_retention(dev, retention);
	}
	ti_device_set_state(dev, host_idx, enable);
	if (retention == false) {
		ti_device_set_retention(dev, retention);
	}
	/* Check the device state after processing device_set_state function */
	current_device_state = ti_device_get_state(dev);
	if (state == TI_DEVICE_SW_STATE_ON) {
		if (current_device_state != TI_DEVICE_HW_STATE_ON) {
			return -EIO;
		}
	} else {
		if (current_device_state == TI_DEVICE_HW_STATE_TRANS) {
			/* Device with multiple psc's might be in transition state during the
			 * requested state is off/retention because of some psc's sibling devices
			 * might be on which keep that psc's on, this results in mixed state of
			 * psc's which is an exception to overcome with this exception below
			 * condition is written.
			 */
			if (((struct ti_dev_data *)(ti_get_dev_data(dev)))->soc.psc_idx !=
			      TI_PSC_DEV_MULTIPLE) {
				return -EIO;
			}
		}
	}

	return 0;
}

bool ti_get_device_handler(uint32_t dev_id)
{
	struct ti_device *dev = NULL;
	uint8_t host_id = TI_HOST_ID_TIFS;
	uint8_t host_idx;
	uint8_t programmed_state;
	uint8_t current_state;
	int32_t ret;

	VERBOSE("GET_DEVICE: dev_id=%u\n", dev_id);

	ret = ti_device_prepare_nonexclusive(host_id, dev_id, &host_idx, &dev);
	if (ret != 0) {
		return false;
	}

	if ((dev->flags & TI_DEV_FLAG_ENABLED(host_idx)) != 0UL) {
		programmed_state = TI_DEVICE_SW_STATE_ON;
	} else if ((dev->flags & TI_DEV_FLAG_RETENTION) != 0UL) {
		programmed_state = TI_DEVICE_SW_STATE_RETENTION;
	} else {
		programmed_state = TI_DEVICE_SW_STATE_AUTO_OFF;
	}

	switch (ti_device_get_state(dev)) {
	case TI_DEVICE_STATE_DISABLED:
		current_state = TI_DEVICE_HW_STATE_OFF;
		break;
	case TI_DEVICE_STATE_ENABLED:
		current_state = TI_DEVICE_HW_STATE_ON;
		break;
	default:
		current_state = TI_DEVICE_HW_STATE_TRANS;
		break;
	}

	if ((programmed_state == (uint8_t)TI_DEVICE_SW_STATE_ON) &&
	    (current_state == (uint8_t)TI_DEVICE_HW_STATE_ON)) {
		return true;
	}

	return false;
}
