/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdint.h>

#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/ras.h>

#include <plat/common/platform.h>
#include <services/el3_spmd_logical_sp.h>
#include <services/ffa_svc.h>
#include <services/sdei.h>


#define CACTUS_SP_RAS_DELEGATE_CMD 0x72617365
#define EVENT_NOTIFY_OS_RAS_ERROR U(5000)

/*
 * Note: Typical RAS error handling flow with Firmware First Handling
 *
 * Step 1: Exception resulting from a RAS error in the normal world is routed to
 *         EL3.
 * Step 2: This exception is typically signaled as either a synchronous external
 *         abort or SError or interrupt. TF-A (EL3 firmware) delegates the
 *         control to platform specific handler built on top of the RAS helper
 *         utilities.
 * Step 3: With the help of a Logical Secure Partition, TF-A sends a direct
 *         message to dedicated S-EL0 (or S-EL1) RAS Partition managed by SPMC.
 *         TF-A also populates a shared buffer with a data structure containing
 *         enough information (such as system registers) to identify and triage
 *         the RAS error.
 * Step 4: RAS SP generates the Common Platform Error Record (CPER) and shares
 *         it with normal world firmware and/or OS kernel through a reserved
 *         buffer memory.
 * Step 5: RAS SP responds to the direct message with information necessary for
 *         TF-A to notify the OS kernel.
 * Step 6: Consequently, TF-A dispatches an SDEI event to notify the OS kernel
 *         about the CPER records for further logging.
 */

static int injected_fault_handler(const struct err_record_info *info,
		int probe_data, const struct err_handler_data *const data)
{
	/*
	 * At the moment, an FF-A compatible SP that supports RAS firmware is
	 * not available. Hence the sequence below does not exactly follow the
	 * steps outlined above. Therefore, some steps are essentially spoofed.
	 * The handling of RAS error is completely done in EL3 firmware.
	 */
	uint64_t status, cactus_cmd_ret;
	int ret, event_num;
	cpu_context_t *ns_cpu_context;

	/* Get a reference to the non-secure context */
	ns_cpu_context = cm_get_context(NON_SECURE);
	assert(ns_cpu_context != NULL);

	/*
	 * The faulting error record is already selected by the SER probe
	 * function.
	 */
	status = read_erxstatus_el1();

	ERROR("Fault reported by system error record %d on 0x%lx: status=0x%" PRIx64 "\n",
			probe_data, read_mpidr_el1(), status);
	ERROR(" exception reason=%u syndrome=0x%" PRIx64 "\n", data->ea_reason,
			data->flags);

	/* Clear error */
	write_erxstatus_el1(status);

	/*
	 * Initiate an EL3 direct message from LSP to Cactus RAS Secure
	 * Partition (ID 8001). Currently, the payload is being spoofed.
	 * The direct message response contains the SDEI event ID for the
	 * associated RAS error.
	 */
	(void)plat_spmd_logical_sp_smc_handler(0, 0, 0, CACTUS_SP_RAS_DELEGATE_CMD,
						EVENT_NOTIFY_OS_RAS_ERROR,
						NULL, ns_cpu_context, 0);

	cactus_cmd_ret = read_ctx_reg(get_gpregs_ctx(ns_cpu_context), CTX_GPREG_X3);
	event_num = (int)read_ctx_reg(get_gpregs_ctx(ns_cpu_context), CTX_GPREG_X4);

	if (cactus_cmd_ret != 0) {
		ERROR("RAS error could not be handled by SP: %lx\n", cactus_cmd_ret);
		panic();
	}

	if (event_num != EVENT_NOTIFY_OS_RAS_ERROR) {
		ERROR("Unexpected event id sent by RAS SP: %d\n", event_num);
		panic();
	}

	/* Dispatch the event to the SDEI client */
	ret = sdei_dispatch_event(event_num);
	if (ret < 0) {
		ERROR("Can't dispatch event to SDEI\n");
		panic();
	} else {
		INFO("SDEI event dispatched\n");
	}

	return 0;
}

struct ras_interrupt fvp_ras_interrupts[] = {
};

struct err_record_info fvp_err_records[] = {
	/* Record for injected fault */
	ERR_RECORD_SYSREG_V1(0, 2, ras_err_ser_probe_sysreg,
			injected_fault_handler, NULL),
};

REGISTER_ERR_RECORD_INFO(fvp_err_records);
REGISTER_RAS_INTERRUPTS(fvp_ras_interrupts);
