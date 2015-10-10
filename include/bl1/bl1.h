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

#ifndef __BL1_FWU_H__
#define __BL1_FWU_H__

#include <bl_common.h>

/*
 * Defines for BL1 SMC function ids.
 */
#define BL1_SMC_CALL_COUNT		0x0
#define BL1_SMC_UID			0x1
/* SMC #0x2 reserved */
#define BL1_SMC_VERSION			0x3

/*
 * Corresponds to the function ID of the SMC that
 * the BL1 exception handler service to execute BL31.
 */
#define BL1_SMC_RUN_IMAGE		0x4

/*
 * BL1 SMC version
 */
#define BL1_SMC_MAJOR_VER		0x0
#define BL1_SMC_MINOR_VER		0x1

/*
 * Defines for FWU SMC function ids.
 */

#define FWU_SMC_IMAGE_COPY		0x10
#define FWU_SMC_IMAGE_AUTH		0x11
#define FWU_SMC_IMAGE_EXECUTE		0x12
#define FWU_SMC_IMAGE_RESUME		0x13
#define FWU_SMC_SEC_IMAGE_DONE		0x14
#define FWU_SMC_UPDATE_DONE		0x15

/*
 * Number of FWU calls (above) implemented
 */
#define FWU_NUM_SMC_CALLS		6

#if TRUSTED_BOARD_BOOT
# define BL1_NUM_SMC_CALLS		(FWU_NUM_SMC_CALLS + 4)
#else
# define BL1_NUM_SMC_CALLS		4
#endif

/*
 * The macros below are used to identify FWU
 * calls from the SMC function ID
 */
#define FWU_SMC_FID_START		FWU_SMC_IMAGE_COPY
#define FWU_SMC_FID_END			FWU_SMC_UPDATE_DONE
#define is_fwu_fid(_fid) \
    ((_fid >= FWU_SMC_FID_START) && (_fid <= FWU_SMC_FID_END))

#ifndef __ASSEMBLY__
#include <cassert.h>

/*
 * Check if the total number of FWU SMC calls are as expected.
 */
CASSERT(FWU_NUM_SMC_CALLS == 	\
		(FWU_SMC_FID_END - FWU_SMC_FID_START + 1),\
		assert_FWU_NUM_SMC_CALLS_mismatch);

#endif /* __ASSEMBLY__ */
#endif /* __BL1_FWU_H__ */
