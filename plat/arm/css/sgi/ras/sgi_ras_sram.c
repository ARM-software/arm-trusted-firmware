/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/interrupt_mgmt.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>
#include <services/sdei.h>
#include <services/spm_mm_svc.h>

#include <platform_def.h>
#include <sgi_ras.h>

/* Base Element RAM Error Record offsets. */
#define ERRSTATUS	U(0)
#define ERRCODE		U(8)
#define ERRADDR		U(12)

/*
 * Base Element RAM error information data structure communicated as part of MM
 * Communication data payload.
 */
typedef struct sgi_sram_err_info {
	uint32_t err_status;
	uint32_t err_code;
	uint32_t err_addr;
} sgi_sram_err_info_t;

/*
 * MM Communicate message header GUID to indicate the payload is intended for
 * base element RAM MM driver.
 */
struct efi_guid sram_ecc_event_guid = {
	0x7312db4f, 0xd0c4, 0x4fb5,
	{ 0x81, 0x2c, 0xb7, 0x4b, 0xc6, 0xc4, 0xa9, 0x38 }
};

/* Base element RAM RAS error interrupt handler */
int sgi_ras_sram_intr_handler(const struct err_record_info *err_rec,
				int probe_data,
				const struct err_handler_data *const data)
{
	struct sgi_ras_ev_map *ras_map;
	mm_communicate_header_t *header;
	sgi_sram_err_info_t sram_info;
	uintptr_t base_addr;
	uint32_t clear_status, intr;
	int ret;

	cm_el1_sysregs_context_save(NON_SECURE);
	intr = data->interrupt;

	INFO("SGI: Base element RAM interrupt [%d] handler\n", intr);

	/* Determine error record base address to read. */
	base_addr = 0;
	if (intr == NS_RAM_ECC_CE_INT || intr == NS_RAM_ECC_UE_INT) {
		base_addr = SOC_NS_RAM_ERR_REC_BASE;
	}
	sram_info.err_status = mmio_read_32(base_addr + ERRSTATUS);
	sram_info.err_code = mmio_read_32(base_addr + ERRCODE);
	sram_info.err_addr = mmio_read_32(base_addr + ERRADDR);

	/* Clear the interrupt. */
	clear_status = mmio_read_32(base_addr + ERRSTATUS);
	mmio_write_32((base_addr + ERRSTATUS), clear_status);

	/*
	 * Prepare the MM Communication buffer to pass the base element RAM
	 * error information to Secure Partition.
	 */
	header = (void *)PLAT_SPM_BUF_BASE;
	memset(header, 0, sizeof(*header));
	memcpy(&header->data, &sram_info, sizeof(sram_info));
	header->message_len = sizeof(sram_info);
	memcpy(&header->header_guid, (void *)&sram_ecc_event_guid,
		sizeof(struct efi_guid));

	spm_mm_sp_call(MM_COMMUNICATE_AARCH64, (uint64_t)header, 0,
			plat_my_core_pos());

	plat_ic_end_of_interrupt(intr);

	/*
	 * Find if this is a RAS interrupt. There must be an event against
	 * this interrupt
	 */
	ras_map = sgi_find_ras_event_map_by_intr(intr);
	if (ras_map == NULL) {
		ERROR("SGI: RAS error info for interrupt id: %d not found\n",
			intr);
		return -1;
	}

	/* Dispatch the event to the SDEI client */
	ret = sdei_dispatch_event(ras_map->sdei_ev_num);
	if (ret != 0) {
		/*
		 * sdei_dispatch_event() may return failing result in some
		 * cases, for example kernel may not have registered a handler
		 * or RAS event may happen early during boot. We restore the NS
		 * context when sdei_dispatch_event() returns failing result.
		 */
		ERROR("SDEI dispatch failed: %d", ret);
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);
	}

	return ret;
}
