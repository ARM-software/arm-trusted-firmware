/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdint.h>

#include <lib/extensions/ras.h>
#include <services/sdei.h>

#ifdef PLATFORM_TEST_RAS_FFH
static int injected_fault_handler(const struct err_record_info *info,
		int probe_data, const struct err_handler_data *const data)
{
	uint64_t status;
	int ret;

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

	ret = sdei_dispatch_event(5000);
	if (ret < 0) {
		ERROR("Can't dispatch event to SDEI\n");
		panic();
	} else {
		INFO("SDEI event dispatched\n");
	}

	return 0;
}

void plat_handle_uncontainable_ea(void)
{
	/* Do not change the string, CI expects it. Wait forever */
	INFO("Injected Uncontainable Error\n");
	while (true) {
		wfe();
	}
}
#endif

struct ras_interrupt fvp_ras_interrupts[] = {
};

struct err_record_info fvp_err_records[] = {
#ifdef PLATFORM_TEST_RAS_FFH
	/* Record for injected fault */
	ERR_RECORD_SYSREG_V1(0, 2, ras_err_ser_probe_sysreg,
			injected_fault_handler, NULL),
#endif
};

REGISTER_ERR_RECORD_INFO(fvp_err_records);
REGISTER_RAS_INTERRUPTS(fvp_ras_interrupts);
