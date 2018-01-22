/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TLK_H
#define TLK_H

#include <lib/utils_def.h>

/*
 * Generate function IDs for the Trusted OS/Apps
 */
#define TLK_TOS_YIELD_FID(fid)	((fid) | 0x72000000 | (0 << 31))
#define TLK_TA_YIELD_FID(fid)	((fid) | 0x70000000 | (0 << 31))

/*
 * Trusted OS specific function IDs
 */
#define TLK_REGISTER_LOGBUF	TLK_TOS_YIELD_FID(0x1)
#define TLK_REGISTER_REQBUF	TLK_TOS_YIELD_FID(0x2)
#define TLK_SS_REGISTER_HANDLER	TLK_TOS_YIELD_FID(0x3)
#define TLK_REGISTER_NS_DRAM_RANGES	TLK_TOS_YIELD_FID(0x4)
#define TLK_SET_ROOT_OF_TRUST	TLK_TOS_YIELD_FID(0x5)
#define TLK_RESUME_FID		TLK_TOS_YIELD_FID(0x100)
#define TLK_SYSTEM_SUSPEND	TLK_TOS_YIELD_FID(0xE001)
#define TLK_SYSTEM_RESUME	TLK_TOS_YIELD_FID(0xE002)
#define TLK_SYSTEM_OFF		TLK_TOS_YIELD_FID(0xE003)

/*
 * SMC function IDs that TLK uses to signal various forms of completions
 * to the secure payload dispatcher.
 */
#define TLK_REQUEST_DONE	(0x32000001 | (ULL(1) << 31))
#define TLK_PREEMPTED		(0x32000002 | (ULL(1) << 31))
#define TLK_ENTRY_DONE		(0x32000003 | (ULL(1) << 31))
#define TLK_VA_TRANSLATE	(0x32000004 | (ULL(1) << 31))
#define TLK_SUSPEND_DONE	(0x32000005 | (ULL(1) << 31))
#define TLK_RESUME_DONE		(0x32000006 | (ULL(1) << 31))
#define TLK_SYSTEM_OFF_DONE	(0x32000007 | (ULL(1) << 31))

/*
 * Trusted Application specific function IDs
 */
#define TLK_OPEN_TA_SESSION	TLK_TA_YIELD_FID(0x1)
#define TLK_CLOSE_TA_SESSION	TLK_TA_YIELD_FID(0x2)
#define TLK_TA_LAUNCH_OP	TLK_TA_YIELD_FID(0x3)
#define TLK_TA_SEND_EVENT	TLK_TA_YIELD_FID(0x4)

/*
 * Total number of function IDs implemented for services offered to NS clients.
 */
#define TLK_NUM_FID		7

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

#endif /* TLK_H */
