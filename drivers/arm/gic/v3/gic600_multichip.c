/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 * Copyright (c) 2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * GIC-600 driver extension for multichip setup
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/arm/arm_gicv3_common.h>
#include <drivers/arm/gic600_multichip.h>
#include <drivers/arm/gicv3.h>

#include "../common/gic_common_private.h"
#include "gic600_multichip_private.h"

/*******************************************************************************
 * GIC-600 multichip operation related helper functions
 ******************************************************************************/
static void gicd_dchipr_wait_for_power_update_progress(uintptr_t base)
{
	unsigned int retry = GICD_PUP_UPDATE_RETRIES;

	while ((read_gicd_dchipr(base) & GICD_DCHIPR_PUP_BIT) != 0U) {
		if (retry-- == 0) {
			ERROR("GIC-600 connection to Routing Table Owner timed "
					 "out\n");
			panic();
		}
	}
}

/*******************************************************************************
 * Sets up the routing table owner.
 ******************************************************************************/
static void set_gicd_dchipr_rt_owner(uintptr_t base, unsigned int rt_owner)
{
	/*
	 * Ensure that Group enables in GICD_CTLR are disabled and no pending
	 * register writes to GICD_CTLR.
	 */
	if ((gicd_read_ctlr(base) &
			(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1S_BIT |
			 CTLR_ENABLE_G1NS_BIT | GICD_CTLR_RWP_BIT)) != 0) {
		ERROR("GICD_CTLR group interrupts are either enabled or have "
				"pending writes. Cannot set RT owner.\n");
		panic();
	}

	/* Poll till PUP is zero before intiating write */
	gicd_dchipr_wait_for_power_update_progress(base);

	write_gicd_dchipr(base, read_gicd_dchipr(base) |
			(rt_owner << GICD_DCHIPR_RT_OWNER_SHIFT));

	/* Poll till PUP is zero to ensure write is complete */
	gicd_dchipr_wait_for_power_update_progress(base);
}

/*******************************************************************************
 * Configures the Chip Register to make connections to GICDs on
 * a multichip platform.
 ******************************************************************************/
static void set_gicd_chipr_n(uintptr_t base,
				unsigned int chip_id,
				uint64_t chip_addr,
				unsigned int spi_id_min,
				unsigned int spi_id_max)
{
	unsigned int spi_block_min, spi_blocks;
	unsigned int gicd_iidr_val = gicd_read_iidr(base);
	uint64_t chipr_n_val;

	/*
	 * Ensure that group enables in GICD_CTLR are disabled and no pending
	 * register writes to GICD_CTLR.
	 */
	if ((gicd_read_ctlr(base) &
			(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1S_BIT |
			 CTLR_ENABLE_G1NS_BIT | GICD_CTLR_RWP_BIT)) != 0) {
		ERROR("GICD_CTLR group interrupts are either enabled or have "
				"pending writes. Cannot set CHIPR register.\n");
		panic();
	}

	/*
	 * spi_id_min and spi_id_max of value 0 is used to intidicate that the
	 * chip doesn't own any SPI block. Re-assign min and max values as SPI
	 * id starts from 32.
	 */
	if (spi_id_min == 0 && spi_id_max == 0) {
		spi_id_min = GIC600_SPI_ID_MIN;
		spi_id_max = GIC600_SPI_ID_MIN;
	}

	switch ((gicd_iidr_val & IIDR_MODEL_MASK)) {
	case IIDR_MODEL_ARM_GIC_600:
		spi_block_min = SPI_BLOCK_MIN_VALUE(spi_id_min);
		spi_blocks    = SPI_BLOCKS_VALUE(spi_id_min, spi_id_max);

		chipr_n_val = GICD_CHIPR_VALUE_GIC_600(chip_addr,
						       spi_block_min,
						       spi_blocks);
		break;
	case IIDR_MODEL_ARM_GIC_700:
		/* Calculate the SPI_ID_MIN value for ESPI */
		if (spi_id_min >= GIC700_ESPI_ID_MIN) {
			spi_block_min = ESPI_BLOCK_MIN_VALUE(spi_id_min);
			spi_block_min += SPI_BLOCKS_VALUE(GIC700_SPI_ID_MIN,
				GIC700_SPI_ID_MAX);
		} else {
			spi_block_min = SPI_BLOCK_MIN_VALUE(spi_id_min);
		}

		/* Calculate the total number of blocks */
		spi_blocks = SPI_BLOCKS_VALUE(spi_id_min, spi_id_max);

		chipr_n_val = GICD_CHIPR_VALUE_GIC_700(chip_addr,
						       spi_block_min,
						       spi_blocks);
		break;
	default:
		ERROR("Unsupported GIC model 0x%x for multichip setup.\n",
		      gicd_iidr_val);
		panic();
		break;
	}
	chipr_n_val |= GICD_CHIPRx_SOCKET_STATE;

	/*
	 * Wait for DCHIPR.PUP to be zero before commencing writes to
	 * GICD_CHIPRx.
	 */
	gicd_dchipr_wait_for_power_update_progress(base);

	/*
	 * Assign chip addr, spi min block, number of spi blocks and bring chip
	 * online by setting SocketState.
	 */
	write_gicd_chipr_n(base, chip_id, chipr_n_val);

	/*
	 * Poll until DCHIP.PUP is zero to verify connection to rt_owner chip
	 * is complete.
	 */
	gicd_dchipr_wait_for_power_update_progress(base);

	/*
	 * Ensure that write to GICD_CHIPRx is successful and the chip_n came
	 * online.
	 */
	if (read_gicd_chipr_n(base, chip_id) != chipr_n_val) {
		ERROR("GICD_CHIPR%u write failed\n", chip_id);
		panic();
	}

	/* Ensure that chip is in consistent state */
	if (((read_gicd_chipsr(base) & GICD_CHIPSR_RTS_MASK) >>
				GICD_CHIPSR_RTS_SHIFT) !=
			GICD_CHIPSR_RTS_STATE_CONSISTENT) {
		ERROR("Chip %u routing table is not in consistent state\n",
				chip_id);
		panic();
	}
}

