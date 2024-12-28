/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/**
 * \file address_types.h
 *
 * \brief Include file to indicate various valid address types for System
 *	  firmware. These types are expected to be used to refer to addressing
 *	  schemes expected to access various entities.
 */

#ifndef ADDRESS_TYPES_H
#define ADDRESS_TYPES_H

#include <types/short_types.h>

/**
 * \brief Physical address representing SoC Bus level physical address.
 *
 * This may be used for register programming (example buffer addresses) OR be
 * used to map back to addresses visible to System Control Processor (such as
 * DMSC). See \ref mapped_addr_t for a remapped address
 */
typedef uint64_t soc_phys_addr_t;

/** Get the lower uint32_t from a \ref soc_phys_addr_t address */
static inline uint32_t soc_phys_low_uint32_t(soc_phys_addr_t w)
{
	return (((uint32_t) (w)) >> 0) & 0xFFFFFFFFU;
}

/** Get the Higher uint32_t from a \ref soc_phys_addr_t address */
static inline uint32_t soc_phys_high_uint32_t(soc_phys_addr_t w)
{
	return ((uint32_t) ((w) >> 32)) & 0xFFFFFFFFU;
}

/** Create soc_phys_addr_t address from 32-bit lo and hi portions */
static inline soc_phys_addr_t soc_phys_create(uint32_t l, uint32_t h)
{
	return (soc_phys_addr_t) (((uint64_t) ((uint64_t) (h) << 32)) | (uint64_t) l);
}

/**
 * \brief Represents the local addressing type for System Control processor.
 *
 * This is expected to be usable without any remapping and indicates that this
 * address map belongs to local address map of System Control Processor (such
 * as DMSC).
 */
typedef uintptr_t local_phys_addr_t;

/**
 * \brief Represents the remapped address for System Control processor.
 *
 * This address is used to represent a remapped address which is visible over
 * a remapping scheme implemented by System control processor. Typically this
 * may be implemented via RAT - See \ref soc_phys_addr_t for further details.
 */
typedef uint32_t mapped_addr_t;

#endif
