/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#if USE_TBBR_DEFS
#include <tbbr_oid.h>
#else
#include <platform_oid.h>
#endif

#include "ext.h"
#include "tbbr/tbb_ext.h"
#include "tbbr/tbb_key.h"

#include <pdef_tbb_ext.h>
#include <pdef_tbb_key.h>

static ext_t pdef_tbb_ext[] = {
	[DDR_FW_CONTENT_CERT_PK_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_FW_CONTENT_CERT_PK_OID,
		.sn = "DDR FirmwareContentCertPK",
		.ln = "DDR Firmware content certificate public key",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_PKEY,
		.attr.key = DDR_FW_CONTENT_KEY
	},
	[DDR_IMEM_UDIMM_1D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_IMEM_UDIMM_1D_HASH_OID,
		.opt = "ddr-immem-udimm-1d",
		.help_msg = "DDR Firmware IMEM UDIMM 1D image file",
		.sn = "DDR UDIMM IMEM 1D FirmwareHash",
		.ln = "DDR UDIMM IMEM 1D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[DDR_IMEM_UDIMM_2D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_IMEM_UDIMM_2D_HASH_OID,
		.opt = "ddr-immem-udimm-2d",
		.help_msg = "DDR Firmware IMEM UDIMM 2D image file",
		.sn = "DDR UDIMM IMEM 2D FirmwareHash",
		.ln = "DDR UDIMM IMEM 2D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[DDR_DMEM_UDIMM_1D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_DMEM_UDIMM_1D_HASH_OID,
		.opt = "ddr-dmmem-udimm-1d",
		.help_msg = "DDR Firmware DMEM UDIMM 1D image file",
		.sn = "DDR UDIMM DMEM 1D FirmwareHash",
		.ln = "DDR UDIMM DMEM 1D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[DDR_DMEM_UDIMM_2D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_DMEM_UDIMM_2D_HASH_OID,
		.opt = "ddr-dmmem-udimm-2d",
		.help_msg = "DDR Firmware DMEM UDIMM 2D image file",
		.sn = "DDR UDIMM DMEM 2D FirmwareHash",
		.ln = "DDR UDIMM DMEM 2D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[DDR_IMEM_RDIMM_1D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_IMEM_RDIMM_1D_HASH_OID,
		.opt = "ddr-immem-rdimm-1d",
		.help_msg = "DDR Firmware IMEM RDIMM 1D image file",
		.sn = "DDR RDIMM IMEM 1D FirmwareHash",
		.ln = "DDR RDIMM IMEM 1D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[DDR_IMEM_RDIMM_2D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_IMEM_RDIMM_2D_HASH_OID,
		.opt = "ddr-immem-rdimm-2d",
		.help_msg = "DDR Firmware IMEM RDIMM 2D image file",
		.sn = "DDR RDIMM IMEM 2D FirmwareHash",
		.ln = "DDR RDIMM IMEM 2D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[DDR_DMEM_RDIMM_1D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_DMEM_RDIMM_1D_HASH_OID,
		.opt = "ddr-dmmem-rdimm-1d",
		.help_msg = "DDR Firmware DMEM RDIMM 1D image file",
		.sn = "DDR RDIMM DMEM 1D FirmwareHash",
		.ln = "DDR RDIMM DMEM 1D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	},
	[DDR_DMEM_RDIMM_2D_HASH_EXT - DDR_FW_CONTENT_CERT_PK_EXT] = {
		.oid = DDR_DMEM_RDIMM_2D_HASH_OID,
		.opt = "ddr-dmmem-rdimm-2d",
		.help_msg = "DDR Firmware DMEM RDIMM 2D image file",
		.sn = "DDR RDIMM DMEM 2D FirmwareHash",
		.ln = "DDR RDIMM DMEM 2D Firmware hash (SHA256)",
		.asn1_type = V_ASN1_OCTET_STRING,
		.type = EXT_TYPE_HASH
	}
};

PLAT_REGISTER_EXTENSIONS(pdef_tbb_ext);
