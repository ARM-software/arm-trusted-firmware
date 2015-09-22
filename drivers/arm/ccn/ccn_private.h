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

#ifndef __CCN_PRIVATE_H__
#define __CCN_PRIVATE_H__

/*
 * A CCN implementation can have a maximum of 64 Request nodes with node IDs
 * from 0-63. These IDs are split across the three types of Request nodes
 * i.e. RN-F, RN-D and RN-I.
 */
#define MAX_RN_NODES		64

/* Enum used to loop through the 3 types of Request nodes */
typedef enum rn_types {
	RN_TYPE_RNF = 0,
	RN_TYPE_RNI,
	RN_TYPE_RND,
	NUM_RN_TYPES
} rn_types_t;

/* Macro to convert a region id to its base address */
#define region_id_to_base(id)	((id) << 16)

/*
 * Macro to calculate the number of master interfaces resident on a RN-I/RN-D.
 * Value of first two bits of the RN-I/D node type + 1 == Maximum number of
 * ACE-Lite or ACE-Lite+DVM interfaces supported on this node. E.g.
 *
 * 0x14 : RN-I with 1 ACE-Lite interface
 * 0x15 : RN-I with 2 ACE-Lite interfaces
 * 0x16 : RN-I with 3 ACE-Lite interfaces
 */
#define rn_type_id_to_master_cnt(id)	(((id) & 0x3) + 1)

/*
 * Constants used to identify a region in the programmer's view. These are
 * common for all regions.
 */
#define REGION_ID_LIMIT		256
#define REGION_ID_OFFSET	0xFF00

#define REGION_NODE_ID_SHIFT	8
#define REGION_NODE_ID_MASK	0x7f
#define get_node_id(id_reg)	(((id_reg) >> REGION_NODE_ID_SHIFT) \
				 & REGION_NODE_ID_MASK)

#define REGION_NODE_TYPE_SHIFT	0
#define REGION_NODE_TYPE_MASK	0x1f
#define get_node_type(id_reg)	(((id_reg) >> REGION_NODE_TYPE_SHIFT) \
				 & REGION_NODE_TYPE_MASK)

/* Common offsets of registers to enter or exit a snoop/dvm domain */
#define DOMAIN_CTRL_STAT_OFFSET	0x0200
#define DOMAIN_CTRL_SET_OFFSET	0x0210
#define DOMAIN_CTRL_CLR_OFFSET	0x0220

/*
 * Thess macros are used to determine if an operation to add or remove a Request
 * node from the snoop/dvm domain has completed. 'rn_id_map' is a bit map of
 * nodes. It was used to program the SET or CLEAR control register. The type of
 * register is specified by 'op_reg_offset'. 'status_reg' is the bit map of
 * nodes currently present in the snoop/dvm domain. 'rn_id_map' and 'status_reg'
 * are logically ANDed and the result it stored back in the 'status_reg'. There
 * are two outcomes of this operation:
 *
 * 1. If the DOMAIN_CTRL_SET_OFFSET register was programmed, then the set bits in
 *    'rn_id_map' should appear in 'status_reg' when the operation completes. So
 *    after the AND operation, at some point of time 'status_reg' should equal
 *    'rn_id_map'.
 *
 * 2. If the DOMAIN_CTRL_CLR_OFFSET register was programmed, then the set bits in
 *    'rn_id_map' should disappear in 'status_reg' when the operation
 *    completes. So after the AND operation, at some point of time 'status_reg'
 *    should equal 0.
 */
#define WAIT_FOR_DOMAIN_CTRL_OP_COMPLETION(region_id, stat_reg_offset,		\
					   op_reg_offset, rn_id_map)		\
	{									\
		uint64_t status_reg;						\
		do {								\
			status_reg = ccn_reg_read((ccn_plat_desc->periphbase),	\
						  (region_id),			\
						  (stat_reg_offset));		\
			status_reg &= (rn_id_map);				\
		} while ((op_reg_offset) == DOMAIN_CTRL_SET_OFFSET ?		\
			 (rn_id_map) != status_reg : status_reg);		\
	}

/*
 * Region ID of the Miscellaneous Node is always 0 as its located at the base of
 * the programmer's view.
 */
#define MN_REGION_ID		0

#define MN_REGION_ID_START	0
#define DEBUG_REGION_ID_START	1
#define HNI_REGION_ID_START	8
#define SBSX_REGION_ID_START	16
#define HNF_REGION_ID_START	32
#define XP_REGION_ID_START	64
#define RNI_REGION_ID_START	128

/* Selected register offsets from the base of a HNF region */
#define HNF_CFG_CTRL_OFFSET	0x0000
#define HNF_SAM_CTRL_OFFSET	0x0008
#define HNF_PSTATE_REQ_OFFSET	0x0010
#define HNF_PSTATE_STAT_OFFSET	0x0018
#define HNF_SDC_STAT_OFFSET	DOMAIN_CTRL_STAT_OFFSET
#define HNF_SDC_SET_OFFSET	DOMAIN_CTRL_SET_OFFSET
#define HNF_SDC_CLR_OFFSET	DOMAIN_CTRL_CLR_OFFSET
#define HNF_AUX_CTRL_OFFSET	0x0500

