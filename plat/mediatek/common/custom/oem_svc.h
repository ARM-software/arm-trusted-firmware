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

#ifndef __OEM_SVC_H__
#define __OEM_SVC_H__

/*******************************************************************************
 * Defines for runtime services func ids
 ******************************************************************************/
/*
 * Number of OEM calls (above) implemented.
 */
#define OEM_SVC_NUM_CALLS		3

/*******************************************************************************
 * Defines for OEM Service queries
 ******************************************************************************/
/* 0x83000000 - 0x8300FEFF is OEM service calls */
#define OEM_SVC_CALL_COUNT		0x8300ff00
#define OEM_SVC_UID			0x8300ff01
/* 0x8300ff02 is reserved */
#define OEM_SVC_VERSION			0x8300ff03
/* 0x8300ff04 - 0x8300FFFF is reserved for future expansion */

/* OEM Service Calls version numbers */
#define OEM_VERSION_MAJOR		0x0
#define OEM_VERSION_MINOR		0x1

/* The macros below are used to identify OEM calls from the SMC function ID */
/* SMC32 ID range from 0x83000000 to 0x83000FFF */
/* SMC64 ID range from 0xC3000000 to 0xC3000FFF */
#define OEM_FID_MASK			0xf000u
#define OEM_FID_VALUE			0u
#define is_oem_fid(_fid) \
	(((_fid) & OEM_FID_MASK) == OEM_FID_VALUE)

#define OEM_SVC_E_SUCCESS		0
#define OEM_SVC_E_NOT_SUPPORTED		-1
#define OEM_SVC_E_INVALID_PARAMS	-2

#endif /* __OEM_SVC_H__ */
