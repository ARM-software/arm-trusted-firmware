/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions and communication with PMU via
 * IPI interrupts
 */

#include <arch_helpers.h>
#include <plat/common/platform.h>

#include "pm_api_clock.h"
#include "pm_api_ioctl.h"
#include "pm_api_pinctrl.h"
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_common.h"
#include "pm_ipi.h"

/* default shutdown/reboot scope is system(2) */
static unsigned int pm_shutdown_scope = PMF_SHUTDOWN_SUBTYPE_SYSTEM;

/**
 * pm_get_shutdown_scope() - Get the currently set shutdown scope
 *
 * @return	Shutdown scope value
 */
unsigned int pm_get_shutdown_scope(void)
{
	return pm_shutdown_scope;
}

/**
 * Assigning of argument values into array elements.
 */
#define PM_PACK_PAYLOAD1(pl, arg0) {	\
	pl[0] = (uint32_t)(arg0);	\
}

#define PM_PACK_PAYLOAD2(pl, arg0, arg1) {	\
	pl[1] = (uint32_t)(arg1);		\
	PM_PACK_PAYLOAD1(pl, arg0);		\
}

#define PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2) {	\
	pl[2] = (uint32_t)(arg2);			\
	PM_PACK_PAYLOAD2(pl, arg0, arg1);		\
}

#define PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3) {	\
	pl[3] = (uint32_t)(arg3);			\
	PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2);		\
}

#define PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4) {	\
	pl[4] = (uint32_t)(arg4);				\
	PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3);		\
}

#define PM_PACK_PAYLOAD6(pl, arg0, arg1, arg2, arg3, arg4, arg5) {	\
	pl[5] = (uint32_t)(arg5);					\
	PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4);		\
}

/**
 * pm_self_suspend() - PM call for processor to suspend itself
 * @nid		Node id of the processor or subsystem
 * @latency	Requested maximum wakeup latency (not supported)
 * @state	Requested state
 * @address	Resume address
 *
 * This is a blocking call, it will return only once PMU has responded.
 * On a wakeup, resume address will be automatically set by PMU.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_self_suspend(enum pm_node_id nid,
				   unsigned int latency,
				   unsigned int state,
				   uintptr_t address)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	unsigned int cpuid = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpuid);

	/*
	 * Do client specific suspend operations
	 * (e.g. set powerdown request bit)
	 */
	pm_client_suspend(proc, state);
	/* Send request to the PMU */
	PM_PACK_PAYLOAD6(payload, PM_SELF_SUSPEND, proc->node_id, latency,
			 state, address, (address >> 32));
	return pm_ipi_send_sync(proc, payload, NULL, 0);
}

/**
 * pm_req_suspend() - PM call to request for another PU or subsystem to
 *		      be suspended gracefully.
 * @target	Node id of the targeted PU or subsystem
 * @ack		Flag to specify whether acknowledge is requested
 * @latency	Requested wakeup latency (not supported)
 * @state	Requested state (not supported)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_req_suspend(enum pm_node_id target,
				  enum pm_request_ack ack,
				  unsigned int latency, unsigned int state)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_REQ_SUSPEND, target, ack, latency, state);
	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_req_wakeup() - PM call for processor to wake up selected processor
 *		     or subsystem
 * @target	Node id of the processor or subsystem to wake up
 * @ack		Flag to specify whether acknowledge requested
 * @set_address	Resume address presence indicator
 *				1 resume address specified, 0 otherwise
 * @address	Resume address
 *
 * This API function is either used to power up another APU core for SMP
 * (by PSCI) or to power up an entirely different PU or subsystem, such
 * as RPU0, RPU, or PL_CORE_xx. Resume address for the target PU will be
 * automatically set by PMU.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_req_wakeup(enum pm_node_id target,
				 unsigned int set_address,
				 uintptr_t address,
				 enum pm_request_ack ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	uint64_t encoded_address;


	/* encode set Address into 1st bit of address */
	encoded_address = address;
	encoded_address |= !!set_address;

	/* Send request to the PMU to perform the wake of the PU */
	PM_PACK_PAYLOAD5(payload, PM_REQ_WAKEUP, target, encoded_address,
			 encoded_address >> 32, ack);

	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_force_powerdown() - PM call to request for another PU or subsystem to
 *			  be powered down forcefully
 * @target	Node id of the targeted PU or subsystem
 * @ack		Flag to specify whether acknowledge is requested
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_force_powerdown(enum pm_node_id target,
				      enum pm_request_ack ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD3(payload, PM_FORCE_POWERDOWN, target, ack);

	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_abort_suspend() - PM call to announce that a prior suspend request
 *			is to be aborted.
 * @reason	Reason for the abort
 *
 * Calling PU expects the PMU to abort the initiated suspend procedure.
 * This is a non-blocking call without any acknowledge.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	pm_client_abort_suspend();
	/* Send request to the PMU */
	/* TODO: allow passing the node ID of the affected CPU */
	PM_PACK_PAYLOAD3(payload, PM_ABORT_SUSPEND, reason,
			 primary_proc->node_id);
	return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_set_wakeup_source() - PM call to specify the wakeup source while suspended
 * @target	Node id of the targeted PU or subsystem
 * @wkup_node	Node id of the wakeup peripheral
 * @enable	Enable or disable the specified peripheral as wake source
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_wakeup_source(enum pm_node_id target,
					enum pm_node_id wkup_node,
					unsigned int enable)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD4(payload, PM_SET_WAKEUP_SOURCE, target, wkup_node,
			 enable);
	return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_system_shutdown() - PM call to request a system shutdown or restart
 * @type	Shutdown or restart? 0=shutdown, 1=restart, 2=setscope
 * @subtype	Scope: 0=APU-subsystem, 1=PS, 2=system
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_system_shutdown(unsigned int type, unsigned int subtype)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	if (type == PMF_SHUTDOWN_TYPE_SETSCOPE_ONLY) {
		/* Setting scope for subsequent PSCI reboot or shutdown */
		pm_shutdown_scope = subtype;
		return PM_RET_SUCCESS;
	}

	PM_PACK_PAYLOAD3(payload, PM_SYSTEM_SHUTDOWN, type, subtype);
	return pm_ipi_send_non_blocking(primary_proc, payload);
}

