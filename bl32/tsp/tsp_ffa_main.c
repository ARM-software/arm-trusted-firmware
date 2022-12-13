/*
 * Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include "../../services/std_svc/spm/el3_spmc/spmc.h"
#include "../../services/std_svc/spm/el3_spmc/spmc_shared_mem.h"
#include <arch_features.h>
#include <arch_helpers.h>
#include <bl32/tsp/tsp.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include "ffa_helpers.h"
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <platform_tsp.h>
#include <services/ffa_svc.h>
#include "tsp_private.h"

#include <platform_def.h>

static ffa_endpoint_id16_t tsp_id, spmc_id;
uint8_t mem_region_buffer[4096 * 2]  __aligned(PAGE_SIZE);

/* Partition Mailbox. */
static uint8_t send_page[PAGE_SIZE] __aligned(PAGE_SIZE);
static uint8_t recv_page[PAGE_SIZE] __aligned(PAGE_SIZE);

/*
 * Declare a global mailbox for use within the TSP.
 * This will be initialized appropriately when the buffers
 * are mapped with the SPMC.
 */
static struct mailbox mailbox;

/*******************************************************************************
 * This enum is used to handle test cases driven from the FF-A Test Driver.
 ******************************************************************************/
/* Keep in Sync with FF-A Test Driver. */
enum message_t {
	/* Partition Only Messages. */
	FF_A_RELAY_MESSAGE = 0,

	/* Basic Functionality. */
	FF_A_ECHO_MESSAGE,
	FF_A_RELAY_MESSAGE_EL3,

	/* Memory Sharing. */
	FF_A_MEMORY_SHARE,
	FF_A_MEMORY_SHARE_FRAGMENTED,
	FF_A_MEMORY_LEND,
	FF_A_MEMORY_LEND_FRAGMENTED,

	FF_A_MEMORY_SHARE_MULTI_ENDPOINT,
	FF_A_MEMORY_LEND_MULTI_ENDPOINT,

	LAST,
	FF_A_RUN_ALL = 255,
	FF_A_OP_MAX = 256
};

#if SPMC_AT_EL3
extern void tsp_cpu_on_entry(void);
#endif

/*******************************************************************************
 * Test Functions.
 ******************************************************************************/

/*******************************************************************************
 * Enable the TSP to forward the received message to another partition and ask
 * it to echo the value back in order to validate direct messages functionality.
 ******************************************************************************/
static int ffa_test_relay(uint64_t arg0,
			  uint64_t arg1,
			  uint64_t arg2,
			  uint64_t arg3,
			  uint64_t arg4,
			  uint64_t arg5,
			  uint64_t arg6,
			  uint64_t arg7)
{
	smc_args_t ffa_forward_result;
	ffa_endpoint_id16_t receiver = arg5;

	ffa_forward_result = ffa_msg_send_direct_req(ffa_endpoint_source(arg1),
						     receiver,
						     FF_A_ECHO_MESSAGE, arg4,
						     0, 0, 0);
	return ffa_forward_result._regs[3];
}

/*******************************************************************************
 * This function handles memory management tests, currently share and lend.
 * This test supports the use of FRAG_RX to use memory descriptors that do not
 * fit in a single 4KB buffer.
 ******************************************************************************/
