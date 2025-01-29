/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_IF_H
#define LPDDR4_IF_H

#include <inttypes.h>
#include <stdbool.h>
#ifdef DDR_16BIT
#include <lpddr4_16bit_if.h>
#else
#include <lpddr4_32bit_if.h>
#endif

typedef struct lpddr4_config_s lpddr4_config;
typedef struct lpddr4_privatedata_s lpddr4_privatedata;
typedef struct lpddr4_debuginfo_s lpddr4_debuginfo;
typedef struct lpddr4_fspmoderegs_s lpddr4_fspmoderegs;

typedef enum {
	LPDDR4_CTL_REGS		= 0U,
	LPDDR4_PHY_REGS		= 1U,
	LPDDR4_PHY_INDEP_REGS	= 2U
} lpddr4_regblock;

typedef enum {
	LPDDR4_DRV_NONE				= 0U,
	LPDDR4_DRV_SOC_PLL_UPDATE		= 1U,
	LPDDR4_DRV_SOC_PHY_REDUCED_FREQ		= 2U,
	LPDDR4_DRV_SOC_PHY_DATA_RET_ASSERT	= 3U,
	LPDDR4_DRV_SOC_PHY_DATA_RET_DEASSERT	= 4U
} lpddr4_infotype;

typedef enum {
	LPDDR4_LPI_PD_WAKEUP_FN				= 0U,
	LPDDR4_LPI_SR_SHORT_WAKEUP_FN			= 1U,
	LPDDR4_LPI_SR_LONG_WAKEUP_FN			= 2U,
	LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN		= 3U,
	LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN			= 4U,
	LPDDR4_LPI_SRPD_LONG_WAKEUP_FN			= 5U,
	LPDDR4_LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_FN	= 6U
} lpddr4_lpiwakeupparam;

typedef enum {
	LPDDR4_REDUC_ON		= 0U,
	LPDDR4_REDUC_OFF	= 1U
} lpddr4_reducmode;

typedef enum {
	LPDDR4_ECC_DISABLED		= 0U,
	LPDDR4_ECC_ENABLED		= 1U,
	LPDDR4_ECC_ERR_DETECT		= 2U,
	LPDDR4_ECC_ERR_DETECT_CORRECT	= 3U
} lpddr4_eccenable;

typedef enum {
	LPDDR4_DBI_RD_ON	= 0U,
	LPDDR4_DBI_RD_OFF	= 1U,
	LPDDR4_DBI_WR_ON	= 2U,
	LPDDR4_DBI_WR_OFF	= 3U
} lpddr4_dbimode;

typedef enum {
	LPDDR4_FSP_0	= 0U,
	LPDDR4_FSP_1	= 1U,
	LPDDR4_FSP_2	= 2U
} lpddr4_ctlfspnum;

typedef enum {
	LPDDR4_LP_ACTIVE		= 0U,
	LPDDR4_LP_ACTIVE_REDUCED_FREQ	= 1U,
	LPDDR4_LP_IDLE			= 2U,
	LPDDR4_LP_DEEP_SLEEP		= 3U,
	LPDDR4_LP_DEEP_SLEEP_EXIT	= 4U,
	LPDDR4_LP_EXIT			= 5U
} lpddr4_ctllpstate;

typedef void (*lpddr4_infocallback)(const lpddr4_privatedata *pd, lpddr4_infotype infotype);

typedef void (*lpddr4_ctlcallback)(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt ctlinterrupt, uint8_t chipselect);

typedef void (*lpddr4_phyindepcallback)(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt phyindepinterrupt, uint8_t chipselect);

uint32_t lpddr4_probe(const lpddr4_config *config, uint16_t *configsize);

uint32_t lpddr4_init(lpddr4_privatedata *pd, const lpddr4_config *cfg);

uint32_t lpddr4_start(const lpddr4_privatedata *pd);

uint32_t lpddr4_readreg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t *regvalue);

uint32_t lpddr4_writereg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t regvalue);

uint32_t lpddr4_getmmrregister(const lpddr4_privatedata *pd, uint32_t readmoderegval, uint64_t *mmrvalue, uint8_t *mmrstatus);

uint32_t lpddr4_setmmrregister(const lpddr4_privatedata *pd, uint32_t writemoderegval, uint8_t *mrwstatus);

uint32_t lpddr4_writectlconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);
uint32_t lpddr4_writectlconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount);

uint32_t lpddr4_writephyconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);
uint32_t lpddr4_writephyconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount);

uint32_t lpddr4_writephyindepconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);
uint32_t lpddr4_writephyindepconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount);

uint32_t lpddr4_readctlconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);

uint32_t lpddr4_readphyconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);

uint32_t lpddr4_readphyindepconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);

uint32_t lpddr4_getctlinterruptmask(const lpddr4_privatedata *pd, uint64_t *mask);

uint32_t lpddr4_setctlinterruptmask(const lpddr4_privatedata *pd, const uint64_t *mask);

uint32_t lpddr4_checkctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr, bool *irqstatus);

uint32_t lpddr4_ackctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr);

uint32_t lpddr4_getphyindepinterruptmask(const lpddr4_privatedata *pd, uint32_t *mask);

uint32_t lpddr4_setphyindepinterruptmask(const lpddr4_privatedata *pd, const uint32_t *mask);

uint32_t lpddr4_checkphyindepinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr, bool *irqstatus);

uint32_t lpddr4_ackphyindepinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr);

uint32_t lpddr4_getdebuginitinfo(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo);

uint32_t lpddr4_getlpiwakeuptime(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, uint32_t *cycles);

uint32_t lpddr4_setlpiwakeuptime(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const uint32_t *cycles);

uint32_t lpddr4_geteccenable(const lpddr4_privatedata *pd, lpddr4_eccenable *eccparam);

uint32_t lpddr4_seteccenable(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam);

uint32_t lpddr4_getreducmode(const lpddr4_privatedata *pd, lpddr4_reducmode *mode);

uint32_t lpddr4_setreducmode(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode);

uint32_t lpddr4_getdbireadmode(const lpddr4_privatedata *pd, bool *on_off);

uint32_t lpddr4_getdbiwritemode(const lpddr4_privatedata *pd, bool *on_off);

uint32_t lpddr4_setdbimode(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode);

uint32_t lpddr4_getrefreshrate(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, uint32_t *tref, uint32_t *tras_max);

uint32_t lpddr4_setrefreshrate(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const uint32_t *tref, const uint32_t *tras_max);

uint32_t lpddr4_refreshperchipselect(const lpddr4_privatedata *pd, const uint32_t trefinterval);

uint32_t lpddr4_setmclowpowermode(const lpddr4_privatedata *pd, lpddr4_ctllpstate LpState);

uint32_t lpddr4_getmclowpowermode(const lpddr4_privatedata *pd, lpddr4_ctllpstate *getlpstate);

uint32_t lpddr4_deferredregverify(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount);

#endif  /* LPDDR4_IF_H */
