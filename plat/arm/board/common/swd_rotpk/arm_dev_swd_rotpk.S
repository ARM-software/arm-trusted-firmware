/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.global arm_swd_rotpk_hash
	.global arm_swd_rotpk_hash_end

	.section .rodata.arm_swd_rotpk_hash, "a"

/*
* The swd_roptpk header is dependent only on the algorithm used to
* generate the hash.
* ASN1_HASH_ALG is the last byte used to encode the OID for
* the hash algorithm into the header,
* this byte distinguishes between SHA256, SHA384 and SHA512.
*/
.equ HASH_ASN1_LEN, ARM_ROTPK_HASH_LEN
#if ARM_ROTPK_HASH_LEN == 48
	.equ ASN1_HASH_ALG, 0x02
	.equ TOTAL_ASN1_LEN, 0x41
#elif ARM_ROTPK_HASH_LEN == 64
	.equ ASN1_HASH_ALG, 0x03
	.equ TOTAL_ASN1_LEN, 0x51
#elif ARM_ROTPK_HASH_LEN == 32
	.equ ASN1_HASH_ALG, 0x01
	.equ TOTAL_ASN1_LEN, 0x31
#else
	.error "Invalid SWD_ROTPK hash length."
#endif

arm_swd_rotpk_hash:
	/* DER header. */
	.byte 0x30, TOTAL_ASN1_LEN, 0x30, 0x0D, 0x06, 0x09, 0x60, 0x86, 0x48
	.byte 0x01, 0x65, 0x03, 0x04, 0x02, ASN1_HASH_ALG, 0x05, 0x00, 0x04, HASH_ASN1_LEN
	/* Key hash. */
	.incbin ARM_SWD_ROTPK
arm_swd_rotpk_hash_end:
