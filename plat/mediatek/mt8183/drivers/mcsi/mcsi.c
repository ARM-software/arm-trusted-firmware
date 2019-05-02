/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <scu.h>
#include <mcucfg.h>
#include <drivers/delay_timer.h>
#include <mcsi/mcsi.h>

#define MAX_CLUSTERS		5

static unsigned long cci_base_addr;
static unsigned int cci_cluster_ix_to_iface[MAX_CLUSTERS];

void mcsi_init(unsigned long cci_base,
		unsigned int num_cci_masters)
{
	int i;

	assert(cci_base);
	assert(num_cci_masters < MAX_CLUSTERS);

	cci_base_addr = cci_base;

	for (i = 0; i < num_cci_masters; i++)
		cci_cluster_ix_to_iface[i] = SLAVE_IFACE_OFFSET(i);
}

void mcsi_cache_flush(void)
{
	/* timeout is 10ms */
	int timeout = 10000;

	/* to make flush by SF safe, need to disable BIU DCM */
	mmio_clrbits_32(CCI_CLK_CTRL, 1 << 8);
	mmio_write_32(cci_base_addr + FLUSH_SF, 0x1);

	for (; timeout; timeout--, udelay(1)) {
		if ((mmio_read_32(cci_base_addr + FLUSH_SF) & 0x1) == 0x0)
			break;
	}

	if (!timeout) {
		INFO("SF lush timeout\n");
		return;
	}

	/* enable BIU DCM as it was */
	mmio_setbits_32(CCI_CLK_CTRL, 1 << 8);
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
	unsigned long slave_base;
	unsigned int support_ability;
	unsigned int config = 0;
	unsigned int pending = 0;

	assert(cci_base_addr);
	slave_base  = get_slave_iface_base(mpidr);
	support_ability = mmio_read_32(slave_base);

	pending = (mmio_read_32(
		   cci_base_addr + SNP_PENDING_REG)) >> SNP_PENDING;
	while (pending) {
		pending = (mmio_read_32(
			   cci_base_addr + SNP_PENDING_REG)) >> SNP_PENDING;
	}

	if (support_ability & SNP_SUPPORT)
		config |= SNOOP_EN_BIT;
	if (support_ability & DVM_SUPPORT)
		config |= DVM_EN_BIT;

	mmio_write_32(slave_base, support_ability | config);

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING)
		;
}

#if ERRATA_MCSIB_SW
#pragma weak mcsib_sw_workaround_main
#endif

void cci_disable_cluster_coherency(unsigned long mpidr)
{
	unsigned long slave_base;
	unsigned int config = 0;

	assert(cci_base_addr);
	slave_base = get_slave_iface_base(mpidr);

	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING)
		;

	config = mmio_read_32(slave_base);
	config &= ~(DVM_EN_BIT | SNOOP_EN_BIT);

	/* Disable Snoops and DVM messages */
	mmio_write_32(slave_base, config);

#if ERRATA_MCSIB_SW
	mcsib_sw_workaround_main();
#endif

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING)
		;
}

void cci_secure_switch(unsigned int status)
{
	unsigned int config;

	config = mmio_read_32(cci_base_addr + CENTRAL_CTRL_REG);
	if (status == NS_ACC)
		config |= SECURE_ACC_EN;
	else
		config &= ~SECURE_ACC_EN;
	mmio_write_32(cci_base_addr + CENTRAL_CTRL_REG, config);
}

void cci_pmu_secure_switch(unsigned int status)
{
	unsigned int config;

	config = mmio_read_32(cci_base_addr + CENTRAL_CTRL_REG);
	if (status == NS_ACC)
		config |= PMU_SECURE_ACC_EN;
	else
		config &= ~PMU_SECURE_ACC_EN;
	mmio_write_32(cci_base_addr + CENTRAL_CTRL_REG, config);
}

void cci_init_sf(void)
{
	while (mmio_read_32(cci_base_addr + SNP_PENDING_REG) >> SNP_PENDING)
		;
	/* init sf1 */
	mmio_write_32(cci_base_addr + SF_INIT_REG, TRIG_SF1_INIT);
	while (mmio_read_32(cci_base_addr + SF_INIT_REG) & TRIG_SF1_INIT)
		;
	while (!(mmio_read_32(cci_base_addr + SF_INIT_REG) & SF1_INIT_DONE))
		;
	/* init sf2 */
	mmio_write_32(cci_base_addr + SF_INIT_REG, TRIG_SF2_INIT);
	while (mmio_read_32(cci_base_addr + SF_INIT_REG) & TRIG_SF2_INIT)
		;
	while (!(mmio_read_32(cci_base_addr + SF_INIT_REG) & SF2_INIT_DONE))
		;
}

void cci_interrupt_en(void)
{
	mmio_setbits_32(cci_base_addr + CENTRAL_CTRL_REG, INT_EN);
}

unsigned long cci_reg_access(unsigned int op, unsigned long offset,
			     unsigned long val)
{
	unsigned long ret = 0;

	if ((cci_base_addr == 0) || (offset > MSCI_MEMORY_SZ))
		panic();

	switch (op) {
	case MCSI_REG_ACCESS_READ:
		ret = mmio_read_32(cci_base_addr + offset);
		break;
	case MCSI_REG_ACCESS_WRITE:
		mmio_write_32(cci_base_addr + offset, val);
		dsb();
		break;
	case MCSI_REG_ACCESS_SET_BITMASK:
		mmio_setbits_32(cci_base_addr + offset, val);
		dsb();
		break;
	case MCSI_REG_ACCESS_CLEAR_BITMASK:
		mmio_clrbits_32(cci_base_addr + offset, val);
		dsb();
		break;
	default:
		break;
	}
	return ret;
}
