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
#ifndef __PLAT_SIP_SVC_H__
#define __PLAT_SIP_SVC_H__

#include <stdint.h>

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT    0x8200ff00
#define SIP_SVC_UID           0x8200ff01
/* 0x8200ff02 is reserved */
#define SIP_SVC_VERSION       0x8200ff03

/* Mediatek SiP Service Calls version numbers */
#define MTK_SIP_SVC_VERSION_MAJOR 0x0
#define MTK_SIP_SVC_VERSION_MINOR 0x1

/* Number of Mediatek SiP Calls implemented */
#define MTK_SIP_NUM_CALLS   1

/* Mediatek SiP Service Calls function IDs */
#define MTK_SIP_SET_AUTHORIZED_SECURE_REG 0x82000001

/* For MTK SMC from Secure OS */
/* 0x82000000 - 0x820000FF & 0xC2000000 - 0xC20000FF */
#define MTK_SIP_TBASE_HWUID_AARCH32         0x82000000
#define MTK_SIP_LK_WDT_AARCH32              0x82000106
#define MTK_SIP_LK_WDT_AARCH64              0xC2000106
/* astone added for debug purpose */
#define MTK_SIP_KERNEL_MSG_AARCH32          0x820002ff
#define MTK_SIP_KERNEL_MSG_AARCH64          0xC20002ff
#define MTK_SIP_KERNEL_WDT_AARCH32          0x82000204
#define MTK_SIP_KERNEL_WDT_AARCH64          0xC2000204
#define MTK_SIP_KERNEL_BOOT_AARCH32         0x82000200
#define MTK_SIP_KERNEL_BOOT_AARCH64         0xC2000200
/*
 *Please follow the naming rule
 *#define SIP_SVC_E_XXX error_num (should be negative value)
 */
#define SIP_SVC_E_SUCCESS           0
#define SIP_SVC_E_NOT_SUPPORTED     -1
#define SIP_SVC_E_INVALID_PARAMS    -2
#define SIP_SVC_E_INVALID_Range     -3
#define SIP_SVC_E_PERMISSION_DENY   -4
#define SIP_SVC_E_LOCK_FAIL         -5

struct kernel_info {
	uint64_t pc;
	uint64_t r0;
	uint64_t r1;
	uint64_t r2;
	uint64_t k32_64;
};

typedef struct mtk_bl_param {
	uint64_t bootarg_loc;
	uint64_t bootarg_size;
	uint64_t bl33_start_addr;
	uint64_t tee_info_addr;
} mtk_bl_param_t;

/* Mediatek SiP Calls error code */
enum {
	MTK_SIP_E_SUCCESS = 0,
	MTK_SIP_E_INVALID_PARAM = -1
};

/*
 * This function should be implemented in Mediatek SOC directory. It fullfills
 * MTK_SIP_SET_AUTHORIZED_SECURE_REG SiP call by checking the sreg with the
 * predefined secure register list, if a match was found, set val to sreg.
 *
 * Return MTK_SIP_E_SUCCESS on success, and MTK_SIP_E_INVALID_PARAM on failure.
 */
uint64_t mt_sip_set_authorized_sreg(uint32_t sreg, uint32_t val);
void save_kernel_info(uint64_t pc, uint64_t r0, uint64_t r1, uint64_t k32_64);

#endif /* __PLAT_SIP_SVC_H__ */
