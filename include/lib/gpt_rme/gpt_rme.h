/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_RME_H
#define GPT_RME_H

#include <stdint.h>
#include <lib/extensions/rme.h>
#include <lib/gpt_rme/gpc_fault.h>
#include <lib/spinlock.h>

/******************************************************************************/
/* GPT helper macros and definitions for locks                                */
/******************************************************************************/

#if (RME_GPT_BITLOCK_BLOCK != 0)
#define LOCK_SIZE	sizeof(((bitlock_t *)NULL)->lock)
#define LOCK_TYPE	typeof(((bitlock_t *)NULL)->lock)
#define LOCK_BITS	(LOCK_SIZE * UL(8))

CASSERT((UL(1) == LOCK_SIZE), assert_bitlock_type_not_uint8_t);
#endif /* RME_GPT_BITLOCK_BLOCK */

/*
 * Public API to initialize the bitlocks using by legacy GPT library to
 * transition GPI at runtime using delegate/undelegate calls. Granule protection
 * checks must be enabled already or this function will return an error.
 *
 * TODO: Currently FIRME uses legacy gpt_bitlocks. This won't be compiled when
 * FIRME_SUPPORT is enabled and FIRME has internal locks to handle granule
 * transition.
 *
 * Parameters
 *   l1_bitlocks_base	Base address of memory for L1 tables bitlocks.
 *   l1_bitlocks_size	Total size of memory available for L1 tables bitlocks.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
#if ENABLE_FEAT_RME
int gpt_runtime_init(uintptr_t l1_bitlocks_base, size_t l1_bitlocks_size);
#else
static inline int gpt_runtime_init(uintptr_t l1_bitlocks_base, size_t l1_bitlocks_size)
{
	return -1;
}
#endif

/*
 * This function is the core of the granule transition service. When a granule
 * transition request occurs it is routed to this function where the request is
 * validated then fulfilled if possible.
 *
 * Parameters
 *   base: Base address of the region to transition, must be aligned to granule
 *         size.
 *   size: Size of region to transition, must be aligned to granule size.
 *   src_sec_state: Security state of the originating SMC invoking the API.
 *
 * Return
 *    Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_delegate_pas(uint64_t base, size_t size, unsigned int src_sec_state);
int gpt_undelegate_pas(uint64_t base, size_t size, unsigned int src_sec_state);

#if FIRME_SUPPORT
/*
 * The existing bitlocks are used by FIRME to synchorize GPT updates. This
 * will removed once FIRME supports native locks on PA range.
 */
void firme_gpt_lock(uint64_t base);
void firme_gpt_unlock(uint64_t base);
#endif /* FIRME_SUPPORT */
#endif /* GPT_RME_H */
