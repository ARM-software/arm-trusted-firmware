/*
 * Copyright (c) 2022-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <common/debug.h>
#include <common/feat_detect.h>
#include <plat/common/platform.h>

static bool detection_done[PLATFORM_CORE_COUNT] = { false };

static inline bool __attribute((__always_inline__))
check_feature(int state, unsigned long field, const char *feat_name,
	      unsigned int min, unsigned int max)
{
	if (state == FEAT_STATE_ALWAYS && field < min) {
		ERROR("%s not supported by the PE\n", feat_name);
		return true;
	}
	if (state >= FEAT_STATE_ALWAYS && field > max) {
		ERROR("%s is version %lu, but is only known up to version %u\n",
		      feat_name, field, max);
		return true;
	}

	return false;
}

#define CHECK_FEATURE(name, idreg, guard, field, min, max, worlds)		\
	do {									\
		tainted = tainted || check_feature(guard, _read_ ## name ## _id_field(), #name, min, max); \
	} while (0);

/*
 * Check for a valid combination of build time flags and feature availability on
 * the hardware, according to the ID field value that is required for that
 * feature. Triggers a panic if a feature is forcefully enabled, but not
 * available on the PE. Also will panic if the hardware feature ID field is
 * larger than the maximum known and supported number.
 *
 * This mechanism helps in validating these build flags in the early boot phase
 * and assists in identifying and notifying a mismatch between software and
 * hardware's view of feature on a PE.
 */
void detect_arch_features(unsigned int core_pos)
{
	/* No need to keep checking after the first time for each core. */
	if (detection_done[core_pos]) {
		return;
	}

	bool tainted = false;

	CPUFEAT_LIST(CHECK_FEATURE)
	CPUFEAT_PERCPU_LIST(CHECK_FEATURE)
	CPUFEAT_SPECIAL_FEAT_DETECT_LIST(CHECK_FEATURE)

	/* This is a pseudo feature that depends on FEAT_AMU */
	if (is_feat_amu_supported()) {
		FEAT_AMU_AUX(CHECK_FEATURE)
	}

	if (is_feat_mpam_supported()) {
		FEAT_MPAM_PE_BW_CTRL(CHECK_FEATURE)
	}

	/*
	 * even though this is a "DISABLE" it does confusingly perform feature
	 * enablement duties like all other flags here. Check it against the HW
	 * feature when we intend to diverge from the default behaviour
	 */
	if (_read_feat_mtpmu_id_field() == 15) {
		WARN("DISABLE_MTPMU is implemented in hardware, flag is redundant.\n");
	}

	if (tainted) {
		panic();
	}

	detection_done[core_pos] = true;
}
