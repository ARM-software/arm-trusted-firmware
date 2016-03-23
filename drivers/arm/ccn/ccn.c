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

#include <arch.h>
#include <assert.h>
#include <bakery_lock.h>
#include <ccn.h>
#include <debug.h>
#include <errno.h>
#include <mmio.h>
#include "ccn_private.h"

static const ccn_desc_t *ccn_plat_desc;
#if IMAGE_BL31
DEFINE_BAKERY_LOCK(ccn_lock);
#endif

/*******************************************************************************
 * This function takes the base address of the CCN's programmer's view (PV), a
 * region ID of one of the 256 regions (0-255) and a register offset within the
 * region. It converts the first two parameters into a base address and uses it
 * to read the register at the offset.
 ******************************************************************************/
static inline unsigned long long ccn_reg_read(uintptr_t periphbase,
			     unsigned int region_id,
			     unsigned int register_offset)
{
	uintptr_t region_base;

	assert(periphbase);
	assert(region_id < REGION_ID_LIMIT);

	region_base = periphbase + region_id_to_base(region_id);
	return mmio_read_64(region_base + register_offset);
}

/*******************************************************************************
 * This function takes the base address of the CCN's programmer's view (PV), a
 * region ID of one of the 256 regions (0-255), a register offset within the
 * region and a value. It converts the first two parameters into a base address
 * and uses it to write the value in the register at the offset.
 ******************************************************************************/
static inline void ccn_reg_write(uintptr_t periphbase,
			  unsigned int region_id,
			  unsigned int register_offset,
			  unsigned long long value)
{
	uintptr_t region_base;

	assert(periphbase);
	assert(region_id < REGION_ID_LIMIT);

	region_base = periphbase + region_id_to_base(region_id);
	mmio_write_64(region_base + register_offset, value);
}

#if DEBUG

typedef struct rn_info {
		unsigned char node_desc[MAX_RN_NODES];
	} rn_info_t;

/*******************************************************************************
 * This function takes the base address of the CCN's programmer's view (PV) and
 * the node ID of a Request Node (RN-D or RN-I). It returns the maximum number
 * of master interfaces resident on that node. This number is equal to the least
 * significant two bits of the node type ID + 1.
 ******************************************************************************/
static unsigned int ccn_get_rni_mcount(uintptr_t periphbase,
				       unsigned int rn_id)
{
	unsigned int rn_type_id;

	/* Use the node id to find the type of RN-I/D node */
	rn_type_id = get_node_type(ccn_reg_read(periphbase,
						rn_id + RNI_REGION_ID_START,
						REGION_ID_OFFSET));

	/* Return the number master interfaces based on node type */
	return rn_type_id_to_master_cnt(rn_type_id);
}

/*******************************************************************************
 * This function reads the CCN registers to find the following information about
 * the ACE/ACELite/ACELite+DVM/CHI interfaces resident on the various types of
 * Request Nodes (RN-Fs, RN-Is and RN-Ds) in the system:
 *
 * 1. The total number of such interfaces that this CCN IP supports. This is the
 *    cumulative number of interfaces across all Request node types. It is
 *    passed back as the return value of this function.
 *
 * 2. The maximum number of interfaces of a type resident on a Request node of
 *    one of the three types. This information is populated in the 'info'
 *    array provided by the caller as described next.
 *
 *    The array has 64 entries. Each entry corresponds to a Request node. The
 *    Miscellaneous node's programmer's view has RN-F, RN-I and RN-D ID
 *    registers. For each RN-I and RN-D ID indicated as being present in these
 *    registers, its identification register (offset 0xFF00) is read. This
 *    register specifies the maximum number of master interfaces the node
 *    supports. For RN-Fs it is assumed that there can be only a single fully
 *    coherent master resident on each node. The counts for each type of node
 *    are use to populate the array entry at the index corresponding to the node
 *    ID i.e. rn_info[node ID] = <number of master interfaces>
 ******************************************************************************/
