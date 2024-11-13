/*
 * Copyright (c) 2021-2024, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/board/common/rotpk/rotpk_def.h>

#if ARM_ROTPK_IS_HASH == 1
	.global arm_rotpk_hash_der_header
	.global arm_rotpk_hash_end
	.section .rodata.arm_rotpk_hash, "a"

	#ifndef ARM_ROTPK_HASH_LEN
	.error "ROTPK hash not defined."
	#endif

	/*
	* The rotpk header is dependent only on the algorithm used to
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
		.error "Invalid ROTPK hash length."
	#endif

arm_rotpk_hash_der_header:
	.byte 0x30, TOTAL_ASN1_LEN, 0x30, 0x0D, 0x06, 0x09, 0x60, 0x86, 0x48
	.byte 0x01, 0x65, 0x03, 0x04, 0x02, ASN1_HASH_ALG, 0x05, 0x00, 0x04, HASH_ASN1_LEN
arm_rotpk_hash_der_header_len:

#ifdef ARM_ROTPK
	.incbin ARM_ROTPK
arm_rotpk_hash_end:
#endif

	.if ARM_ROTPK_HASH_DER_HEADER_LEN != arm_rotpk_hash_der_header_len - arm_rotpk_hash_der_header
		.error "Invalid ROTPK header length."
	.endif

#else
	/*
	* The rotpk is an unhashed public key in DER format.
	* We do not need a header in this case.
	*/
	.global arm_rotpk_key
	.section .rodata.arm_rotpk_key, "a"

arm_rotpk_key:
	#ifdef ARM_ROTPK
		.global arm_rotpk_key_end
		.incbin ARM_ROTPK
arm_rotpk_key_end:
	#else
		.error "ROTPK not found."
	#endif
#endif