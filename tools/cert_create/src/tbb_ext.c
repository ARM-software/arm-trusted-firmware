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

ext_t tbb_ext[] = {
	{
		.oid = TZ_FW_NVCOUNTER_OID,
		.sn = "TrustedNvCounter",
		.ln = "Non-volatile trusted counter",
		.type = V_ASN1_INTEGER
	},
	{
		.oid = NTZ_FW_NVCOUNTER_OID,
		.sn = "NonTrustedNvCounter",
		.ln = "Non-volatile non-trusted counter",
		.type = V_ASN1_INTEGER
	},
	{
		.oid = BL2_HASH_OID,
		.sn = "TrustedBootFirmwareHash",
		.ln = "Trusted Boot Firmware (BL2) hash (SHA256)",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = TZ_WORLD_PK_OID,
		.sn = "TrustedWorldPublicKey",
		.ln = "Trusted World Public Key",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = NTZ_WORLD_PK_OID,
		.sn = "NonTrustedWorldPublicKey",
		.ln = "Non-Trusted World Public Key",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL31_CONTENT_CERT_PK_OID,
		.sn = "SoCFirmwareContentCertPK",
		.ln = "SoC Firmware content certificate public key",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL31_HASH_OID,
		.sn = "APROMPatchHash",
		.ln = "AP ROM patch hash",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL30_CONTENT_CERT_PK_OID,
		.sn = "SCPFirmwareContentCertPK",
		.ln = "SCP Firmware content certificate public key",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL30_HASH_OID,
		.sn = "SCPFirmwareHash",
		.ln = "SCP Firmware (BL30) hash (SHA256)",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL32_CONTENT_CERT_PK_OID,
		.sn = "TrustedOSFirmwareContentCertPK",
		.ln = "Trusted OS Firmware content certificate public key",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL32_HASH_OID,
		.sn = "TrustedOSHash",
		.ln = "Trusted OS (BL32) hash (SHA256)",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL33_CONTENT_CERT_PK_OID,
		.sn = "NonTrustedFirmwareContentCertPK",
		.ln = "Non-Trusted Firmware content certificate public key",
		.type = V_ASN1_OCTET_STRING
	},
	{
		.oid = BL33_HASH_OID,
		.sn = "NonTrustedWorldBootloaderHash",
		.ln = "Non-Trusted World (BL33) hash (SHA256)",
		.type = V_ASN1_OCTET_STRING
	},
	{ 0, 0, 0, 0 }
};