static unsigned int ccn_get_rn_master_info(uintptr_t periphbase,
					   rn_info_t *info)
{
	unsigned int num_masters = 0;
	rn_types_t rn_type;

	assert (info);

	for (rn_type = RN_TYPE_RNF; rn_type < NUM_RN_TYPES; rn_type++) {
		unsigned int mn_reg_off, node_id;
		unsigned long long rn_bitmap;

		/*
		 * RN-F, RN-I, RN-D node registers in the MN region occupy
		 * contiguous 16 byte apart offsets.
		 */
		mn_reg_off = MN_RNF_NODEID_OFFSET + (rn_type << 4);
		rn_bitmap = ccn_reg_read(periphbase, MN_REGION_ID, mn_reg_off);

		FOR_EACH_PRESENT_NODE_ID(node_id, rn_bitmap) {
			unsigned int node_mcount;

			/*
			 * A RN-F does not have a node type since it does not
			 * export a programmer's interface. It can only have a
			 * single fully coherent master residing on it. If the
			 * offset of the MN(Miscellaneous Node) register points
			 * to a RN-I/D node then the master count is set to the
			 * maximum number of master interfaces that can possibly
			 * reside on the node.
			 */
			node_mcount = (mn_reg_off == MN_RNF_NODEID_OFFSET ? 1 :
				       ccn_get_rni_mcount(periphbase, node_id));

			/*
			 * Use this value to increment the maximum possible
			 * master interfaces in the system.
			 */
			num_masters += node_mcount;

			/*
			 * Update the entry in 'info' for this node ID with
			 * the maximum number of masters than can sit on
			 * it. This information will be used to validate the
			 * node information passed by the platform later.
			 */
			info->node_desc[node_id] = node_mcount;
		}
	}

	return num_masters;
}

/*******************************************************************************
 * This function validates parameters passed by the platform (in a debug build).
 * It collects information about the maximum number of master interfaces that:
 * a) the CCN IP can accommodate and
 * b) can exist on each Request node.
 * It compares this with the information provided by the platform to determine
 * the validity of the latter.
 ******************************************************************************/
static void ccn_validate_plat_params(const ccn_desc_t *plat_desc)
{
	unsigned int master_id, num_rn_masters;
	rn_info_t info = { {0} };

	assert(plat_desc);
	assert(plat_desc->periphbase);
	assert(plat_desc->master_to_rn_id_map);
	assert(plat_desc->num_masters);
	assert(plat_desc->num_masters < CCN_MAX_RN_MASTERS);

	/*
	 * Find the number and properties of fully coherent, IO coherent and IO
	 * coherent + DVM master interfaces
	 */
	num_rn_masters = ccn_get_rn_master_info(plat_desc->periphbase, &info);
	assert(plat_desc->num_masters < num_rn_masters);

	/*
	 * Iterate through the Request nodes specified by the platform.
	 * Decrement the count of the masters in the 'info' array for each
	 * Request node encountered. If the count would drop below 0 then the
	 * platform's view of this aspect of CCN configuration is incorrect.
	 */
	for (master_id = 0; master_id < plat_desc->num_masters; master_id++) {
		unsigned int node_id;

		node_id = plat_desc->master_to_rn_id_map[master_id];
		assert(node_id < MAX_RN_NODES);
		assert(info.node_desc[node_id]);
		info.node_desc[node_id]--;
	}
}
#endif /* DEBUG */

/*******************************************************************************
 * This function validates parameters passed by the platform (in a debug build)
 * and initialises its internal data structures. A lock is required to prevent
 * simultaneous CCN operations at runtime (only BL31) to add and remove Request
 * nodes from coherency.
 ******************************************************************************/
void ccn_init(const ccn_desc_t *plat_desc)
{
#if DEBUG
	ccn_validate_plat_params(plat_desc);
#endif

	ccn_plat_desc = plat_desc;
}

/*******************************************************************************
 * This function converts a bit map of master interface IDs to a bit map of the
 * Request node IDs that they reside on.
 ******************************************************************************/
static unsigned long long ccn_master_to_rn_id_map(unsigned long long master_map)
{
	unsigned long long rn_id_map = 0;
	unsigned int node_id, iface_id;

	assert(master_map);
	assert(ccn_plat_desc);

	FOR_EACH_PRESENT_MASTER_INTERFACE(iface_id, master_map) {
		assert(iface_id < ccn_plat_desc->num_masters);

		/* Convert the master ID into the node ID */
		node_id = ccn_plat_desc->master_to_rn_id_map[iface_id];

		/* Set the bit corresponding to this node ID */
		rn_id_map |= (1UL << node_id);
	}

	return rn_id_map;
}

/*******************************************************************************
 * This function executes the necessary operations to add or remove Request node
 * IDs specified in the 'rn_id_map' bitmap from the snoop/DVM domains specified
 * in the 'hn_id_map'. The 'region_id' specifies the ID of the first HN-F/MN
 * on which the operation should be performed. 'op_reg_offset' specifies the
 * type of operation (add/remove). 'stat_reg_offset' specifies the register
 * which should be polled to determine if the operation has completed or not.
 ******************************************************************************/
