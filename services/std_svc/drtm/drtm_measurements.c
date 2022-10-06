/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM measurements into TPM PCRs.
 *
 * Authors:
 *      Lucian Paul-Trifu <lucian.paultrifu@gmail.com>
 *
 */
#include <assert.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/event_log/event_log.h>
#include "drtm_main.h"
#include "drtm_measurements.h"
#include <lib/xlat_tables/xlat_tables_v2.h>

/* Event Log buffer */
static uint8_t drtm_event_log[PLAT_DRTM_EVENT_LOG_MAX_SIZE];

/*
 * Calculate and write hash of various payloads as per DRTM specification
 * to Event Log.
 *
 * @param[in] data_base         Address of data
 * @param[in] data_size         Size of data
 * @param[in] event_type        Type of Event
 * @param[in] event_name        Name of the Event
 * @return:
 *      0 = success
 *    < 0 = error
 */
static int drtm_event_log_measure_and_record(uintptr_t data_base,
					     uint32_t data_size,
					     uint32_t event_type,
					     const char *event_name,
					     unsigned int pcr)
{
	int rc;
	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	event_log_metadata_t metadata = {0};

	metadata.name = event_name;
	metadata.pcr = pcr;

	/*
	 * Measure the payloads requested by D-CRTM and DCE commponents
	 * Hash algorithm decided by the Event Log driver at build-time
	 */
	rc = event_log_measure(data_base, data_size, hash_data);
	if (rc != 0) {
		return rc;
	}

	/* Record the mesasurement in the EventLog buffer */
	event_log_record(hash_data, event_type, &metadata);

	return 0;
}

/*
 * Initialise Event Log global variables, used during the recording
 * of various payload measurements into the Event Log buffer
 *
 * @param[in] event_log_start           Base address of Event Log buffer
 * @param[in] event_log_finish          End address of Event Log buffer,
 *                                      it is a first byte past end of the
 *                                      buffer
 */
static void drtm_event_log_init(uint8_t *event_log_start,
				uint8_t *event_log_finish)
{
	event_log_buf_init(event_log_start, event_log_finish);
	event_log_write_specid_event();
}

enum drtm_retc drtm_take_measurements(const struct_drtm_dl_args *a)
{
	int rc;
	uintptr_t dlme_img_mapping;
	uint64_t dlme_img_ep;
	size_t dlme_img_mapping_bytes;
	uint8_t drtm_null_data = 0U;
	uint8_t pcr_schema = DL_ARGS_GET_PCR_SCHEMA(a);
	const char *drtm_event_arm_sep_data = "ARM_DRTM";

	/* Initialise the EventLog driver */
	drtm_event_log_init(drtm_event_log, drtm_event_log +
			    sizeof(drtm_event_log));

	/**
	 * Measurements extended into PCR-17.
	 *
	 * PCR-17: Measure the DCE image.  Extend digest of (char)0 into PCR-17
	 * since the D-CRTM and the DCE are not separate.
	 */
	rc = drtm_event_log_measure_and_record((uintptr_t)&drtm_null_data,
					       sizeof(drtm_null_data),
					       DRTM_EVENT_ARM_DCE, NULL,
					       PCR_17);
	CHECK_RC(rc, drtm_event_log_measure_and_record(DRTM_EVENT_ARM_DCE));

