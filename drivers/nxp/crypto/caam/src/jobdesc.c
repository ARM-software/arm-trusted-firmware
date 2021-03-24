/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "caam.h"
#include <common/debug.h>
#include "jobdesc.h"
#include "rsa.h"
#include "sec_hw_specific.h"


/* Return Length of desctiptr from first word */
uint32_t desc_length(uint32_t *desc)
{
	return desc[0] & DESC_LEN_MASK;
}

/*Update start index in first word of descriptor */
void desc_update_start_index(uint32_t *desc, uint32_t index)
{
	desc[0] |= (index << DESC_START_SHIFT);
}

/* Initialize the descriptor */
void desc_init(uint32_t *desc)
{
	*desc = 0;
}

/* Add word in the descriptor and increment the length */
void desc_add_word(uint32_t *desc, uint32_t word)
{
	uint32_t len = desc_length(desc);

	/* Add Word at Last */
	uint32_t *last = desc + len;
	*last = word;

	/* Increase the length */
	desc[0] += 1;
}

/* Add Pointer to the descriptor */
void desc_add_ptr(uint32_t *desc, phys_addr_t *ptr)
{
	uint32_t len = desc_length(desc);

	/* Add Word at Last */
	phys_addr_t *last = (phys_addr_t *) (desc + len);

#ifdef CONFIG_PHYS_64BIT
	ptr_addr_t *ptr_addr = (ptr_addr_t *) last;

	ptr_addr->m_halves.high = PHYS_ADDR_HI(ptr);
	ptr_addr->m_halves.low = PHYS_ADDR_LO(ptr);
#else
	*last = ptr;
#endif

	/* Increase the length */
	desc[0] += (uint32_t) (sizeof(phys_addr_t) / sizeof(uint32_t));
}

/* Descriptor to generate Random words */
int cnstr_rng_jobdesc(uint32_t *desc, uint32_t state_handle,
		      uint32_t *add_inp, uint32_t add_ip_len,
		      uint8_t *out_data, uint32_t len)
{
	phys_addr_t *phys_addr_out = vtop(out_data);

	/* Current descriptor support only 64K length */
	if (len > U(0xffff))
		return -1;
	/* Additional Input not supported by current descriptor */
	if (add_ip_len > 0U)
		return -1;

	VERBOSE("Constructing descriptor\n");
	desc_init(desc);
	/* Class1 Alg Operation,RNG Optype, Generate */
	desc_add_word(desc, U(0xb0800000));
	desc_add_word(desc, U(0x82500000) | (state_handle << ALG_AAI_SH_SHIFT));
	desc_add_word(desc, U(0x60340000) | len);
	desc_add_ptr(desc, phys_addr_out);

	return 0;

}

/* Construct descriptor to instantiate RNG */
int cnstr_rng_instantiate_jobdesc(uint32_t *desc)
{
	desc_init(desc);
	desc_add_word(desc, U(0xb0800000));
	/* Class1 Alg Operation,RNG Optype, Instantiate */
	desc_add_word(desc, U(0x82500004));
	/* Wait for done */
	desc_add_word(desc, U(0xa2000001));
	/*Load to clear written */
	desc_add_word(desc, U(0x10880004));
	/*Pri Mode Reg clear */
	desc_add_word(desc, U(0x00000001));
	/* Generate secure keys */
	desc_add_word(desc, U(0x82501000));

	return 0;
}

/* Construct descriptor to generate hw key blob */
int cnstr_hw_encap_blob_jobdesc(uint32_t *desc,
				uint8_t *key_idnfr, uint32_t key_sz,
				uint32_t key_class, uint8_t *plain_txt,
				uint32_t in_sz, uint8_t *enc_blob,
				uint32_t out_sz, uint32_t operation)
{
	phys_addr_t *phys_key_idnfr, *phys_addr_in, *phys_addr_out;
	int i = 0;

	phys_key_idnfr = vtop((void *)key_idnfr);
	phys_addr_in = vtop((void *)plain_txt);
	phys_addr_out = vtop((void *)enc_blob);

	desc_init(desc);

	desc_add_word(desc, U(0xb0800000));

	/* Key Identifier */
	desc_add_word(desc, (key_class | key_sz));
	desc_add_ptr(desc, phys_key_idnfr);

	/* Source Address */
	desc_add_word(desc, U(0xf0400000));
	desc_add_ptr(desc, phys_addr_in);

	/* In Size = 0x10 */
	desc_add_word(desc, in_sz);

	/* Out Address */
	desc_add_word(desc, U(0xf8400000));
	desc_add_ptr(desc, phys_addr_out);

	/* Out Size = 0x10 */
	desc_add_word(desc, out_sz);

	/* Operation */
	desc_add_word(desc, operation);

	for (i = 0; i < 15; i++)
		VERBOSE("desc word %x\n", desc[i]);

	return 0;
}

