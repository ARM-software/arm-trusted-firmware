/*
 * Copyright (c) 2023-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <services/errata_abi_svc.h>
#include <smccc_helpers.h>

/* Function to check if the errata exists for the specific CPU and rxpx */
int32_t verify_errata_implemented(uint32_t errata_id)
{
	struct erratum_entry *entry;
	long rev_var;

	rev_var = cpu_get_rev_var();

	entry = find_erratum_entry(errata_id);
	if (entry == NULL) {
		return EM_UNKNOWN_ERRATUM;
	}
	if (entry->check_func(rev_var) != ERRATA_NOT_APPLIES) {
		if ((entry->chosen & WA_ENABLED_MASK) != 0U) {
			if ((entry->chosen & SPLIT_WA_MASK) != 0U) {
				return EM_AFFECTED;
			} else {
				return EM_HIGHER_EL_MITIGATION;
			}
		} else {
			return EM_AFFECTED;
		}
	}
	return EM_NOT_AFFECTED;
}

/* Predicate indicating that a function id is part of EM_ABI */
bool is_errata_fid(uint32_t smc_fid)
{
	switch (smc_fid) {
	case ARM_EM_VERSION:
	case ARM_EM_VERSION | (SMC_64 << FUNCID_CC_SHIFT):
	case ARM_EM_FEATURES:
	case ARM_EM_FEATURES | (SMC_64 << FUNCID_CC_SHIFT):
	case ARM_EM_CPU_ERRATUM_FEATURES:
	case ARM_EM_CPU_ERRATUM_FEATURES | (SMC_64 << FUNCID_CC_SHIFT):
	case ARM_EM_CPU_WORKAROUND_CONFIG:
		return true;
	};

	return false;
}

bool validate_spsr_mode(void)
{
	/* In AArch64, if the caller is EL1, return true */

	#ifdef __aarch64__
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
	case ARM_EM_VERSION | (SMC_64 << FUNCID_CC_SHIFT):
		SMC_RET1(handle, MAKE_SMCCC_VERSION(
			EM_VERSION_MAJOR, EM_VERSION_MINOR
		));
		break; /* unreachable */
	case ARM_EM_FEATURES:
	case ARM_EM_FEATURES | (SMC_64 << FUNCID_CC_SHIFT):
		if (is_errata_fid((uint32_t)x1)) {
			SMC_RET1(handle, EM_SUCCESS);
		}

		SMC_RET1(handle, EM_NOT_SUPPORTED);
		break; /* unreachable */
	case ARM_EM_CPU_ERRATUM_FEATURES:
	case ARM_EM_CPU_ERRATUM_FEATURES | (SMC_64 << FUNCID_CC_SHIFT):

		/*
		 * If the forward flag is greater than zero and the calling EL
		 * is EL1 in AArch64 or in system mode or svc mode in case of AArch32,
		 * return Invalid Parameters.
		 */
		if (((uint32_t)x2 != 0) && (validate_spsr_mode())) {
			SMC_RET1(handle, EM_INVALID_PARAMETERS);
		}
		ret_id = verify_errata_implemented((uint32_t)x1);
		SMC_RET1(handle, ret_id);
		break; /* unreachable */
	/* SMC64 only */
	case ARM_EM_CPU_WORKAROUND_CONFIG:
		SMC_RET1(handle, EM_UNKNOWN_ERRATUM);
		break; /* unreachable */
	default:
		{
		   WARN("Unimplemented Errata ABI Service Call: 0x%x\n", smc_fid);
		   SMC_RET1(handle, EM_UNKNOWN_ERRATUM);
		   break; /* unreachable */
		}
	}
}
