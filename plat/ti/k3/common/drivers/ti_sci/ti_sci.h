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
 * Clock control operations
 *
 * - ti_sci_clock_set_state - Set clock state helper
 *              @flags: Header flags as needed
 *              @state: State to request for the clock.
 * - ti_sci_clock_get_state - Get clock state helper
 *              @programmed_state: State requested for clock to move to
 *              @current_state: State that the clock is currently in
 * - ti_sci_clock_get - Get control of a clock from TI SCI
 *              @needs_ssc: 'true' iff Spread Spectrum clock is desired
 *              @can_change_freq: 'true' iff frequency change is desired
 *              @enable_input_term: 'true' iff input termination is desired
 * - ti_sci_clock_idle - Idle a clock which is in our control
 * - ti_sci_clock_put - Release a clock from our control
 * - ti_sci_clock_is_auto - Is the clock being auto managed
 *              @req_state: state indicating if the clock is auto managed
 * - ti_sci_clock_is_on - Is the clock ON
 *              @req_state: state indicating if the clock is managed by us and enabled
 *              @curr_state: state indicating if the clock is ready for operation
 * - ti_sci_clock_is_off - Is the clock OFF
 *              @req_state: state indicating if the clock is managed by us and disabled
 *              @curr_state: state indicating if the clock is NOT ready for operation
 * - ti_sci_clock_set_parent - Set the clock source of a specific device clock
 *              @parent_id: Parent clock identifier to set
 * - ti_sci_clock_get_parent - Get current parent clock source
 *              @parent_id: Current clock parent
 * - ti_sci_clock_get_num_parents - Get num parents of the current clk source
 *              @num_parents: Returns he number of parents to the current clock.
 * - ti_sci_clock_get_match_freq - Find a good match for frequency
 *              @match_freq: Frequency match in Hz response.
 * - ti_sci_clock_set_freq - Set a frequency for clock
 * - ti_sci_clock_get_freq - Get current frequency
 *              @freq: Currently frequency in Hz
 *
 * NOTE: for all these functions, the following are generic in nature:
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * @max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * Returns 0 for successful request, else returns corresponding error message.
 *
 * Request for the clock - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_clock with put_clock. No refcounting is
 * managed by driver for that purpose.
 */
int ti_sci_clock_set_state(uint32_t dev_id, uint8_t clk_id,
			   uint32_t flags, uint8_t state);
int ti_sci_clock_get_state(uint32_t dev_id, uint8_t clk_id,
			   uint8_t *programmed_state, uint8_t *current_state);
int ti_sci_clock_get(uint32_t dev_id, uint8_t clk_id,
		     bool needs_ssc, bool can_change_freq,
		     bool enable_input_term);
int ti_sci_clock_idle(uint32_t dev_id, uint8_t clk_id);
int ti_sci_clock_put(uint32_t dev_id, uint8_t clk_id);
int ti_sci_clock_is_auto(uint32_t dev_id, uint8_t clk_id,
			 bool *req_state);
int ti_sci_clock_is_on(uint32_t dev_id, uint8_t clk_id,
		       bool *req_state, bool *curr_state);
int ti_sci_clock_is_off(uint32_t dev_id, uint8_t clk_id,
			bool *req_state, bool *curr_state);
int ti_sci_clock_set_parent(uint32_t dev_id, uint8_t clk_id,
			    uint8_t parent_id);
int ti_sci_clock_get_parent(uint32_t dev_id, uint8_t clk_id,
			    uint8_t *parent_id);
int ti_sci_clock_get_num_parents(uint32_t dev_id, uint8_t clk_id,
				 uint8_t *num_parents);
int ti_sci_clock_get_match_freq(uint32_t dev_id, uint8_t clk_id,
				uint64_t min_freq, uint64_t target_freq,
				uint64_t max_freq, uint64_t *match_freq);
int ti_sci_clock_set_freq(uint32_t dev_id, uint8_t clk_id,
			  uint64_t min_freq, uint64_t target_freq,
			  uint64_t max_freq);
int ti_sci_clock_get_freq(uint32_t dev_id, uint8_t clk_id, uint64_t *freq);

/**
 * Core control operations
 *
 * - ti_sci_core_reboot() - Command to request system reset
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
int ti_sci_core_reboot(void);

/**
 * Processor control operations
 *
 * - ti_sci_proc_request - Command to request a physical processor control
 * - ti_sci_proc_release - Command to release a physical processor control
 * - ti_sci_proc_handover - Command to handover a physical processor control to
 *                          a host in the processor's access control list.
 *              @host_id: Host ID to get the control of the processor
 * - ti_sci_proc_set_boot_cfg - Command to set the processor boot configuration flags
 *              @config_flags_set: Configuration flags to be set
 *              @config_flags_clear: Configuration flags to be cleared.
 * - ti_sci_proc_set_boot_ctrl - Command to set the processor boot control flags
 *              @control_flags_set: Control flags to be set
 *              @control_flags_clear: Control flags to be cleared
 * - ti_sci_proc_auth_boot_image - Command to authenticate and load the image
 *                                 and then set the processor configuration flags.
 *              @cert_addr: Memory address at which payload image certificate is located.
 * - ti_sci_proc_get_boot_status - Command to get the processor boot status
 *
 * NOTE: for all these functions, the following are generic in nature:
 * @proc_id:	Processor ID
 * Returns 0 for successful request, else returns corresponding error message.
 */
int ti_sci_proc_request(uint8_t proc_id);
int ti_sci_proc_release(uint8_t proc_id);
int ti_sci_proc_handover(uint8_t proc_id, uint8_t host_id);
int ti_sci_proc_set_boot_cfg(uint8_t proc_id, uint64_t bootvector,
			     uint32_t config_flags_set,
			     uint32_t config_flags_clear);
int ti_sci_proc_set_boot_ctrl(uint8_t proc_id, uint32_t control_flags_set,
			      uint32_t control_flags_clear);
int ti_sci_proc_auth_boot_image(uint8_t proc_id, uint64_t cert_addr);
int ti_sci_proc_get_boot_status(uint8_t proc_id, uint64_t *bv,
				uint32_t *cfg_flags,
				uint32_t *ctrl_flags,
				uint32_t *sts_flags);

/**
 * ti_sci_init() - Basic initialization
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
int ti_sci_init(void);

#endif	/* __TI_SCI_H */
