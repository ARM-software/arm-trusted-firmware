/*
 * Copyright 2017-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CSF_HDR_H
#define CSF_HDR_H

#include "caam.h"
#include "hash.h"
#include "rsa.h"

/* Barker code size in bytes */
#define CSF_BARKER_LEN	4	/* barker code length in ESBC uboot client */
				/* header */

#ifdef CSF_HDR_CH3
struct csf_hdr {
	uint8_t barker[CSF_BARKER_LEN];	/* 0x00 Barker code */
	uint32_t srk_tbl_off;		/* 0x04 SRK Table Offset */

	struct {
		uint8_t num_srk;	/* 0x08 No. of keys */
		uint8_t srk_sel;	/*  Key no. to be used */
		uint8_t reserve;	/* 0x0a rseerved */
	} len_kr;
	uint8_t ie_flag;

	uint32_t uid_flag;

	uint32_t psign;			/* 0x10 signature offset */
	uint32_t sign_len;			/* 0x14 length of signature */

	union {
		struct {
			uint32_t sg_table_offset; /* 0x18 SG Table Offset */
			uint32_t sg_entries;	  /* 0x1c no of entries in SG */
		} sg_isbc;
		uint64_t img_addr;	/* 64 bit pointer to ESBC Image */
	};

	union {
		struct {
			uint32_t img_size;   /* ESBC client img size in bytes */
			uint32_t ie_key_sel;
		} img;
		uint64_t entry_point;	  /* 0x20-0x24 ESBC entry point */
	};

	uint32_t fsl_uid_0;			/* 0x28 Freescale unique id 0 */
	uint32_t fsl_uid_1;			/* 0x2c Freescale unique id 1 */
	uint32_t oem_uid_0;			/* 0x30 OEM unique id 0 */
	uint32_t oem_uid_1;			/* 0x34 OEM unique id 1 */
	uint32_t oem_uid_2;			/* 0x38 OEM unique id 2 */
	uint32_t oem_uid_3;			/* 0x3c OEM unique id 3 */
	uint32_t oem_uid_4;			/* 0x40 OEM unique id 4 */

	uint32_t reserved[3];		/* 0x44 - 0x4f */
};

/* Srk table and key revocation check */
#define UNREVOCABLE_KEY	8
#define REVOC_KEY_ALIGN 7
#define MAX_KEY_ENTRIES 8

#else

/* CSF header for Chassis 2 */
struct csf_hdr {
	uint8_t barker[CSF_BARKER_LEN];	/* barker code */
	union {
		uint32_t pkey;		/* public key offset */
		uint32_t srk_tbl_off;
	};

	union {
		uint32_t key_len;		/* pub key length in bytes */
		struct {
			uint32_t srk_table_flag:8;
			uint32_t srk_sel:8;
			uint32_t num_srk:16;
		} len_kr;
	};

	uint32_t psign;		/* signature offset */
	uint32_t sign_len;		/* length of the signature in bytes */

	/* SG Table used by ISBC header */
	union {
		struct {
			uint32_t sg_table_offset; /* 0x14 SG Table Offset */
			uint32_t sg_entries;	/* no of entries in SG table */
		} sg_isbc;
		struct {
			uint32_t reserved1;	/* Reserved field */
			uint32_t img_size;	/* ESBC img size in bytes */
		} img;
	};

	uint32_t entry_point;		/* ESBC client entry point */
	uint32_t reserved2;		/* Scatter gather flag */
	uint32_t uid_flag;
	uint32_t fsl_uid_0;
	uint32_t oem_uid_0;
	uint32_t reserved3[2];
	uint32_t fsl_uid_1;
	uint32_t oem_uid_1;

	/* The entries below aren't present in ISBC header */
	uint64_t img_addr;	/* 64 bit pointer to ESBC Image */
	uint32_t ie_flag;
	uint32_t ie_key_sel;
};

/* Srk table and key revocation check */
#define UNREVOCABLE_KEY	4
#define REVOC_KEY_ALIGN 3
#define MAX_KEY_ENTRIES 4

#endif

struct srk_table {
	uint32_t key_len;
	uint8_t pkey[2 * RSA_4K_KEY_SZ_BYTES];
};

/*
 * This struct contains the following fields
 * length of the segment
 * Destination Target ID
 * source address
 * destination address
 */
struct sg_table {
	uint32_t len;			/* Length of Image */
	uint32_t res1;
	union {
		uint64_t src_addr;	/* SRC Address of Image */
		struct {
			uint32_t src_addr;
			uint32_t dst_addr;
		} img;
	};
};

int validate_esbc_header(void *img_hdr, void **img_key, uint32_t *key_len,
			 void **img_sign, uint32_t *sign_len,
			 enum sig_alg *algo);

int calc_img_hash(struct csf_hdr *hdr, void *img_addr, uint32_t img_size,
		  uint8_t *img_hash, uint32_t *hash_len);

#endif