/***************************************************************************
 * Function	: inline_cnstr_jobdesc_pkha_rsaexp
 * Arguments	: desc - Pointer to Descriptor
 *		  pkin - Pointer to Input Params
 *		  out - Pointer to Output
 *		  out_siz - Output Size
 * Return	: Void
 * Description	: Creates the descriptor for PKHA RSA
 ***************************************************************************/
void cnstr_jobdesc_pkha_rsaexp(uint32_t *desc,
			       struct pk_in_params *pkin, uint8_t *out,
			       uint32_t out_siz)
{
	phys_addr_t *ptr_addr_e, *ptr_addr_a, *ptr_addr_n, *ptr_addr_out;

	ptr_addr_e = vtop((void *)(pkin->e));
	ptr_addr_a = vtop((void *)(pkin->a));
	ptr_addr_n = vtop((void *)(pkin->n));
	ptr_addr_out = vtop((void *)(out));

	desc_init(desc);
	desc_add_word(desc, U(0xb0800000));
	desc_add_word(desc, U(0x02010000) | pkin->e_siz);
	desc_add_ptr(desc, ptr_addr_e);
	desc_add_word(desc, U(0x220c0000) | pkin->a_siz);
	desc_add_ptr(desc, ptr_addr_a);
	desc_add_word(desc, U(0x22080000) | pkin->n_siz);
	desc_add_ptr(desc, ptr_addr_n);
	desc_add_word(desc, U(0x81800006));
	desc_add_word(desc, U(0x620d0000) | out_siz);
	desc_add_ptr(desc, ptr_addr_out);
}

/***************************************************************************
 * Function	: inline_cnstr_jobdesc_sha256
 * Arguments	: desc - Pointer to Descriptor
 *		  msg - Pointer to SG Table
 *		  msgsz - Size of SG Table
 *		  digest - Pointer to Output Digest
 * Return	: Void
 * Description	: Creates the descriptor for SHA256 HASH calculation
 ***************************************************************************/
void cnstr_hash_jobdesc(uint32_t *desc, uint8_t *msg, uint32_t msgsz,
			uint8_t *digest)
{
	/* SHA 256 , output is of length 32 words */
	phys_addr_t *ptr_addr_in, *ptr_addr_out;

	ptr_addr_in = (void *)vtop(msg);
	ptr_addr_out = (void *)vtop(digest);

	desc_init(desc);
	desc_add_word(desc, U(0xb0800000));

	/* Operation Command
	 * OP_TYPE_CLASS2_ALG | OP_ALG_ALGSEL_SHA256 | OP_ALG_AAI_HASH |
	 * OP_ALG_AS_INITFINAL | OP_ALG_ENCRYPT | OP_ALG_ICV_OFF)
	 */
	desc_add_word(desc, U(0x8443000d));

	if (msgsz > U(0xffff)) {
		desc_add_word(desc, U(0x25540000));	/* FIFO Load */
		desc_add_ptr(desc, ptr_addr_in);	/* Pointer to msg */
		desc_add_word(desc, msgsz);	/* Size */
		desc_add_word(desc, U(0x54200020));	/* FIFO Store */
		desc_add_ptr(desc, ptr_addr_out);	/* Pointer to Result */
	} else {
		desc_add_word(desc, U(0x25140000) | msgsz);
		desc_add_ptr(desc, ptr_addr_in);
		desc_add_word(desc, U(0x54200020));
		desc_add_ptr(desc, ptr_addr_out);
	}

}
