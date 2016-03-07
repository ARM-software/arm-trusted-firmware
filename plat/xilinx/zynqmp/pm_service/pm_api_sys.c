/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * ZynqMP system level PM-API functions and communication with PMU via
 * IPI interrupts
 */

#include <arch_helpers.h>
#include <platform.h>
#include "pm_client.h"
#include "pm_common.h"
#include "pm_api_sys.h"

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
 * @state	Requested state (not supported)
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
	pm_client_suspend(proc);
	/* Send request to the PMU */
	PM_PACK_PAYLOAD6(payload, PM_SELF_SUSPEND, proc->node_id, latency,
			 state, address, (address >> 32));
	return pm_ipi_send_sync(proc, payload, NULL);
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
		return pm_ipi_send_sync(primary_proc, payload, NULL);
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
	const struct pm_proc *proc = pm_get_proc_by_node(target);

	/* invoke APU-specific code for waking up another APU core */
	pm_client_wakeup(proc);

	/* encode set Address into 1st bit of address */
	encoded_address = address;
	encoded_address |= !!set_address;

	/* Send request to the PMU to perform the wake of the PU */
	PM_PACK_PAYLOAD5(payload, PM_REQ_WAKEUP, target, encoded_address,
			 encoded_address >> 32, ack);

	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL);
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
		return pm_ipi_send_sync(primary_proc, payload, NULL);
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
 * @restart	Shutdown or restart? 0 for shutdown, 1 for restart
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_system_shutdown(unsigned int restart)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD2(payload, PM_SYSTEM_SHUTDOWN, restart);
	return pm_ipi_send(primary_proc, payload);
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
		return pm_ipi_send_sync(primary_proc, payload, NULL);
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
		return pm_ipi_send_sync(primary_proc, payload, NULL);
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
	return pm_ipi_send_sync(primary_proc, payload, version);
}

/**
 * pm_set_configuration() - PM call to set system configuration
 * @phys_addr	Physical 32-bit address of data structure in memory
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_configuration(unsigned int phys_addr)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_get_node_status() - PM call to request a node's current power state
 * @nid		Node id of the slave
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_node_status(enum pm_node_id nid)
{
	/* TODO: Add power state argument!! */
	uint32_t payload[PAYLOAD_ARG_CNT];

	PM_PACK_PAYLOAD2(payload, PM_GET_NODE_STATUS, nid);
	return pm_ipi_send(primary_proc, payload);
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
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_get_op_characteristic() - PM call to get a particular operating
 *				characteristic of a node
 * @nid	Node ID
 * @type	Operating characterstic type to be returned
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_op_characteristic(enum pm_node_id nid,
					    enum pm_opchar_type type)
{
	return PM_RET_ERROR_NOTSUPPORTED;
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
	return pm_ipi_send_sync(primary_proc, payload, reset_status);
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
	return pm_ipi_send(primary_proc, payload);
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
	return pm_ipi_send_sync(primary_proc, payload, value);
}
