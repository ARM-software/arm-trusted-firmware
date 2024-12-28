/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file ftbool.h
 *
 * \brief Fault tolerant boolean types and checks
 *
 */
#ifndef FTBOOL_H
#define FTBOOL_H

#include <types/short_types.h>

/**
 * Fault tolerant boolean type
 */
typedef uint8_t ftbool;

/*
 * function prototypes
 */
static inline bool ft_is_true(ftbool val);
static inline bool ft_is_false(ftbool val);
static inline bool ft_s_is_true(uint32_t val);

/**
 * Fault tolerant "true" value
 */
#define FT_TRUE		0x5AU
/**
 * Fault tolerant "false" value
 */
#define FT_FALSE	0xA5U

/**
 * Fault tolerant check for "true"
 */
static inline bool ft_is_true(ftbool val)
{
	return (val) == FT_TRUE;
}

/**
 * Fault tolerant check for "false"
 */
static inline bool ft_is_false(ftbool val)
{
	return (val) == FT_FALSE;
}

/**
 * Short fault tolerant "true" value (4-bits) used in H/W registers
 */
#define FT_S_TRUE	0xAU

/**
 * Short fault tolerant "false" value
 */
#define FT_S_FALSE	  0x5U

/**
 * Fault tolerant check for "true" (short value)
 */
static inline bool ft_s_is_true(uint32_t val)
{
	return (val) == FT_S_TRUE;
}

#endif /* FTBOOL */
