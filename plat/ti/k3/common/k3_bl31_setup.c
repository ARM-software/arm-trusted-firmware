/*
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 * K3 SOC specific bl31_setup
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <ti_sci.h>
#include <ti_sci_transport.h>

#include <plat_private.h>

/* Table of regions to map using the MMU */
const mmap_region_t plat_k3_mmap[] = {
	K3_MAP_REGION_FLAT(K3_USART_BASE,       K3_USART_SIZE,       MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GIC_BASE,         K3_GIC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(K3_GTC_BASE,         K3_GTC_SIZE,         MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_RT_BASE,   SEC_PROXY_RT_SIZE,   MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_SCFG_BASE, SEC_PROXY_SCFG_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	K3_MAP_REGION_FLAT(SEC_PROXY_DATA_BASE, SEC_PROXY_DATA_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{ /* sentinel */ }
};

int ti_soc_init(void)
{
	struct ti_sci_msg_version version;
	int ret;

	ret = ti_sci_get_revision(&version);
	if (ret) {
		ERROR("Unable to communicate with the control firmware (%d)\n", ret);
		return ret;
	}

	INFO("SYSFW ABI: %d.%d (firmware rev 0x%04x '%s')\n",
	     version.abi_major, version.abi_minor,
	     version.firmware_revision,
	     version.firmware_description);

	/*
	 * Older firmware have a timing issue with DM that crashes few TF-A
	 * lite devices while trying to make calls to DM. Since there is no way
	 * to detect what current DM version we are running - we rely on the
	 * corresponding TIFS versioning to handle this check and ensure that
	 * the platform boots up
	 *
	 * Upgrading to TIFS version 9.1.7 along with the corresponding DM from
	 * ti-linux-firmware will enable this functionality.
	 */
	if (version.firmware_revision > 9 ||
	    (version.firmware_revision == 9 && version.sub_version > 1) ||
	    (version.firmware_revision == 9 && version.sub_version == 1 &&
		 version.patch_version >= 7)
	) {
		if (ti_sci_device_get(PLAT_BOARD_DEVICE_ID)) {
			WARN("Unable to take system power reference\n");
		}
	} else {
		NOTICE("Upgrade Firmwares for Power off functionality\n");
	}
	return 0;
}
