/*
 * Texas Instruments System Control Interface API
 *   Based on Linux and U-Boot implementation
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TI_SCI_H
#define __TI_SCI_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Device control operations
 *
 * - ti_sci_device_set_state - Set device state helper
 *              @flags: flags to setup for the device
 *              @state: State to move the device to
 * - ti_sci_device_get_state - Get device state helper
 *              @clcnt: Pointer to Context Loss Count
 *              @resets: pointer to resets
 *              @p_state: pointer to p_state
 *              @c_state: pointer to c_state
 * - ti_sci_device_get - command to request for device managed by TISCI
 * - ti_sci_device_idle - Command to idle a device managed by TISCI
 * - ti_sci_device_put - command to release a device managed by TISCI
 * - ti_sci_device_is_valid - Is the device valid
 * - ti_sci_device_get_clcnt - Get context loss counter
 *              @count: Pointer to Context Loss counter to populate
 * - ti_sci_device_is_idle - Check if the device is requested to be idle
 *              @r_state: true if requested to be idle
 * - ti_sci_device_is_stop - Check if the device is requested to be stopped
 *              @r_state: true if requested to be stopped
 *              @curr_state: true if currently stopped.
 * - ti_sci_device_is_on - Check if the device is requested to be ON
 *              @r_state: true if requested to be ON
 *              @curr_state: true if currently ON and active
 * - ti_sci_device_is_trans - Check if the device is currently transitioning
 *              @curr_state: true if currently transitioning.
 * - ti_sci_device_set_resets - Command to set resets for
 *                              device managed by TISCI
 *              @reset_state: Device specific reset bit field
 * - ti_sci_device_get_resets - Get reset state for device managed by TISCI
 *              @reset_state: Pointer to reset state to populate
 *
 * NOTE: for all these functions, the following are generic in nature:
 * @id:		Device Identifier
 * Returns 0 for successful request, else returns corresponding error message.
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 */
int ti_sci_device_set_state(uint32_t id, uint32_t flags, uint8_t state);
int ti_sci_device_get_state(uint32_t id,  uint32_t *clcnt,  uint32_t *resets,
			    uint8_t *p_state,  uint8_t *c_state);
int ti_sci_device_get(uint32_t id);
int ti_sci_device_idle(uint32_t id);
int ti_sci_device_put(uint32_t id);
int ti_sci_device_is_valid(uint32_t id);
int ti_sci_device_get_clcnt(uint32_t id, uint32_t *count);
int ti_sci_device_is_idle(uint32_t id, bool *r_state);
int ti_sci_device_is_stop(uint32_t id, bool *r_state,  bool *curr_state);
int ti_sci_device_is_on(uint32_t id, bool *r_state,  bool *curr_state);
int ti_sci_device_is_trans(uint32_t id, bool *curr_state);
int ti_sci_device_set_resets(uint32_t id, uint32_t reset_state);
int ti_sci_device_get_resets(uint32_t id, uint32_t *reset_state);

/**
 * ti_sci_init() - Basic initialization
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
int ti_sci_init(void);

#endif	/* __TI_SCI_H */
