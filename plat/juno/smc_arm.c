/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <stdint.h>
#include <context.h>
#include <runtime_svc.h>
#include <psci_private.h>


#define ARM_SMC_ARM_CPU_SUSPEND   0x80100001
#define ARM_SMC_ARM_CPU_OFF       0x80100002
#define ARM_SMC_ARM_CPU_ON        0x80100003
#define ARM_SMC_ARM_MIGRATE       0x80100004

#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_PRESENCE	0x80FFFF00
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_UID	0x80FFFF10

#define ARM_TRUSTZONE_UID_4LETTERID	0x1
#define ARM_TRUSTZONE_ARM_UID		0x40524d48 /* "ARMH" */


static uint64_t smc_arm_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
				uint64_t x3, uint64_t x4, void *cookie,
				void *handle, uint64_t flags)
{
	switch (smc_fid) {

	/*
	 * HACK WARNING: Below we use SMC_RET4 to return the original contents
	 * of x1-x3. We do this because UEFI is expecting these values to be
	 * preserved across the SMC call.
	 */

	case ARM_TRUSTZONE_ARM_FAST_SMC_ID_PRESENCE:
		SMC_RET4(handle, 1, x1, x2, x3);
		break;

	case ARM_TRUSTZONE_ARM_FAST_SMC_ID_UID + 0:
		SMC_RET4(handle, ARM_TRUSTZONE_UID_4LETTERID, x1, x2, x3);
		break;

	case ARM_TRUSTZONE_ARM_FAST_SMC_ID_UID + 1:
		SMC_RET4(handle, ARM_TRUSTZONE_ARM_UID, x1, x2, x3);
		break;

	/* The following 3 cases translate functions into the PSCI equivalent */

	case ARM_SMC_ARM_CPU_OFF:
		smc_fid = PSCI_CPU_OFF;
		break;

	case ARM_SMC_ARM_CPU_SUSPEND:
		smc_fid = PSCI_CPU_SUSPEND_AARCH64;
		break;

	case ARM_SMC_ARM_CPU_ON:
		smc_fid = PSCI_CPU_ON_AARCH64;
		break;

	}

	return psci_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
}

static int32_t smc_arm_setup(void)
{
	return 0;
}

DECLARE_RT_SVC(arm, OEN_ARM_START, OEN_ARM_END, SMC_TYPE_FAST,
			smc_arm_setup, smc_arm_handler);
