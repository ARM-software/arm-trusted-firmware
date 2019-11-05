/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <plat_marvell.h>

/* The power domain tree descriptor */
unsigned char marvell_power_domain_tree_desc[PLAT_MARVELL_CLUSTER_COUNT + 1];

/*****************************************************************************
 * This function dynamically constructs the topology according to
 * PLAT_MARVELL_CLUSTER_COUNT and returns it.
 *****************************************************************************
 */
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	int i;

	/*
	 * The power domain tree does not have a single system level power
	 * domain i.e. a single root node. The first entry in the power domain
	 * descriptor specifies the number of power domains at the highest power
	 * level.
	 * For Marvell Platform this is the number of cluster power domains.
	 */
	marvell_power_domain_tree_desc[0] = PLAT_MARVELL_CLUSTER_COUNT;

	for (i = 0; i < PLAT_MARVELL_CLUSTER_COUNT; i++)
		marvell_power_domain_tree_desc[i + 1] =
					PLAT_MARVELL_CLUSTER_CORE_COUNT;

	return marvell_power_domain_tree_desc;
}

/*****************************************************************************
 * This function validates an MPIDR by checking whether it falls within the
 * acceptable bounds. An error code (-1) is returned if an incorrect mpidr
 * is passed.
 *****************************************************************************
 */
int marvell_check_mpidr(u_register_t mpidr)
{
	unsigned int nb_id, cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK |
	    MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT))
		return -1;

	/* Get north bridge ID */
	nb_id = MPIDR_AFFLVL3_VAL(mpidr);
	cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (nb_id >= PLAT_MARVELL_CLUSTER_COUNT)
		return -1;

	if (cluster_id >= PLAT_MARVELL_CLUSTER_COUNT)
		return -1;

	if (cpu_id >= PLAT_MARVELL_CLUSTER_CORE_COUNT)
		return -1;

	return 0;
}

/*****************************************************************************
 * This function implements a part of the critical interface between the PSCI
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 *****************************************************************************
 */
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	if (marvell_check_mpidr(mpidr) == -1)
		return -1;

	return plat_marvell_calc_core_pos(mpidr);
}
