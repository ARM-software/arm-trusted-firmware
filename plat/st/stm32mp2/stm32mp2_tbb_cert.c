/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tbbr/tbb_ext.h>
#include <tbbr/tbb_key.h>

#include <platform_oid.h>
#include <tbbr/stm32mp2_tbb_cert.h>

/*
 * Certificates used in the chain of trust
 *
 * The order of the certificates must follow the enumeration specified in
 * stm32mp2_tbb_cert.h. All certificates are self-signed, so the issuer certificate
 * field points to itself.
 */
static cert_t stm32mp2_tbb_certs[] = {
	[0] = {
		.id = STM32MP_CONFIG_CERT,
		.opt = "stm32mp-cfg-cert",
		.help_msg = "STM32MP Config Certificate (output file)",
		.fn = NULL,
		.cn = "STM32MP config FW Certificate",
		.key = ROT_KEY,
		.issuer = STM32MP_CONFIG_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			HW_CONFIG_HASH_EXT,
			FW_CONFIG_HASH_EXT,
			DDR_FW_HASH_EXT
		},
		.num_ext = 4
	},
};

PLAT_REGISTER_COT(stm32mp2_tbb_certs);

static ext_t stm32mp2_tbb_ext[] = {
	[0] = {
		.oid = DDR_FW_HASH_OID,
		.opt = "ddr-fw",
		.help_msg = "DDR Firmware image file",
		.sn = "DDR Firmware Hash",
		.ln = "DDR Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH,
		.optional = 1
	}
};

PLAT_REGISTER_EXTENSIONS(stm32mp2_tbb_ext);
