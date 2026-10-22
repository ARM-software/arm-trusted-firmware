/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Kryo 2xx Silver is a Cortex-A53 r0p4 behind a Qualcomm MIDR: every ID
 * register matches the Cortex-A53 r0p4 and REVIDR_EL1 reports the 835769
 * and 843419 fixes. Build the Cortex-A53 driver with the Kryo MIDR rather
 * than copying it; a platform using this driver must not compile
 * cortex_a53.S as well.
 *
 * Qualcomm's variant and revision encoding (rAp4) has no relation to the
 * Cortex-A53 rNpM scheme, but it compares above every Cortex-A53 revision,
 * so the revision-gated errata resolve as for a late Cortex-A53: 855873 and
 * 1530924 apply, the r0p0-r0p3 ones do not.
 */
#define CORTEX_A53_MIDR		U(0x51AF8014)

#include "../../../../../lib/cpus/aarch64/cortex_a53.S"
