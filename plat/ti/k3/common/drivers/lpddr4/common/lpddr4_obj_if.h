/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef lpddr4_obj_if_h
#define lpddr4_obj_if_h

#include "lpddr4_if.h"

typedef struct lpddr4_obj_s {
	uint32_t (*probe)(const lpddr4_config *config, uint16_t *configsize);

	uint32_t (*init)(lpddr4_privatedata *pd, const lpddr4_config *cfg);

	uint32_t (*start)(const lpddr4_privatedata *pd);

	uint32_t (*readreg)(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t *regvalue);

	uint32_t (*writereg)(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t regvalue);

	uint32_t (*getmmrregister)(const lpddr4_privatedata *pd, uint32_t readmoderegval, uint64_t *mmrvalue, uint8_t *mmrstatus);

	uint32_t (*setmmrregister)(const lpddr4_privatedata *pd, uint32_t writemoderegval, uint8_t *mrwstatus);

	uint32_t (*writectlconfig)(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);
	uint32_t (*writectlconfigex)(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount);

	uint32_t (*writephyconfig)(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);
	uint32_t (*writephyconfigex)(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount);

	uint32_t (*writephyindepconfig)(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);
	uint32_t (*writephyindepconfigex)(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount);

	uint32_t (*readctlconfig)(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);

	uint32_t (*readphyconfig)(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);

	uint32_t (*readphyindepconfig)(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);

	uint32_t (*getctlinterruptmask)(const lpddr4_privatedata *pd, uint64_t *mask);

	uint32_t (*setctlinterruptmask)(const lpddr4_privatedata *pd, const uint64_t *mask);

	uint32_t (*checkctlinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr, bool *irqstatus);

	uint32_t (*ackctlinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr);

	uint32_t (*getphyindepinterruptmask)(const lpddr4_privatedata *pd, uint32_t *mask);

	uint32_t (*setphyindepinterruptmask)(const lpddr4_privatedata *pd, const uint32_t *mask);

	uint32_t (*checkphyindepinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr, bool *irqstatus);

	uint32_t (*ackphyindepinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr);

	uint32_t (*getdebuginitinfo)(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo);

	uint32_t (*getlpiwakeuptime)(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, uint32_t *cycles);

	uint32_t (*setlpiwakeuptime)(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const uint32_t *cycles);

	uint32_t (*geteccenable)(const lpddr4_privatedata *pd, lpddr4_eccenable *eccparam);

	uint32_t (*seteccenable)(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam);

	uint32_t (*getreducmode)(const lpddr4_privatedata *pd, lpddr4_reducmode *mode);

	uint32_t (*setreducmode)(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode);

	uint32_t (*getdbireadmode)(const lpddr4_privatedata *pd, bool *on_off);

	uint32_t (*getdbiwritemode)(const lpddr4_privatedata *pd, bool *on_off);

	uint32_t (*setdbimode)(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode);

	uint32_t (*getrefreshrate)(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, uint32_t *tref, uint32_t *tras_max);

	uint32_t (*setrefreshrate)(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const uint32_t *tref, const uint32_t *tras_max);

	uint32_t (*refreshperchipselect)(const lpddr4_privatedata *pd, const uint32_t trefinterval);

	uint32_t (*setmclowpowermode)(const lpddr4_privatedata *pd, lpddr4_ctllpstate LpState);

	uint32_t (*getmclowpowermode)(const lpddr4_privatedata *pd, lpddr4_ctllpstate *getlpstate);

	uint32_t (*deferredregverify)(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);
} lpddr4_obj;

extern lpddr4_obj *lpddr4_getinstance(void);

#endif  /* lpddr4_obj_if_h */
