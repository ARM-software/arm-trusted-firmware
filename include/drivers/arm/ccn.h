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

#endif /* __ASSEMBLY__ */
#endif /* __CCN_H__ */
