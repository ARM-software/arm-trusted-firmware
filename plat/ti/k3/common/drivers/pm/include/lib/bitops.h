/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BITOPS_H
#define BITOPS_H

#include <types/short_types.h>

/** BIT macro for easily calculating a bit at offset (n) */
/* #define BIT(n)  (((uint32_t) 1U) << (n)) */

#define ORDER_1B	0U
#define ORDER_2B	1U
#define ORDER_4B	2U
#define ORDER_8B	3U
#define ORDER_16B	4U
#define ORDER_32B	5U
#define ORDER_64B	6U
#define ORDER_128B	7U
#define ORDER_256B	8U
#define ORDER_512B	9U
#define ORDER_1KB	10U
#define ORDER_2KB	11U
#define ORDER_4KB	12U
#define ORDER_8KB	13U
#define ORDER_16KB	14U
#define ORDER_32KB	15U
#define ORDER_64KB	16U
#define ORDER_128KB	17U
#define ORDER_256KB	18U
#define ORDER_512KB	19U
#define ORDER_1MB	20U
#define ORDER_2MB	21U
#define ORDER_4MB	22U
#define ORDER_8MB	23U
#define ORDER_16MB	24U
#define ORDER_32MB	25U
#define ORDER_64MB	26U
#define ORDER_128MB	27U
#define ORDER_256MB	28U
#define ORDER_512MB	29U
#define ORDER_1GB	30U
#define ORDER_2GB	31U

static inline uint32_t get_field(uint32_t val, uint32_t end, uint32_t start);
static inline uint32_t set_field(uint32_t tgt, uint32_t end, uint32_t start, uint32_t val);
static inline uint32_t get_mask(uint32_t end, uint32_t start);

/**
 * \brief Generate a mask for the give bit field [end:start]
 *
 * All the bits between the two specified bit indices including
 * the specified indices are set to 1. Rest of the bits are set
 * to zero.
 *
 * \param end upper bit index of the mask
 * \param start lower bit index of the mask
 *
 * \return generated mask
 */
static inline uint32_t get_mask(uint32_t end, uint32_t start)
{
	return (uint32_t) ((1U << (end - start + 1U)) - 1U) << start;
}

/**
 * \brief extract the specified bit field from a 32 bit unsigned integer.
 *
 * \param val input from which the bitfield is extracted
 * \param end upper bit index of the bitfield
 * \param start lower bit index of the bitfield
 *
 * \return extracted bit field, right shifted by start bits.
 */
static inline uint32_t get_field(uint32_t val, uint32_t end, uint32_t start)
{
	uint32_t mask = get_mask(end, start);

	return (val & mask) >> start;
}

/**
 * \brief set the specified bit field from a 32 bit unsigned integer
 * to provided value
 *
 * \param tgt input in which the bitfield is to be set
 * \param end upper bit index of the bitfield
 * \param start lower bit index of the bitfield
 * \param val  value to which the bit field is to be set.
 *
 * \return input value with bitfield updated as specified.
 */
static inline uint32_t set_field(uint32_t tgt, uint32_t end, uint32_t start, uint32_t val)
{
	uint32_t ret = 0U;
	uint32_t mask = get_mask(end, start);

	ret = (tgt & (~mask));

	ret = (ret | ((val << start) & mask));

	return ret;
}

#endif