/* APIs for managing PM slaves: */

/**
 * pm_req_node() - PM call to request a node with specific capabilities
 * @nid		Node id of the slave
 * @capabilities Requested capabilities of the slave
 * @qos		Quality of service (not supported)
 * @ack		Flag to specify whether acknowledge is requested
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_req_node(enum pm_node_id nid,
			       unsigned int capabilities,
			       unsigned int qos,
			       enum pm_request_ack ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD5(payload, PM_REQ_NODE, nid, capabilities, qos, ack);

	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_set_requirement() - PM call to set requirement for PM slaves
 * @nid		Node id of the slave
 * @capabilities Requested capabilities of the slave
 * @qos		Quality of service (not supported)
 * @ack		Flag to specify whether acknowledge is requested
 *
 * This API function is to be used for slaves a PU already has requested
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_requirement(enum pm_node_id nid,
				      unsigned int capabilities,
				      unsigned int qos,
				      enum pm_request_ack ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD5(payload, PM_SET_REQUIREMENT, nid, capabilities, qos,
			 ack);

	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_release_node() - PM call to release a node
 * @nid		Node id of the slave
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_release_node(enum pm_node_id nid)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD2(payload, PM_RELEASE_NODE, nid);
	return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_set_max_latency() - PM call to set wakeup latency requirements
 * @nid		Node id of the slave
 * @latency	Requested maximum wakeup latency
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_max_latency(enum pm_node_id nid,
				      unsigned int latency)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD3(payload, PM_SET_MAX_LATENCY, nid, latency);
	return pm_ipi_send(primary_proc, payload);
}

/* Miscellaneous API functions */

/**
 * pm_get_api_version() - Get version number of PMU PM firmware
 * @version	Returns 32-bit version number of PMU Power Management Firmware
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_api_version(unsigned int *version)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD1(payload, PM_GET_API_VERSION);
	return pm_ipi_send_sync(primary_proc, payload, version, 1);
}

/**
 * pm_set_configuration() - PM call to set system configuration
 * @phys_addr	Physical 32-bit address of data structure in memory
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_configuration(unsigned int phys_addr)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD2(payload, PM_SET_CONFIGURATION, phys_addr);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_init_finalize() - Call to notify PMU firmware that master has power
 *			management enabled and that it has finished its
 *			initialization
 *
 * @return	Status returned by the PMU firmware
 */
enum pm_ret_status pm_init_finalize(void)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD1(payload, PM_INIT_FINALIZE);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_get_node_status() - PM call to request a node's current status
 * @nid		Node id
 * @ret_buff	Buffer for the return values:
 *		[0] - Current power state of the node
 *		[1] - Current requirements for the node (slave nodes only)
 *		[2] - Current usage status for the node (slave nodes only)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_node_status(enum pm_node_id nid,
				      uint32_t *ret_buff)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD2(payload, PM_GET_NODE_STATUS, nid);
	return pm_ipi_send_sync(primary_proc, payload, ret_buff, 3);
}

