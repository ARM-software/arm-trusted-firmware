/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CCN_H__
#define __CCN_H__

/*
 * This macro defines the maximum number of master interfaces that reside on
 * Request nodes which the CCN driver can accommodate. The driver APIs to add
 * and remove Request nodes from snoop/dvm domains take a bit map of master
 * interfaces as inputs. The largest C data type that can be used is a 64-bit
 * unsigned integer. Hence the value of 64. The platform will have to ensure
 * that the master interfaces are numbered from 0-63.
 */
#define CCN_MAX_RN_MASTERS	64

/*
 * The following constants define the various run modes that the platform can
 * request the CCN driver to place the L3 cache in. These map to the
 * programmable P-State values in a HN-F P-state register.
 */
#define CCN_L3_RUN_MODE_NOL3	0x0	/* HNF_PM_NOL3 */
#define CCN_L3_RUN_MODE_SFONLY	0x1	/* HNF_PM_SFONLY */
#define CCN_L3_RUN_MODE_HAM	0x2	/* HNF_PM_HALF */
#define CCN_L3_RUN_MODE_FAM	0x3	/* HNF_PM_FULL */

/* part 0 IDs for various CCN variants */
#define CCN_502_PART0_ID	0x30
#define CCN_504_PART0_ID	0x26
#define CCN_505_PART0_ID	0x27
#define CCN_508_PART0_ID	0x28
#define CCN_512_PART0_ID	0x29

/*
 * The following macro takes the value returned from a read of a HN-F P-state
 * status register and returns the retention state value.
 */
#define CCN_GET_RETENTION_STATE(pstate)	((pstate >> 4) & 0x3)

/*
 * The following macro takes the value returned from a read of a HN-F P-state
 * status register and returns the run state value.
 */
#define CCN_GET_RUN_STATE(pstate)	(pstate & 0xf)

#ifndef __ASSEMBLY__
#include <stdint.h>

/*
 * This structure describes some of the implementation defined attributes of the
 * CCN IP. It is used by the platform port to specify these attributes in order
 * to initialise the CCN driver. The attributes are described below.
 *
 * 1. The 'num_masters' field specifies the total number of master interfaces
 *    resident on Request nodes.
 *
 * 2. The 'master_to_rn_id_map' field is a ponter to an array in which each
 *    index corresponds to a master interface and its value corresponds to the
 *    Request node on which the master interface resides.
 *    This field is not simply defined as an array of size CCN_MAX_RN_MASTERS.
 *    In reality, a platform will have much fewer master * interfaces than
 *    CCN_MAX_RN_MASTERS. With an array of this size, it would also have to
 *    set the unused entries to a suitable value. Zeroing the array would not
 *    be enough since 0 is also a valid node id. Hence, such an array is not
 *    used.
 *
 * 3. The 'periphbase' field is the base address of the programmer's view of the
 *    CCN IP.
 */
typedef struct ccn_desc {
	unsigned int num_masters;
	const unsigned char *master_to_rn_id_map;
	uintptr_t periphbase;
} ccn_desc_t;


void ccn_init(const ccn_desc_t *plat_ccn_desc);
void ccn_enter_snoop_dvm_domain(unsigned long long master_iface_map);
void ccn_exit_snoop_dvm_domain(unsigned long long master_iface_map);
void ccn_enter_dvm_domain(unsigned long long master_iface_map);
void ccn_exit_dvm_domain(unsigned long long master_iface_map);
void ccn_set_l3_run_mode(unsigned int mode);
void ccn_program_sys_addrmap(unsigned int sn0_id,
		 unsigned int sn1_id,
		 unsigned int sn2_id,
		 unsigned int top_addr_bit0,
		 unsigned int top_addr_bit1,
		 unsigned char three_sn_en);
unsigned int ccn_get_l3_run_mode(void);
int ccn_get_part0_id(uintptr_t periphbase);

#endif /* __ASSEMBLY__ */
#endif /* __CCN_H__ */
