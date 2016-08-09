/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <stddef.h>

#include "firmware_image_package.h"
#include "tbbr_config.h"

/* The images used depends on the platform. */
toc_entry_t toc_entries[] = {
	{ "SCP Firmware Updater Configuration FWU SCP_BL2U", UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U,
	  "scp-fwu-cfg", NULL, 0, NULL },
	{ "AP Firmware Updater Configuration BL2U", UUID_TRUSTED_UPDATE_FIRMWARE_BL2U,
	  "ap-fwu-cfg", NULL, 0, NULL },
	{ "Firmware Updater NS_BL2U", UUID_TRUSTED_UPDATE_FIRMWARE_NS_BL2U,
	  "fwu", NULL, 0, NULL },
	{ "Non-Trusted Firmware Updater certificate", UUID_TRUSTED_FWU_CERT,
	  "fwu-cert", NULL, 0, NULL },
	{ "Trusted Boot Firmware BL2", UUID_TRUSTED_BOOT_FIRMWARE_BL2,
	  "tb-fw", NULL, 0, NULL },
	{ "SCP Firmware SCP_BL2", UUID_SCP_FIRMWARE_SCP_BL2,
	  "scp-fw", NULL, 0, NULL },
	{ "EL3 Runtime Firmware BL31", UUID_EL3_RUNTIME_FIRMWARE_BL31,
	  "soc-fw", NULL, 0, NULL },
	{ "Secure Payload BL32 (Trusted OS)", UUID_SECURE_PAYLOAD_BL32,
	  "tos-fw", NULL, 0, NULL },
	{ "Non-Trusted Firmware BL33", UUID_NON_TRUSTED_FIRMWARE_BL33,
	  "nt-fw", NULL, 0, NULL },
	/* Key Certificates */
	{ "Root Of Trust key certificate", UUID_ROT_KEY_CERT,
	  "rot-cert", NULL, 0, NULL },
	{ "Trusted key certificate", UUID_TRUSTED_KEY_CERT,
	  "trusted-key-cert", NULL, 0, NULL },
	{ "SCP Firmware key certificate", UUID_SCP_FW_KEY_CERT,
	  "scp-fw-key-cert", NULL, 0, NULL },
	{ "SoC Firmware key certificate", UUID_SOC_FW_KEY_CERT,
	  "soc-fw-key-cert", NULL, 0, NULL },
	{ "Trusted OS Firmware key certificate", UUID_TRUSTED_OS_FW_KEY_CERT,
	  "tos-fw-key-cert", NULL, 0, NULL },
	{ "Non-Trusted Firmware key certificate", UUID_NON_TRUSTED_FW_KEY_CERT,
	  "nt-fw-key-cert", NULL, 0, NULL },
	/* Content certificates */
	{ "Trusted Boot Firmware BL2 certificate", UUID_TRUSTED_BOOT_FW_CERT,
	  "tb-fw-cert", NULL, 0, NULL },
	{ "SCP Firmware content certificate", UUID_SCP_FW_CONTENT_CERT,
	  "scp-fw-cert", NULL, 0, NULL },
	{ "SoC Firmware content certificate", UUID_SOC_FW_CONTENT_CERT,
	  "soc-fw-cert", NULL, 0, NULL },
	{ "Trusted OS Firmware content certificate", UUID_TRUSTED_OS_FW_CONTENT_CERT,
	  "tos-fw-cert", NULL, 0, NULL },
	{ "Non-Trusted Firmware content certificate", UUID_NON_TRUSTED_FW_CONTENT_CERT,
	  "nt-fw-cert", NULL, 0, NULL },
	{ NULL, { 0 }, NULL, NULL, 0, NULL }
};

size_t toc_entries_len = sizeof(toc_entries) / sizeof(toc_entries[0]);