/* Selected register offsets from the base of a MN region */
#define MN_SAR_OFFSET		0x0000
#define MN_RNF_NODEID_OFFSET	0x0180
#define MN_RNI_NODEID_OFFSET	0x0190
#define MN_RND_NODEID_OFFSET	0x01A0
#define MN_HNF_NODEID_OFFSET	0x01B0
#define MN_HNI_NODEID_OFFSET	0x01C0
#define MN_SN_NODEID_OFFSET	0x01D0
#define MN_DDC_STAT_OFFSET	DOMAIN_CTRL_STAT_OFFSET
#define MN_DDC_SET_OFF		DOMAIN_CTRL_SET_OFFSET
#define MN_DDC_CLR_OFFSET	DOMAIN_CTRL_CLR_OFFSET
#define MN_ID_OFFSET		REGION_ID_OFFSET

/* HNF System Address Map register bit masks and shifts */
#define HNF_SAM_CTRL_SN_ID_MASK		0x7f
#define HNF_SAM_CTRL_SN0_ID_SHIFT	0
#define HNF_SAM_CTRL_SN1_ID_SHIFT	8
#define HNF_SAM_CTRL_SN2_ID_SHIFT	16

#define HNF_SAM_CTRL_TAB0_MASK		0x3fUL
#define HNF_SAM_CTRL_TAB0_SHIFT		48
#define HNF_SAM_CTRL_TAB1_MASK		0x3fUL
#define HNF_SAM_CTRL_TAB1_SHIFT		56

#define HNF_SAM_CTRL_3SN_ENB_SHIFT	32
#define HNF_SAM_CTRL_3SN_ENB_MASK	0x01UL

/*
 * Macro to create a value suitable for programming into a HNF SAM Control
 * register for enabling 3SN striping.
 */
#define MAKE_HNF_SAM_CTRL_VALUE(sn0, sn1, sn2, tab0, tab1, three_sn_en)     \
	((((sn0) & HNF_SAM_CTRL_SN_ID_MASK) << HNF_SAM_CTRL_SN0_ID_SHIFT) | \
	 (((sn1) & HNF_SAM_CTRL_SN_ID_MASK) << HNF_SAM_CTRL_SN1_ID_SHIFT) | \
	 (((sn2) & HNF_SAM_CTRL_SN_ID_MASK) << HNF_SAM_CTRL_SN2_ID_SHIFT) | \
	 (((tab0) & HNF_SAM_CTRL_TAB0_MASK) << HNF_SAM_CTRL_TAB0_SHIFT)   | \
	 (((tab1) & HNF_SAM_CTRL_TAB1_MASK) << HNF_SAM_CTRL_TAB1_SHIFT)   | \
	 (((three_sn_en) & HNF_SAM_CTRL_3SN_ENB_MASK) << HNF_SAM_CTRL_3SN_ENB_SHIFT))

/* Mask to read the power state value from an HN-F P-state register */
#define HNF_PSTATE_MASK		0xf

/* Macro to extract the run mode from a p-state value */
#define PSTATE_TO_RUN_MODE(pstate)	(((pstate) & HNF_PSTATE_MASK) >> 2)

/*
 * Helper macro that iterates through a given bit map. In each iteration,
 * it returns the position of the set bit.
 * It can be used by other utility macros to iterates through all nodes
 * or masters given a bit map of them.
 */
#define FOR_EACH_BIT(bit_pos, bit_map)			\
	for (bit_pos = __builtin_ctzll(bit_map);	\
	     bit_map;					\
	     bit_map &= ~(1UL << bit_pos),		\
	     bit_pos = __builtin_ctzll(bit_map))

/*
 * Utility macro that iterates through a bit map of node IDs. In each
 * iteration, it returns the ID of the next present node in the bit map. Node
 * ID of a present node == Position of set bit == Number of zeroes trailing the
 * bit.
 */
#define FOR_EACH_PRESENT_NODE_ID(node_id, bit_map)	\
		FOR_EACH_BIT(node_id, bit_map)

/*
 * Helper function to return number of set bits in bitmap
 */
static inline unsigned int count_set_bits(uint64_t bitmap)
{
	unsigned int count = 0;

	for (; bitmap; bitmap &= bitmap - 1)
		++count;

	return count;
}

/*
 * Utility macro that iterates through a bit map of node IDs. In each iteration,
 * it returns the ID of the next present region corresponding to a node present
 * in the bit map. Region ID of a present node is in between passed region id
 * and region id + number of set bits in the bitmap i.e. the number of present
 * nodes.
 */
#define FOR_EACH_PRESENT_REGION_ID(region_id, bit_map)				\
	for (unsigned long long region_id_limit = count_set_bits(bit_map)	\
							+ region_id;		\
	    region_id < region_id_limit;					\
	    region_id++)

/*
 * Same macro as FOR_EACH_PRESENT_NODE, but renamed to indicate it traverses
 * through a bit map of master interfaces.
 */
#define FOR_EACH_PRESENT_MASTER_INTERFACE(iface_id, bit_map)	\
			FOR_EACH_BIT(iface_id, bit_map)
#endif /* __CCN_PRIVATE_H__ */
