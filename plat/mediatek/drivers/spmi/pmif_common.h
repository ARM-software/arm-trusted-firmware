/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIF_COMMON_H
#define PMIF_COMMON_H

#include <stdint.h>

enum {
	PMIF_CMD_REG_0,
	PMIF_CMD_REG,
	PMIF_CMD_EXT_REG,
	PMIF_CMD_EXT_REG_LONG,
};

struct pmif {
	void *base;
	uint16_t *regs;
	void *spmimst_base;
	uint16_t *spmimst_regs;
	uint32_t mstid;
	int (*read_cmd)(struct pmif *arb, uint8_t opc, uint8_t sid, uint16_t addr, uint8_t *buf,
			uint8_t len);
	int (*write_cmd)(struct pmif *arb, uint8_t opc, uint8_t sid, uint16_t addr,
			 const uint8_t *buf, uint8_t len);
};

enum {
	PMIF_SPMI,
	PMIF_SPI
};

int pmif_spmi_read_cmd(struct pmif *arb, uint8_t opc, uint8_t sid, uint16_t addr, uint8_t *buf,
		       uint8_t len);
int pmif_spmi_write_cmd(struct pmif *arb, uint8_t opc, uint8_t sid, uint16_t addr,
			const uint8_t *buf, uint8_t len);
struct pmif *get_pmif_controller(int inf, int mstid);
extern struct pmif pmif_spmi_arb[];
#endif
