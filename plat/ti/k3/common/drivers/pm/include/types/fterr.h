/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file types/fterr.h
 *
 * \brief Fault tolerant error codes, types and checks
 *
 */
#ifndef FTERR_H
#define FTERR_H

#include <types/short_types.h>
#include <types/errno.h>

/**
 * Fault tolerant non-boolean return type
 */
typedef int32_t fterr;

/*
 * Function prototypes
 */
static inline bool ft_is_ok(fterr val);
static inline fterr fterr_from_common(int32_t e);
static inline int32_t fterr_to_common(fterr e);

/**
 * No error (Not using zero for success for fault tolerance). Must be returned
 * as a positive value. (Not as -EFTOK)
 */
#define EFTOK		((fterr) 0x5A)

/**
 * Translate from common error to fterr, only the success value is different
 * Other error codes are common to allow for easy translation. Does not impact
 * fault tolerance.
 */
static inline fterr fterr_from_common(int32_t e)
{
	return (fterr) (((e) == 0) ? EFTOK : (e));
}

/**
 * Translate to common error from fterr, only the success value is different
 * Other error codes are common to allow for easy translation. Does not impact
 * fault tolerance.
 */
static inline int32_t fterr_to_common(fterr e)
{
	return (int32_t) (((e) == EFTOK) ? 0 : (e));
}


/**
 * Check of EFTOK
 */
static inline bool ft_is_ok(fterr val)
{
	return (val) == EFTOK;
}


#endif /* FTERR_H */
