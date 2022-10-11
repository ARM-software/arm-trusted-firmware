/*
 * Copyright (c) 2014-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/pmf/pmf.h>
#include <lib/psci/psci.h>
#include <lib/runtime_instr.h>
#include <services/drtm_svc.h>
#include <services/pci_svc.h>
#include <services/rmmd_svc.h>
#include <services/sdei.h>
#include <services/spm_mm_svc.h>
#include <services/spmc_svc.h>
#include <services/spmd_svc.h>
#include <services/std_svc.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

/* Standard Service UUID */
static uuid_t arm_svc_uid = {
	{0x5b, 0x90, 0x8d, 0x10},
	{0x63, 0xf8},
	{0xe8, 0x47},
	0xae, 0x2d,
	{0xc0, 0xfb, 0x56, 0x41, 0xf6, 0xe2}
};

/* Setup Standard Services */
static int32_t std_svc_setup(void)
{
	uintptr_t svc_arg;
	int ret = 0;

	svc_arg = get_arm_std_svc_args(PSCI_FID_MASK);
	assert(svc_arg);

	/*
	 * PSCI is one of the specifications implemented as a Standard Service.
	 * The `psci_setup()` also does EL3 architectural setup.
	 */
	if (psci_setup((const psci_lib_args_t *)svc_arg) != PSCI_E_SUCCESS) {
		ret = 1;
	}

#if SPM_MM
	if (spm_mm_setup() != 0) {
		ret = 1;
	}
#endif

#if defined(SPD_spmd)
	if (spmd_setup() != 0) {
		ret = 1;
	}
#endif

#if ENABLE_RME
	if (rmmd_setup() != 0) {
		ret = 1;
	}
#endif

#if SDEI_SUPPORT
	/* SDEI initialisation */
	sdei_init();
#endif

#if TRNG_SUPPORT
	/* TRNG initialisation */
	trng_setup();
#endif /* TRNG_SUPPORT */

#if DRTM_SUPPORT
	if (drtm_setup() != 0) {
		ret = 1;
	}
#endif /* DRTM_SUPPORT */

	return ret;
}

/*
 * Top-level Standard Service SMC handler. This handler will in turn dispatch
 * calls to PSCI SMC handler
 */
static uintptr_t std_svc_smc_handler(uint32_t smc_fid,
			     u_register_t x1,
			     u_register_t x2,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	if (((smc_fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK) == SMC_32) {
		/* 32-bit SMC function, clear top parameter bits */

		x1 &= UINT32_MAX;
		x2 &= UINT32_MAX;
		x3 &= UINT32_MAX;
		x4 &= UINT32_MAX;
	}

	/*
	 * Dispatch PSCI calls to PSCI SMC handler and return its return
	 * value
	 */
	if (is_psci_fid(smc_fid)) {
		uint64_t ret;

#if ENABLE_RUNTIME_INSTRUMENTATION

		/*
		 * Flush cache line so that even if CPU power down happens
		 * the timestamp update is reflected in memory.
		 */
		PMF_WRITE_TIMESTAMP(rt_instr_svc,
		    RT_INSTR_ENTER_PSCI,
		    PMF_CACHE_MAINT,
		    get_cpu_data(cpu_data_pmf_ts[CPU_DATA_PMF_TS0_IDX]));
#endif

		ret = psci_smc_handler(smc_fid, x1, x2, x3, x4,
		    cookie, handle, flags);

#if ENABLE_RUNTIME_INSTRUMENTATION
		PMF_CAPTURE_TIMESTAMP(rt_instr_svc,
		    RT_INSTR_EXIT_PSCI,
		    PMF_NO_CACHE_MAINT);
#endif

		SMC_RET1(handle, ret);
	}

#if SPM_MM
	/*
	 * Dispatch SPM calls to SPM SMC handler and return its return
	 * value
	 */
	if (is_spm_mm_fid(smc_fid)) {
		return spm_mm_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
					  handle, flags);
	}
#endif

#if defined(SPD_spmd)
	/*
	 * Dispatch FFA calls to the FFA SMC handler implemented by the SPM
	 * dispatcher and return its return value
	 */
	if (is_ffa_fid(smc_fid)) {
		return spmd_ffa_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
					    handle, flags);
	}
#endif

#if SDEI_SUPPORT
	if (is_sdei_fid(smc_fid)) {
		return sdei_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle,
				flags);
	}
#endif

#if TRNG_SUPPORT
	if (is_trng_fid(smc_fid)) {
		return trng_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle,
				flags);
	}
#endif /* TRNG_SUPPORT */

#if ENABLE_RME

	if (is_rmmd_el3_fid(smc_fid)) {
		return rmmd_rmm_el3_handler(smc_fid, x1, x2, x3, x4, cookie,
					    handle, flags);
	}

	if (is_rmi_fid(smc_fid)) {
		return rmmd_rmi_handler(smc_fid, x1, x2, x3, x4, cookie,
					handle, flags);
	}
#endif

#if SMC_PCI_SUPPORT
	if (is_pci_fid(smc_fid)) {
		return pci_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle,
				       flags);
	}
#endif

#if DRTM_SUPPORT
	if (is_drtm_fid(smc_fid)) {
		return drtm_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle,
					flags);
	}
#endif /* DRTM_SUPPORT */

	switch (smc_fid) {
	case ARM_STD_SVC_CALL_COUNT:
		/*
		 * Return the number of Standard Service Calls. PSCI is the only
		 * standard service implemented; so return number of PSCI calls
		 */
		SMC_RET1(handle, PSCI_NUM_CALLS);

	case ARM_STD_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, arm_svc_uid);

	case ARM_STD_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, STD_SVC_VERSION_MAJOR, STD_SVC_VERSION_MINOR);

	default:
		VERBOSE("Unimplemented Standard Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(
		std_svc,

		OEN_STD_START,
		OEN_STD_END,
		SMC_TYPE_FAST,
		std_svc_setup,
		std_svc_smc_handler
);
