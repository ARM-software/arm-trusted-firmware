/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CAAM_H
#define CAAM_H

#include "sec_jr_driver.h"

enum sig_alg {
	RSA,
	ECC
};

/* This function does basic SEC Initialization */
int sec_init(void);

/* This function is used to submit jobs to JR */
int run_descriptor_jr(struct job_descriptor *desc);

/* This function is used to instatiate the HW RNG is already not instantiated */
int hw_rng_instantiate(void);

/* This function is used to return random bytes of byte_len from HW RNG */
int get_rand_bytes_hw(uint8_t *bytes, int byte_len);

/* This function is used to set the hw unique key from HW CAAM */
int get_hw_unq_key_blob_hw(uint8_t *hw_key, int size);

/* This function is used to fetch random number from
 * CAAM of length either of 4 bytes or 8 bytes depending
 * rngWidth value.
 */
unsigned long long get_random(int rngWidth);

#endif /* CAAM_H */