/**
 * pm_register_notifier() - Register the PU to be notified of PM events
 * @nid		Node id of the slave
 * @event	The event to be notified about
 * @wake	Wake up on event
 * @enable	Enable or disable the notifier
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_register_notifier(enum pm_node_id nid,
					unsigned int event,
					unsigned int wake,
					unsigned int enable)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD5(payload, PM_REGISTER_NOTIFIER,
			 nid, event, wake, enable);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_get_op_characteristic() - PM call to request operating characteristics
 *				of a node
 * @nid		Node id of the slave
 * @type	Type of the operating characteristic
 *		(power, temperature and latency)
 * @result	Returns the operating characteristic for the requested node,
 *		specified by the type
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_op_characteristic(enum pm_node_id nid,
					    enum pm_opchar_type type,
					    uint32_t *result)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD3(payload, PM_GET_OP_CHARACTERISTIC, nid, type);
	return pm_ipi_send_sync(primary_proc, payload, result, 1);
}

/* Direct-Control API functions */

/**
 * pm_reset_assert() - Assert reset
 * @reset	Reset ID
 * @assert	Assert (1) or de-assert (0)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_reset_assert(unsigned int reset,
				   unsigned int assert)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD3(payload, PM_RESET_ASSERT, reset, assert);
	return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_reset_get_status() - Get current status of a reset line
 * @reset	Reset ID
 * @reset_status Returns current status of selected reset line
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_reset_get_status(unsigned int reset,
				       unsigned int *reset_status)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD2(payload, PM_RESET_GET_STATUS, reset);
	return pm_ipi_send_sync(primary_proc, payload, reset_status, 1);
}

/**
 * pm_mmio_write() - Perform write to protected mmio
 * @address	Address to write to
 * @mask	Mask to apply
 * @value	Value to write
 *
 * This function provides access to PM-related control registers
 * that may not be directly accessible by a particular PU.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_mmio_write(uintptr_t address,
				 unsigned int mask,
				 unsigned int value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, PM_MMIO_WRITE, address, mask, value);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_mmio_read() - Read value from protected mmio
 * @address	Address to write to
 * @value	Value to write
 *
 * This function provides access to PM-related control registers
 * that may not be directly accessible by a particular PU.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_mmio_read(uintptr_t address, unsigned int *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD2(payload, PM_MMIO_READ, address);
	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_fpga_load() - Load the bitstream into the PL.
 *
 * This function provides access to the xilfpga library to load
 * the Bit-stream into PL.
 *
 * address_low: lower 32-bit Linear memory space address
 *
 * address_high: higher 32-bit Linear memory space address
 *
 * size:	Number of 32bit words
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_fpga_load(uint32_t address_low,
				uint32_t address_high,
				uint32_t size,
				uint32_t flags)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_FPGA_LOAD, address_high, address_low,
						size, flags);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_fpga_get_status() - Read value from fpga status register
 * @value       Value to read
 *
 * This function provides access to the xilfpga library to get
 * the fpga status
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_fpga_get_status(unsigned int *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD1(payload, PM_FPGA_GET_STATUS);
	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_get_chipid() - Read silicon ID registers
 * @value       Buffer for return values. Must be large enough
 *		to hold 8 bytes.
 *
 * @return      Returns silicon ID registers
 */
enum pm_ret_status pm_get_chipid(uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD1(payload, PM_GET_CHIPID);
	return pm_ipi_send_sync(primary_proc, payload, value, 2);
}

/**
 * pm_secure_rsaaes() - Load the secure images.
 *
 * This function provides access to the xilsecure library to load
 * the authenticated, encrypted, and authenicated/encrypted images.
 *
 * address_low: lower 32-bit Linear memory space address
 *
 * address_high: higher 32-bit Linear memory space address
 *
 * size:	Number of 32bit words
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_secure_rsaaes(uint32_t address_low,
				uint32_t address_high,
				uint32_t size,
				uint32_t flags)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_SECURE_RSA_AES, address_high, address_low,
			 size, flags);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_aes_engine() - Aes data blob encryption/decryption
 * This function provides access to the xilsecure library to
 * encrypt/decrypt data blobs.
 *
 * address_low: lower 32-bit address of the AesParams structure
 *
 * address_high: higher 32-bit address of the AesParams structure
 *
 * value:        Returned output value
 *
 * @return       Returns status, either success or error+reason
 */
