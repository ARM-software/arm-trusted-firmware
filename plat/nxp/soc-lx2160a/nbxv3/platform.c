/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 CPU Module - board-specific platform hooks.
 *
 * This file implements board-level callbacks that the common NXP
 * platform code invokes during boot. Currently only POVDD control
 * is board-specific; everything else uses the common LX2160A code
 * in soc.c and the shared NXP platform layer.
 */

#include <plat_common.h>

/*
 * POVDD (Program-OTP VDD) control on Nodebox v3.
 *
 * POVDD is the supply rail that powers the Secure Fuse Processor
 * (SFP) OTP array during a fuse-program transaction (SRKH hash,
 * OTPMK, ITS, DEBUG_LOCK). It must be raised for the duration of
 * the SFP program cycle and held at ground otherwise so that no
 * stray fuse write can occur. On LX2160A the SoC pin is TA_PROG_SFP;
 * per the LX2160A RM "Signal descriptions" it wants 1.8 V applied
 * for fuse programming and 0 V otherwise.
 *
 * Nodebox v3 design decision: fuse programming is handled entirely
 * by an external factory, NOT by TF-A.
 *
 * Consequences:
 *   - POVDD_ENABLE stays 'no' in platform.mk.
 *   - board_enable_povdd() / board_disable_povdd() return false
 *     unconditionally.
 *   - The Trusted Board Boot in-system fuse-provisioning path
 *     (POLICY_FUSE_PROVISION) is not expected to run on Nodebox v3.
 *     SRKH / OTPMK / ITS / DEBUG_LOCK are provisioned out-of-band
 *     by a dedicated fuse-burn fixture that drives the TA_PROG_SFP
 *     test point directly; no platform change is required here to
 *     support that external flow.
 */

#pragma weak board_enable_povdd
#pragma weak board_disable_povdd

bool board_enable_povdd(void)
{
	return false;
}

bool board_disable_povdd(void)
{
	return false;
}
