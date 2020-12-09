/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CAAM_IO_H
#define CAAM_IO_H

#include <endian.h>
#include <lib/mmio.h>

typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;

/* Return higher 32 bits of physical address */
#define PHYS_ADDR_HI(phys_addr) \
	    (uint32_t)(((uint64_t)phys_addr) >> 32)

/* Return lower 32 bits of physical address */
#define PHYS_ADDR_LO(phys_addr) \
	    (uint32_t)(((uint64_t)phys_addr) & 0xFFFFFFFF)

#ifdef NXP_SEC_BE
#define sec_in32(a)	bswap32(mmio_read_32((uintptr_t)(a)))
#define sec_out32(a, v)	mmio_write_32((uintptr_t)(a), bswap32(v))
#define sec_in64(addr)  (					\
	((uint64_t)sec_in32((uintptr_t)(addr)) << 32) |	\
	(sec_in32(((uintptr_t)(addr)) + 4)))
#define sec_out64(addr, val) ({					\
	sec_out32(((uintptr_t)(addr)), (uint32_t)((val) >> 32));	\
	sec_out32(((uintptr_t)(addr)) + 4, (uint32_t)(val)); })
#elif defined(NXP_SEC_LE)
#define sec_in32(a)	mmio_read_32((uintptr_t)(a))
#define sec_out32(a, v)	mmio_write_32((uintptr_t)(a), (v))
#define sec_in64(addr)	(					\
	((uint64_t)sec_in32((uintptr_t)(addr) + 4) << 32) |	\
	(sec_in32((uintptr_t)(addr))))
#define sec_out64(addr, val) ({						\
	sec_out32(((uintptr_t)(addr)) + 4, (uint32_t)((val) >> 32));	\
	sec_out32(((uintptr_t)(addr)), (uint32_t)(val)); })
#else
#error Please define CCSR SEC register endianness
#endif

static inline void *ptov(phys_addr_t *ptr)
{
	return (void *)ptr;
}

static inline phys_addr_t *vtop(void *ptr)
{
	return (phys_addr_t *)ptr;
}
#endif /* CAAM_IO_H */
