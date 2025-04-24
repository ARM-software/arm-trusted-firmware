/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/cpus/errata.h>
#include <lib/smccc.h>
#include <services/arm_arch_svc.h>
#include <smccc_helpers.h>
#include <plat/common/platform.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/context_mgmt.h>

static int32_t smccc_version(void)
{
	return MAKE_SMCCC_VERSION(SMCCC_MAJOR_VERSION, SMCCC_MINOR_VERSION);
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
#if WORKAROUND_CVE_2017_5715
	case SMCCC_ARCH_WORKAROUND_1:
		if (check_erratum_applies(CVE(2017, 5715))
			== ERRATA_NOT_APPLIES) {
			return 1;
		}

		return 0; /* ERRATA_APPLIES || ERRATA_MISSING */
#endif

#if WORKAROUND_CVE_2018_3639
	case SMCCC_ARCH_WORKAROUND_2: {
#if DYNAMIC_WORKAROUND_CVE_2018_3639
		unsigned long long ssbs;

		/*
		 * Firmware doesn't have to carry out dynamic workaround if the
		 * PE implements architectural Speculation Store Bypass Safe
		 * (SSBS) feature.
		 */
		ssbs = (read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_SSBS_SHIFT) &
			ID_AA64PFR1_EL1_SSBS_MASK;

		/*
		 * If architectural SSBS is available on this PE, no firmware
		 * mitigation via SMCCC_ARCH_WORKAROUND_2 is required.
		 */
		if (ssbs != SSBS_NOT_IMPLEMENTED)
			return 1;

		/*
		 * On a platform where at least one CPU requires
		 * dynamic mitigation but others are either unaffected
		 * or permanently mitigated, report the latter as not
		 * needing dynamic mitigation.
		 */
		if (check_erratum_applies(ERRATUM(ARCH_WORKAROUND_2))
			== ERRATA_NOT_APPLIES)
			return 1;

		/*
		 * If we get here, this CPU requires dynamic mitigation
		 * so report it as such.
		 */
		return 0;
#else
		/* Either the CPUs are unaffected or permanently mitigated */
		return SMC_ARCH_CALL_NOT_REQUIRED;
#endif
	}
#endif

#if (WORKAROUND_CVE_2022_23960 || WORKAROUND_CVE_2017_5715)
	case SMCCC_ARCH_WORKAROUND_3:
		/*
		 * SMCCC_ARCH_WORKAROUND_3 should also take into account
		 * CVE-2017-5715 since this SMC can be used instead of
		 * SMCCC_ARCH_WORKAROUND_1.
		 */
		if ((check_erratum_applies(ERRATUM(ARCH_WORKAROUND_3))
			== ERRATA_NOT_APPLIES) &&
		    (check_erratum_applies(CVE(2017, 5715))
			== ERRATA_NOT_APPLIES)) {
			return 1;
		}

		return 0; /* ERRATA_APPLIES || ERRATA_MISSING */
#endif

#if ARCH_FEATURE_AVAILABILITY
	case SMCCC_ARCH_FEATURE_AVAILABILITY:
		return SMC_ARCH_CALL_SUCCESS;
#endif /* ARCH_FEATURE_AVAILABILITY */

#if WORKAROUND_CVE_2024_7881
	case SMCCC_ARCH_WORKAROUND_4:
		if (check_erratum_applies(CVE(2024, 7881)) != ERRATA_APPLIES) {
			return SMC_ARCH_CALL_NOT_SUPPORTED;
		}
		return 0;
#endif /* WORKAROUND_CVE_2024_7881 */

#endif /* __aarch64__ */

	/* Fallthrough */

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
 * Reads a system register, sanitises its value, and returns a bitmask
 * representing which feature in that sysreg has been enabled by firmware. The
 * bitmask is a 1:1 mapping to the register's fields.
 */
#if ARCH_FEATURE_AVAILABILITY
static uintptr_t smccc_arch_feature_availability(u_register_t reg,
						 void *handle,
						 u_register_t flags)
{
	cpu_context_t *caller_context;
	per_world_context_t *caller_per_world_context;
	el3_state_t *state;
	u_register_t bitmask, check;

	/* check the caller security state */
	if (is_caller_secure(flags)) {
		caller_context = cm_get_context(SECURE);
		caller_per_world_context = &per_world_context[CPU_CONTEXT_SECURE];
	} else if (is_caller_non_secure(flags)) {
		caller_context = cm_get_context(NON_SECURE);
		caller_per_world_context = &per_world_context[CPU_CONTEXT_NS];
	} else {
#if ENABLE_RME
		caller_context = cm_get_context(REALM);
		caller_per_world_context = &per_world_context[CPU_CONTEXT_REALM];
#else /* !ENABLE_RME */
		assert(0); /* shouldn't be possible */
#endif /* ENABLE_RME */
	}

	state = get_el3state_ctx(caller_context);

	switch (reg) {
	case SCR_EL3_OPCODE:
		bitmask  = read_ctx_reg(state, CTX_SCR_EL3);
		bitmask &= ~SCR_EL3_IGNORED;
		check    = bitmask & ~SCR_EL3_FEATS;
		bitmask &= SCR_EL3_FEATS;
		bitmask ^= SCR_EL3_FLIPPED;
		/* will only report 0 if neither is implemented */
		if (is_feat_rng_trap_supported() || is_feat_rng_present())
			bitmask |= SCR_TRNDR_BIT;
		break;
	case CPTR_EL3_OPCODE:
		bitmask  = caller_per_world_context->ctx_cptr_el3;
		check    = bitmask & ~CPTR_EL3_FEATS;
		bitmask &= CPTR_EL3_FEATS;
		bitmask ^= CPTR_EL3_FLIPPED;
		break;
	case MDCR_EL3_OPCODE:
		bitmask  = read_ctx_reg(state, CTX_MDCR_EL3);
		bitmask &= ~MDCR_EL3_IGNORED;
		check    = bitmask & ~MDCR_EL3_FEATS;
		bitmask &= MDCR_EL3_FEATS;
		bitmask ^= MDCR_EL3_FLIPPED;
		break;
#if ENABLE_FEAT_MPAM
	case MPAM3_EL3_OPCODE:
		bitmask  = caller_per_world_context->ctx_mpam3_el3;
		bitmask &= ~MPAM3_EL3_IGNORED;
		check    = bitmask & ~MPAM3_EL3_FEATS;
		bitmask &= MPAM3_EL3_FEATS;
		bitmask ^= MPAM3_EL3_FLIPPED;
		break;
#endif /* ENABLE_FEAT_MPAM */
	default:
		SMC_RET2(handle, SMC_INVALID_PARAM, ULL(0));
	}

	/*
	 * failing this means that the requested register has a bit set that
	 * hasn't been declared as a known feature bit or an ignore bit. This is
	 * likely to happen when support for a new feature is added but the
	 * bitmask macros are not updated.
	 */
	if (ENABLE_ASSERTIONS && check != 0) {
		ERROR("Unexpected bits 0x%lx were set in register %lx!\n", check, reg);
		assert(0);
	}

	SMC_RET2(handle, SMC_ARCH_CALL_SUCCESS, bitmask);
}
#endif /* ARCH_FEATURE_AVAILABILITY */

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
#if ARCH_FEATURE_AVAILABILITY
	/* return is 64 bit so only reply on SMC64 requests */
	case SMCCC_ARCH_FEATURE_AVAILABILITY | (SMC_64 << FUNCID_CC_SHIFT):
		return smccc_arch_feature_availability(x1, handle, flags);
#endif /* ARCH_FEATURE_AVAILABILITY */
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
