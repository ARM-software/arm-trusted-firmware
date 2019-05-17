/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA194_RAS_PRIVATE
#define TEGRA194_RAS_PRIVATE

#include <stdint.h>

/* Implementation defined RAS error and corresponding error message */
struct ras_error {
	const char *error_msg;
	/* IERR(bits[15:8]) from ERR<n>STATUS */
	uint8_t error_code;
};

/* RAS error node-specific auxiliary data */
struct ras_aux_data {
	/* name for current RAS node. */
	const char *name;
	/* point to null-terminated ras_error array to convert error code to msg. */
	const struct ras_error *error_records;
	/*
	 * function to return an value which needs to be programmed into ERXCTLR_EL1
	 * to enable all specified RAS errors for current node.
	 */
	uint64_t (*err_ctrl)(void);
};

/* IFU Uncorrectable RAS ERROR */
#define IFU_UNCORR_RAS_ERROR_LIST(X)

/* JSR_RET Uncorrectable RAS ERROR */
#define JSR_RET_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(JSR_RET, 35, 0x13, "Floating Point Register File Parity Error")	\
	X(JSR_RET, 34, 0x12, "Integer Register File Parity Error")		\
	X(JSR_RET, 33, 0x11, "Garbage Bundle")					\
	X(JSR_RET, 32, 0x10, "Bundle Completion Timeout")

/* JSR_MTS Uncorrectable RAS ERROR */
#define JSR_MTS_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(JSR_MTS, 40, 0x28, "CoreSight Access Error")				\
	X(JSR_MTS, 39, 0x27, "Dual Execution Uncorrectable Error")		\
	X(JSR_MTS, 37, 0x25, "CTU MMIO Region")					\
	X(JSR_MTS, 36, 0x24, "MTS MMCRAB Region Access")			\
	X(JSR_MTS, 35, 0x23, "MTS_CARVEOUT Access from ARM SW")			\
	X(JSR_MTS, 34, 0x22, "NAFLL PLL Failure to Lock")			\
	X(JSR_MTS, 32, 0x20, "Internal Uncorrectable  MTS Error")

/* LSD_STQ Uncorrectable RAS ERROR */
#define LSD_STQ_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(LSD_STQ, 41, 0x39, "Coherent Cache Data Store Multi-Line ECC Error")	\
	X(LSD_STQ, 40, 0x38, "Coherent Cache Data Store Uncorrectable ECC Error") \
	X(LSD_STQ, 38, 0x36, "Coherent Cache Data Load Uncorrectable ECC Error") \
	X(LSD_STQ, 33, 0x31, "Coherent Cache Tag Store Parity Error")		\
	X(LSD_STQ, 32, 0x30, "Coherent Cache Tag Load Parity Error")

/* LSD_DCC Uncorrectable RAS ERROR */
#define LSD_DCC_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(LSD_DCC, 41, 0x49, "BTU Copy Mini-Cache PPN Multi-Hit Error")		\
	X(LSD_DCC, 39, 0x47, "Coherent Cache Data Uncorrectable ECC Error")	\
	X(LSD_DCC, 37, 0x45, "Version Cache Byte-Enable Parity Error")		\
	X(LSD_DCC, 36, 0x44, "Version Cache Data Uncorrectable ECC Error")	\
	X(LSD_DCC, 33, 0x41, "BTU Copy Coherent Cache PPN Parity Error")	\
	X(LSD_DCC, 32, 0x40, "BTU Copy Coherent Cache VPN Parity Error")

/* LSD_L1HPF Uncorrectable RAS ERROR */
#define LSD_L1HPF_UNCORR_RAS_ERROR_LIST(X)

/* L2 Uncorrectable RAS ERROR */
#define L2_UNCORR_RAS_ERROR_LIST(X)						\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(L2, 56, 0x68, "URT Timeout")						\
	X(L2, 55, 0x67, "L2 Protocol Violation")				\
	X(L2, 54, 0x66, "SCF to L2 Slave Error Read")				\
	X(L2, 53, 0x65, "SCF to L2 Slave Error Write")				\
	X(L2, 52, 0x64, "SCF to L2 Decode Error Read")				\
	X(L2, 51, 0x63, "SCF to L2 Decode Error Write")				\
	X(L2, 50, 0x62, "SCF to L2 Request Response Interface Parity Errors")	\
	X(L2, 49, 0x61, "SCF to L2 Advance notice interface parity errors")	\
	X(L2, 48, 0x60, "SCF to L2 Filldata Parity Errors")			\
	X(L2, 47, 0x5F, "SCF to L2 UnCorrectable ECC Data Error on interface")	\
	X(L2, 45, 0x5D, "Core 1 to L2 Parity Error")				\
	X(L2, 44, 0x5C, "Core 0 to L2 Parity Error")				\
	X(L2, 43, 0x5B, "L2 Multi-Hit")						\
	X(L2, 42, 0x5A, "L2 URT Tag Parity Error")				\
	X(L2, 41, 0x59, "L2 NTT Tag Parity Error")				\
	X(L2, 40, 0x58, "L2 MLT Tag Parity Error")				\
	X(L2, 39, 0x57, "L2 URD Data")						\
	X(L2, 38, 0x56, "L2 NTP Data")						\
	X(L2, 36, 0x54, "L2 MLC Uncorrectable Clean")				\
	X(L2, 35, 0x53, "L2 URD Uncorrectable Dirty")				\
	X(L2, 34, 0x52, "L2 MLC Uncorrectable Dirty")

