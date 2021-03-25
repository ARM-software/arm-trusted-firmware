/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#if !defined(FUSE_PROV_H) && defined(POLICY_FUSE_PROVISION)
#define FUSE_PROV_H

#include <endian.h>
#include <lib/mmio.h>

#define MASK_NONE		U(0xFFFFFFFF)
#define ERROR_WRITE		U(0xA)
#define ERROR_ALREADY_BLOWN	U(0xB)

/* Flag bit shifts */
#define FLAG_POVDD_SHIFT	U(0)
#define FLAG_SYSCFG_SHIFT	U(1)
#define FLAG_SRKH_SHIFT		U(2)
#define FLAG_MC_SHIFT		U(3)
#define FLAG_DCV0_SHIFT		U(4)
#define FLAG_DCV1_SHIFT		U(5)
#define FLAG_DRV0_SHIFT		U(6)
#define FLAG_DRV1_SHIFT		U(7)
#define FLAG_OUID0_SHIFT	U(8)
#define FLAG_OUID1_SHIFT	U(9)
#define FLAG_OUID2_SHIFT	U(10)
#define FLAG_OUID3_SHIFT	U(11)
#define FLAG_OUID4_SHIFT	U(12)
#define FLAG_DBG_LVL_SHIFT	U(13)
#define FLAG_OTPMK_SHIFT	U(16)
#define FLAG_OUID_MASK		U(0x1F)
#define FLAG_DEBUG_MASK		U(0xF)
#define FLAG_OTPMK_MASK		U(0xF)

/* OTPMK flag values */
#define PROG_OTPMK_MIN		U(0x0)
#define PROG_OTPMK_RANDOM	U(0x1)
#define PROG_OTPMK_USER		U(0x2)
#define PROG_OTPMK_RANDOM_MIN	U(0x5)
#define PROG_OTPMK_USER_MIN	U(0x6)
#define PROG_NO_OTPMK		U(0x8)

#define OTPMK_MIM_BITS_MASK	U(0xF0000000)

/* System configuration bit shifts */
#define SCB_WP_SHIFT		U(0)
#define SCB_ITS_SHIFT		U(2)
#define SCB_NSEC_SHIFT		U(4)
#define SCB_ZD_SHIFT		U(5)
#define SCB_K0_SHIFT		U(15)
#define SCB_K1_SHIFT		U(14)
#define SCB_K2_SHIFT		U(13)
#define SCB_K3_SHIFT		U(12)
#define SCB_K4_SHIFT		U(11)
#define SCB_K5_SHIFT		U(10)
#define SCB_K6_SHIFT		U(9)
#define SCB_FR0_SHIFT		U(30)
#define SCB_FR1_SHIFT		U(31)

/* Fuse Header Structure */
struct fuse_hdr_t {
	uint8_t barker[4];          /* 0x00 Barker code */
	uint32_t flags;             /* 0x04 Script flags */
	uint32_t povdd_gpio;        /* 0x08 GPIO for POVDD */
	uint32_t otpmk[8];          /* 0x0C-0x2B OTPMK */
	uint32_t srkh[8];           /* 0x2C-0x4B SRKH */
	uint32_t oem_uid[5];        /* 0x4C-0x5F OEM unique id's */
	uint32_t dcv[2];            /* 0x60-0x67 Debug Challenge */
	uint32_t drv[2];            /* 0x68-0x6F Debug Response */
	uint32_t ospr1;             /* 0x70 OSPR1 */
	uint32_t sc;                /* 0x74 OSPR0 (System Configuration) */
	uint32_t reserved[2];       /* 0x78-0x7F Reserved */
};

/* Function to do fuse provisioning */
int provision_fuses(unsigned long long fuse_scr_addr,
		    bool en_povdd_status);

#define EFUSE_POWERUP_DELAY_mSec	U(25)
#endif	/* FUSE_PROV_H  */
