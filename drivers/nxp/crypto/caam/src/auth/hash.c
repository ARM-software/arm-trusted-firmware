/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch_helpers.h>
#include "caam.h"
#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>

#include "hash.h"
#include "jobdesc.h"
#include "sec_hw_specific.h"

/* Since no Allocator is available . Taking a global static ctx.
 * This would mean that only one active ctx can be there at a time.
 */

static struct hash_ctx glbl_ctx;

static void hash_done(uint32_t *desc, uint32_t status, void *arg,
		      void *job_ring)
{
	INFO("Hash Desc SUCCESS with status %x\n", status);
}

/***************************************************************************
 * Function	: hash_init
 * Arguments	: ctx - SHA context
 * Return	: init,
 * Description	: This function initializes the context for SHA calculation
 ***************************************************************************/
int hash_init(enum hash_algo algo, void **ctx)
{
	if (glbl_ctx.active == false) {
		memset(&glbl_ctx, 0, sizeof(struct hash_ctx));
		glbl_ctx.active = true;
		glbl_ctx.algo = algo;
		*ctx = &glbl_ctx;
		return 0;
	} else {
		return -1;
	}
}

/***************************************************************************
 * Function	: hash_update
 * Arguments	: ctx - SHA context
 *		  buffer - Data
 *		  length - Length
 * Return	: -1 on error
 *		  0 on SUCCESS
 * Description	: This function creates SG entry of the data provided
 ***************************************************************************/
int hash_update(enum hash_algo algo, void *context, void *data_ptr,
		unsigned int data_len)
{
	struct hash_ctx *ctx = context;
	/* MAX_SG would be MAX_SG_ENTRIES + key + hdr + sg table */
	if (ctx->sg_num >= MAX_SG) {
		ERROR("Reached limit for calling %s\n", __func__);
		ctx->active = false;
		return -EINVAL;

	}

	if (ctx->algo != algo) {
		ERROR("ctx for algo not correct\n");
		ctx->active = false;
		return -EINVAL;
	}

#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
	flush_dcache_range((uintptr_t)data_ptr, data_len);
	dmbsy();
#endif

#ifdef CONFIG_PHYS_64BIT
	sec_out32(&ctx->sg_tbl[ctx->sg_num].addr_hi,
		  (uint32_t) ((uintptr_t) data_ptr >> 32));
#else
	sec_out32(&ctx->sg_tbl[ctx->sg_num].addr_hi, 0x0);
#endif
	sec_out32(&ctx->sg_tbl[ctx->sg_num].addr_lo, (uintptr_t) data_ptr);

	sec_out32(&ctx->sg_tbl[ctx->sg_num].len_flag,
		  (data_len & SG_ENTRY_LENGTH_MASK));

	ctx->sg_num++;

	ctx->len += data_len;

	return 0;
}

/***************************************************************************
 * Function	: hash_final
 * Arguments	: ctx - SHA context
 * Return	: SUCCESS or FAILURE
 * Description	: This function sets the final bit and enqueues the decriptor
 ***************************************************************************/
int hash_final(enum hash_algo algo, void *context, void *hash_ptr,
	       unsigned int hash_len)
{
	int ret = 0;
	struct hash_ctx *ctx = context;
	uint32_t final = 0U;

	struct job_descriptor jobdesc __aligned(CACHE_WRITEBACK_GRANULE);

	jobdesc.arg = NULL;
	jobdesc.callback = hash_done;

	if (ctx->algo != algo) {
		ERROR("ctx for algo not correct\n");
		ctx->active = false;
		return -EINVAL;
	}

	final = sec_in32(&ctx->sg_tbl[ctx->sg_num - 1].len_flag) |
	    SG_ENTRY_FINAL_BIT;
	sec_out32(&ctx->sg_tbl[ctx->sg_num - 1].len_flag, final);

	dsb();

	/* create the hw_rng descriptor */
	cnstr_hash_jobdesc(jobdesc.desc, (uint8_t *) ctx->sg_tbl,
			   ctx->len, hash_ptr);

#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
	flush_dcache_range((uintptr_t)ctx->sg_tbl,
			   (sizeof(struct sg_entry) * MAX_SG));
	inv_dcache_range((uintptr_t)hash_ptr, hash_len);

	dmbsy();
#endif

	/* Finally, generate the requested random data bytes */
	ret = run_descriptor_jr(&jobdesc);
	if (ret != 0) {
		ERROR("Error in running descriptor\n");
		ret = -1;
	}
	ctx->active = false;
	return ret;
}
