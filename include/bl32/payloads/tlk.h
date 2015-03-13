/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __TLK_H__
#define __TLK_H__

/*
 * SMC function IDs that TLK uses to signal various forms of completions
 * to the secure payload dispatcher.
 */
#define TLK_ENTRY_DONE		(0x32000003 | (1 << 31))
#define TLK_FID_SHARED_MEMBUF	(0x32000005 | (1 << 31))

/*
 * Total number of function IDs implemented for services offered to NS clients.
 */
#define TLK_NUM_FID		0

/* TLK implementation version numbers */
#define TLK_VERSION_MAJOR	0x0 /* Major version */
#define TLK_VERSION_MINOR	0x1 /* Minor version */

/*
 * Standard Trusted OS Function IDs that fall under Trusted OS call range
 * according to SMC calling convention
 */
#define TOS_CALL_COUNT		0xbf00ff00 /* Number of calls implemented */
#define TOS_UID			0xbf00ff01 /* Implementation UID */
#define TOS_CALL_VERSION	0xbf00ff03 /* Trusted OS Call Version */

#endif /* __TLK_H__ */