static int test_memory_send(ffa_endpoint_id16_t sender, uint64_t handle,
			    ffa_mtd_flag32_t flags, bool multi_endpoint)
{
	struct ffa_mtd *m;
	struct ffa_emad_v1_0 *receivers;
	struct ffa_comp_mrd *composite;
	int ret, status = 0;
	unsigned int mem_attrs;
	char *ptr;
	ffa_endpoint_id16_t source = sender;
	uint32_t total_length, recv_length = 0;

	/*
	 * In the case that we're testing multiple endpoints choose a partition
	 * ID that resides in the normal world so the SPMC won't detect it as
	 * invalid.
	 * TODO: Should get endpoint receiver id and flag as input from NWd.
	 */
	uint32_t receiver_count = multi_endpoint ? 2 : 1;
	ffa_endpoint_id16_t test_receivers[2] = { tsp_id, 0x10 };

	/* Ensure that the sender ID resides in the normal world. */
	if (ffa_is_secure_world_id(sender)) {
		ERROR("Invalid sender ID 0x%x.\n", sender);
		return FFA_ERROR_DENIED;
	}

	if (!memory_retrieve(&mailbox, &m, handle, source, test_receivers,
			     receiver_count, flags, &recv_length,
			     &total_length)) {
		return FFA_ERROR_INVALID_PARAMETER;
	}

	receivers = (struct ffa_emad_v1_0 *)
		    ((uint8_t *) m + m->emad_offset);
	while (total_length != recv_length) {
		smc_args_t ffa_return;
		uint32_t frag_length;

		ffa_return = ffa_mem_frag_rx(handle, recv_length);

		if (ffa_return._regs[0] == FFA_ERROR) {
			WARN("TSP: failed to resume mem with handle %lx\n",
			     handle);
			return ffa_return._regs[2];
		}
		frag_length = ffa_return._regs[3];

		/* Validate frag_length is less than total_length and mailbox size. */
		if (frag_length > total_length ||
				frag_length > (mailbox.rxtx_page_count * PAGE_SIZE)) {
			ERROR("Invalid parameters!\n");
			return FFA_ERROR_INVALID_PARAMETER;
		}

		/* Validate frag_length is less than remaining mem_region_buffer size. */
		if (frag_length + recv_length >= REGION_BUF_SIZE) {
			ERROR("Out of memory!\n");
			return FFA_ERROR_INVALID_PARAMETER;
		}

		memcpy(&mem_region_buffer[recv_length], mailbox.rx_buffer,
		       frag_length);

		if (ffa_rx_release()) {
			ERROR("Failed to release buffer!\n");
			return FFA_ERROR_DENIED;
		}

		recv_length += frag_length;

		assert(recv_length <= total_length);
	}

	composite = ffa_memory_region_get_composite(m, 0);
	if (composite == NULL) {
		WARN("Failed to get composite descriptor!\n");
		return FFA_ERROR_INVALID_PARAMETER;
	}

	VERBOSE("Address: %p; page_count: %x %lx\n",
		(void *)composite->address_range_array[0].address,
		composite->address_range_array[0].page_count, PAGE_SIZE);

	/* This test is only concerned with RW permissions. */
	if (ffa_get_data_access_attr(
	    receivers[0].mapd.memory_access_permissions) != FFA_MEM_PERM_RW) {
		ERROR("Data permission in retrieve response %x does not match share/lend %x!\n",
		      ffa_get_data_access_attr(receivers[0].mapd.memory_access_permissions),
		      FFA_MEM_PERM_RW);
		return FFA_ERROR_INVALID_PARAMETER;
	}

	mem_attrs = MT_RW_DATA | MT_EXECUTE_NEVER;

	/* Only expecting to be sent memory from NWd so map accordingly. */
	mem_attrs |= MT_NS;

	for (uint32_t i = 0U; i < composite->address_range_count; i++) {
		size_t size = composite->address_range_array[i].page_count * PAGE_SIZE;

		ptr = (char *) composite->address_range_array[i].address;
		ret = mmap_add_dynamic_region(
				(uint64_t)ptr,
				(uint64_t)ptr,
				size, mem_attrs);

		if (ret != 0) {
			ERROR("Failed [%u] mmap_add_dynamic_region %u (%lx) (%lx) (%x)!\n",
				i, ret,
				(uint64_t)composite->address_range_array[i].address,
				size, mem_attrs);

			/* Remove mappings previously created in this transaction. */
			for (i--; i >= 0; i--) {
				ret = mmap_remove_dynamic_region(
					(uint64_t)composite->address_range_array[i].address,
					composite->address_range_array[i].page_count * PAGE_SIZE);

				if (ret != 0) {
					ERROR("Failed [%d] mmap_remove_dynamic_region!\n", i);
					panic();
				}
			}

			return FFA_ERROR_NO_MEMORY;
		}

		/* Increment memory region for validation purposes. */
		++(*ptr);

		/*
		 * Read initial magic number from memory region for
		 * validation purposes.
		 */
		if (!i) {
			status = *ptr;
		}
	}

	for (uint32_t i = 0U; i < composite->address_range_count; i++) {
		ret = mmap_remove_dynamic_region(
			(uint64_t)composite->address_range_array[i].address,
			composite->address_range_array[i].page_count * PAGE_SIZE);

		if (ret != 0) {
			ERROR("Failed [%d] mmap_remove_dynamic_region!\n", i);
			return FFA_ERROR_NO_MEMORY;
		}
	}

	if (!memory_relinquish((struct ffa_mem_relinquish_descriptor *)mailbox.tx_buffer,
				m->handle, tsp_id)) {
		ERROR("Failed to relinquish memory region!\n");
		return FFA_ERROR_INVALID_PARAMETER;
	}
	return status;
}

