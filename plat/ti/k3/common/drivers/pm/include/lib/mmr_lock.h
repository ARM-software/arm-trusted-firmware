/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MMR_LOCK_H
#define MMR_LOCK_H

/** \brief Function to unlock a Control module MMR partition.
 *
 *  \param base address of the region to be unlocked
 *  \param partition which partition to unlock
 */
void mmr_unlock(uint32_t base, uint32_t partition);

/** \brief Function to lock a Control module MMR partition.
 *
 *  \param base address of the region to be locked
 *  \param partition which partition to lock
 */
void mmr_lock(uint32_t base, uint32_t partition);

/** \brief Function to unlock the Control module MMRs needed for PM to
 *	   function.
 */
void mmr_unlock_all(void);

/** \brief Function to lock the Control module MMRs needed for PM to
 *	   ensure no other software entity can play around with these
 *	   registers.
 */
void mmr_lock_all(void);

#endif /* MMR_LOCK_H */
