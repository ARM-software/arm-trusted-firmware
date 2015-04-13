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
#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

/******************************************************************************
 * Required platform porting definitions that are expected to be common to
 * all platforms
 *****************************************************************************/

/*
 * Platform binary types for linking
 */
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64


/*
 * Generic platform constants
 */
#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

/* Firmware Image Package */
#define FIP_IMAGE_ID			0

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_ID			1

/* SCP Firmware BL3-0 */
#define BL30_IMAGE_ID			2

/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_ID			3

/* Secure Payload BL32 (Trusted OS) */
#define BL32_IMAGE_ID			4

/* Non-Trusted Firmware BL33 */
#define BL33_IMAGE_ID			5

#if TRUSTED_BOARD_BOOT

/* Certificates */
#define BL2_CERT_ID			6
#define TRUSTED_KEY_CERT_ID		7

#define BL30_KEY_CERT_ID		8
#define BL31_KEY_CERT_ID		9
#define BL32_KEY_CERT_ID		10
#define BL33_KEY_CERT_ID		11

#define BL30_CERT_ID			12
#define BL31_CERT_ID			13
#define BL32_CERT_ID			14
#define BL33_CERT_ID			15

#endif /* TRUSTED_BOARD_BOOT */

/*
 * Some of the platform porting definitions use the 'ull' suffix in order to
 * avoid subtle integer overflow errors due to implicit integer type promotion
 * when working with 32-bit values.
 *
 * The TSP linker script includes some of these definitions to define the BL3-2
 * memory map, but the GNU LD does not support the 'ull' suffix, causing the
 * build process to fail. To solve this problem, the auxiliary macro MAKE_ULL(x)
 * will add the 'ull' suffix only when the macro __LINKER__  is not defined
 * (__LINKER__ is defined in the command line to preprocess the linker script).
 * Constants in the linker script will not have the 'ull' suffix, but this is
 * not a problem since the linker evaluates all constant expressions to 64 bit
 * (assuming the target architecture is 64 bit).
 */
#ifndef __LINKER__
  #define MAKE_ULL(x)			x##ull
#else
  #define MAKE_ULL(x)			x
#endif


#endif /* __COMMON_DEF_H__ */

