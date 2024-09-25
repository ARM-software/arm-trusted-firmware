/*
 * Copyright 2019-2021, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef S32G2_NCORE_H
#define S32G2_NCORE_H

#include <stdbool.h>

#define NCORE_BASE_ADDR			UL(0x50400000)

#define A53_CLUSTER0_CAIU		U(0)
#define A53_CLUSTER1_CAIU		U(1)

/**
 * Directory Unit Registers
 *
 * The directory provides a point of serialization for establishing transaction
 * ordering and sequences coherence operations and memory accesses.
 */
#define NCORE_DIRU(N)			(NCORE_BASE_ADDR + UL(0x80000) + ((N) * UL(0x1000)))

/* DIRU Snoop Filtering Enable */
#define NCORE_DIRUSFE(N)		(NCORE_DIRU(N) + UL(0x10))
#define NCORE_DIRUSFE_SFEN(SF)		BIT_32(SF)

/* DIRU Caching Agent Snoop Enable */
#define NCORE_DIRUCASE(N)		(NCORE_DIRU(N) + UL(0x40))
#define NCORE_DIRUCASE_CASNPEN(CAIU)	BIT_32(CAIU)

/* DIRU Snoop Filter Maintenance Control */
#define NCORE_DIRUSFMC(N)		(NCORE_DIRU(N) + UL(0x80))
#define NCORE_DIRUSFMC_SFID(SF)		((SF) << 16U)
#define NCORE_DIRUSFMC_SFMNTOP_ALL	U(0x0)

/* DIRU Snoop Filter Maintenance Activity */
#define NCORE_DIRUSFMA(N)		(NCORE_DIRU(N) + UL(0x84))
#define NCORE_DIRUSFMA_MNTOPACTV	BIT_32(0)

/**
 * Coherent Agent Interface Unit Registers
 *
 * CAI provides a means for a fully-coherent agent to be connected to the Ncore.
 * The CAI behaves as a fully-coherent slave.
 */
#define NCORE_CAIU(N)			(NCORE_BASE_ADDR + ((N) * UL(0x1000)))
#define NCORE_CAIU0_BASE_ADDR		NCORE_BASE_ADDR

/* CAIU Transaction Control */
#define NCORE_CAIUTC_OFF		UL(0x0)
#define NCORE_CAIUTC_ISOLEN_SHIFT	U(1)
#define NCORE_CAIUTC_ISOLEN_MASK	BIT_32(NCORE_CAIUTC_ISOLEN_SHIFT)

#define NCORE_CAIUTC(N)			(NCORE_CAIU(N) + NCORE_CAIUTC_OFF)

/* CAIU Identification */
#define NCORE_CAIUID(n)			(NCORE_CAIU(n) + UL(0xFFC))
#define NCORE_CAIUID_TYPE		GENMASK_32(U(19), U(16))
#define NCORE_CAIUID_TYPE_ACE_DVM	U(0x0)

/**
 * Coherent Subsystem Registers
 */
#define NCORE_CSR			(NCORE_BASE_ADDR + UL(0xFF000))

/* Coherent Subsystem ACE DVM Snoop Enable */
#define NCORE_CSADSE			(NCORE_CSR + UL(0x40))
#define NCORE_CSADSE_DVMSNPEN(CAIU)	BIT_32(CAIU)

/* Coherent Subsystem Identification */
#define NCORE_CSID			(NCORE_CSR + UL(0xFFC))
#define NCORE_CSID_NUMSFS_SHIFT		U(18)
#define NCORE_CSID_NUMSFS_MASK		GENMASK_32(U(22), NCORE_CSID_NUMSFS_SHIFT)
#define NCORE_CSID_NUMSFS(CSIDR)	(((CSIDR) & NCORE_CSID_NUMSFS_MASK) \
					  >> NCORE_CSID_NUMSFS_SHIFT)

/* Coherent Subsystem Unit Identification */
#define NCORE_CSUID			(NCORE_CSR + UL(0xFF8))
#define NCORE_CSUID_NUMCMIUS_SHIFT	U(24)
#define NCORE_CSUID_NUMCMIUS_MASK	GENMASK_32(U(29), NCORE_CSUID_NUMCMIUS_SHIFT)
#define NCORE_CSUID_NUMCMIUS(CSUIDR)	(((CSUIDR) & NCORE_CSUID_NUMCMIUS_MASK) \
					 >> NCORE_CSUID_NUMCMIUS_SHIFT)
#define NCORE_CSUID_NUMDIRUS_SHIFT	U(16)
#define NCORE_CSUID_NUMDIRUS_MASK	GENMASK_32(U(21), NCORE_CSUID_NUMDIRUS_SHIFT)
#define NCORE_CSUID_NUMDIRUS(CSUIDR)	(((CSUIDR) & NCORE_CSUID_NUMDIRUS_MASK) \
					 >> NCORE_CSUID_NUMDIRUS_SHIFT)
#define NCORE_CSUID_NUMNCBUS_SHIFT	U(8)
#define NCORE_CSUID_NUMNCBUS_MASK	GENMASK_32(U(13), NCORE_CSUID_NUMNCBUS_SHIFT)
#define NCORE_CSUID_NUMNCBUS(CSUIDR)	(((CSUIDR) & NCORE_CSUID_NUMNCBUS_MASK) \
					 >> NCORE_CSUID_NUMNCBUS_SHIFT)

#ifndef __ASSEMBLER__
void ncore_caiu_online(uint32_t caiu);
void ncore_caiu_offline(uint32_t caiu);
void ncore_init(void);
bool ncore_is_caiu_online(uint32_t caiu);
void ncore_disable_caiu_isolation(uint32_t caiu);
#endif /* __ASSEMBLER__ */

#endif /* S32G2_NCORE_H */
