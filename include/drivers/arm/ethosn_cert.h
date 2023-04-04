/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ETHOSN_CERT_H
#define ETHOSN_CERT_H

#include "ethosn_oid.h"
#include <tbbr/tbb_ext.h>
#include <tbbr/tbb_key.h>

/* Arm(R) Ethos(TM)-N NPU Certificates */
#define ETHOSN_NPU_FW_KEY_CERT_DEF {							\
	.id = ETHOSN_NPU_FW_KEY_CERT,							\
	.opt = "npu-fw-key-cert",							\
	.help_msg = "Arm(R) Ethos(TM)-N NPU Firmware Key Certificate (output file)",	\
	.fn = NULL,									\
	.cn = "NPU Firmware Key Certificate",						\
	.key = NON_TRUSTED_WORLD_KEY,							\
	.issuer = ETHOSN_NPU_FW_KEY_CERT,						\
	.ext = {									\
		NON_TRUSTED_FW_NVCOUNTER_EXT,						\
		ETHOSN_NPU_FW_CONTENT_CERT_PK_EXT,					\
	},										\
	.num_ext = 2 \
}

#define ETHOSN_NPU_FW_CONTENT_CERT_DEF {							\
	.id = ETHOSN_NPU_FW_CONTENT_CERT,						\
	.opt = "npu-fw-cert",								\
	.help_msg = "Arm(R) Ethos(TM)-N NPU Firmware Content Certificate (output file)",\
	.fn = NULL,									\
	.cn = "NPU Firmware Content Certificate",					\
	.key = ETHOSN_NPU_FW_CONTENT_CERT_KEY,						\
	.issuer = ETHOSN_NPU_FW_CONTENT_CERT,						\
	.ext = {									\
		NON_TRUSTED_FW_NVCOUNTER_EXT,						\
		ETHOSN_NPU_FW_HASH_EXT,							\
	},										\
	.num_ext = 2 \
}

/* NPU Extensions */
#define ETHOSN_NPU_FW_CONTENT_CERT_PK_EXT_DEF {						\
	.oid = ETHOSN_NPU_FW_CONTENT_CERT_PK_OID,					\
	.help_msg = "Arm(R) Ethos(TM)-N NPU Firmware content certificate public key",	\
	.sn = "NPUFirmwareContentCertPK",						\
	.ln = "NPU Firmware content cerificate public key",				\
	.asn1_type = V_ASN1_OCTET_STRING,						\
	.type = EXT_TYPE_PKEY,								\
	.attr.key = ETHOSN_NPU_FW_CONTENT_CERT_KEY \
}

#define ETHOSN_NPU_FW_HASH_EXT_DEF {						\
	.oid = ETHOSN_NPU_FW_BINARY_OID,					\
	.opt = "npu-fw",							\
	.help_msg = "Arm(R) Ethos(TM)-N NPU Firmware image file (input file)",	\
	.sn = "NPUFirmwareHash",						\
	.ln = "NPU Firmware Hash (SHA256)",					\
	.asn1_type = V_ASN1_OCTET_STRING,					\
	.type = EXT_TYPE_HASH \
}

/* NPU Keys */
#define ETHOSN_NPU_FW_CONTENT_CERT_KEY_DEF {							  \
	.id = ETHOSN_NPU_FW_CONTENT_CERT_KEY,							  \
	.opt = "npu-fw-key",									  \
	.help_msg = "Arm(R) Ethos(TM)-N NPU Firmware Content Certificate key (input/output file)",\
	.desc = "NPU Firmware Content Certificate key"						  \
}

#endif  /* ETHOSN_CERT_H */
