/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __HASH_H__
#define __HASH_H__

#include <stdbool.h>

/* List of hash algorithms */
enum hash_algo {
	SHA1 = 0,
	SHA256
};

/* number of bytes in the SHA256-256 digest */
#define SHA256_DIGEST_SIZE 32

/*
 * number of words in the digest - Digest is kept internally
 * as 8 32-bit words
 */
#define _SHA256_DIGEST_LENGTH 8

/*
 * block length - A block, treated as a sequence of
 * 32-bit words
 */
#define SHA256_BLOCK_LENGTH 16

/* number of bytes in the block */
#define SHA256_DATA_SIZE 64

#define MAX_SG		12

struct sg_entry {
#if defined(NXP_SEC_LE)
	uint32_t addr_lo;	/* Memory Address - lo */
	uint32_t addr_hi;	/* Memory Address of start of buffer - hi */
#else
	uint32_t addr_hi;	/* Memory Address of start of buffer - hi */
	uint32_t addr_lo;	/* Memory Address - lo */
#endif

	uint32_t len_flag;	/* Length of the data in the frame */
#define SG_ENTRY_LENGTH_MASK	0x3FFFFFFF
#define SG_ENTRY_EXTENSION_BIT	0x80000000
#define SG_ENTRY_FINAL_BIT	0x40000000
	uint32_t bpid_offset;
#define SG_ENTRY_BPID_MASK	0x00FF0000
#define SG_ENTRY_BPID_SHIFT	16
#define SG_ENTRY_OFFSET_MASK	0x00001FFF
#define SG_ENTRY_OFFSET_SHIFT	0
};

/*
 * SHA256-256 context
 * contain the following fields
 * State
 * count low
 * count high
 * block data buffer
 * index to the buffer
 */
struct hash_ctx {
	struct sg_entry sg_tbl[MAX_SG];
	uint32_t hash_desc[64];
	uint8_t hash[SHA256_DIGEST_SIZE];
	uint32_t sg_num;
	uint32_t len;
	uint8_t *data;
	enum hash_algo algo;
	bool active;
};

int hash_init(enum hash_algo algo, void **ctx);
int hash_update(enum hash_algo algo, void *context, void *data_ptr,
		unsigned int data_len);
int hash_final(enum hash_algo algo, void *context, void *hash_ptr,
	       unsigned int hash_len);

#endif