static void ccn_snoop_dvm_do_op(unsigned long long rn_id_map,
				unsigned long long hn_id_map,
				unsigned int region_id,
				unsigned int op_reg_offset,
				unsigned int stat_reg_offset)
{
	unsigned int start_region_id;

	assert(ccn_plat_desc);
	assert(ccn_plat_desc->periphbase);

#if IMAGE_BL31
	bakery_lock_get(&ccn_lock);
#endif
	start_region_id = region_id;
	FOR_EACH_PRESENT_REGION_ID(start_region_id, hn_id_map) {
		ccn_reg_write(ccn_plat_desc->periphbase,
			      start_region_id,
			      op_reg_offset,
			      rn_id_map);
	}

	start_region_id = region_id;

	FOR_EACH_PRESENT_REGION_ID(start_region_id, hn_id_map) {
		WAIT_FOR_DOMAIN_CTRL_OP_COMPLETION(start_region_id,
						   stat_reg_offset,
						   op_reg_offset,
						   rn_id_map);
	}

#if IMAGE_BL31
	bakery_lock_release(&ccn_lock);
#endif
}

/*******************************************************************************
 * The following functions provide the boot and runtime API to the platform for
 * adding and removing master interfaces from the snoop/DVM domains. A bitmap of
 * master interfaces IDs is passed as a parameter. It is converted into a bitmap
 * of Request node IDs using the mapping provided by the platform while
 * initialising the driver.
 * For example, consider a dual cluster system where the clusters have values 0
 * & 1 in the affinity level 1 field of their respective MPIDRs. While
 * initialising this driver, the platform provides the mapping between each
 * cluster and the corresponding Request node. To add or remove a cluster from
 * the snoop and dvm domain, the bit position corresponding to the cluster ID
 * should be set in the 'master_iface_map' i.e. to remove both clusters the
 * bitmap would equal 0x11.
 ******************************************************************************/
void ccn_enter_snoop_dvm_domain(unsigned long long master_iface_map)
{
	unsigned long long rn_id_map;

	rn_id_map = ccn_master_to_rn_id_map(master_iface_map);
	ccn_snoop_dvm_do_op(rn_id_map,
			    CCN_GET_HN_NODEID_MAP(ccn_plat_desc->periphbase,
						  MN_HNF_NODEID_OFFSET),
			    HNF_REGION_ID_START,
			    HNF_SDC_SET_OFFSET,
			    HNF_SDC_STAT_OFFSET);

	ccn_snoop_dvm_do_op(rn_id_map,
			    CCN_GET_MN_NODEID_MAP(ccn_plat_desc->periphbase),
			    MN_REGION_ID,
			    MN_DDC_SET_OFFSET,
			    MN_DDC_STAT_OFFSET);
}

void ccn_exit_snoop_dvm_domain(unsigned long long master_iface_map)
{
	unsigned long long rn_id_map;

	rn_id_map = ccn_master_to_rn_id_map(master_iface_map);
	ccn_snoop_dvm_do_op(rn_id_map,
			    CCN_GET_HN_NODEID_MAP(ccn_plat_desc->periphbase,
						  MN_HNF_NODEID_OFFSET),
			    HNF_REGION_ID_START,
			    HNF_SDC_CLR_OFFSET,
			    HNF_SDC_STAT_OFFSET);

	ccn_snoop_dvm_do_op(rn_id_map,
			    CCN_GET_MN_NODEID_MAP(ccn_plat_desc->periphbase),
			    MN_REGION_ID,
			    MN_DDC_CLR_OFFSET,
			    MN_DDC_STAT_OFFSET);
}

void ccn_enter_dvm_domain(unsigned long long master_iface_map)
{
	unsigned long long rn_id_map;

	rn_id_map = ccn_master_to_rn_id_map(master_iface_map);
	ccn_snoop_dvm_do_op(rn_id_map,
			    CCN_GET_MN_NODEID_MAP(ccn_plat_desc->periphbase),
			    MN_REGION_ID,
			    MN_DDC_SET_OFFSET,
			    MN_DDC_STAT_OFFSET);
}

void ccn_exit_dvm_domain(unsigned long long master_iface_map)
{
	unsigned long long rn_id_map;

	rn_id_map = ccn_master_to_rn_id_map(master_iface_map);
	ccn_snoop_dvm_do_op(rn_id_map,
			    CCN_GET_MN_NODEID_MAP(ccn_plat_desc->periphbase),
			    MN_REGION_ID,
			    MN_DDC_CLR_OFFSET,
			    MN_DDC_STAT_OFFSET);
}

/*******************************************************************************
 * This function returns the run mode of all the L3 cache partitions in the
 * system. The state is expected to be one of NO_L3, SF_ONLY, L3_HAM or
 * L3_FAM. Instead of comparing the states reported by all HN-Fs, the state of
 * the first present HN-F node is reported. Since the driver does not export an
 * interface to program them seperately, there is no reason to perform this
 * check. An HN-F could report that the L3 cache is transitioning from one mode
 * to another e.g. HNF_PM_NOL3_2_SFONLY. In this case, the function waits for
 * the transition to complete and reports the final state.
 ******************************************************************************/
