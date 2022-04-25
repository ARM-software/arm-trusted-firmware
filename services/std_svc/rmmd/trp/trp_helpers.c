/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <plat/common/platform.h>
#include <services/rmmd_svc.h>
#include "trp_private.h"

/*
 * Per cpu data structure to populate parameters for an SMC in C code and use
 * a pointer to this structure in assembler code to populate x0-x7
 */
static trp_args_t trp_smc_args[PLATFORM_CORE_COUNT];

/*
 * Set the arguments for SMC call
 */
trp_args_t *set_smc_args(uint64_t arg0,
			uint64_t arg1,
			uint64_t arg2,
			uint64_t arg3,
			uint64_t arg4,
			uint64_t arg5,
			uint64_t arg6,
			uint64_t arg7)
{
	uint32_t linear_id;
	trp_args_t *pcpu_smc_args;

	/*
	 * Return to Secure Monitor by raising an SMC. The results of the
	 * service are passed as an arguments to the SMC
	 */
	linear_id = plat_my_core_pos();
	pcpu_smc_args = &trp_smc_args[linear_id];
	write_trp_arg(pcpu_smc_args, TRP_ARG0, arg0);
	write_trp_arg(pcpu_smc_args, TRP_ARG1, arg1);
	write_trp_arg(pcpu_smc_args, TRP_ARG2, arg2);
	write_trp_arg(pcpu_smc_args, TRP_ARG3, arg3);
	write_trp_arg(pcpu_smc_args, TRP_ARG4, arg4);
	write_trp_arg(pcpu_smc_args, TRP_ARG5, arg5);
	write_trp_arg(pcpu_smc_args, TRP_ARG6, arg6);
	write_trp_arg(pcpu_smc_args, TRP_ARG7, arg7);

	return pcpu_smc_args;
}

/*
 * Abort the boot process with the reason given in err.
 */
__dead2 void trp_boot_abort(uint64_t err)
{
	(void)trp_smc(set_smc_args(RMM_BOOT_COMPLETE, err, 0, 0, 0, 0, 0, 0));
	panic();
}