/* CLUSTER_CLOCKS Uncorrectable RAS ERROR */
#define CLUSTER_CLOCKS_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(CLUSTER_CLOCKS, 32, 0xE4, "Frequency Monitor Error")

/* MMU Uncorrectable RAS ERROR */
#define MMU_UNCORR_RAS_ERROR_LIST(X)

/* L3 Uncorrectable RAS ERROR */
#define L3_UNCORR_RAS_ERROR_LIST(X)						\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(L3, 43, 0x7B, "SNOC Interface Parity Error")				\
	X(L3, 42, 0x7A, "MCF Interface Parity Error")				\
	X(L3, 41, 0x79, "L3 Tag Parity Error")					\
	X(L3, 40, 0x78, "L3 Dir Parity Error")					\
	X(L3, 39, 0x77, "L3 Uncorrectable ECC Error")				\
	X(L3, 37, 0x75, "Multi-Hit CAM Error")					\
	X(L3, 36, 0x74, "Multi-Hit Tag Error")					\
	X(L3, 35, 0x73, "Unrecognized Command Error")				\
	X(L3, 34, 0x72, "L3 Protocol Error")

/* CCPMU Uncorrectable RAS ERROR */
#define CCPMU_UNCORR_RAS_ERROR_LIST(X)						\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(CCPMU, 40, 0x87, "CoreSight Access Error")				\
	X(CCPMU, 36, 0x84, "MCE Ucode Error")					\
	X(CCPMU, 35, 0x83, "MCE IL1 Parity Error")				\
	X(CCPMU, 34, 0x82, "MCE Timeout Error")					\
	X(CCPMU, 33, 0x81, "CRAB Access Error")					\
	X(CCPMU, 32, 0x80, "MCE Memory Access Error")

/* SCF_IOB Uncorrectable RAS ERROR */
#define SCF_IOB_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(SCF_IOB, 41, 0x99, "Request parity error")				\
	X(SCF_IOB, 40, 0x98, "Putdata parity error")				\
	X(SCF_IOB, 39, 0x97, "Uncorrectable ECC on Putdata")			\
	X(SCF_IOB, 38, 0x96, "CBB Interface Error")				\
	X(SCF_IOB, 37, 0x95, "MMCRAB Error")					\
	X(SCF_IOB, 36, 0x94, "IHI Interface Error")				\
	X(SCF_IOB, 35, 0x93, "CRI Error")					\
	X(SCF_IOB, 34, 0x92, "TBX Interface Error")				\
	X(SCF_IOB, 33, 0x91, "EVP Interface Error")

/* SCF_SNOC Uncorrectable RAS ERROR */
#define SCF_SNOC_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(SCF_SNOC, 42, 0xAA, "Misc Client Parity Error")			\
	X(SCF_SNOC, 41, 0xA9, "Misc Filldata Parity Error")			\
	X(SCF_SNOC, 40, 0xA8, "Uncorrectable ECC Misc Client")			\
	X(SCF_SNOC, 39, 0xA7, "DVMU Interface Parity Error")			\
	X(SCF_SNOC, 38, 0xA6, "DVMU Interface Timeout Error")			\
	X(SCF_SNOC, 37, 0xA5, "CPE Request Error")				\
	X(SCF_SNOC, 36, 0xA4, "CPE Response Error")				\
	X(SCF_SNOC, 35, 0xA3, "CPE Timeout Error")				\
	X(SCF_SNOC, 34, 0xA2, "Uncorrectable Carveout Error")

