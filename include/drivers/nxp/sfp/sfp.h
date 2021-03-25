/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SFP_H
#define SFP_H

#include <endian.h>
#include <lib/mmio.h>

/* SFP Configuration Register Offsets */
#define SFP_INGR_OFFSET		U(0x20)
#define SFP_SVHESR_OFFSET	U(0x24)
#define SFP_SFPCR_OFFSET	U(0x28)
#define SFP_VER_OFFSET		U(0x38)

/* SFP Hamming register masks for OTPMK and DRV */
#define SFP_SVHESR_DRV_MASK	U(0x7F)
#define SFP_SVHESR_OTPMK_MASK	U(0x7FC00)

/* SFP commands */
#define SFP_INGR_READFB_CMD	U(0x1)
#define SFP_INGR_PROGFB_CMD	U(0x2)
#define SFP_INGR_ERROR_MASK	U(0x100)

/* SFPCR Masks */
#define SFP_SFPCR_WD		U(0x80000000)
#define SFP_SFPCR_WDL		U(0x40000000)

/* SFPCR Masks */
#define SFP_SFPCR_WD		U(0x80000000)
#define SFP_SFPCR_WDL		U(0x40000000)

#define SFP_FUSE_REGS_OFFSET	U(0x200)

#ifdef NXP_SFP_VER_3_4
#define OSPR0_SC_MASK		U(0xC000FE35)
#elif defined(NXP_SFP_VER_3_2)
#define OSPR0_SC_MASK		U(0x0000E035)
#endif

#if defined(NXP_SFP_VER_3_4)
#define OSPR_KEY_REVOC_SHIFT	U(9)
#define OSPR_KEY_REVOC_MASK	U(0x0000fe00)
#elif defined(NXP_SFP_VER_3_2)
#define OSPR_KEY_REVOC_SHIFT	U(13)
#define OSPR_KEY_REVOC_MASK	U(0x0000e000)
#endif /* NXP_SFP_VER_3_4 */

#define OSPR1_MC_MASK		U(0xFFFF0000)
#define OSPR1_DBG_LVL_MASK	U(0x00000007)

#define OSPR_ITS_MASK		U(0x00000004)
#define OSPR_WP_MASK		U(0x00000001)

#define MAX_OEM_UID		U(5)
#define SRK_HASH_SIZE		U(32)

/* SFP CCSR Register Map */
struct sfp_ccsr_regs_t {
	uint32_t ospr;			/* 0x200 OSPR0 */
	uint32_t ospr1;			/* 0x204 OSPR1 */
	uint32_t dcv[2];		/* 0x208 Debug Challenge Value */
	uint32_t drv[2];		/* 0x210 Debug Response Value */
	uint32_t fswpr;			/* 0x218 FSL Section Write Protect */
	uint32_t fsl_uid[2];		/* 0x21c FSL UID 0 */
	uint32_t isbcr;			/* 0x224 ISBC Configuration */
	uint32_t fsspr[3];		/* 0x228 FSL Scratch Pad */
	uint32_t otpmk[8];		/* 0x234 OTPMK */
	uint32_t srk_hash[SRK_HASH_SIZE/sizeof(uint32_t)];
					/* 0x254 Super Root Key Hash */
	uint32_t oem_uid[MAX_OEM_UID];	/* 0x274 OEM UID 0 */
};

uintptr_t get_sfp_addr(void);
void sfp_init(uintptr_t nxp_sfp_addr);
uint32_t *get_sfp_srk_hash(void);
int sfp_check_its(void);
int sfp_check_oem_wp(void);
uint32_t get_key_revoc(void);
void set_sfp_wr_disable(void);
int sfp_program_fuses(void);

uint32_t sfp_read_oem_uid(uint8_t oem_uid);
uint32_t sfp_write_oem_uid(uint8_t oem_uid, uint32_t sfp_val);

#ifdef NXP_SFP_BE
#define sfp_read32(a)           bswap32(mmio_read_32((uintptr_t)(a)))
#define sfp_write32(a, v)       mmio_write_32((uintptr_t)(a), bswap32(v))
#elif defined(NXP_SFP_LE)
#define sfp_read32(a)           mmio_read_32((uintptr_t)(a))
#define sfp_write32(a, v)       mmio_write_32((uintptr_t)(a), (v))
#else
#error Please define CCSR SFP register endianness
#endif

#endif/* SFP_H */