	/* PCR-17: Measure the PCR schema DRTM launch argument. */
	rc = drtm_event_log_measure_and_record((uintptr_t)&pcr_schema,
					       sizeof(pcr_schema),
					       DRTM_EVENT_ARM_PCR_SCHEMA,
					       NULL, PCR_17);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_PCR_SCHEMA));

	/* PCR-17: Measure the enable state of external-debug, and trace. */
	/*
	 * TODO: Measure the enable state of external-debug and trace.  This should
	 * be returned through a platform-specific hook.
	 */

	/* PCR-17: Measure the security lifecycle state. */
	/*
	 * TODO: Measure the security lifecycle state.  This is an implementation-
	 * defined value, retrieved through an implementation-defined mechanisms.
	 */

	/*
	 * PCR-17: Optionally measure the NWd DCE.
	 * It is expected that such subsequent DCE stages are signed and verified.
	 * Whether they are measured in addition to signing is implementation
	 * -defined.
	 * Here the choice is to not measure any NWd DCE, in favour of PCR value
	 * resilience to any NWd DCE updates.
	 */

	/* PCR-17: End of DCE measurements. */
	rc = drtm_event_log_measure_and_record((uintptr_t)drtm_event_arm_sep_data,
					       strlen(drtm_event_arm_sep_data),
					       DRTM_EVENT_ARM_SEPARATOR, NULL,
					       PCR_17);
	CHECK_RC(rc, drtm_event_log_measure_and_record(DRTM_EVENT_ARM_SEPARATOR));

	/**
	 * Measurements extended into PCR-18.
	 *
	 * PCR-18: Measure the PCR schema DRTM launch argument.
	 */
	rc = drtm_event_log_measure_and_record((uintptr_t)&pcr_schema,
					       sizeof(pcr_schema),
					       DRTM_EVENT_ARM_PCR_SCHEMA,
					       NULL, PCR_18);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_PCR_SCHEMA));

	/*
	 * PCR-18: Measure the public key used to verify DCE image(s) signatures.
	 * Extend digest of (char)0, since we do not expect the NWd DCE to be
	 * present.
	 */
	assert(a->dce_nwd_size == 0);
	rc = drtm_event_log_measure_and_record((uintptr_t)&drtm_null_data,
					       sizeof(drtm_null_data),
					       DRTM_EVENT_ARM_DCE_PUBKEY,
					       NULL, PCR_18);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_DCE_PUBKEY));

	/* PCR-18: Measure the DLME image. */
	dlme_img_mapping_bytes = page_align(a->dlme_img_size, UP);
	rc = mmap_add_dynamic_region_alloc_va(a->dlme_paddr + a->dlme_img_off,
					      &dlme_img_mapping,
					      dlme_img_mapping_bytes, MT_RO_DATA | MT_NS);
	if (rc) {
		WARN("DRTM: %s: mmap_add_dynamic_region() failed rc=%d\n",
		     __func__, rc);
		return INTERNAL_ERROR;
	}

	rc = drtm_event_log_measure_and_record(dlme_img_mapping, a->dlme_img_size,
					       DRTM_EVENT_ARM_DLME, NULL,
					       PCR_18);
	CHECK_RC(rc, drtm_event_log_measure_and_record(DRTM_EVENT_ARM_DLME));

	rc = mmap_remove_dynamic_region(dlme_img_mapping, dlme_img_mapping_bytes);
	CHECK_RC(rc, mmap_remove_dynamic_region);

	/* PCR-18: Measure the DLME image entry point. */
	dlme_img_ep = DL_ARGS_GET_DLME_ENTRY_POINT(a);
	drtm_event_log_measure_and_record((uintptr_t)&dlme_img_ep,
					  sizeof(dlme_img_ep),
					  DRTM_EVENT_ARM_DLME_EP, NULL,
					  PCR_18);
	CHECK_RC(rc, drtm_event_log_measure_and_record(DRTM_EVENT_ARM_DLME_EP));

	/* PCR-18: End of DCE measurements. */
	rc = drtm_event_log_measure_and_record((uintptr_t)drtm_event_arm_sep_data,
					       strlen(drtm_event_arm_sep_data),
					       DRTM_EVENT_ARM_SEPARATOR, NULL,
					       PCR_18);
	CHECK_RC(rc,
		 drtm_event_log_measure_and_record(DRTM_EVENT_ARM_SEPARATOR));
	/*
	 * If the DCE is unable to log a measurement because there is no available
	 * space in the event log region, the DCE must extend a hash of the value
	 * 0xFF (1 byte in size) into PCR[17] and PCR[18] and enter remediation.
	 */

	return SUCCESS;
}

void drtm_serialise_event_log(uint8_t *dst, size_t *event_log_size_out)
{
	*event_log_size_out = event_log_get_cur_size(drtm_event_log);
	memcpy(dst, drtm_event_log, *event_log_size_out);
}
