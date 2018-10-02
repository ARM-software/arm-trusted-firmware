/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SMCCC_H__
#define __SMCCC_H__

#include <utils_def.h>

#define SMCCC_VERSION_MAJOR_SHIFT	U(16)
#define SMCCC_VERSION_MAJOR_MASK	U(0x7FFF)
#define SMCCC_VERSION_MINOR_SHIFT	U(0)
#define SMCCC_VERSION_MINOR_MASK	U(0xFFFF)
#define MAKE_SMCCC_VERSION(_major, _minor) \
	((((uint32_t)(_major) & SMCCC_VERSION_MAJOR_MASK) << \
						SMCCC_VERSION_MAJOR_SHIFT) \
	| (((uint32_t)(_minor) & SMCCC_VERSION_MINOR_MASK) << \
						SMCCC_VERSION_MINOR_SHIFT))

#if SMCCC_MAJOR_VERSION == 1
# define SMCCC_MINOR_VERSION U(1)
# include <smccc_v1.h>
#elif SMCCC_MAJOR_VERSION == 2
# define SMCCC_MINOR_VERSION U(0)
# include <smccc_v2.h>
#else
# error "Unsupported version of SMCCC."
#endif

/* Various flags passed to SMC handlers */
#define SMC_FROM_SECURE		(U(0) << 0)
#define SMC_FROM_NON_SECURE	(U(1) << 0)

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <stdint.h>

#define is_caller_non_secure(_f)	(((_f) & SMC_FROM_NON_SECURE) != U(0))
#define is_caller_secure(_f)		(!is_caller_non_secure(_f))

/* The macro below is used to identify a Standard Service SMC call */
#define is_std_svc_call(_fid)		(GET_SMC_OEN(_fid) == OEN_STD_START)

/* The macro below is used to identify a Arm Architectural Service SMC call */
#define is_arm_arch_svc_call(_fid)	(GET_SMC_OEN(_fid) == OEN_ARM_START)

/* The macro below is used to identify a valid Fast SMC call */
#define is_valid_fast_smc(_fid)		((!(((_fid) >> 16) & U(0xff))) && \
					   (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST))

/*
 * Macro to define UUID for services. Apart from defining and initializing a
 * uuid_t structure, this macro verifies that the first word of the defined UUID
 * does not equal SMC_UNK. This is to ensure that the caller won't mistake the
 * returned UUID in x0 for an invalid SMC error return
 */
#define DEFINE_SVC_UUID2(_name, _tl, _tm, _th, _cl, _ch,		\
		_n0, _n1, _n2, _n3, _n4, _n5)				\
	CASSERT((uint32_t)(_tl) != (uint32_t) SMC_UNK, invalid_svc_uuid);\
	static const uuid_t _name = {					\
		{(_tl >> 24) & 0xFF,					\
		 (_tl >> 16) & 0xFF,					\
		 (_tl >> 8)  & 0xFF,					\
		 (_tl & 0xFF)},						\
		{(_tm >> 8) & 0xFF,					\
		 (_tm  & 0xFF)},					\
		{(_th >> 8) & 0xFF,					\
		 (_th & 0xFF)},						\
		_cl, _ch,						\
		{ _n0, _n1, _n2, _n3, _n4, _n5 }			\
	}

/*
 * Return a UUID in the SMC return registers.
 *
 * Acccording to section 5.3 of the SMCCC, UUIDs are returned as a single
 * 128-bit value using the SMC32 calling convention. This value is mapped to
 * argument registers x0-x3 on AArch64 (resp. r0-r3 on AArch32). x0 for example
 * shall hold bytes 0 to 3, with byte 0 in the low-order bits.
 */
static inline uint32_t smc_uuid_word(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
	return ((uint32_t) b0) | (((uint32_t) b1) << 8) |
		(((uint32_t) b2) << 16) | (((uint32_t) b3) << 24);
}

#define SMC_UUID_RET(_h, _uuid)							\
	SMC_RET4(handle,							\
		smc_uuid_word((_uuid).time_low[0], (_uuid).time_low[1],		\
			      (_uuid).time_low[2], (_uuid).time_low[3]),	\
		smc_uuid_word((_uuid).time_mid[0], (_uuid).time_mid[1],		\
			      (_uuid).time_hi_and_version[0],			\
			      (_uuid).time_hi_and_version[1]),			\
		smc_uuid_word((_uuid).clock_seq_hi_and_reserved,		\
			      (_uuid).clock_seq_low, (_uuid).node[0],		\
			      (_uuid).node[1]),					\
		smc_uuid_word((_uuid).node[2], (_uuid).node[3],			\
			      (_uuid).node[4], (_uuid).node[5]))

#endif /*__ASSEMBLY__*/
#endif /* __SMCCC_H__ */
