/*
 * Copyright (c) 2018-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/cpus/errata.h>
#include <lib/smccc.h>
#include <services/arm_arch_svc.h>
#include <smccc_helpers.h>
#include <plat/common/platform.h>

static int32_t smccc_version(void)
{
	return MAKE_SMCCC_VERSION(SMCCC_MAJOR_VERSION, SMCCC_MINOR_VERSION);
}

static inline __unused int32_t smccc_check_for_wa_1(void)
{
#if WORKAROUND_CVE_2017_5715
		switch (check_erratum_applies(CVE(2017, 5715))) {
		case ERRATA_APPLIES:
			return SMC_WA_DO;
		case ERRATA_NOT_APPLIES:
			return SMC_WA_DO_NOT;
		}
#endif
		/* ERRATA_MISSING, CVE not compiled in, or not vulnerable */
		return SMC_ARCH_CALL_NOT_SUPPORTED;
}

static int32_t smccc_arch_features(u_register_t arg1)
{
	switch (arg1) {
	case SMCCC_VERSION:
	case SMCCC_ARCH_FEATURES:
		return SMC_ARCH_CALL_SUCCESS;
	case SMCCC_ARCH_SOC_ID:
		return plat_is_smccc_feature_available(arg1);
#ifdef __aarch64__
	/* Workaround checks are currently only implemented for aarch64 */
	case SMCCC_ARCH_WORKAROUND_1:
		return smccc_check_for_wa_1();

	/* see note for WA_3 */
	case SMCCC_ARCH_WORKAROUND_2: {
#if WORKAROUND_CVE_2018_3639
#if DYNAMIC_WORKAROUND_CVE_2018_3639
		/*
		 * Firmware doesn't have to carry out dynamic workaround if the
		 * PE implements architectural Speculation Store Bypass Safe
		 * (SSBS) feature.
		 */
		if (is_feat_ssbs_present()) {
			return SMC_WA_DO_NOT;
		}

		switch (check_erratum_applies(ERRATUM(ARCH_WORKAROUND_2))) {
		case ERRATA_APPLIES:
			return SMC_WA_DO;
		case ERRATA_NOT_APPLIES:
			return SMC_WA_DO_NOT;
		}
#else
		/* Either the CPUs are unaffected or permanently mitigated */
		return SMC_ARCH_CALL_NOT_REQUIRED;
#endif
#endif
		/* ERRATA_MISSING, CVE not compiled in, or not vulnerable */
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}

	/*
	 * NOTE: this uses the ARCH_WORKAROUND_3 pseudo-erratum instead of the
	 * one registered for CVE_2022_23960 on purpose. This is because not all
	 * cores affected by the CVE need the SMC workaround. For newer cores,
	 * it is assumed that lower EL software is capable of working around the
	 * problem itself and so no firmware involvement is needed. Select cores
	 * that do not have such software can register for the WA_3 SMC
	 * explicitly.
	 */
	case SMCCC_ARCH_WORKAROUND_3:
#if WORKAROUND_CVE_2022_23960
		switch (check_erratum_applies(ERRATUM(ARCH_WORKAROUND_3))) {
		case ERRATA_APPLIES:
			return SMC_WA_DO;
		case ERRATA_NOT_APPLIES:
			return SMC_WA_DO_NOT;
		}
#endif /* WORKAROUND_CVE_2022_23960 */
		/* WA_3 can be used instead of WA_1 */
		return smccc_check_for_wa_1();

	case SMCCC_ARCH_WORKAROUND_4:
#if WORKAROUND_CVE_2024_7881
		/* WA_4 does not have a SMC_WA_DO_NOT */
		if (check_erratum_applies(CVE(2024, 7881)) == ERRATA_APPLIES) {
				return SMC_WA_DO;
		}
#endif /* WORKAROUND_CVE_2024_7881 */
		/* ERRATA_MISSING, CVE not compiled in, or not vulnerable */
		return SMC_ARCH_CALL_NOT_SUPPORTED;

#if ARCH_FEATURE_AVAILABILITY
	case SMCCC_ARCH_FEATURE_AVAILABILITY | (SMC_64 << FUNCID_CC_SHIFT):
		return SMC_ARCH_CALL_SUCCESS;
#endif /* ARCH_FEATURE_AVAILABILITY */

#endif /* __aarch64__ */

	default:
		return SMC_UNK;
	}
}

/* return soc revision or soc version on success otherwise
 * return invalid parameter */
static int32_t smccc_arch_id(u_register_t arg1)
{
	if (arg1 == SMCCC_GET_SOC_REVISION) {
		return plat_get_soc_revision();
	}
	if (arg1 == SMCCC_GET_SOC_VERSION) {
		return plat_get_soc_version();
	}
	return SMC_ARCH_CALL_INVAL_PARAM;
}

/*
 * Top-level Arm Architectural Service SMC handler.
 */
static uintptr_t arm_arch_svc_smc_handler(uint32_t smc_fid,
	u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *cookie,
	void *handle,
	u_register_t flags)
{
	switch (smc_fid) {
	case SMCCC_VERSION:
		SMC_RET1(handle, smccc_version());
	case SMCCC_ARCH_FEATURES:
		SMC_RET1(handle, smccc_arch_features(x1));
	case SMCCC_ARCH_SOC_ID:
		SMC_RET1(handle, smccc_arch_id(x1));
#ifdef __aarch64__
#if WORKAROUND_CVE_2017_5715
	case SMCCC_ARCH_WORKAROUND_1:
		/*
		 * The workaround has already been applied on affected PEs
		 * during entry to EL3. On unaffected PEs, this function
		 * has no effect.
		 */
		SMC_RET0(handle);
#endif
#if WORKAROUND_CVE_2018_3639
	case SMCCC_ARCH_WORKAROUND_2:
		/*
		 * The workaround has already been applied on affected PEs
		 * requiring dynamic mitigation during entry to EL3.
		 * On unaffected or statically mitigated PEs, this function
		 * has no effect.
		 */
		SMC_RET0(handle);
#endif
#if (WORKAROUND_CVE_2022_23960 || WORKAROUND_CVE_2017_5715)
	case SMCCC_ARCH_WORKAROUND_3:
		/*
		 * The workaround has already been applied on affected PEs
		 * during entry to EL3. On unaffected PEs, this function
		 * has no effect.
		 */
		SMC_RET0(handle);
#endif
#if WORKAROUND_CVE_2024_7881
	case SMCCC_ARCH_WORKAROUND_4:
		/*
		 * The workaround has already been applied on affected PEs
		 * during cold boot. This function has no effect whether PE is
		 * affected or not.
		 */
		SMC_RET0(handle);
#endif /* WORKAROUND_CVE_2024_7881 */
#endif /* __aarch64__ */
	default:
		WARN("Unimplemented Arm Architecture Service Call: 0x%x \n",
			smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(
		arm_arch_svc,
		OEN_ARM_START,
		OEN_ARM_END,
		SMC_TYPE_FAST,
		NULL,
		arm_arch_svc_smc_handler
);
