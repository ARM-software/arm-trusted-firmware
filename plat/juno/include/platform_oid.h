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

#ifndef PLATFORM_OID_H_
#define PLATFORM_OID_H_

/*
 * This is the list of the different extensions containing relevant information
 * to establish the chain of trust.
 *
 * The OIDs shown here are just an example. Real OIDs should be obtained from
 * the ITU-T.
 */

/* Non-volatile counter extensions */
#define TZ_FW_NVCOUNTER_OID		"1.2.3.1"
#define NTZ_FW_NVCOUNTER_OID		"1.2.3.2"

/* BL2 extensions */
#define BL2_HASH_OID			"1.2.3.3"

/* Trusted Key extensions */
#define TZ_WORLD_PK_OID			"1.2.3.4"
#define NTZ_WORLD_PK_OID		"1.2.3.5"

/* BL3-1 extensions */
#define BL31_CONTENT_CERT_PK_OID	"1.2.3.6"
#define BL31_HASH_OID			"1.2.3.7"

/* BL3-0 extensions */
#define BL30_CONTENT_CERT_PK_OID	"1.2.3.8"
#define BL30_HASH_OID			"1.2.3.9"

/* BL3-2 extensions */
#define BL32_CONTENT_CERT_PK_OID	"1.2.3.10"
#define BL32_HASH_OID			"1.2.3.11"

/* BL3-3 extensions */
#define BL33_CONTENT_CERT_PK_OID	"1.2.3.12"
#define BL33_HASH_OID			"1.2.3.13"

#endif /* PLATFORM_OID_H_ */
