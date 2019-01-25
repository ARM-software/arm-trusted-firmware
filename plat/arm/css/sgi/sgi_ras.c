/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <bl31/interrupt_mgmt.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/ras.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/common/platform.h>
#include <services/mm_svc.h>
#include <services/sdei.h>
#include <services/spm_svc.h>

#include <sgi_ras.h>

static int sgi_ras_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data);
struct efi_guid {
	uint32_t	data1;
	uint16_t	data2;
	uint16_t	data3;
	uint8_t		data4[8];
};

typedef struct mm_communicate_header {
	struct efi_guid	header_guid;
	size_t		message_len;
	uint8_t		data[8];
} mm_communicate_header_t;

struct sgi_ras_ev_map sgi575_ras_map[] = {

	/* DMC620 error overflow interrupt*/
	{SP_DMC_ERROR_OVERFLOW_EVENT_AARCH64, SGI_SDEI_DS_EVENT_1, 33},

	/* DMC620 error ECC error interrupt*/
	{SP_DMC_ERROR_ECC_EVENT_AARCH64, SGI_SDEI_DS_EVENT_0, 35},
};

#define SGI575_RAS_MAP_SIZE	ARRAY_SIZE(sgi575_ras_map)

struct err_record_info sgi_err_records[] = {
	{
		.handler = &sgi_ras_intr_handler,
	},
};

struct ras_interrupt sgi_ras_interrupts[] = {
	{
		.intr_number = 33,
		.err_record = &sgi_err_records[0],
	},
	{
		.intr_number = 35,
		.err_record = &sgi_err_records[0],
	}
};

REGISTER_ERR_RECORD_INFO(sgi_err_records);
REGISTER_RAS_INTERRUPTS(sgi_ras_interrupts);

static struct sgi_ras_ev_map *plat_sgi_get_ras_ev_map(void)
{
	return sgi575_ras_map;
}

static int plat_sgi_get_ras_ev_map_size(void)
{
	return SGI575_RAS_MAP_SIZE;
}

/*
 * Find event mapping for a given interrupt number: On success, returns pointer
 * to the event mapping. On error, returns NULL.
 */
static struct sgi_ras_ev_map *find_ras_event_map_by_intr(uint32_t intr_num)
{
	struct sgi_ras_ev_map *map = plat_sgi_get_ras_ev_map();
	int i;
	int size = plat_sgi_get_ras_ev_map_size();

	for (i = 0; i < size; i++) {
		if (map->intr == intr_num)
			return map;

		map++;
	}

	return NULL;
}

static void sgi_ras_intr_configure(int intr)
{
	plat_ic_set_interrupt_type(intr, INTR_TYPE_EL3);
	plat_ic_set_interrupt_priority(intr, PLAT_RAS_PRI);
	plat_ic_clear_interrupt_pending(intr);
	plat_ic_set_spi_routing(intr, INTR_ROUTING_MODE_ANY,
				(u_register_t)read_mpidr_el1());
	plat_ic_enable_interrupt(intr);
}

static int sgi_ras_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data)
{
	struct sgi_ras_ev_map *ras_map;
	mm_communicate_header_t *header;
	uint32_t intr;

	cm_el1_sysregs_context_save(NON_SECURE);
	intr = data->interrupt;

	/*
	 * Find if this is a RAS interrupt. There must be an event against
	 * this interrupt
	 */
	ras_map = find_ras_event_map_by_intr(intr);
	assert(ras_map);

	/*
	 * Populate the MM_COMMUNICATE payload to share the
	 * event info with StandaloneMM code. This allows us to use
	 * MM_COMMUNICATE as a common entry mechanism into S-EL0. The
	 * header data will be parsed in StandaloneMM to process the
	 * corresponding event.
	 *
	 * TBD - Currently, the buffer allocated by SPM for communication
	 * between EL3 and S-EL0 is being used(PLAT_SPM_BUF_BASE). But this
	 * should happen via a dynamic mem allocation, which should be
	 * managed by SPM -- the individual platforms then call the mem
	 * alloc api to get memory for the payload.
	 */
	header = (void *) PLAT_SPM_BUF_BASE;
	memset(header, 0, sizeof(*header));
	memcpy(&header->data, &ras_map->ras_ev_num,
	       sizeof(ras_map->ras_ev_num));
	header->message_len = 4;

	spm_sp_call(MM_COMMUNICATE_AARCH64, (uint64_t)header, 0,
		    plat_my_core_pos());

	/*
	 * Do an EOI of the RAS interuupt. This allows the
	 * sdei event to be dispatched at the SDEI event's
	 * priority.
	 */
	plat_ic_end_of_interrupt(intr);

	/* Dispatch the event to the SDEI client */
	sdei_dispatch_event(ras_map->sdei_ev_num);

	return 0;
}

int sgi_ras_intr_handler_setup(void)
{
	int i;
	struct sgi_ras_ev_map *map = plat_sgi_get_ras_ev_map();
	int size = plat_sgi_get_ras_ev_map_size();

	for (i = 0; i < size; i++) {
		sgi_ras_intr_configure(map->intr);
		map++;
	}

	INFO("SGI: RAS Interrupt Handler successfully registered\n");

	return 0;
}
