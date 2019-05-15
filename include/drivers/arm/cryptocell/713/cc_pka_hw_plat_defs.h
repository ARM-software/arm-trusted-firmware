/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CC_PKA_HW_PLAT_DEFS_H
#define _CC_PKA_HW_PLAT_DEFS_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "cc_pal_types.h"
/*!
@file
@brief Contains the enums and definitions that are used in the PKA code (definitions that are platform dependent).
*/

/*! The size of the PKA engine word. */
#define CC_PKA_WORD_SIZE_IN_BITS		     128

/*! The maximal supported size of modulus in RSA in bits. */
#define CC_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS      4096
/*! The maximal supported size of key-generation in RSA in bits. */
#define CC_RSA_MAX_KEY_GENERATION_HW_SIZE_BITS       4096

/*! Secure boot/debug certificate RSA public modulus key size in bits. */
#if (KEY_SIZE == 3072)
    #define BSV_CERT_RSA_KEY_SIZE_IN_BITS 3072
#else
    #define BSV_CERT_RSA_KEY_SIZE_IN_BITS 2048
#endif
/*! Secure boot/debug certificate RSA public modulus key size in bytes. */
#define BSV_CERT_RSA_KEY_SIZE_IN_BYTES    (BSV_CERT_RSA_KEY_SIZE_IN_BITS/CC_BITS_IN_BYTE)
/*! Secure boot/debug certificate RSA public modulus key size in words. */
#define BSV_CERT_RSA_KEY_SIZE_IN_WORDS    (BSV_CERT_RSA_KEY_SIZE_IN_BITS/CC_BITS_IN_32BIT_WORD)

/*! The maximal count of extra bits in PKA operations. */
#define PKA_EXTRA_BITS  8
/*! The number of memory registers in PKA operations. */
#define PKA_MAX_COUNT_OF_PHYS_MEM_REGS  32

/*! Size of buffer for Barrett modulus tag in words. */
#define RSA_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS  5
/*! Size of buffer for Barrett modulus tag in bytes. */
#define RSA_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_BYTES  (RSA_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS*CC_32BIT_WORD_SIZE)



#ifdef __cplusplus
}
#endif

#endif //_CC_PKA_HW_PLAT_DEFS_H

/**
 @}
 */