enum pm_ret_status pm_aes_engine(uint32_t address_high,
				 uint32_t address_low,
				 uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD3(payload, PM_SECURE_AES, address_high, address_low);
	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_pinctrl_request() - Request Pin from firmware
 * @pin		Pin number to request
 *
 * This function requests pin from firmware.
 *
 * @return	Returns status, either success or error+reason.
 */
enum pm_ret_status pm_pinctrl_request(unsigned int pin)
{
	return PM_RET_SUCCESS;
}

/**
 * pm_pinctrl_release() - Release Pin from firmware
 * @pin		Pin number to release
 *
 * This function releases pin from firmware.
 *
 * @return	Returns status, either success or error+reason.
 */
enum pm_ret_status pm_pinctrl_release(unsigned int pin)
{
	return PM_RET_SUCCESS;
}

/**
 * pm_pinctrl_get_function() - Read function id set for the given pin
 * @pin		Pin number
 * @nid		Node ID of function currently set for given pin
 *
 * This function provides the function currently set for the given pin.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_get_function(unsigned int pin,
					   enum pm_node_id *nid)
{
	return pm_api_pinctrl_get_function(pin, nid);
}

/**
 * pm_pinctrl_set_function() - Set function id set for the given pin
 * @pin		Pin number
 * @nid		Node ID of function to set for given pin
 *
 * This function provides the function currently set for the given pin.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_set_function(unsigned int pin,
					   enum pm_node_id nid)
{
	return pm_api_pinctrl_set_function(pin, (unsigned int)nid);
}

/**
 * pm_pinctrl_get_config() - Read value of requested config param for given pin
 * @pin		Pin number
 * @param	Parameter values to be read
 * @value	Buffer for configuration Parameter value
 *
 * This function provides the configuration parameter value for the given pin.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_get_config(unsigned int pin,
					 unsigned int param,
					 unsigned int *value)
{
	return pm_api_pinctrl_get_config(pin, param, value);
}

/**
 * pm_pinctrl_set_config() - Read value of requested config param for given pin
 * @pin		Pin number
 * @param	Parameter to set
 * @value	Parameter value to set
 *
 * This function provides the configuration parameter value for the given pin.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_set_config(unsigned int pin,
					 unsigned int param,
					 unsigned int value)
{
	return pm_api_pinctrl_set_config(pin, param, value);
}

/**
 * pm_ioctl() -  PM IOCTL API for device control and configs
 * @node_id	Node ID of the device
 * @ioctl_id	ID of the requested IOCTL
 * @arg1	Argument 1 to requested IOCTL call
 * @arg2	Argument 2 to requested IOCTL call
 * @out		Returned output value
 *
 * This function calls IOCTL to firmware for device control and configuration.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_ioctl(enum pm_node_id nid,
			    unsigned int ioctl_id,
			    unsigned int arg1,
			    unsigned int arg2,
			    unsigned int *value)
{
	return pm_api_ioctl(nid, ioctl_id, arg1, arg2, value);
}

/**
 * pm_clock_get_num_clocks - PM call to request number of clocks
 * @nclockss: Number of clocks
 *
 * This function is used by master to get number of clocks.
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_clock_get_num_clocks(uint32_t *nclocks)
{
	return pm_api_clock_get_num_clocks(nclocks);
}

/**
 * pm_clock_get_name() - PM call to request a clock's name
 * @clock_id	Clock ID
 * @name	Name of clock (max 16 bytes)
 *
 * This function is used by master to get nmae of clock specified
 * by given clock ID.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_clock_get_name(unsigned int clock_id, char *name)
{
	return pm_api_clock_get_name(clock_id, name);
}

/**
 * pm_clock_get_topology() - PM call to request a clock's topology
 * @clock_id	Clock ID
 * @index	Topology index for next toplogy node
 * @topology	Buffer to store nodes in topology and flags
 *
 * This function is used by master to get topology information for the
 * clock specified by given clock ID. Each response would return 3
 * topology nodes. To get next nodes, caller needs to call this API with
 * index of next node. Index starts from 0.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_clock_get_topology(unsigned int clock_id,
						unsigned int index,
						uint32_t *topology)
{
	return pm_api_clock_get_topology(clock_id, index, topology);
}

/**
 * pm_clock_get_fixedfactor_params() - PM call to request a clock's fixed factor
 *				 parameters for fixed clock
 * @clock_id	Clock ID
 * @mul		Multiplication value
 * @div		Divisor value
 *
 * This function is used by master to get fixed factor parameers for the
 * fixed clock. This API is application only for the fixed clock.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_clock_get_fixedfactor_params(unsigned int clock_id,
							  uint32_t *mul,
							  uint32_t *div)
{
	return pm_api_clock_get_fixedfactor_params(clock_id, mul, div);
}

/**
 * pm_clock_get_parents() - PM call to request a clock's first 3 parents
 * @clock_id	Clock ID
 * @index	Index of next parent
 * @parents	Parents of the given clock
 *
 * This function is used by master to get clock's parents information.
 * This API will return 3 parents with a single response. To get other
 * parents, master should call same API in loop with new parent index
 * till error is returned.
 *
 * E.g First call should have index 0 which will return parents 0, 1 and
 * 2. Next call, index should be 3 which will return parent 3,4 and 5 and
 * so on.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_clock_get_parents(unsigned int clock_id,
					       unsigned int index,
					       uint32_t *parents)
{
	return pm_api_clock_get_parents(clock_id, index, parents);
}

/**
 * pm_clock_get_attributes() - PM call to request a clock's attributes
 * @clock_id	Clock ID
 * @attr	Clock attributes
 *
 * This function is used by master to get clock's attributes
 * (e.g. valid, clock type, etc).
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_clock_get_attributes(unsigned int clock_id,
						  uint32_t *attr)
{
	return pm_api_clock_get_attributes(clock_id, attr);
}

/**
 * pm_clock_gate() - Configure clock gate
 * @clock_id	Id of the clock to be configured
 * @enable	Flag 0=disable (gate the clock), !0=enable (activate the clock)
 *
 * @return	Error if an argument is not valid or status as returned by the
 *		PM controller (PMU)
 */
static enum pm_ret_status pm_clock_gate(unsigned int clock_id,
					unsigned char enable)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	enum pm_ret_status status;
	enum pm_api_id api_id;

	/* Check if clock ID is valid and return an error if it is not */
	status = pm_clock_id_is_valid(clock_id);
	if (status != PM_RET_SUCCESS)
		return status;

	if (enable)
		api_id = PM_CLOCK_ENABLE;
	else
		api_id = PM_CLOCK_DISABLE;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD2(payload, api_id, clock_id);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_enable() - Enable the clock for given id
 * @clock_id: Id of the clock to be enabled
 *
 * This function is used by master to enable the clock
 * including peripherals and PLL clocks.
 *
 * @return:	Error if an argument is not valid or status as returned by the
 *		pm_clock_gate
 */
enum pm_ret_status pm_clock_enable(unsigned int clock_id)
{
	struct pm_pll *pll;

	/* First try to handle it as a PLL */
	pll = pm_clock_get_pll(clock_id);
	if (pll)
		return pm_clock_pll_enable(pll);

	/* It's an on-chip clock, PMU should configure clock's gate */
	return pm_clock_gate(clock_id, 1);
}

/**
 * pm_clock_disable - Disable the clock for given id
 * @clock_id: Id of the clock to be disable
 *
 * This function is used by master to disable the clock
 * including peripherals and PLL clocks.
 *
 * @return:	Error if an argument is not valid or status as returned by the
 *		pm_clock_gate
 */
enum pm_ret_status pm_clock_disable(unsigned int clock_id)
{
	struct pm_pll *pll;

	/* First try to handle it as a PLL */
	pll = pm_clock_get_pll(clock_id);
	if (pll)
		return pm_clock_pll_disable(pll);

	/* It's an on-chip clock, PMU should configure clock's gate */
	return pm_clock_gate(clock_id, 0);
}

/**
 * pm_clock_getstate - Get the clock state for given id
 * @clock_id: Id of the clock to be queried
 * @state: 1/0 (Enabled/Disabled)
 *
 * This function is used by master to get the state of clock
 * including peripherals and PLL clocks.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_clock_getstate(unsigned int clock_id,
				     unsigned int *state)
{
	struct pm_pll *pll;
	uint32_t payload[PAYLOAD_ARG_CNT];
	enum pm_ret_status status;

	/* First try to handle it as a PLL */
	pll = pm_clock_get_pll(clock_id);
	if (pll)
		return pm_clock_pll_get_state(pll, state);

	/* Check if clock ID is a valid on-chip clock */
	status = pm_clock_id_is_valid(clock_id);
	if (status != PM_RET_SUCCESS)
		return status;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD2(payload, PM_CLOCK_GETSTATE, clock_id);
	return pm_ipi_send_sync(primary_proc, payload, state, 1);
}

