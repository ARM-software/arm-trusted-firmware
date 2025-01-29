/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef CPS_V3_H
#define CPS_V3_H

#ifdef ERROR_INJECTION
#include "cps_ei.h"
#else

#include "inttypes.h"

extern uint32_t cps_readreg32(volatile uint32_t *address);

extern void cps_writereg32(volatile uint32_t *address, uint32_t value);

extern uint64_t cps_readreg64(volatile uint64_t *address);

extern void cps_writereg64(volatile uint64_t *address, uint64_t value);

extern uint8_t cps_uncachedread8(volatile uint8_t *address);

extern uint16_t cps_uncachedread16(volatile uint16_t *address);

extern uint32_t cps_uncachedread32(volatile uint32_t *address);

extern uint64_t cps_uncachedread64(volatile uint64_t *address);

extern void cps_uncachedwrite8(volatile uint8_t *address, uint8_t value);

extern void cps_uncachedwrite16(volatile uint16_t *address, uint16_t value);

extern void cps_uncachedwrite32(volatile uint32_t *address, uint32_t value);

extern void cps_uncachedwrite64(volatile uint64_t *address, uint64_t value);

extern void cps_writephysaddress32(volatile uint32_t *location, uint32_t addrvalue);

extern void cps_buffercopy(volatile uint8_t *dst, volatile const uint8_t *src, uint32_t size);

extern void cps_cacheinvalidate(void *address, size_t size, uintptr_t devinfo);

extern void cps_cacheflush(void *address, size_t size, uintptr_t devinfo);

extern void cps_memorybarrier(void);

extern void cps_memorybarrierwrite(void);

extern void cps_memorybarrierread(void);

#endif  /* ERROR_INJECTION */

#endif  /* multiple inclusion protection */
