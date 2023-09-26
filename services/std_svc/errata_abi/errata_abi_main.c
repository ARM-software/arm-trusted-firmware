/*
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include "cpu_errata_info.h"
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <services/errata_abi_svc.h>
#include <smccc_helpers.h>


/*
 * Global pointer that points to the specific
 * structure based on the MIDR part number
 */
struct em_cpu_list *cpu_ptr;

/* Structure array that holds CPU specific errata information */
struct em_cpu_list cpu_list[] = {
#if CORTEX_A78_H_INC
{
	.cpu_partnumber = CORTEX_A78_MIDR,
	.cpu_errata_list = {
		[0] = {2712571, 0x00, 0x12, ERRATA_A78_2712571, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78_H_INC */

#if CORTEX_A78_AE_H_INC
{
	.cpu_partnumber = CORTEX_A78_AE_MIDR,
	.cpu_errata_list = {
		[0] = {2712574, 0x00, 0x02, ERRATA_A78_AE_2712574, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78_AE_H_INC */

#if CORTEX_A78C_H_INC
{
	.cpu_partnumber = CORTEX_A78C_MIDR,
	.cpu_errata_list = {
		[0] = {2712575, 0x01, 0x02, ERRATA_A78C_2712575, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78C_H_INC */

#if NEOVERSE_V1_H_INC
{
	.cpu_partnumber = NEOVERSE_V1_MIDR,
	.cpu_errata_list = {
		[0] = {2701953, 0x00, 0x11, ERRATA_V1_2701953, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_V1_H_INC */

#if CORTEX_A710_H_INC
{
	.cpu_partnumber = CORTEX_A710_MIDR,
	.cpu_errata_list = {
		[0] = {2701952, 0x00, 0x21, ERRATA_A710_2701952, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A710_H_INC */

#if NEOVERSE_N2_H_INC
{
	.cpu_partnumber = NEOVERSE_N2_MIDR,
	.cpu_errata_list = {
		[0] = {2728475, 0x00, 0x02, ERRATA_N2_2728475, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_N2_H_INC */

#if CORTEX_X2_H_INC
{
	.cpu_partnumber = CORTEX_X2_MIDR,
	.cpu_errata_list = {
		[0] = {2701952, 0x00, 0x21, ERRATA_X2_2701952, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_X2_H_INC */

#if NEOVERSE_V2_H_INC
{
	.cpu_partnumber = NEOVERSE_V2_MIDR,
	.cpu_errata_list = {
		[0] = {2719103, 0x00, 0x01, ERRATA_V2_2719103, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_V2_H_INC */

#if CORTEX_A715_H_INC
{
	.cpu_partnumber = CORTEX_A715_MIDR,
	.cpu_errata_list = {
		[0] = {2701951, 0x00, 0x11, ERRATA_A715_2701951, ERRATA_NON_ARM_INTERCONNECT},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A715_H_INC */

};

/* Function to check if the errata exists for the specific CPU and rxpx */
int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag)
{
	int32_t ret_val;
	struct cpu_ops *cpu_ops;
	struct erratum_entry *entry, *end;
	long rev_var;

	ret_val = EM_UNKNOWN_ERRATUM;

	cpu_ops = get_cpu_ops_ptr();
	assert(cpu_ops != NULL);

	entry = cpu_ops->errata_list_start;
	assert(entry != NULL);

	end = cpu_ops->errata_list_end;
	assert(end != NULL);

	rev_var = cpu_get_rev_var();

	end--; /* point to the last erratum entry of the queried cpu */

	while (entry <= end) {
		if (entry->id == errata_id) {
			if (entry->check_func(rev_var)) {
				if (entry->chosen)
					return EM_HIGHER_EL_MITIGATION;
				else
					return EM_AFFECTED;
			}
			return EM_NOT_AFFECTED;
		}
		entry += 1;
	}
	return ret_val;
}

/* Predicate indicating that a function id is part of EM_ABI */
bool is_errata_fid(uint32_t smc_fid)
{
	return ((smc_fid == ARM_EM_VERSION) ||
		(smc_fid == ARM_EM_FEATURES) ||
		(smc_fid == ARM_EM_CPU_ERRATUM_FEATURES));

}

bool validate_spsr_mode(void)
{
	/* In AArch64, if the caller is EL1, return true */

	#if __aarch64__
		if (GET_EL(read_spsr_el3()) == MODE_EL1) {
			return true;
		}
		return false;
	#else

	/* In AArch32, if in system/svc mode, return true */
		uint8_t read_el_state = GET_M32(read_spsr());

		if ((read_el_state == (MODE32_svc)) || (read_el_state == MODE32_sys)) {
			return true;
		}
		return false;
	#endif /* __aarch64__ */
}

uintptr_t errata_abi_smc_handler(uint32_t smc_fid, u_register_t x1,
				u_register_t x2, u_register_t x3, u_register_t x4,
				void *cookie, void *handle, u_register_t flags)
{
	int32_t ret_id = EM_UNKNOWN_ERRATUM;

	switch (smc_fid) {
	case ARM_EM_VERSION:
		SMC_RET1(handle, MAKE_SMCCC_VERSION(
			EM_VERSION_MAJOR, EM_VERSION_MINOR
		));
		break; /* unreachable */
	case ARM_EM_FEATURES:
		if (is_errata_fid((uint32_t)x1)) {
			SMC_RET1(handle, EM_SUCCESS);
		}

		SMC_RET1(handle, EM_NOT_SUPPORTED);
		break; /* unreachable */
	case ARM_EM_CPU_ERRATUM_FEATURES:

		/*
		 * If the forward flag is greater than zero and the calling EL
		 * is EL1 in AArch64 or in system mode or svc mode in case of AArch32,
		 * return Invalid Parameters.
		 */
		if (((uint32_t)x2 != 0) && (validate_spsr_mode())) {
			SMC_RET1(handle, EM_INVALID_PARAMETERS);
		}
		ret_id = verify_errata_implemented((uint32_t)x1, (uint32_t)x2);
		SMC_RET1(handle, ret_id);
		break; /* unreachable */
	default:
		{
		   WARN("Unimplemented Errata ABI Service Call: 0x%x\n", smc_fid);
		   SMC_RET1(handle, EM_UNKNOWN_ERRATUM);
		   break; /* unreachable */
		}
	}
}