/*******************************************************************************
 * Validates the GIC-600 Multichip data structure passed by the platform.
 ******************************************************************************/
static void gic600_multichip_validate_data(
		struct gic600_multichip_data *multichip_data)
{
	unsigned int i, spi_id_min, spi_id_max, blocks_of_32;
	unsigned int multichip_spi_blocks = 0;

	assert(multichip_data != NULL);

	if (multichip_data->chip_count > GIC600_MAX_MULTICHIP) {
		ERROR("GIC-600 Multichip count should not exceed %d\n",
				GIC600_MAX_MULTICHIP);
		panic();
	}

	for (i = 0; i < multichip_data->chip_count; i++) {
		spi_id_min = multichip_data->spi_ids[i][SPI_MIN_INDEX];
		spi_id_max = multichip_data->spi_ids[i][SPI_MAX_INDEX];

		if ((spi_id_min != 0) || (spi_id_max != 0)) {

			/* SPI IDs range check */
			if (!(spi_id_min >= GIC600_SPI_ID_MIN) ||
			    !(spi_id_max < GIC600_SPI_ID_MAX) ||
			    !(spi_id_min <= spi_id_max) ||
			    !((spi_id_max - spi_id_min + 1) % 32 == 0)) {
				ERROR("Invalid SPI IDs {%u, %u} passed for "
						"Chip %u\n", spi_id_min,
						spi_id_max, i);
				panic();
			}

			/* SPI IDs overlap check */
			blocks_of_32 = BLOCKS_OF_32(spi_id_min, spi_id_max);
			if ((multichip_spi_blocks & blocks_of_32) != 0) {
				ERROR("SPI IDs of Chip %u overlapping\n", i);
				panic();
			}
			multichip_spi_blocks |= blocks_of_32;
		}
	}
}

/*******************************************************************************
 * Validates the GIC-700 Multichip data structure passed by the platform.
 ******************************************************************************/
static void gic700_multichip_validate_data(
		struct gic600_multichip_data *multichip_data)
{
	unsigned int i, spi_id_min, spi_id_max, blocks_of_32;
	unsigned int multichip_spi_blocks = 0U, multichip_espi_blocks = 0U;

	assert(multichip_data != NULL);

	if (multichip_data->chip_count > GIC600_MAX_MULTICHIP) {
		ERROR("GIC-700 Multichip count (%u) should not exceed %u\n",
				multichip_data->chip_count, GIC600_MAX_MULTICHIP);
		panic();
	}

