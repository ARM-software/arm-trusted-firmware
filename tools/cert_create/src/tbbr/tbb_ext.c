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

#include <stdio.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include "ext.h"
#include "platform_oid.h"
#include "tbbr/tbb_ext.h"
#include "tbbr/tbb_key.h"

/* TODO: get these values from the command line */
#define TRUSTED_WORLD_NVCTR_VALUE	0
#define NORMAL_WORLD_NVCTR_VALUE	0

static ext_t tbb_ext[] = {
	[TZ_FW_NVCOUNTER_EXT] = {
		.oid = TZ_FW_NVCOUNTER_OID,
		.sn = "TrustedWorldNVCounter",
		.ln = "Trusted World Non-Volatile counter",
		.asn1_type = V_ASN1_INTEGER,
		.type = EXT_TYPE_NVCOUNTER,
		.data.nvcounter = TRUSTED_WORLD_NVCTR_VALUE
	},
	[NTZ_FW_NVCOUNTER_EXT] = {
		.oid = NTZ_FW_NVCOUNTER_OID,
		.sn = "NormalWorldNVCounter",
		.ln = "Normal World Non-Volatile counter",
		.asn1_type = V_ASN1_INTEGER,
		.type = EXT_TYPE_NVCOUNTER,
		.data.nvcounter = NORMAL_WORLD_NVCTR_VALUE
	},
	[BL2_HASH_EXT] = {
		.oid = BL2_HASH_OID,
		.sn = "TrustedBootFirmwareHash",
		.ln = "Trusted Boot Firmware (BL2) hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[TZ_WORLD_PK_EXT] = {
		.oid = TZ_WORLD_PK_OID,
		.sn = "TrustedWorldPublicKey",
		.ln = "Trusted World Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.data.key = TRUSTED_WORLD_KEY
	},
	[NTZ_WORLD_PK_EXT] = {
		.oid = NTZ_WORLD_PK_OID,
		.sn = "NonTrustedWorldPublicKey",
		.ln = "Non-Trusted World Public Key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.data.key = NON_TRUSTED_WORLD_KEY
	},
	[BL30_CONTENT_CERT_PK_EXT] = {
		.oid = BL30_CONTENT_CERT_PK_OID,
		.sn = "SCPFirmwareContentCertPK",
		.ln = "SCP Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.data.key = BL30_KEY
	},
	[BL30_HASH_EXT] = {
		.oid = BL30_HASH_OID,
		.sn = "SCPFirmwareHash",
		.ln = "SCP Firmware (BL30) hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[BL31_CONTENT_CERT_PK_EXT] = {
		.oid = BL31_CONTENT_CERT_PK_OID,
		.sn = "SoCFirmwareContentCertPK",
		.ln = "SoC Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.data.key = BL31_KEY
	},
	[BL31_HASH_EXT] = {
		.oid = BL31_HASH_OID,
		.sn = "SoCAPFirmwareHash",
		.ln = "SoC AP Firmware (BL31) hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[BL32_CONTENT_CERT_PK_EXT] = {
		.oid = BL32_CONTENT_CERT_PK_OID,
		.sn = "TrustedOSFirmwareContentCertPK",
		.ln = "Trusted OS Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.data.key = BL32_KEY
	},
	[BL32_HASH_EXT] = {
		.oid = BL32_HASH_OID,
		.sn = "TrustedOSHash",
		.ln = "Trusted OS (BL32) hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[BL33_CONTENT_CERT_PK_EXT] = {
		.oid = BL33_CONTENT_CERT_PK_OID,
		.sn = "NonTrustedFirmwareContentCertPK",
		.ln = "Non-Trusted Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.data.key = BL33_KEY
	},
	[BL33_HASH_EXT] = {
		.oid = BL33_HASH_OID,
		.sn = "NonTrustedWorldBootloaderHash",
		.ln = "Non-Trusted World (BL33) hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	}
};

REGISTER_EXTENSIONS(tbb_ext);