/* SCF_CTU Uncorrectable RAS ERROR */
#define SCF_CTU_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(SCF_CTU, 39, 0xB7, "Timeout error for TRC_DMA request")		\
	X(SCF_CTU, 38, 0xB6, "Timeout error for CTU Snp")			\
	X(SCF_CTU, 37, 0xB5, "Parity error in CTU TAG RAM")			\
	X(SCF_CTU, 36, 0xB3, "Parity error in CTU DATA RAM")			\
	X(SCF_CTU, 35, 0xB4, "Parity error for Cluster Rsp")			\
	X(SCF_CTU, 34, 0xB2, "Parity error for TRL requests from 9 agents")	\
	X(SCF_CTU, 33, 0xB1, "Parity error for MCF request")			\
	X(SCF_CTU, 32, 0xB0, "TRC DMA fillsnoop parity error")

/* CMU_CLOCKS Uncorrectable RAS ERROR */
#define CMU_CLOCKS_UNCORR_RAS_ERROR_LIST(X)					\
	/* Name, ERR_CTRL, IERR, ISA Desc */					\
	X(CMU_CLOCKS, 39, 0xC7, "Cluster 3 frequency monitor error")		\
	X(CMU_CLOCKS, 38, 0xC6, "Cluster 2 frequency monitor error")		\
	X(CMU_CLOCKS, 37, 0xC5, "Cluster 1 frequency monitor error")		\
	X(CMU_CLOCKS, 36, 0xC3, "Cluster 0 frequency monitor error")		\
	X(CMU_CLOCKS, 35, 0xC4, "Voltage error on ADC1 Monitored Logic")	\
	X(CMU_CLOCKS, 34, 0xC2, "Voltage error on ADC0 Monitored Logic")	\
	X(CMU_CLOCKS, 33, 0xC1, "Lookup Table 1 Parity Error")			\
	X(CMU_CLOCKS, 32, 0xC0, "Lookup Table 0 Parity Error")

/*
 * Define one ras_error entry.
 *
 * This macro wille be used to to generate ras_error records for each node
 * defined by <NODE_NAME>_UNCORR_RAS_ERROR_LIST macro.
 */
#define DEFINE_ONE_RAS_ERROR_MSG(unit, ras_bit, ierr, msg)			\
	{									\
		.error_msg = (msg),						\
		.error_code = (ierr)						\
	},

/*
 * Set one implementation defined bit in ERR<n>CTLR
 *
 * This macro will be used to collect all defined ERR_CTRL bits for each node
 * defined by <NODE_NAME>_UNCORR_RAS_ERROR_LIST macro.
 */
#define DEFINE_ENABLE_RAS_BIT(unit, ras_bit, ierr, msg)				\
	do {									\
		val |= (1ULL << ras_bit##U);					\
	} while (0);

/* Represent one RAS node with 0 or more error bits (ERR_CTLR) enabled */
#define DEFINE_ONE_RAS_NODE(node)						\
static const struct ras_error node##_uncorr_ras_errors[] = {			\
	node##_UNCORR_RAS_ERROR_LIST(DEFINE_ONE_RAS_ERROR_MSG)			\
	{									\
		NULL,								\
		0U								\
	},									\
};										\
static inline uint64_t node##_err_ctrl(void)					\
{										\
	uint64_t val = 0ULL;							\
	node##_UNCORR_RAS_ERROR_LIST(DEFINE_ENABLE_RAS_BIT)			\
	return val;								\
}

#define DEFINE_ONE_RAS_AUX_DATA(node)						\
	{									\
		.name = #node,							\
		.error_records = node##_uncorr_ras_errors,			\
		.err_ctrl = &node##_err_ctrl					\
	},

#define PER_CORE_RAS_NODE_LIST(X)						\
	X(IFU)									\
	X(JSR_RET)								\
	X(JSR_MTS)								\
	X(LSD_STQ)								\
	X(LSD_DCC)								\
	X(LSD_L1HPF)

#define PER_CORE_RAS_GROUP_NODES	PER_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

#define PER_CLUSTER_RAS_NODE_LIST(X)						\
	X(L2)									\
	X(CLUSTER_CLOCKS)							\
	X(MMU)

#define PER_CLUSTER_RAS_GROUP_NODES	PER_CLUSTER_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

#define SCF_L3_BANK_RAS_NODE_LIST(X)	X(L3)

/* we have 4 SCF_L3 nodes:3*256 + L3_Bank_ID(0-3) */
#define SCF_L3_BANK_RAS_GROUP_NODES						\
	SCF_L3_BANK_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)			\
	SCF_L3_BANK_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)			\
	SCF_L3_BANK_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)			\
	SCF_L3_BANK_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

#define CCPLEX_RAS_NODE_LIST(X)							\
	X(CCPMU)								\
	X(SCF_IOB)								\
	X(SCF_SNOC)								\
	X(SCF_CTU)								\
	X(CMU_CLOCKS)

#define CCPLEX_RAS_GROUP_NODES		CCPLEX_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

#endif /* TEGRA194_RAS_PRIVATE */
