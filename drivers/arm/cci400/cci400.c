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

#include <arch.h>
#include <assert.h>
#include <cci400.h>
#include <debug.h>
#include <mmio.h>
#include <stdint.h>

#define MAX_CLUSTERS		2

static uintptr_t cci_base_addr;
static unsigned int cci_cluster_ix_to_iface[MAX_CLUSTERS];


void cci_init(uintptr_t cci_base,
		int slave_iface3_cluster_ix,
		int slave_iface4_cluster_ix)
{
	/*
	 * Check the passed arguments are valid. The cluster indices must be
	 * less than MAX_CLUSTERS, not the same as each other and at least one
	 * of them must refer to a valid cluster index.
	 */
	assert(cci_base);
	assert(slave_iface3_cluster_ix < MAX_CLUSTERS);
	assert(slave_iface4_cluster_ix < MAX_CLUSTERS);
	assert(slave_iface3_cluster_ix != slave_iface4_cluster_ix);
	assert((slave_iface3_cluster_ix >= 0) ||
		(slave_iface4_cluster_ix >= 0));

	WARN("Please migrate to common cci driver, This driver will be" \
		" deprecated in future\n");

	cci_base_addr = cci_base;
	if (slave_iface3_cluster_ix >= 0)
		cci_cluster_ix_to_iface[slave_iface3_cluster_ix] =
			SLAVE_IFACE3_OFFSET;
	if (slave_iface4_cluster_ix >= 0)
		cci_cluster_ix_to_iface[slave_iface4_cluster_ix] =
			SLAVE_IFACE4_OFFSET;
}

static inline unsigned long get_slave_iface_base(unsigned long mpidr)
{
	/*
	 * We assume the TF topology code allocates affinity instances
	 * consecutively from zero.
	 * It is a programming error if this is called without initializing
	 * the slave interface to use for this cluster.
	 */
	unsigned int cluster_id =
		(mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;

	assert(cluster_id < MAX_CLUSTERS);
	assert(cci_cluster_ix_to_iface[cluster_id] != 0);

	return cci_base_addr + cci_cluster_ix_to_iface[cluster_id];
}

void cci_enable_cluster_coherency(unsigned long mpidr)
{
	assert(cci_base_addr);
	/* Enable Snoops and DVM messages */
	mmio_write_32(get_slave_iface_base(mpidr) + SNOOP_CTRL_REG,
		      DVM_EN_BIT | SNOOP_EN_BIT);

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base_addr + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

void cci_disable_cluster_coherency(unsigned long mpidr)
{
	assert(cci_base_addr);
	/* Disable Snoops and DVM messages */
	mmio_write_32(get_slave_iface_base(mpidr) + SNOOP_CTRL_REG,
		      ~(DVM_EN_BIT | SNOOP_EN_BIT));

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base_addr + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

