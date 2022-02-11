/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RSS_MEASURED_BOOT_H
#define RSS_MEASURED_BOOT_H

#include <stdint.h>

#include <common/debug.h>
#include <measured_boot.h>

#define RSS_MBOOT_INVALID_ID	UINT32_MAX

/*
 * Each boot measurement has some metadata (i.e. a string) that identifies
 * what was measured and how. The sw_type field of the rss_mboot_metadata
 * structure represents the role of the software component that was measured.
 * The below macros define strings suitable for the sw_type.
 * The key thing is to choose meaningful strings so that when the attestation
 * token is verified, then the different components can be identified.
 */
#define RSS_MBOOT_BL2_STRING		"BL_2"
#define RSS_MBOOT_BL31_STRING		"SECURE_RT_EL3"
#define RSS_MBOOT_HW_CONFIG_STRING	"HW_CONFIG"
#define RSS_MBOOT_FW_CONFIG_STRING	"FW_CONFIG"
#define RSS_MBOOT_TB_FW_CONFIG_STRING	"TB_FW_CONFIG"
#define RSS_MBOOT_SOC_FW_CONFIG_STRING	"SOC_FW_CONFIG"
#define RSS_MBOOT_RMM_STRING		"RMM"


struct rss_mboot_metadata {
	unsigned int id;
	uint8_t slot;
	uint8_t signer_id[SIGNER_ID_MAX_SIZE];
	size_t  signer_id_size;
	uint8_t version[VERSION_MAX_SIZE];
	size_t  version_size;
	uint8_t sw_type[SW_TYPE_MAX_SIZE];
	size_t  sw_type_size;
	bool    lock_measurement;
};

/* Functions' declarations */
void rss_measured_boot_init(void);
struct rss_mboot_metadata *plat_rss_mboot_get_metadata(void);
int rss_mboot_measure_and_record(uintptr_t data_base, uint32_t data_size,
				 uint32_t data_id);

/* TODO: These metadata are currently not available during TF-A boot */
int rss_mboot_set_signer_id(unsigned int img_id, const void *pk_ptr, size_t pk_len);

#endif /* RSS_MEASURED_BOOT_H */
