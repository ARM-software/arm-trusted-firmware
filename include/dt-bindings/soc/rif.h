/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 *
 */

#ifndef _DT_BINDINGS_RIF_H
#define _DT_BINDINGS_RIF_H

/* RIFSC CIDs */
#define RIF_CID0		0x0
#define RIF_CID1		0x1
#define RIF_CID2		0x2
#define RIF_CID3		0x3
#define RIF_CID4		0x4
#define RIF_CID5		0x5
#define RIF_CID6		0x6
#define RIF_CID7		0x7
#define RIF_CID_MAX		0x8

/* RIFSC semaphore list */
#define EMPTY_SEMWL		0x0
#define RIF_CID0_BF		BIT_32(RIF_CID0)
#define RIF_CID1_BF		BIT_32(RIF_CID1)
#define RIF_CID2_BF		BIT_32(RIF_CID2)
#define RIF_CID3_BF		BIT_32(RIF_CID3)
#define RIF_CID4_BF		BIT_32(RIF_CID4)
#define RIF_CID5_BF		BIT_32(RIF_CID5)
#define RIF_CID6_BF		BIT_32(RIF_CID6)
#define RIF_CID7_BF		BIT_32(RIF_CID7)

/* RIFSC secure levels */
#define RIF_NSEC		0x0
#define RIF_SEC			0x1

/* RIFSC privilege levels */
#define RIF_NPRIV		0x0
#define RIF_PRIV		0x1

/* RIFSC semaphore modes */
#define RIF_SEM_DIS		0x0
#define RIF_SEM_EN		0x1

/* RIFSC CID filtering modes */
#define RIF_CFDIS		0x0
#define RIF_CFEN		0x1

/* RIFSC lock states */
#define RIF_UNLOCK		0x0
#define RIF_LOCK		0x1

/* Used when a field in a macro has no impact */
#define RIF_UNUSED		0x0

#define RIFPROT(rifid, sem_list, sec, priv, scid, sem_en, cfen) \
	(((rifid) << 24) | ((sem_list) << 16) | ((priv) << 9) | ((sec) << 8) | ((scid) << 4) | \
	 ((sem_en) << 1) | (cfen))

/* Masters IDs */
#define RIMU_ID(idx)		(idx)

/* Master configuration modes */
#define RIF_CIDSEL_P		0x0 /* Config from RISUP */
#define RIF_CIDSEL_M		0x1 /* Config from RIMU */

#define RIMUPROT(rimuid, scid, sec, priv, mode) \
	(((rimuid) << 16) | ((priv) << 9) | ((sec) << 8) | ((scid) << 4) | ((mode) << 2))

/* RISAF region IDs */
#define RISAF_REG_ID(idx)	(idx)

/* RISAF base region enable modes */
#define RIF_BREN_DIS		0x0
#define RIF_BREN_EN		0x1

/* RISAF encryption modes */
#define RIF_ENC_DIS		0x0
#define RIF_ENC_EN		0x1

#define RISAFPROT(risaf_region, cid_read_list, cid_write_list, cid_priv_list, sec, enc, enabled) \
	(((cid_write_list) << 24) | ((cid_read_list) << 16) | ((cid_priv_list) << 8) | \
	 ((enc) << 7) | ((sec) << 6) | ((enabled) << 5) | (risaf_region))

/* RISAB page IDs */
#define RISAB_PAGE_ID(idx)	(idx)

/* RISAB control modes */
#define RIF_DDCID_DIS		0x0
#define RIF_DDCID_EN		0x1

#define RISABPROT(risab_page, delegate_en, delegate_cid, sec, default_priv, cid_read_list, \
		  cid_write_list, cid_priv_list, enabled) \
	(((risab_page) << 24) | ((default_priv) << 9) | ((sec) << 8) | ((delegate_cid) << 4) | \
	 ((delegate_en) << 2) | (enabled)) \
	((cid_write_list) << 16 | (cid_read_list) << 8 | (cid_priv_list))

#endif /* _DT_BINDINGS_RIF_H */
