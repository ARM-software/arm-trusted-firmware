/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_STRUCTS_IF_H
#define LPDDR4_STRUCTS_IF_H

#include "inttypes.h"
#include "lpddr4_if.h"

struct lpddr4_config_s {
	struct lpddr4_ctlregs_s *ctlbase;
	lpddr4_infocallback infohandler;
	lpddr4_ctlcallback ctlinterrupthandler;
	lpddr4_phyindepcallback phyindepinterrupthandler;
};

struct lpddr4_privatedata_s {
	struct lpddr4_ctlregs_s *ctlbase;
	lpddr4_infocallback infohandler;
	lpddr4_ctlcallback ctlinterrupthandler;
	lpddr4_phyindepcallback phyindepinterrupthandler;
};

struct lpddr4_debuginfo_s {
	uint8_t pllerror;
	uint8_t iocaliberror;
	uint8_t rxoffseterror;
	uint8_t catraingerror;
	uint8_t wrlvlerror;
	uint8_t gatelvlerror;
	uint8_t readlvlerror;
	uint8_t dqtrainingerror;
};

struct lpddr4_fspmoderegs_s {
	uint8_t mr1data_fn[LPDDR4_INTR_MAX_CS];
	uint8_t mr2data_fn[LPDDR4_INTR_MAX_CS];
	uint8_t mr3data_fn[LPDDR4_INTR_MAX_CS];
	uint8_t mr11data_fn[LPDDR4_INTR_MAX_CS];
	uint8_t mr12data_fn[LPDDR4_INTR_MAX_CS];
	uint8_t mr13data_fn[LPDDR4_INTR_MAX_CS];
	uint8_t mr14data_fn[LPDDR4_INTR_MAX_CS];
	uint8_t mr22data_fn[LPDDR4_INTR_MAX_CS];
};

#endif  /* LPDDR4_STRUCTS_IF_H */