unsigned int ccn_get_l3_run_mode(void)
{
	unsigned long long hnf_pstate_stat;

	assert(ccn_plat_desc);
	assert(ccn_plat_desc->periphbase);

	/*
	 * Wait for a L3 cache paritition to enter any run mode. The pstate
	 * parameter is read from an HN-F P-state status register. A non-zero
	 * value in bits[1:0] means that the cache is transitioning to a run
	 * mode.
	 */
	do {
		hnf_pstate_stat = ccn_reg_read(ccn_plat_desc->periphbase,
					       HNF_REGION_ID_START,
					       HNF_PSTATE_STAT_OFFSET);
	} while (hnf_pstate_stat & 0x3);

	return PSTATE_TO_RUN_MODE(hnf_pstate_stat);
}

/*******************************************************************************
 * This function sets the run mode of all the L3 cache partitions in the
 * system to one of NO_L3, SF_ONLY, L3_HAM or L3_FAM depending upon the state
 * specified by the 'mode' argument.
 ******************************************************************************/
void ccn_set_l3_run_mode(unsigned int mode)
{
	unsigned long long mn_hnf_id_map, hnf_pstate_stat;
	unsigned int region_id;

	assert(ccn_plat_desc);
	assert(ccn_plat_desc->periphbase);
	assert(mode <= CCN_L3_RUN_MODE_FAM);

	mn_hnf_id_map = ccn_reg_read(ccn_plat_desc->periphbase,
				     MN_REGION_ID,
				     MN_HNF_NODEID_OFFSET);
	region_id = HNF_REGION_ID_START;

	/* Program the desired run mode */
	FOR_EACH_PRESENT_REGION_ID(region_id, mn_hnf_id_map) {
		ccn_reg_write(ccn_plat_desc->periphbase,
			      region_id,
			      HNF_PSTATE_REQ_OFFSET,
			      mode);
	}

	/* Wait for the caches to transition to the run mode */
	region_id = HNF_REGION_ID_START;
	FOR_EACH_PRESENT_REGION_ID(region_id, mn_hnf_id_map) {
		/*
		 * Wait for a L3 cache paritition to enter a target run
		 * mode. The pstate parameter is read from an HN-F P-state
		 * status register.
		 */
		do {
			hnf_pstate_stat = ccn_reg_read(ccn_plat_desc->periphbase,
					       region_id,
					       HNF_PSTATE_STAT_OFFSET);
		} while (((hnf_pstate_stat & HNF_PSTATE_MASK) >> 2) != mode);
	}
}

/*******************************************************************************
 * This function configures system address map and provides option to enable the
 * 3SN striping mode of Slave node operation. The Slave node IDs and the Top
 * Address bit1 and bit0 are provided as parameters to this function. This
 * configuration is needed only if network contains a single SN-F or 3 SN-F and
 * must be completed before the first request by the system to normal memory.
 ******************************************************************************/
void ccn_program_sys_addrmap(unsigned int sn0_id,
		 unsigned int sn1_id,
		 unsigned int sn2_id,
		 unsigned int top_addr_bit0,
		 unsigned int top_addr_bit1,
		 unsigned char three_sn_en)
{
	unsigned long long mn_hnf_id_map, hnf_sam_ctrl_value;
	unsigned int region_id;

	assert(ccn_plat_desc);
	assert(ccn_plat_desc->periphbase);

	mn_hnf_id_map = ccn_reg_read(ccn_plat_desc->periphbase,
				     MN_REGION_ID,
				     MN_HNF_NODEID_OFFSET);
	region_id = HNF_REGION_ID_START;
	hnf_sam_ctrl_value = MAKE_HNF_SAM_CTRL_VALUE(sn0_id,
						     sn1_id,
						     sn2_id,
						     top_addr_bit0,
						     top_addr_bit1,
						     three_sn_en);

	FOR_EACH_PRESENT_REGION_ID(region_id, mn_hnf_id_map) {

		/* Program the SAM control register */
		ccn_reg_write(ccn_plat_desc->periphbase,
			      region_id,
			      HNF_SAM_CTRL_OFFSET,
			      hnf_sam_ctrl_value);
	}

}

/*******************************************************************************
 * This function returns the part0 id from the peripheralID 0 register
 * in CCN. This id can be used to distinguish the CCN variant present in the
 * system.
 ******************************************************************************/
int ccn_get_part0_id(uintptr_t periphbase)
{
	assert(periphbase);
	return (int)(mmio_read_64(periphbase
			+ MN_PERIPH_ID_0_1_OFFSET) & 0xFF);
}