static smc_args_t *send_ffa_pm_success(void)
{
	return set_smc_args(FFA_MSG_SEND_DIRECT_RESP_SMC32,
			    ((tsp_id & FFA_DIRECT_MSG_ENDPOINT_ID_MASK)
			    << FFA_DIRECT_MSG_SOURCE_SHIFT) | spmc_id,
			    FFA_FWK_MSG_BIT |
			    (FFA_PM_MSG_PM_RESP & FFA_FWK_MSG_MASK),
			    0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * before this cpu is turned off in response to a psci cpu_off request.
 ******************************************************************************/
smc_args_t *tsp_cpu_off_main(uint64_t arg0,
			     uint64_t arg1,
			     uint64_t arg2,
			     uint64_t arg3,
			     uint64_t arg4,
			     uint64_t arg5,
			     uint64_t arg6,
			     uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/*
	 * This cpu is being turned off, so disable the timer to prevent the
	 * secure timer interrupt from interfering with power down. A pending
	 * interrupt will be lost but we do not care as we are turning off.
	 */
	tsp_generic_timer_stop();

	/* Update this cpu's statistics. */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_off_count++;

	VERBOSE("TSP: cpu 0x%lx off request\n", read_mpidr());
	VERBOSE("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu off requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_off_count);

	return send_ffa_pm_success();
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload before
 * this cpu's architectural state is saved in response to an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
smc_args_t *tsp_cpu_suspend_main(uint64_t arg0,
				 uint64_t arg1,
				 uint64_t arg2,
				 uint64_t arg3,
				 uint64_t arg4,
				 uint64_t arg5,
				 uint64_t arg6,
				 uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/*
	 * Save the time context and disable it to prevent the secure timer
	 * interrupt from interfering with wakeup from the suspend state.
	 */
	tsp_generic_timer_save();
	tsp_generic_timer_stop();

	/* Update this cpu's statistics. */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_suspend_count++;

	VERBOSE("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu suspend requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_suspend_count);

	return send_ffa_pm_success();
}

/*******************************************************************************
 * This function performs any bookkeeping in the test secure payload after this
 * cpu's architectural state has been restored after wakeup from an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
smc_args_t *tsp_cpu_resume_main(uint64_t max_off_pwrlvl,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Restore the generic timer context. */
	tsp_generic_timer_restore();

	/* Update this cpu's statistics. */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_resume_count++;

	VERBOSE("TSP: cpu 0x%lx resumed. maximum off power level %" PRId64 "\n",
	     read_mpidr(), max_off_pwrlvl);
	VERBOSE("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu resume requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_resume_count);

	return send_ffa_pm_success();
}

/*******************************************************************************
 * This function handles framework messages. Currently only PM.
 ******************************************************************************/
static smc_args_t *handle_framework_message(uint64_t arg0,
					    uint64_t arg1,
					    uint64_t arg2,
					    uint64_t arg3,
					    uint64_t arg4,
					    uint64_t arg5,
					    uint64_t arg6,
					    uint64_t arg7)
{
	/* Check if it is a power management message from the SPMC. */
	if (ffa_endpoint_source(arg1) != spmc_id) {
		goto err;
	}

	/* Check if it is a PM request message. */
	if ((arg2 & FFA_FWK_MSG_MASK) == FFA_FWK_MSG_PSCI) {
		/* Check if it is a PSCI CPU_OFF request. */
		if (arg3 == PSCI_CPU_OFF) {
			return tsp_cpu_off_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		} else if (arg3 == PSCI_CPU_SUSPEND_AARCH64) {
			return tsp_cpu_suspend_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		}
	} else if ((arg2 & FFA_FWK_MSG_MASK) == FFA_PM_MSG_WB_REQ) {
		/* Check it is a PSCI Warm Boot request. */
		if (arg3 == FFA_WB_TYPE_NOTS2RAM) {
			return tsp_cpu_resume_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		}
	}

err:
	ERROR("%s: Unknown framework message!\n", __func__);
	panic();
}

/*******************************************************************************
 * Handles partition messages. Exercised from the FF-A Test Driver.
 ******************************************************************************/
static smc_args_t *handle_partition_message(uint64_t arg0,
					    uint64_t arg1,
					    uint64_t arg2,
					    uint64_t arg3,
					    uint64_t arg4,
					    uint64_t arg5,
					    uint64_t arg6,
					    uint64_t arg7)
{
	uint16_t sender = ffa_endpoint_source(arg1);
	uint16_t receiver = ffa_endpoint_destination(arg1);
	int status = -1;
	const bool multi_endpoint = true;

	switch (arg3) {
	case FF_A_MEMORY_SHARE:
		INFO("TSP Tests: Memory Share Request--\n");
		status = test_memory_send(sender, arg4, FFA_FLAG_SHARE_MEMORY, !multi_endpoint);
		break;

	case FF_A_MEMORY_LEND:
		INFO("TSP Tests: Memory Lend Request--\n");
		status = test_memory_send(sender, arg4, FFA_FLAG_LEND_MEMORY, !multi_endpoint);
		break;

	case FF_A_MEMORY_SHARE_MULTI_ENDPOINT:
		INFO("TSP Tests: Multi Endpoint Memory Share Request--\n");
		status = test_memory_send(sender, arg4, FFA_FLAG_SHARE_MEMORY, multi_endpoint);
		break;

	case FF_A_MEMORY_LEND_MULTI_ENDPOINT:
		INFO("TSP Tests: Multi Endpoint Memory Lend Request--\n");
		status = test_memory_send(sender, arg4, FFA_FLAG_LEND_MEMORY, multi_endpoint);
		break;
	case FF_A_RELAY_MESSAGE:
		INFO("TSP Tests: Relaying message--\n");
		status = ffa_test_relay(arg0, arg1, arg2, arg3, arg4,
					arg5, arg6, arg7);
		break;

	case FF_A_ECHO_MESSAGE:
		INFO("TSP Tests: echo message--\n");
		status = arg4;
		break;

	default:
		INFO("TSP Tests: Unknown request ID %d--\n", (int) arg3);
	}

	/* Swap the sender and receiver in the response. */
	return ffa_msg_send_direct_resp(receiver, sender, status, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function implements the event loop for handling FF-A ABI invocations.
 ******************************************************************************/
static smc_args_t *tsp_event_loop(uint64_t smc_fid,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7)
{
	/* Panic if the SPMC did not forward an FF-A call. */
	if (!is_ffa_fid(smc_fid)) {
		ERROR("%s: Unknown SMC FID (0x%lx)\n", __func__, smc_fid);
		panic();
	}

	switch (smc_fid) {
	case FFA_INTERRUPT:
		/*
		 * IRQs were enabled upon re-entry into the TSP. The interrupt
		 * must have been handled by now. Return to the SPMC indicating
		 * the same.
		 */
		return set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0);

	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
		/* Check if a framework message, handle accordingly. */
		if ((arg2 & FFA_FWK_MSG_BIT)) {
			return handle_framework_message(smc_fid, arg1, arg2, arg3,
							arg4, arg5, arg6, arg7);
		}
		return handle_partition_message(smc_fid, arg1, arg2, arg3,
							arg4, arg5, arg6, arg7);
	}

	ERROR("%s: Unsupported FF-A FID (0x%lx)\n", __func__, smc_fid);
	panic();
}

static smc_args_t *tsp_loop(smc_args_t *args)
{
	smc_args_t ret;

	do {
		/* --------------------------------------------
		 * Mask FIQ interrupts to avoid preemption
		 * in case EL3 SPMC delegates an IRQ next or a
		 * managed exit. Lastly, unmask IRQs so that
		 * they can be handled immediately upon re-entry.
		 *  ---------------------------------------------
		 */
		write_daifset(DAIF_FIQ_BIT);
		write_daifclr(DAIF_IRQ_BIT);
		ret = smc_helper(args->_regs[0], args->_regs[1], args->_regs[2],
			       args->_regs[3], args->_regs[4], args->_regs[5],
			       args->_regs[6], args->_regs[7]);
		args = tsp_event_loop(ret._regs[0], ret._regs[1], ret._regs[2],
				ret._regs[3], ret._regs[4], ret._regs[5],
				ret._regs[6], ret._regs[7]);
	} while (1);

	/* Not Reached. */
	return NULL;
}

/*******************************************************************************
 * TSP main entry point where it gets the opportunity to initialize its secure
 * state/applications. Once the state is initialized, it must return to the
 * SPD with a pointer to the 'tsp_vector_table' jump table.
 ******************************************************************************/
uint64_t tsp_main(void)
{
	smc_args_t smc_args = {0};

	NOTICE("TSP: %s\n", version_string);
	NOTICE("TSP: %s\n", build_message);
	INFO("TSP: Total memory base : 0x%lx\n", (unsigned long) BL32_BASE);
	INFO("TSP: Total memory size : 0x%lx bytes\n", BL32_TOTAL_SIZE);
	uint32_t linear_id = plat_my_core_pos();

	/* Initialize the platform. */
	tsp_platform_setup();

	/* Initialize secure/applications state here. */
	tsp_generic_timer_start();

	/* Register secondary entrypoint with the SPMC. */
	smc_args = smc_helper(FFA_SECONDARY_EP_REGISTER_SMC64,
			(uint64_t) tsp_cpu_on_entry,
			0, 0, 0, 0, 0, 0);
	if (smc_args._regs[SMC_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not register secondary ep (0x%lx)\n",
				smc_args._regs[2]);
		panic();
	}
	/* Get TSP's endpoint id. */
	smc_args = smc_helper(FFA_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[SMC_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get own ID (0x%lx) on core%d\n",
				smc_args._regs[2], linear_id);
		panic();
	}

	tsp_id = smc_args._regs[2];
	INFO("TSP FF-A endpoint id = 0x%x\n", tsp_id);

	/* Get the SPMC ID. */
	smc_args = smc_helper(FFA_SPM_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[SMC_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get SPMC ID (0x%lx) on core%d\n",
				smc_args._regs[2], linear_id);
		panic();
	}

	spmc_id = smc_args._regs[2];

	/* Call RXTX_MAP to map a 4k RX and TX buffer. */
	if (ffa_rxtx_map((uintptr_t) send_page,
			 (uintptr_t) recv_page, 1)) {
		ERROR("TSP could not map it's RX/TX Buffers\n");
		panic();
	}

	mailbox.tx_buffer = send_page;
	mailbox.rx_buffer = recv_page;
	mailbox.rxtx_page_count = 1;

	/* Update this cpu's statistics. */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

	VERBOSE("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu on requests\n",
			read_mpidr(),
			tsp_stats[linear_id].smc_count,
			tsp_stats[linear_id].eret_count,
			tsp_stats[linear_id].cpu_on_count);

	/* Tell SPMD that we are done initialising. */
	tsp_loop(set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0));

	/* Not reached. */
	return 0;
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * after this cpu's architectural state has been setup in response to an earlier
 * psci cpu_on request.
 ******************************************************************************/
smc_args_t *tsp_cpu_on_main(void)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Initialize secure/applications state here. */
	tsp_generic_timer_start();

	/* Update this cpu's statistics. */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;
	VERBOSE("TSP: cpu 0x%lx turned on\n", read_mpidr());
	VERBOSE("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu on requests\n",
			read_mpidr(),
			tsp_stats[linear_id].smc_count,
			tsp_stats[linear_id].eret_count,
			tsp_stats[linear_id].cpu_on_count);
	/* ---------------------------------------------
	 * Jump to the main event loop to return to EL3
	 * and be ready for the next request on this cpu.
	 * ---------------------------------------------
	 */
	return tsp_loop(set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0));
}
