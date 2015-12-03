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
#ifndef TBB_EXT_H_
#define TBB_EXT_H_

#include "ext.h"

/* TBBR extensions */
enum {
	TRUSTED_FW_NVCOUNTER_EXT,
	NON_TRUSTED_FW_NVCOUNTER_EXT,
	TRUSTED_BOOT_FW_HASH_EXT,
	TRUSTED_WORLD_PK_EXT,
	NON_TRUSTED_WORLD_PK_EXT,
	SCP_FW_CONTENT_CERT_PK_EXT,
	SCP_FW_HASH_EXT,
	SOC_FW_CONTENT_CERT_PK_EXT,
	SOC_AP_FW_HASH_EXT,
	TRUSTED_OS_FW_CONTENT_CERT_PK_EXT,
	TRUSTED_OS_FW_HASH_EXT,
	NON_TRUSTED_FW_CONTENT_CERT_PK_EXT,
	NON_TRUSTED_WORLD_BOOTLOADER_HASH_EXT,
	SCP_FWU_CFG_HASH_EXT,
	AP_FWU_CFG_HASH_EXT,
	FWU_HASH_EXT
};

#endif /* TBB_EXT_H_ */