/**
 * pm_clock_setdivider - Set the clock divider for given id
 * @clock_id: Id of the clock
 * @divider: divider value
 *
 * This function is used by master to set divider for any clock
 * to achieve desired rate.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_clock_setdivider(unsigned int clock_id,
				       unsigned int divider)
{
	enum pm_ret_status status;
	enum pm_node_id nid;
	enum pm_clock_div_id div_id;
	uint32_t payload[PAYLOAD_ARG_CNT];
	const uint32_t div0 = 0xFFFF0000;
	const uint32_t div1 = 0x0000FFFF;
	uint32_t val;

	/* Get PLL node ID using PLL clock ID */
	status = pm_clock_get_pll_node_id(clock_id, &nid);
	if (status == PM_RET_SUCCESS)
		return pm_pll_set_parameter(nid, PM_PLL_PARAM_FBDIV, divider);

	/* Check if clock ID is a valid on-chip clock */
	status = pm_clock_id_is_valid(clock_id);
	if (status != PM_RET_SUCCESS)
		return status;

	if (div0 == (divider & div0)) {
		div_id = PM_CLOCK_DIV0_ID;
		val = divider & ~div0;
	} else if (div1 == (divider & div1)) {
		div_id = PM_CLOCK_DIV1_ID;
		val = (divider & ~div1) >> 16;
	} else {
		return PM_RET_ERROR_ARGS;
	}

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, PM_CLOCK_SETDIVIDER, clock_id, div_id, val);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_getdivider - Get the clock divider for given id
 * @clock_id: Id of the clock
 * @divider: divider value
 *
 * This function is used by master to get divider values
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_clock_getdivider(unsigned int clock_id,
				       unsigned int *divider)
{
	enum pm_ret_status status;
	enum pm_node_id nid;
	uint32_t payload[PAYLOAD_ARG_CNT];
	uint32_t val;

	/* Get PLL node ID using PLL clock ID */
	status = pm_clock_get_pll_node_id(clock_id, &nid);
	if (status == PM_RET_SUCCESS)
		return pm_pll_get_parameter(nid, PM_PLL_PARAM_FBDIV, divider);

	/* Check if clock ID is a valid on-chip clock */
	status = pm_clock_id_is_valid(clock_id);
	if (status != PM_RET_SUCCESS)
		return status;

	if (pm_clock_has_div(clock_id, PM_CLOCK_DIV0_ID)) {
		/* Send request to the PMU to get div0 */
		PM_PACK_PAYLOAD3(payload, PM_CLOCK_GETDIVIDER, clock_id,
				 PM_CLOCK_DIV0_ID);
		status = pm_ipi_send_sync(primary_proc, payload, &val, 1);
		if (status != PM_RET_SUCCESS)
			return status;
		*divider = val;
	}

	if (pm_clock_has_div(clock_id, PM_CLOCK_DIV1_ID)) {
		/* Send request to the PMU to get div1 */
		PM_PACK_PAYLOAD3(payload, PM_CLOCK_GETDIVIDER, clock_id,
				 PM_CLOCK_DIV1_ID);
		status = pm_ipi_send_sync(primary_proc, payload, &val, 1);
		if (status != PM_RET_SUCCESS)
			return status;
		*divider |= val << 16;
	}

	return status;
}

