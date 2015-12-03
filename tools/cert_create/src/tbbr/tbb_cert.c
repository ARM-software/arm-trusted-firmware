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

#include "tbbr/tbb_cert.h"
#include "tbbr/tbb_ext.h"
#include "tbbr/tbb_key.h"

/*
 * Certificates used in the chain of trust
 *
 * The order of the certificates must follow the enumeration specified in
 * tbb_cert.h. All certificates are self-signed, so the issuer certificate
 * field points to itself.
 */
static cert_t tbb_certs[] = {
	[TRUSTED_BOOT_FW_CERT] = {
		.id = TRUSTED_BOOT_FW_CERT,
		.opt = "tb-fw-cert",
		.fn = NULL,
		.cn = "Trusted Boot FW Certificate",
		.key = ROT_KEY,
		.issuer = TRUSTED_BOOT_FW_CERT,
		.ext = {
			TRUSTED_BOOT_FW_HASH_EXT
		},
		.num_ext = 1
	},
	[TRUSTED_KEY_CERT] = {
		.id = TRUSTED_KEY_CERT,
		.opt = "trusted-key-cert",
		.fn = NULL,
		.cn = "Trusted Key Certificate",
		.key = ROT_KEY,
		.issuer = TRUSTED_KEY_CERT,
		.ext = {
			TRUSTED_WORLD_PK_EXT,
			NON_TRUSTED_WORLD_PK_EXT
		},
		.num_ext = 2
	},
	[SCP_FW_KEY_CERT] = {
		.id = SCP_FW_KEY_CERT,
		.opt = "scp-fw-key-cert",
		.fn = NULL,
		.cn = "SCP Firmware Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = SCP_FW_KEY_CERT,
		.ext = {
			SCP_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[SCP_FW_CONTENT_CERT] = {
		.id = SCP_FW_CONTENT_CERT,
		.opt = "scp-fw-cert",
		.fn = NULL,
		.cn = "SCP Firmware Content Certificate",
		.key = SCP_FW_CONTENT_CERT_KEY,
		.issuer = SCP_FW_CONTENT_CERT,
		.ext = {
			SCP_FW_HASH_EXT
		},
		.num_ext = 1
	},
	[SOC_FW_KEY_CERT] = {
		.id = SOC_FW_KEY_CERT,
		.opt = "soc-fw-key-cert",
		.fn = NULL,
		.cn = "SoC Firmware Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = SOC_FW_KEY_CERT,
		.ext = {
			SOC_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[SOC_FW_CONTENT_CERT] = {
		.id = SOC_FW_CONTENT_CERT,
		.opt = "soc-fw-cert",
		.fn = NULL,
		.cn = "SoC Firmware Content Certificate",
		.key = SOC_FW_CONTENT_CERT_KEY,
		.issuer = SOC_FW_CONTENT_CERT,
		.ext = {
			SOC_AP_FW_HASH_EXT
		},
		.num_ext = 1
	},
	[TRUSTED_OS_FW_KEY_CERT] = {
		.id = TRUSTED_OS_FW_KEY_CERT,
		.opt = "tos-fw-key-cert",
		.fn = NULL,
		.cn = "Trusted OS Firmware Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = TRUSTED_OS_FW_KEY_CERT,
		.ext = {
			TRUSTED_OS_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[TRUSTED_OS_FW_CONTENT_CERT] = {
		.id = TRUSTED_OS_FW_CONTENT_CERT,
		.opt = "tos-fw-cert",
		.fn = NULL,
		.cn = "Trusted OS Firmware Content Certificate",
		.key = TRUSTED_OS_FW_CONTENT_CERT_KEY,
		.issuer = TRUSTED_OS_FW_CONTENT_CERT,
		.ext = {
			TRUSTED_OS_FW_HASH_EXT
		},
		.num_ext = 1
	},
	[NON_TRUSTED_FW_KEY_CERT] = {
		.id = NON_TRUSTED_FW_KEY_CERT,
		.opt = "nt-fw-key-cert",
		.fn = NULL,
		.cn = "Non-Trusted Firmware Key Certificate",
		.key = NON_TRUSTED_WORLD_KEY,
		.issuer = NON_TRUSTED_FW_KEY_CERT,
		.ext = {
			NON_TRUSTED_FW_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[NON_TRUSTED_FW_CONTENT_CERT] = {
		.id = NON_TRUSTED_FW_CONTENT_CERT,
		.opt = "nt-fw-cert",
		.fn = NULL,
		.cn = "Non-Trusted Firmware Content Certificate",
		.key = NON_TRUSTED_FW_CONTENT_CERT_KEY,
		.issuer = NON_TRUSTED_FW_CONTENT_CERT,
		.ext = {
			NON_TRUSTED_WORLD_BOOTLOADER_HASH_EXT
		},
		.num_ext = 1
	},
	[FWU_CERT] = {
		.id = FWU_CERT,
		.opt = "fwu-cert",
		.fn = NULL,
		.cn = "FWU Certificate",
		.key = ROT_KEY,
		.issuer = FWU_CERT,
		.ext = {
			SCP_FWU_CFG_HASH_EXT,
			AP_FWU_CFG_HASH_EXT,
			FWU_HASH_EXT
		},
		.num_ext = 3
	}
};

REGISTER_COT(tbb_certs);
