/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <thunder_common.h>
#include <thunder_private.h>

int thunder_get_lmc_per_node(void)
{
	return 1;
}

int thunder_get_num_ecams_per_node(void)
{
	return 1;
}

int thunder_get_sata_count(void)
{
	return 2;
}

/* Return the highest GSER number, which can be configured as SATA */
int thunder_get_max_sata_gser(void)
{
	return 6;
}

/*
 * SATA to GSER mapping
 * SATA(0-1) --- GSER3
 */
int thunder_sata_to_gser(int ctrlr)
{
	if (ctrlr > 1)
		return -1;

	return 3;
}

int thunder_sata_to_lane(int ctrlr)
{
	if (ctrlr > 1)
		return -1;

	return ctrlr % 2;
}

void plat_add_mmio_node(unsigned long node)
{
	unsigned long attr;
	int i;

	attr = MT_DEVICE | MT_RW | MT_SECURE;
	add_map_record(CSR_PA(node, CAVM_RST_PF_BAR0), CAVM_RST_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_L2C_PF_BAR0), CAVM_L2C_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_MIO_BOOT_PF_BAR0), CAVM_MIO_BOOT_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_MIO_EMM_PF_BAR0), CAVM_MIO_EMM_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_MIO_FUS_BAR_E_MIO_FUS_PF_BAR0),
		       CAVM_MIO_FUS_BAR_E_MIO_FUS_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_MPI_PF_BAR0), CAVM_MPI_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_GIC_PF_BAR2), CAVM_GIC_PF_BAR2_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_GIC_PF_BAR4), CAVM_GIC_PF_BAR4_SIZE, attr);
#if TRUSTED_BOARD_BOOT
	add_map_record(CSR_PA(node, CAVM_KEY_BAR_E_KEY_PF_BAR2),
		       CAVM_KEY_BAR_E_KEY_PF_BAR2_SIZE, (MT_DEVICE | MT_RO | MT_SECURE));
	add_map_record(CSR_PA(node, CAVM_FUSF_BAR_E_FUSF_PF_BAR0),
		       CAVM_FUSF_BAR_E_FUSF_PF_BAR0_SIZE, (MT_DEVICE | MT_RO | MT_SECURE));
#endif


	add_map_record(CSR_PA(node, CAVM_SMMUX_PF_BAR0(0)), CAVM_SMMUX_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_SMMU_BAR_E_SMMUX_PF_BAR4(0)),
		       CAVM_SMMU_BAR_E_SMMUX_PF_BAR4_SIZE, attr);

	add_map_record(CSR_PA(node, CAVM_GTI_PF_BAR0), CAVM_GTI_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_GTI_PF_BAR4), CAVM_GTI_PF_BAR4_SIZE, attr);


	add_map_record(CSR_PA(node, CAVM_LMCX_PF_BAR0(0)), CAVM_LMCX_PF_BAR0_SIZE, attr);

	for (i = 0; i < 2; i++) {
		add_map_record(CSR_PA(node, CAVM_MIO_TWSX_PF_BAR0(i)), CAVM_MIO_TWSX_PF_BAR0_SIZE, attr);
		add_map_record(CSR_PA(node, CAVM_MIO_TWS_BAR_E_MIO_TWSX_PF_BAR4(i)),
			       CAVM_MIO_TWS_BAR_E_MIO_TWSX_PF_BAR4_SIZE, attr);
	}

	add_map_record(CSR_PA(node, CAVM_L2C_CBCX_PF_BAR0(0)), CAVM_L2C_CBCX_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_L2C_MCIX_PF_BAR0(0)), CAVM_L2C_MCIX_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_L2C_TADX_PF_BAR0(0)), CAVM_L2C_TADX_PF_BAR0_SIZE, attr);

	for (i = 0; i < 3; i++) {
		add_map_record(CSR_PA(node, CAVM_PEMX_PF_BAR0(i)), CAVM_PEMX_PF_BAR0_SIZE, attr);

		add_map_record(CSR_PA(node, CAVM_PEM_BAR_E_PEMX_PF_BAR4(i)),
			       CAVM_PEM_BAR_E_PEMX_PF_BAR4_SIZE, attr);
	}

	for (i = 0; i < 4; i++)
		add_map_record(CSR_PA(node, CAVM_GSERX_PF_BAR0(i)), CAVM_GSERX_PF_BAR0_SIZE, attr);

	for (i = 0; i < 2; i++)
		add_map_record(CSR_PA(node, CAVM_BGXX_PF_BAR0(i)), CAVM_BGXX_PF_BAR0_SIZE, attr);

	add_map_record(CSR_PA(node, CAVM_DAP_PF_BAR0), CAVM_DAP_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_CIM_CSR_BASE), CAVM_CIM_CSR_SIZE, attr);

	add_map_record(CSR_PA(node, CAVM_GPIO_PF_BAR0), CAVM_GPIO_PF_BAR0_SIZE, attr);
	add_map_record(CSR_PA(node, CAVM_GPIO_PF_BAR4), CAVM_GPIO_PF_BAR4_SIZE, attr);

	for (i = 0; i < 4; i++) {
		add_map_record(CSR_PA(node, CAVM_UAAX_PF_BAR0(i)), CAVM_UAAX_PF_BAR0_SIZE, attr);
		add_map_record(CSR_PA(node, CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN81XX(i)),
			       CAVM_UAA_BAR_E_UAAX_PF_BAR4_CN81XX_SIZE, attr);
	}

	add_map_record(CSR_PA(node, CAVM_ECAM_BAR_E_ECAMX_PF_BAR0(0)),
		       CAVM_ECAM_BAR_E_ECAMX_PF_BAR0_SIZE, attr);

	add_map_record(CSR_PA(node, CAVM_ECAMX_PF_BAR2(0)), CAVM_ECAMX_PF_BAR2_SIZE, attr);

	add_map_record(CSR_PA(node, CAVM_SLIX_PF_BAR0(0)), CAVM_SLIX_PF_BAR0_SIZE, attr);

	for (i = 0; i < 2; ++i)
	{
		add_map_record(CSR_PA(node, CAVM_SATAX_PF_BAR0(i)),
			       CAVM_SATAX_PF_BAR0_SIZE, attr);
	}
}
