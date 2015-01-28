/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __POLARSSL_CONFIG_H__
#define __POLARSSL_CONFIG_H__


/*
 * Configuration file to build PolarSSL with the required features for
 * Trusted Boot
 */

#define POLARSSL_PLATFORM_MEMORY
#define POLARSSL_PLATFORM_NO_STD_FUNCTIONS

#define POLARSSL_PKCS1_V15
#define POLARSSL_PKCS1_V21

#define POLARSSL_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
#define POLARSSL_X509_CHECK_KEY_USAGE
#define POLARSSL_X509_CHECK_EXTENDED_KEY_USAGE

#define POLARSSL_ASN1_PARSE_C
#define POLARSSL_ASN1_WRITE_C

#define POLARSSL_BASE64_C
#define POLARSSL_BIGNUM_C

#define POLARSSL_ERROR_C
#define POLARSSL_MD_C

#define POLARSSL_MEMORY_BUFFER_ALLOC_C
#define POLARSSL_OID_C

#define POLARSSL_PK_C
#define POLARSSL_PK_PARSE_C
#define POLARSSL_PK_WRITE_C

#define POLARSSL_PLATFORM_C

#define POLARSSL_RSA_C
#define POLARSSL_SHA1_C
#define POLARSSL_SHA256_C

#define POLARSSL_VERSION_C

#define POLARSSL_X509_USE_C
#define POLARSSL_X509_CRT_PARSE_C

/* MPI / BIGNUM options */
#define POLARSSL_MPI_WINDOW_SIZE              2
#define POLARSSL_MPI_MAX_SIZE               256

/* Memory buffer allocator options */
#define POLARSSL_MEMORY_ALIGN_MULTIPLE        8

#include "polarssl/check_config.h"

#endif /* __POLARSSL_CONFIG_H__ */