/**
 * pm_clock_setrate - Set the clock rate for given id
 * @clock_id: Id of the clock
 * @rate: rate value in hz
 *
 * This function is used by master to set rate for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_clock_setrate(unsigned int clock_id,
				    uint64_t rate)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_clock_getrate - Get the clock rate for given id
 * @clock_id: Id of the clock
 * @rate: rate value in hz
 *
 * This function is used by master to get rate
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_clock_getrate(unsigned int clock_id,
				    uint64_t *rate)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_clock_setparent - Set the clock parent for given id
 * @clock_id: Id of the clock
 * @parent_index: Index of the parent clock into clock's parents array
 *
 * This function is used by master to set parent for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_clock_setparent(unsigned int clock_id,
				      unsigned int parent_index)
{
	struct pm_pll *pll;
	uint32_t payload[PAYLOAD_ARG_CNT];
	enum pm_ret_status status;

	/* First try to handle it as a PLL */
	pll = pm_clock_get_pll_by_related_clk(clock_id);
	if (pll)
		return pm_clock_pll_set_parent(pll, clock_id, parent_index);

	/* Check if clock ID is a valid on-chip clock */
	status = pm_clock_id_is_valid(clock_id);
	if (status != PM_RET_SUCCESS)
		return status;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD3(payload, PM_CLOCK_SETPARENT, clock_id, parent_index);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_getparent - Get the clock parent for given id
 * @clock_id: Id of the clock
 * @parent_index: parent index
 *
 * This function is used by master to get parent index
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_clock_getparent(unsigned int clock_id,
				      unsigned int *parent_index)
{
	struct pm_pll *pll;
	uint32_t payload[PAYLOAD_ARG_CNT];
	enum pm_ret_status status;

	/* First try to handle it as a PLL */
	pll = pm_clock_get_pll_by_related_clk(clock_id);
	if (pll)
		return pm_clock_pll_get_parent(pll, clock_id, parent_index);

	/* Check if clock ID is a valid on-chip clock */
	status = pm_clock_id_is_valid(clock_id);
	if (status != PM_RET_SUCCESS)
		return status;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD2(payload, PM_CLOCK_GETPARENT, clock_id);
	return pm_ipi_send_sync(primary_proc, payload, parent_index, 1);
}

