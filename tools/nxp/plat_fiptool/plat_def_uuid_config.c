/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <firmware_image_package.h>

#include "tbbr_config.h"

toc_entry_t plat_def_toc_entries[] = {
	/* DDR PHY firmwares */
	{
		.name = "DDR UDIMM PHY IMEM 1d FW",
		.uuid = UUID_DDR_IMEM_UDIMM_1D,
		.cmdline_name = "ddr-immem-udimm-1d"
	},
	{
		.name = "DDR UDIMM PHY IMEM 2d FW",
		.uuid = UUID_DDR_IMEM_UDIMM_2D,
		.cmdline_name = "ddr-immem-udimm-2d"
	},
	{
		.name = "DDR UDIMM PHY DMEM 1d FW",
		.uuid = UUID_DDR_DMEM_UDIMM_1D,
		.cmdline_name = "ddr-dmmem-udimm-1d"
	},
	{
		.name = "DDR UDIMM PHY DMEM 2d FW",
		.uuid = UUID_DDR_DMEM_UDIMM_2D,
		.cmdline_name = "ddr-dmmem-udimm-2d"
	},
	{
		.name = "DDR RDIMM PHY IMEM 1d FW",
		.uuid = UUID_DDR_IMEM_RDIMM_1D,
		.cmdline_name = "ddr-immem-rdimm-1d"
	},
	{
		.name = "DDR RDIMM PHY IMEM 2d FW",
		.uuid = UUID_DDR_IMEM_RDIMM_2D,
		.cmdline_name = "ddr-immem-rdimm-2d"
	},
	{
		.name = "DDR RDIMM PHY DMEM 1d FW",
		.uuid = UUID_DDR_DMEM_RDIMM_1D,
		.cmdline_name = "ddr-dmmem-rdimm-1d"
	},
	{
		.name = "DDR RDIMM PHY DMEM 2d FW",
		.uuid = UUID_DDR_DMEM_RDIMM_2D,
		.cmdline_name = "ddr-dmmem-rdimm-2d"
	},
	{
		.name = "FUSE PROV FW",
		.uuid = UUID_FUSE_PROV,
		.cmdline_name = "fuse-prov"
	},
	{
		.name = "FUSE UPGRADE FW",
		.uuid = UUID_FUSE_UP,
		.cmdline_name = "fuse-upgrade"
	},

	/* Key Certificates */
	{
		.name = "DDR Firmware key certificate",
		.uuid = UUID_DDR_FW_KEY_CERT,
		.cmdline_name = "ddr-fw-key-cert"
	},

	/* Content certificates */
	{
		.name = "DDR UDIMM Firmware content certificate",
		.uuid = UUID_DDR_UDIMM_FW_CONTENT_CERT,
		.cmdline_name = "ddr-udimm-fw-cert"
	},
	{
		.name = "DDR RDIMM Firmware content certificate",
		.uuid = UUID_DDR_RDIMM_FW_CONTENT_CERT,
		.cmdline_name = "ddr-rdimm-fw-cert"
	},

	{
		.name = NULL,
		.uuid = { {0} },
		.cmdline_name = NULL,
	}
};
