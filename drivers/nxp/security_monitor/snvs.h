/*
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SNVS_H
#define SNVS_H

#include <platform_def.h>
#include <lib/mmio.h>
#include <endian.h>

struct snvs_regs {
	uint32_t reserved1;
	uint32_t hp_com;		/* 0x04 SNVS_HP Command Register */
	uint32_t reserved2[3];
	uint32_t hp_stat;		/* 0x14 SNVS_HP Status Register */
};

/* SSM_ST field in SNVS status reg */
#define HPSTS_CHECK_SSM_ST	0x900	/* SNVS is in check state */
#define HPSTS_NON_SECURE_SSM_ST	0xb00	/* SNVS is in non secure state */
#define HPSTS_TRUST_SSM_ST	0xd00	/* SNVS is in trusted state */
#define HPSTS_SECURE_SSM_ST	0xf00	/* SNVS is in secure state */
#define HPSTS_SOFT_FAIL_SSM_ST	0x300	/* SNVS is in soft fail state */
#define HPSTS_MASK_SSM_ST	0xf00	/* SSM_ST field mask in SNVS reg */

/* SNVS register bits */
#define HPCOM_SW_SV		0x100	/* Security Violation bit */
#define HPCOM_SW_FSV		0x200	/* Fatal Security Violation bit */
#define HPCOM_SSM_ST		0x1	/* SSM_ST field in SNVS command reg */
#define HPCOM_SSM_ST_DIS	0x2	/* Disable Secure to Trusted State */
#define HPCOM_SSM_SFNS_DIS	0x4	/* Disable Soft Fail to Non-Secure */

#ifdef NXP_SNVS_BE
#define snvs_read32(a)           bswap32(mmio_read_32((uintptr_t)a))
#define snvs_write32(a, v)       mmio_write_32((uintptr_t)a, bswap32(v))
#elif defined(NXP_SNVS_LE)
#define snvs_read32(a)           mmio_read_32((uintptr_t)a)
#define snvs_write32(a, v)       mmio_write_32((uintptr_t)a, v)
#else
#error Please define CCSR SNVS register endianness
#endif

uint32_t get_snvs_state(void);
void transition_snvs_non_secure(void);
void transition_snvs_soft_fail(void);
uint32_t transition_snvs_trusted(void);
uint32_t transition_snvs_secure(void);

#endif
