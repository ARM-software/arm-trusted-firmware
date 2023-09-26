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
		[0] = {2712571, 0x00, 0x12},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78_H_INC */

#if CORTEX_A78_AE_H_INC
{
	.cpu_partnumber = CORTEX_A78_AE_MIDR,
	.cpu_errata_list = {
		[0] = {2712574, 0x00, 0x02},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78_AE_H_INC */

#if CORTEX_A78C_H_INC
{
	.cpu_partnumber = CORTEX_A78C_MIDR,
	.cpu_errata_list = {
		[0] = {2712575, 0x01, 0x02},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A78C_H_INC */

#if NEOVERSE_V1_H_INC
{
	.cpu_partnumber = NEOVERSE_V1_MIDR,
	.cpu_errata_list = {
		[0] = {2701953, 0x00, 0x11},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_V1_H_INC */

#if CORTEX_A710_H_INC
{
	.cpu_partnumber = CORTEX_A710_MIDR,
	.cpu_errata_list = {
		[0] = {2701952, 0x00, 0x21},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A710_H_INC */

#if NEOVERSE_N2_H_INC
{
	.cpu_partnumber = NEOVERSE_N2_MIDR,
	.cpu_errata_list = {
		[0] = {2728475, 0x00, 0x02},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_N2_H_INC */

#if CORTEX_X2_H_INC
{
	.cpu_partnumber = CORTEX_X2_MIDR,
	.cpu_errata_list = {
		[0] = {2701952, 0x00, 0x21},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_X2_H_INC */

#if NEOVERSE_V2_H_INC
{
	.cpu_partnumber = NEOVERSE_V2_MIDR,
	.cpu_errata_list = {
		[0] = {2719103, 0x00, 0x01},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* NEOVERSE_V2_H_INC */

#if CORTEX_A715_H_INC
{
	.cpu_partnumber = CORTEX_A715_MIDR,
	.cpu_errata_list = {
		[0] = {2701951, 0x00, 0x11},
		[1 ... ERRATA_LIST_END] = UNDEF_ERRATA,
	}
},
#endif /* CORTEX_A715_H_INC */

};

#if ERRATA_NON_ARM_INTERCONNECT

/* Check if the errata is enabled for non-arm interconnect */
static int32_t non_arm_interconnect_errata(uint32_t errata_id, long rev_var)
{
	int32_t ret_val = EM_UNKNOWN_ERRATUM;

	/* Determine the number of cpu listed in the cpu list */
	uint8_t size_cpulist = ARRAY_SIZE(cpu_list);

	/* Read the midr reg to extract cpu, revision and variant info */
	uint32_t midr_val = read_midr();

	for (uint8_t i = 0U; i < size_cpulist; i++) {
		cpu_ptr = &cpu_list[i];
		/*
		 * If the cpu partnumber in the cpu list, matches the midr
		 * part number, check to see if the errata ID matches
		 */
		if (EXTRACT_PARTNUM(midr_val) == EXTRACT_PARTNUM(cpu_ptr->cpu_partnumber)) {

			struct em_cpu *ptr = NULL;

			for (int j = 0; j < MAX_PLAT_CPU_ERRATA_ENTRIES; j++) {
				ptr = &cpu_ptr->cpu_errata_list[j];
				assert(ptr != NULL);
				if (errata_id == ptr->em_errata_id) {
					if (RXPX_RANGE(rev_var, ptr->em_rxpx_lo, ptr->em_rxpx_hi)) {
						ret_val = EM_AFFECTED;
						break;
					}
					ret_val = EM_NOT_AFFECTED;
					break;
				}
			}
			break;
		}
	}
	return ret_val;
}
#endif

/* Function to check if the errata exists for the specific CPU and rxpx */
int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag)
{
	int32_t ret_val;
	struct cpu_ops *cpu_ops;
	struct erratum_entry *entry, *end;
	long rev_var;

	ret_val = EM_UNKNOWN_ERRATUM;
	rev_var = cpu_get_rev_var();

#if ERRATA_NON_ARM_INTERCONNECT
	ret_val = non_arm_interconnect_errata(errata_id, rev_var);
	if (ret_val != EM_UNKNOWN_ERRATUM) {
		return ret_val;
	}
#endif

	cpu_ops = get_cpu_ops_ptr();
	assert(cpu_ops != NULL);

	entry = cpu_ops->errata_list_start;
	assert(entry != NULL);

	end = cpu_ops->errata_list_end;
	assert(end != NULL);

	end--; /* point to the last erratum entry of the queried cpu */

	while ((entry <= end) && (ret_val == EM_UNKNOWN_ERRATUM)) {
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
