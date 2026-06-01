/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_TCS_H
#define PDC_TCS_H

#include <stdint.h>

/* TCS command option flags */
#define TCS_CFG_OPT_NONE		0x0000U
#define TCS_CFG_OPT_CMD_WAIT		0x0001U	/* Wait for this and all prior cmds */
#define TCS_CFG_OPT_CMD_RESP_REQ	0x0002U	/* Response required */
#define TCS_CFG_OPT_READ		0x0004U	/* Read (default is write) */
#define TCS_CFG_OPT_NO_ENABLE		0x0008U	/* Do not auto-enable this command */
#define TCS_CFG_OPT_NOT_USED		0x0010U	/* Slot unused */

/* Resource descriptor */
struct pdc_tcs_resource {
	char		*name;
	uint32_t	 base_addr;
};

/* Per-command resource data */
struct pdc_tcs_resource_data {
	uint32_t	res_val;
	uint32_t	options;
	uint32_t	addr_offset;
};

/* TCS command/data pair */
struct pdc_tcs_config {
	union {
		uint8_t			  index;	/* Resource list index (init time) */
		struct pdc_tcs_resource	 *resource_p;	/* Resolved pointer (after init) */
	} cmd;
	struct pdc_tcs_resource_data	data;
};

void pdc_tcs_initialize(void);

#endif /* PDC_TCS_H */
