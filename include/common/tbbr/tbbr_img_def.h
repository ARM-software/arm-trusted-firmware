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

#ifndef __TBBR_IMG_DEF_H__
#define __TBBR_IMG_DEF_H__

/* Firmware Image Package */
#define FIP_IMAGE_ID			0

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_ID			1

/* SCP Firmware SCP_BL2 */
#define SCP_BL2_IMAGE_ID		2

/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_ID			3

/* Secure Payload BL32 (Trusted OS) */
#define BL32_IMAGE_ID			4

/* Non-Trusted Firmware BL33 */
#define BL33_IMAGE_ID			5

/* Certificates */
#define TRUSTED_BOOT_FW_CERT_ID		6
#define TRUSTED_KEY_CERT_ID		7

#define SCP_FW_KEY_CERT_ID		8
#define SOC_FW_KEY_CERT_ID		9
#define TRUSTED_OS_FW_KEY_CERT_ID	10
#define NON_TRUSTED_FW_KEY_CERT_ID	11

#define SCP_FW_CONTENT_CERT_ID		12
#define SOC_FW_CONTENT_CERT_ID		13
#define TRUSTED_OS_FW_CONTENT_CERT_ID	14
#define NON_TRUSTED_FW_CONTENT_CERT_ID	15

/* Non-Trusted ROM Firmware NS_BL1U */
#define NS_BL1U_IMAGE_ID		16

/* Trusted FWU Certificate */
#define FWU_CERT_ID			17

/* Trusted FWU SCP Firmware SCP_BL2U */
#define SCP_BL2U_IMAGE_ID		18

/* Trusted FWU Boot Firmware BL2U */
#define BL2U_IMAGE_ID			19

/* Non-Trusted FWU Firmware NS_BL2U */
#define NS_BL2U_IMAGE_ID		20

#endif /* __TBBR_IMG_DEF_H__ */