/**
 * pm_pinctrl_get_num_pins - PM call to request number of pins
 * @npins: Number of pins
 *
 * This function is used by master to get number of pins
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_pinctrl_get_num_pins(uint32_t *npins)
{
	return pm_api_pinctrl_get_num_pins(npins);
}

/**
 * pm_pinctrl_get_num_functions - PM call to request number of functions
 * @nfuncs: Number of functions
 *
 * This function is used by master to get number of functions
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_pinctrl_get_num_functions(uint32_t *nfuncs)
{
	return pm_api_pinctrl_get_num_functions(nfuncs);
}

/**
 * pm_pinctrl_get_num_function_groups - PM call to request number of
 *					function groups
 * @fid: Id of function
 * @ngroups: Number of function groups
 *
 * This function is used by master to get number of function groups specified
 * by given function Id
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_pinctrl_get_num_function_groups(unsigned int fid,
							     uint32_t *ngroups)
{
	return pm_api_pinctrl_get_num_func_groups(fid, ngroups);
}

/**
 * pm_pinctrl_get_function_name - PM call to request function name
 * @fid: Id of function
 * @name: Name of function
 *
 * This function is used by master to get name of function specified
 * by given function Id
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_pinctrl_get_function_name(unsigned int fid,
						       char *name)
{
	return pm_api_pinctrl_get_function_name(fid, name);
}

/**
 * pm_pinctrl_get_function_groups - PM call to request function groups
 * @fid: Id of function
 * @index: Index of next function groups
 * @groups: Function groups
 *
 * This function is used by master to get function groups specified
 * by given function Id. This API will return 6 function groups with
 * a single response. To get other function groups, master should call
 * same API in loop with new function groups index till error is returned.
 *
 * E.g First call should have index 0 which will return function groups
 * 0, 1, 2, 3, 4 and 5. Next call, index should be 6 which will return
 * function groups 6, 7, 8, 9, 10 and 11 and so on.
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_pinctrl_get_function_groups(unsigned int fid,
							 unsigned int index,
							 uint16_t *groups)
{
	return pm_api_pinctrl_get_function_groups(fid, index, groups);
}

/**
 * pm_pinctrl_get_pin_groups - PM call to request pin groups
 * @pin_id: Id of pin
 * @index: Index of next pin groups
 * @groups: pin groups
 *
 * This function is used by master to get pin groups specified
 * by given pin Id. This API will return 6 pin groups with
 * a single response. To get other pin groups, master should call
 * same API in loop with new pin groups index till error is returned.
 *
 * E.g First call should have index 0 which will return pin groups
 * 0, 1, 2, 3, 4 and 5. Next call, index should be 6 which will return
 * pin groups 6, 7, 8, 9, 10 and 11 and so on.
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_pinctrl_get_pin_groups(unsigned int pin_id,
						    unsigned int index,
						    uint16_t *groups)
{
	return pm_api_pinctrl_get_pin_groups(pin_id, index, groups);
}

/**
 * pm_query_data() -  PM API for querying firmware data
 * @arg1	Argument 1 to requested IOCTL call
 * @arg2	Argument 2 to requested IOCTL call
 * @arg3	Argument 3 to requested IOCTL call
 * @arg4	Argument 4 to requested IOCTL call
 * @data	Returned output data
 *
 * This function returns requested data.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_query_data(enum pm_query_id qid,
				 unsigned int arg1,
				 unsigned int arg2,
				 unsigned int arg3,
				 unsigned int *data)
{
	enum pm_ret_status ret;

	switch (qid) {
	case PM_QID_CLOCK_GET_NAME:
		ret = pm_clock_get_name(arg1, (char *)data);
		break;
	case PM_QID_CLOCK_GET_TOPOLOGY:
		ret = pm_clock_get_topology(arg1, arg2, &data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS:
		ret = pm_clock_get_fixedfactor_params(arg1, &data[1], &data[2]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_CLOCK_GET_PARENTS:
		ret = pm_clock_get_parents(arg1, arg2, &data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_CLOCK_GET_ATTRIBUTES:
		ret = pm_clock_get_attributes(arg1, &data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_PINCTRL_GET_NUM_PINS:
		ret = pm_pinctrl_get_num_pins(&data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_PINCTRL_GET_NUM_FUNCTIONS:
		ret = pm_pinctrl_get_num_functions(&data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS:
		ret = pm_pinctrl_get_num_function_groups(arg1, &data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_PINCTRL_GET_FUNCTION_NAME:
		ret = pm_pinctrl_get_function_name(arg1, (char *)data);
		break;
	case PM_QID_PINCTRL_GET_FUNCTION_GROUPS:
		ret = pm_pinctrl_get_function_groups(arg1, arg2,
						     (uint16_t *)&data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_PINCTRL_GET_PIN_GROUPS:
		ret = pm_pinctrl_get_pin_groups(arg1, arg2,
						(uint16_t *)&data[1]);
		data[0] = (unsigned int)ret;
		break;
	case PM_QID_CLOCK_GET_NUM_CLOCKS:
		ret = pm_clock_get_num_clocks(&data[1]);
		data[0] = (unsigned int)ret;
		break;
	default:
		ret = PM_RET_ERROR_ARGS;
		WARN("Unimplemented query service call: 0x%x\n", qid);
		break;
	}

	return ret;
}

enum pm_ret_status pm_sha_hash(uint32_t address_high,
				    uint32_t address_low,
				    uint32_t size,
				    uint32_t flags)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_SECURE_SHA, address_high, address_low,
				 size, flags);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

enum pm_ret_status pm_rsa_core(uint32_t address_high,
				    uint32_t address_low,
				    uint32_t size,
				    uint32_t flags)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_SECURE_RSA, address_high, address_low,
				 size, flags);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

enum pm_ret_status pm_secure_image(uint32_t address_low,
				   uint32_t address_high,
				   uint32_t key_lo,
				   uint32_t key_hi,
				   uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_SECURE_IMAGE, address_high, address_low,
			 key_hi, key_lo);
	return pm_ipi_send_sync(primary_proc, payload, value, 2);
}

/**
 * pm_fpga_read - Perform the fpga configuration readback
 *
 * @reg_numframes: Configuration register offset (or) Number of frames to read
 * @address_low: lower 32-bit Linear memory space address
 * @address_high: higher 32-bit Linear memory space address
 * @readback_type: Type of fpga readback operation
 *		   0 -- Configuration Register readback
 *		   1 -- Configuration Data readback
 * @value:	Value to read
 *
 * This function provides access to the xilfpga library to read
 * the PL configuration.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_fpga_read(uint32_t reg_numframes,
				uint32_t address_low,
				uint32_t address_high,
				uint32_t readback_type,
				uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_FPGA_READ, reg_numframes, address_low,
			 address_high, readback_type);
	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/*
 * pm_pll_set_parameter() - Set the PLL parameter value
 * @nid		Node id of the target PLL
 * @param_id	ID of the PLL parameter
 * @value	Parameter value to be set
 *
 * Setting the parameter will have physical effect once the PLL mode is set to
 * integer or fractional.
 *
 * @return	Error if an argument is not valid or status as returned by the
 *		PM controller (PMU)
 */