	for (i = 0U; i < multichip_data->chip_count; i++) {
		spi_id_min = multichip_data->spi_ids[i][SPI_MIN_INDEX];
		spi_id_max = multichip_data->spi_ids[i][SPI_MAX_INDEX];

		if ((spi_id_min == 0U) || (spi_id_max == 0U)) {
			continue;
		}

		/* MIN SPI ID check */
		if ((spi_id_min < GIC700_SPI_ID_MIN) ||
		    ((spi_id_min >= GIC700_SPI_ID_MAX) &&
		     (spi_id_min < GIC700_ESPI_ID_MIN))) {
			ERROR("Invalid MIN SPI ID {%u} passed for "
					"Chip %u\n", spi_id_min, i);
			panic();
		}

		if ((spi_id_min > spi_id_max) ||
		    ((spi_id_max - spi_id_min + 1) % 32 != 0)) {
			ERROR("Unaligned SPI IDs {%u, %u} passed for "
					"Chip %u\n", spi_id_min,
					spi_id_max, i);
			panic();
		}

		/* ESPI IDs range check */
		if ((spi_id_min >= GIC700_ESPI_ID_MIN) &&
		    (spi_id_max > GIC700_ESPI_ID_MAX)) {
			ERROR("Invalid ESPI IDs {%u, %u} passed for "
					"Chip %u\n", spi_id_min,
					spi_id_max, i);
			panic();

		}

		/* SPI IDs range check */
		if (((spi_id_min < GIC700_SPI_ID_MAX) &&
		     (spi_id_max > GIC700_SPI_ID_MAX))) {
			ERROR("Invalid SPI IDs {%u, %u} passed for "
					"Chip %u\n", spi_id_min,
					spi_id_max, i);
			panic();
		}

		/* SPI IDs overlap check */
		if (spi_id_max < GIC700_SPI_ID_MAX) {
			blocks_of_32 = BLOCKS_OF_32(spi_id_min, spi_id_max);
			if ((multichip_spi_blocks & blocks_of_32) != 0) {
				ERROR("SPI IDs of Chip %u overlapping\n", i);
				panic();
			}
			multichip_spi_blocks |= blocks_of_32;
		}

		/* ESPI IDs overlap check */
		if (spi_id_max > GIC700_ESPI_ID_MIN) {
			blocks_of_32 = BLOCKS_OF_32(spi_id_min - GIC700_ESPI_ID_MIN,
					spi_id_max - GIC700_ESPI_ID_MIN);
			if ((multichip_espi_blocks & blocks_of_32) != 0) {
				ERROR("SPI IDs of Chip %u overlapping\n", i);
				panic();
			}
			multichip_espi_blocks |= blocks_of_32;
		}
	}
}

/*******************************************************************************
 * Intialize GIC-600 and GIC-700 Multichip operation.
 ******************************************************************************/
void gic600_multichip_init(struct gic600_multichip_data *multichip_data)
{
	unsigned int i;
	uint32_t gicd_iidr_val = gicd_read_iidr(multichip_data->rt_owner_base);

	if ((gicd_iidr_val & IIDR_MODEL_MASK) == IIDR_MODEL_ARM_GIC_600) {
		gic600_multichip_validate_data(multichip_data);
	}

	if ((gicd_iidr_val & IIDR_MODEL_MASK) == IIDR_MODEL_ARM_GIC_700) {
		gic700_multichip_validate_data(multichip_data);
	}

	/*
	 * Ensure that G0/G1S/G1NS interrupts are disabled. This also ensures
	 * that GIC-600 Multichip configuration is done first.
	 */
	if ((gicd_read_ctlr(multichip_data->rt_owner_base) &
			(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1S_BIT |
			 CTLR_ENABLE_G1NS_BIT | GICD_CTLR_RWP_BIT)) != 0) {
		ERROR("GICD_CTLR group interrupts are either enabled or have "
				"pending writes.\n");
		panic();
	}

	/* Ensure that the routing table owner is in disconnected state */
	if (((read_gicd_chipsr(multichip_data->rt_owner_base) &
		GICD_CHIPSR_RTS_MASK) >> GICD_CHIPSR_RTS_SHIFT) !=
			GICD_CHIPSR_RTS_STATE_DISCONNECTED) {
		ERROR("GIC-600 routing table owner is not in disconnected "
				"state to begin multichip configuration\n");
		panic();
	}

	/* Initialize the GICD which is marked as routing table owner first */
	set_gicd_dchipr_rt_owner(multichip_data->rt_owner_base,
			multichip_data->rt_owner);

	set_gicd_chipr_n(multichip_data->rt_owner_base, multichip_data->rt_owner,
			multichip_data->chip_addrs[multichip_data->rt_owner],
			multichip_data->
			spi_ids[multichip_data->rt_owner][SPI_MIN_INDEX],
			multichip_data->
			spi_ids[multichip_data->rt_owner][SPI_MAX_INDEX]);

	for (i = 0; i < multichip_data->chip_count; i++) {
		if (i == multichip_data->rt_owner)
			continue;

		set_gicd_chipr_n(multichip_data->rt_owner_base, i,
				multichip_data->chip_addrs[i],
				multichip_data->spi_ids[i][SPI_MIN_INDEX],
				multichip_data->spi_ids[i][SPI_MAX_INDEX]);
	}
}
