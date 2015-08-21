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
	[BL2_CERT] = {
		.id = BL2_CERT,
		.opt = "bl2-cert",
		.fn = NULL,
		.cn = "BL2 Certificate",
		.key = ROT_KEY,
		.issuer = BL2_CERT,
		.ext = {
			BL2_HASH_EXT
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
			TZ_WORLD_PK_EXT,
			NTZ_WORLD_PK_EXT
		},
		.num_ext = 2
	},
	[BL30_KEY_CERT] = {
		.id = BL30_KEY_CERT,
		.opt = "bl30-key-cert",
		.fn = NULL,
		.cn = "BL3-0 Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = BL30_KEY_CERT,
		.ext = {
			BL30_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[BL30_CERT] = {
		.id = BL30_CERT,
		.opt = "bl30-cert",
		.fn = NULL,
		.cn = "BL3-0 Content Certificate",
		.key = BL30_KEY,
		.issuer = BL30_CERT,
		.ext = {
			BL30_HASH_EXT
		},
		.num_ext = 1
	},
	[BL31_KEY_CERT] = {
		.id = BL31_KEY_CERT,
		.opt = "bl31-key-cert",
		.fn = NULL,
		.cn = "BL3-1 Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = BL31_KEY_CERT,
		.ext = {
			BL31_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[BL31_CERT] = {
		.id = BL31_CERT,
		.opt = "bl31-cert",
		.fn = NULL,
		.cn = "BL3-1 Content Certificate",
		.key = BL31_KEY,
		.issuer = BL31_CERT,
		.ext = {
			BL31_HASH_EXT
		},
		.num_ext = 1
	},
	[BL32_KEY_CERT] = {
		.id = BL32_KEY_CERT,
		.opt = "bl32-key-cert",
		.fn = NULL,
		.cn = "BL3-2 Key Certificate",
		.key = TRUSTED_WORLD_KEY,
		.issuer = BL32_KEY_CERT,
		.ext = {
			BL32_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[BL32_CERT] = {
		.id = BL32_CERT,
		.opt = "bl32-cert",
		.fn = NULL,
		.cn = "BL3-2 Content Certificate",
		.key = BL32_KEY,
		.issuer = BL32_CERT,
		.ext = {
			BL32_HASH_EXT
		},
		.num_ext = 1
	},
	[BL33_KEY_CERT] = {
		.id = BL33_KEY_CERT,
		.opt = "bl33-key-cert",
		.fn = NULL,
		.cn = "BL3-3 Key Certificate",
		.key = NON_TRUSTED_WORLD_KEY,
		.issuer = BL33_KEY_CERT,
		.ext = {
			BL33_CONTENT_CERT_PK_EXT
		},
		.num_ext = 1
	},
	[BL33_CERT] = {
		.id = BL33_CERT,
		.opt = "bl33-cert",
		.fn = NULL,
		.cn = "BL3-3 Content Certificate",
		.key = BL33_KEY,
		.issuer = BL33_CERT,
		.ext = {
			BL33_HASH_EXT
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
			SCP_BL2U_HASH_EXT,
			BL2U_HASH_EXT,
			NS_BL2U_HASH_EXT
		},
		.num_ext = 3
	}
};

REGISTER_COT(tbb_certs);