enum pm_ret_status pm_pll_set_parameter(enum pm_node_id nid,
					enum pm_pll_param param_id,
					unsigned int value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Check if given node ID is a PLL node */
	if (nid < NODE_APLL || nid > NODE_IOPLL)
		return PM_RET_ERROR_ARGS;

	/* Check if parameter ID is valid and return an error if it's not */
	if (param_id >= PM_PLL_PARAM_MAX)
		return PM_RET_ERROR_ARGS;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, PM_PLL_SET_PARAMETER, nid, param_id, value);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_parameter() - Get the PLL parameter value
 * @nid		Node id of the target PLL
 * @param_id	ID of the PLL parameter
 * @value	Location to store the parameter value
 *
 * @return	Error if an argument is not valid or status as returned by the
 *		PM controller (PMU)
 */
enum pm_ret_status pm_pll_get_parameter(enum pm_node_id nid,
					enum pm_pll_param param_id,
					unsigned int *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Check if given node ID is a PLL node */
	if (nid < NODE_APLL || nid > NODE_IOPLL)
		return PM_RET_ERROR_ARGS;

	/* Check if parameter ID is valid and return an error if it's not */
	if (param_id >= PM_PLL_PARAM_MAX)
		return PM_RET_ERROR_ARGS;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD3(payload, PM_PLL_GET_PARAMETER, nid, param_id);
	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_pll_set_mode() - Set the PLL mode
 * @nid		Node id of the target PLL
 * @mode	PLL mode to be set
 *
 * If reset mode is set the PM controller will first bypass the PLL and then
 * assert the reset. If integer or fractional mode is set the PM controller will
 * ensure that the complete PLL programming sequence is satisfied. After this
 * function returns success the PLL is locked and its bypass is deasserted.
 *
 * @return	Error if an argument is not valid or status as returned by the
 *		PM controller (PMU)
 */
enum pm_ret_status pm_pll_set_mode(enum pm_node_id nid, enum pm_pll_mode mode)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Check if given node ID is a PLL node */
	if (nid < NODE_APLL || nid > NODE_IOPLL)
		return PM_RET_ERROR_ARGS;

	/* Check if PLL mode is valid */
	if (mode >= PM_PLL_MODE_MAX)
		return PM_RET_ERROR_ARGS;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD3(payload, PM_PLL_SET_MODE, nid, mode);
	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_mode() - Get the PLL mode
 * @nid		Node id of the target PLL
 * @mode	Location to store the mode of the PLL
 *
 * @return	Error if an argument is not valid or status as returned by the
 *		PM controller (PMU)
 */
enum pm_ret_status pm_pll_get_mode(enum pm_node_id nid, enum pm_pll_mode *mode)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Check if given node ID is a PLL node */
	if (nid < NODE_APLL || nid > NODE_IOPLL)
		return PM_RET_ERROR_ARGS;

	/* Send request to the PMU */
	PM_PACK_PAYLOAD2(payload, PM_PLL_GET_MODE, nid);
	return pm_ipi_send_sync(primary_proc, payload, mode, 1);
}
