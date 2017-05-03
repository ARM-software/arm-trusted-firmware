/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

