/*
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NCORE_CCU_H
#define NCORE_CCU_H


#define NCORE_CCU_OFFSET		0xf7000000


/* Coherent Sub-System Address Map */
#define NCORE_CAIU_OFFSET		0x00000
#define NCORE_CAIU_SIZE			0x01000

#define NCORE_NCBU_OFFSET		0x60000
#define NCORE_NCBU_SIZE			0x01000

#define NCORE_DIRU_OFFSET		0x80000
#define NCORE_DIRU_SIZE			0x01000

#define NCORE_CMIU_OFFSET		0xc0000
#define NCORE_CMIU_SIZE			0x01000

#define NCORE_CSR_OFFSET		0xff000
#define NCORE_CSADSERO			0x00040
#define NCORE_CSUIDR			0x00ff8
#define NCORE_CSIDR			0x00ffc

/* Directory Unit Register Map */
#define NCORE_DIRUSFER			0x00010
#define NCORE_DIRUMRHER			0x00070
#define NCORE_DIRUSFMCR			0x00080
#define NCORE_DIRUSFMAR			0x00084

/* Coherent Agent Interface Unit Register Map */
#define NCORE_CAIUIDR			0x00ffc

/* Snoop Enable Register */
#define NCORE_DIRUCASER0		0x00040
#define NCORE_DIRUCASER1		0x00044
#define NCORE_DIRUCASER2		0x00048
#define NCORE_DIRUCASER3		0x0004c

#define NCORE_CSADSER0			0x00040
#define NCORE_CSADSER1			0x00044
#define NCORE_CSADSER2			0x00048
#define NCORE_CSADSER3			0x0004c

/* Protocols Definition */
#define ACE_W_DVM			0
#define ACE_L_W_DVM			1
#define ACE_WO_DVM			2
#define ACE_L_WO_DVM			3

/* Bypass OC Ram Firewall */
#define NCORE_FW_OCRAM_BLK_BASE		0x100200
#define NCORE_FW_OCRAM_BLK_CGF1		0x04
#define NCORE_FW_OCRAM_BLK_CGF2		0x08
#define NCORE_FW_OCRAM_BLK_CGF3		0x0c
#define NCORE_FW_OCRAM_BLK_CGF4		0x10

#define OCRAM_PRIVILEGED_MASK		BIT(29)
#define OCRAM_SECURE_MASK		BIT(30)

/* Macros */
#define NCORE_CCU_REG(base)		(NCORE_CCU_OFFSET + (base))
#define NCORE_CCU_CSR(reg)		(NCORE_CCU_REG(NCORE_CSR_OFFSET)\
						+ (reg))
#define NCORE_CCU_DIR(reg)		(NCORE_CCU_REG(NCORE_DIRU_OFFSET)\
						+ (reg))
#define NCORE_CCU_CAI(reg)		(NCORE_CCU_REG(NCORE_CAIU_OFFSET)\
						+ (reg))

#define DIRECTORY_UNIT(x, reg)		(NCORE_CCU_DIR(reg)\
						+ NCORE_DIRU_SIZE * (x))
#define COH_AGENT_UNIT(x, reg)		(NCORE_CCU_CAI(reg)\
						+ NCORE_CAIU_SIZE * (x))

#define COH_CPU0_BYPASS_REG(reg)	(NCORE_CCU_REG(NCORE_FW_OCRAM_BLK_BASE)\
						+ (reg))

#define CSUIDR_NUM_CMI(x)		(((x) & 0x3f000000) >> 24)
#define CSUIDR_NUM_DIR(x)		(((x) & 0x003f0000) >> 16)
#define CSUIDR_NUM_NCB(x)		(((x) & 0x00003f00) >> 8)
#define CSUIDR_NUM_CAI(x)		(((x) & 0x0000007f) >> 0)

#define CSIDR_NUM_SF(x)			(((x) & 0x007c0000) >> 18)

#define SNOOP_FILTER_ID(x)		(((x) << 16))

#define CACHING_AGENT_BIT(x)		(((x) & 0x08000) >> 15)
#define CACHING_AGENT_TYPE(x)		(((x) & 0xf0000) >> 16)


typedef struct coh_ss_id {
	uint8_t num_coh_mem;
	uint8_t num_directory;
	uint8_t num_non_coh_bridge;
	uint8_t num_coh_agent;
	uint8_t num_snoop_filter;
} coh_ss_id_t;

uint32_t init_ncore_ccu(void);
void ncore_enable_ocram_firewall(void);

#endif
