// SPDX-License-Identifier: BSD-3-Clause
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <errno.h>
#include <common/bl_common.h>
#include <common/debug.h>

#include "cps_drv_lpddr4.h"
#include "lpddr4_if.h"
#include "lpddr4.h"
#include "lpddr4_structs_if.h"

static uint32_t lpddr4_pollphyindepirq(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt irqbit, uint32_t delay);
static uint32_t lpddr4_pollandackirq(const lpddr4_privatedata *pd);
static uint32_t lpddr4_startsequencecontroller(const lpddr4_privatedata *pd);
static uint32_t lpddr4_writemmrregister(const lpddr4_privatedata *pd, uint32_t writemoderegval);
static void lpddr4_checkcatrainingerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr);
static void lpddr4_checkgatelvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr);
static void lpddr4_checkreadlvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr);
static void lpddr4_checkdqtrainingerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr);
static uint8_t lpddr4_seterror(volatile uint32_t *reg, uint32_t errbitmask, uint8_t *errfoundptr, const uint32_t errorinfobits);
static void lpddr4_setphysnapsettings(lpddr4_ctlregs *ctlregbase, const bool errorfound);
static void lpddr4_setphyadrsnapsettings(lpddr4_ctlregs *ctlregbase, const bool errorfound);
static void readpdwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles);
static void readsrshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles);
static void readsrlongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles);
static void readsrlonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles);
static void readsrdpshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles);
static void readsrdplongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles);
static void readsrdplonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles);
static void lpddr4_readlpiwakeuptime(lpddr4_ctlregs *ctlregbase, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, uint32_t *cycles);
static void writepdwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles);
static void writesrshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles);
static void writesrlongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles);
static void writesrlonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles);
static void writesrdpshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles);
static void writesrdplongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles);
static void writesrdplonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles);
static void lpddr4_writelpiwakeuptime(lpddr4_ctlregs *ctlregbase, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const uint32_t *cycles);
static void lpddr4_updatefsp2refrateparams(const lpddr4_privatedata *pd, const uint32_t *tref, const uint32_t *tras_max);
static void lpddr4_updatefsp1refrateparams(const lpddr4_privatedata *pd, const uint32_t *tref, const uint32_t *tras_max);
static void lpddr4_updatefsp0refrateparams(const lpddr4_privatedata *pd, const uint32_t *tref, const uint32_t *tras_max);
static uint32_t lpddr4_getphyrwmask(uint32_t regoffset);
#ifdef REG_WRITE_VERIF
static uint32_t lpddr4_verifyregwrite(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t regvalue);
#endif

uint32_t lpddr4_pollctlirq(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt irqbit, uint32_t delay)
{
	uint32_t result = 0U;
	uint32_t timeout = 0U;
	bool irqstatus = false;

	do {
		if (++timeout == delay) {
			result = (uint32_t)EIO;
			break;
		}
		result = lpddr4_checkctlinterrupt(pd, irqbit, &irqstatus);
	} while ((irqstatus == (bool)false) && (result == (uint32_t)0));

	return result;
}

static uint32_t lpddr4_pollphyindepirq(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt irqbit, uint32_t delay)
{
	uint32_t result = 0U;
	uint32_t timeout = 0U;
	bool irqstatus = false;

	do {
		if (++timeout == delay) {
			result = (uint32_t)EIO;
			break;
		}
		result = lpddr4_checkphyindepinterrupt(pd, irqbit, &irqstatus);
	} while ((irqstatus == (bool)false) && (result == (uint32_t)0));

	return result;
}

static uint32_t lpddr4_pollandackirq(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;

	result = lpddr4_pollphyindepirq(pd, LPDDR4_INTR_PHY_INDEP_INIT_DONE_BIT, LPDDR4_CUSTOM_TIMEOUT_DELAY);

	if (result == (uint32_t)0)
		result = lpddr4_ackphyindepinterrupt(pd, LPDDR4_INTR_PHY_INDEP_INIT_DONE_BIT);
	if (result == (uint32_t)0)
		result = lpddr4_pollctlirq(pd, LPDDR4_INTR_MC_INIT_DONE, LPDDR4_CUSTOM_TIMEOUT_DELAY);
	if (result == (uint32_t)0)
		result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_MC_INIT_DONE);
	return result;
}

static uint32_t lpddr4_startsequencecontroller(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	lpddr4_infotype infotype;

	regval = CPS_FLD_SET(LPDDR4__PI_START__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_START__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_START__REG)), regval);

	regval = CPS_FLD_SET(LPDDR4__START__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__START__REG)));
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__START__REG), regval);

	if (pd->infohandler != (lpddr4_infocallback)NULL) {
		infotype = LPDDR4_DRV_SOC_PLL_UPDATE;
		pd->infohandler(pd, infotype);
	}

	result = lpddr4_pollandackirq(pd);

	return result;
}

volatile uint32_t *lpddr4_addoffset(volatile uint32_t *addr, uint32_t regoffset)
{
	volatile uint32_t *local_addr = addr;
	volatile uint32_t *regaddr = &local_addr[regoffset];

	return regaddr;
}

uint32_t lpddr4_probe(const lpddr4_config *config, uint16_t *configsize)
{
	uint32_t result;

	result = (uint32_t)(lpddr4_probesf(config, configsize));
	if (result == (uint32_t)0)
		*configsize = (uint16_t)(sizeof(lpddr4_privatedata));
	return result;
}

uint32_t lpddr4_init(lpddr4_privatedata *pd, const lpddr4_config *cfg)
{
	uint32_t result = 0U;

	result = lpddr4_initsf(pd, cfg);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)cfg->ctlbase;
		pd->ctlbase = ctlregbase;
		pd->infohandler = (lpddr4_infocallback)cfg->infohandler;
		pd->ctlinterrupthandler = (lpddr4_ctlcallback)cfg->ctlinterrupthandler;
		pd->phyindepinterrupthandler = (lpddr4_phyindepcallback)cfg->phyindepinterrupthandler;
	}
	return result;
}

uint32_t lpddr4_start(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;

	result = lpddr4_startsf(pd);
	if (result == (uint32_t)0) {
		result = lpddr4_enablepiinitiator(pd);
		result = lpddr4_startsequencecontroller(pd);
		// lpddr4_enablelowpower();
	}
	return result;
}

uint32_t lpddr4_readreg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t *regvalue)
{
	uint32_t result = 0U;

	result = lpddr4_readregsf(pd, cpp, regvalue);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		if (cpp == LPDDR4_CTL_REGS) {
			if (regoffset >= LPDDR4_INTR_CTL_REG_COUNT)
				result = (uint32_t)-EINVAL;
			else
				*regvalue = CPS_REG_READ(lpddr4_addoffset(&(ctlregbase->DENALI_CTL_0), regoffset));
		} else if (cpp == LPDDR4_PHY_REGS) {
			if (regoffset >= LPDDR4_INTR_PHY_REG_COUNT)
				result = (uint32_t)-EINVAL;
			else
				*regvalue = CPS_REG_READ(lpddr4_addoffset(&(ctlregbase->DENALI_PHY_0), regoffset));

		} else {
			if (regoffset >= LPDDR4_INTR_PHY_INDEP_REG_COUNT)
				result = (uint32_t)-EINVAL;
			else
				*regvalue = CPS_REG_READ(lpddr4_addoffset(&(ctlregbase->DENALI_PI_0), regoffset));
		}
	}
	return result;
}

static uint32_t lpddr4_getphyrwmask(uint32_t regoffset)
{
	uint32_t rwmask = 0U;
	uint32_t arrayoffset = 0U;
	uint32_t slicenum, sliceoffset = 0U;

	for (slicenum = (uint32_t)0U; slicenum <= (DSLICE_NUM + ASLICE_NUM); slicenum++) {
		sliceoffset = sliceoffset + (uint32_t)SLICE_WIDTH;
		if (regoffset < sliceoffset)
			break;
	}
	arrayoffset = regoffset - (sliceoffset - (uint32_t)SLICE_WIDTH);

	if (slicenum < DSLICE_NUM) {
		rwmask = lpddr4_getdslicemask(slicenum, arrayoffset);
	} else {
		if (slicenum == DSLICE_NUM) {
			if (arrayoffset < ASLICE0_REG_COUNT)
				rwmask = g_lpddr4_address_slice_0_rw_mask[arrayoffset];
		} else {
			if (arrayoffset < PHY_CORE_REG_COUNT)
				rwmask = g_lpddr4_phy_core_rw_mask[arrayoffset];
		}
	}
	return rwmask;
}

#ifdef REG_WRITE_VERIF
static uint32_t lpddr4_verifyregwrite(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t regvalue)
{
	uint32_t result = (uint32_t)0;
	uint32_t regreadval = 0U;
	uint32_t rwmask = 0U;

	result = lpddr4_readreg(pd, cpp, regoffset, &regreadval);

	if (result == (uint32_t)0) {
		switch (cpp) {
		case LPDDR4_PHY_INDEP_REGS:
			rwmask = g_lpddr4_pi_rw_mask[regoffset];
			break;
		case LPDDR4_PHY_REGS:
			rwmask = lpddr4_getphyrwmask(regoffset);
			break;
		default:
			rwmask = g_lpddr4_ddr_controller_rw_mask[regoffset];
			break;
		}

		if ((rwmask & regreadval) != (regvalue & rwmask))
			result = EIO;
	}
	return result;
}
#endif

uint32_t lpddr4_deferredregverify(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result = (uint32_t)0;
	uint32_t aindex;
	uint32_t regreadval = 0U;
	uint32_t rwmask = 0U;

	result = lpddr4_deferredregverifysf(pd, cpp);

	if ((regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++) {
			result = lpddr4_readreg(pd, cpp, (uint32_t)regnum[aindex], &regreadval);

			if (result == (uint32_t)0) {
				switch (cpp) {
				case LPDDR4_PHY_INDEP_REGS:
					rwmask = g_lpddr4_pi_rw_mask[(uint32_t)regnum[aindex]];
					break;
				case LPDDR4_PHY_REGS:
					rwmask = lpddr4_getphyrwmask((uint32_t)regnum[aindex]);
					break;
				default:
					rwmask = g_lpddr4_ddr_controller_rw_mask[(uint32_t)regnum[aindex]];
					break;
				}

				if ((rwmask & regreadval) != ((uint32_t)(regvalues[aindex]) & rwmask)) {
					result = EIO;
					break;
				}
			}
		}
	}
	return result;
}

uint32_t lpddr4_writereg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t regvalue)
{
	uint32_t result = 0U;

	result = lpddr4_writeregsf(pd, cpp);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		if (cpp == LPDDR4_CTL_REGS) {
			if (regoffset >= LPDDR4_INTR_CTL_REG_COUNT)
				result = (uint32_t)-EINVAL;
			else
				CPS_REG_WRITE(lpddr4_addoffset(&(ctlregbase->DENALI_CTL_0), regoffset), regvalue);
		} else if (cpp == LPDDR4_PHY_REGS) {
			if (regoffset >= LPDDR4_INTR_PHY_REG_COUNT)
				result = (uint32_t)-EINVAL;
			else
				CPS_REG_WRITE(lpddr4_addoffset(&(ctlregbase->DENALI_PHY_0), regoffset), regvalue);
		} else {
			if (regoffset >= LPDDR4_INTR_PHY_INDEP_REG_COUNT)
				result = (uint32_t)-EINVAL;
			else
				CPS_REG_WRITE(lpddr4_addoffset(&(ctlregbase->DENALI_PI_0), regoffset), regvalue);
		}
	}
#ifdef REG_WRITE_VERIF
	if (result == (uint32_t)0)
		result = lpddr4_verifyregwrite(pd, cpp, regoffset, regvalue);

#endif

	return result;
}

uint32_t lpddr4_getmmrregister(const lpddr4_privatedata *pd, uint32_t readmoderegval, uint64_t *mmrvalue, uint8_t *mmrstatus)
{
	uint32_t result = 0U;
	uint32_t tdelay = 1000U;
	uint32_t regval = 0U;

	result = lpddr4_getmmrregistersf(pd, mmrvalue, mmrstatus);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		regval = CPS_FLD_WRITE(LPDDR4__READ_MODEREG__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__READ_MODEREG__REG)), readmoderegval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__READ_MODEREG__REG), regval);

		result = lpddr4_pollctlirq(pd, LPDDR4_INTR_MR_READ_DONE, tdelay);
	}
	if (result == (uint32_t)0)
		result = lpddr4_checkmmrreaderror(pd, mmrvalue, mmrstatus);
	return result;
}

static uint32_t lpddr4_writemmrregister(const lpddr4_privatedata *pd, uint32_t writemoderegval)
{
	uint32_t result = (uint32_t)0;
	uint32_t tdelay = 1000U;
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_WRITE(LPDDR4__WRITE_MODEREG__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__WRITE_MODEREG__REG)), writemoderegval);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__WRITE_MODEREG__REG), regval);

	result = lpddr4_pollctlirq(pd, LPDDR4_INTR_MR_WRITE_DONE, tdelay);

	return result;
}

uint32_t lpddr4_setmmrregister(const lpddr4_privatedata *pd, uint32_t writemoderegval, uint8_t *mrwstatus)
{
	uint32_t result = 0U;

	result = lpddr4_setmmrregistersf(pd, mrwstatus);
	if (result == (uint32_t)0) {
		result = lpddr4_writemmrregister(pd, writemoderegval);

		if (result == (uint32_t)0)
			result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_MR_WRITE_DONE);
		if (result == (uint32_t)0) {
			lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
			*mrwstatus = (uint8_t)CPS_FLD_READ(LPDDR4__MRW_STATUS__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MRW_STATUS__REG)));
			if ((*mrwstatus) != 0U)
				result = (uint32_t)EIO;
		}
	}

#ifdef ASILC
#endif

	return result;
}

uint32_t lpddr4_writectlconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_writectlconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL))
		result = -EINVAL;

	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_CTL_REGS, (uint32_t)regnum[aindex],
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}

// hnagalla -- 02/01/24
uint32_t lpddr4_writectlconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_writectlconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL))
		result = -EINVAL;

	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_CTL_REGS, aindex,
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}

uint32_t lpddr4_writephyindepconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_writephyindepconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_INDEP_REGS, (uint32_t)regnum[aindex],
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}
// hnagalla
uint32_t lpddr4_writephyindepconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_writephyindepconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_INDEP_REGS, aindex,
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}

uint32_t lpddr4_writephyconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_writephyconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)regnum[aindex],
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}
// hnagalla
uint32_t lpddr4_writephyconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_writephyconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, aindex,
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}

uint32_t lpddr4_readctlconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_readctlconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_CTL_REGS, (uint32_t)regnum[aindex],
							  (uint32_t *)(&regvalues[aindex]));
	}
	return result;
}

uint32_t lpddr4_readphyindepconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_readphyindepconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_INDEP_REGS, (uint32_t)regnum[aindex],
							  (uint32_t *)(&regvalues[aindex]));
	}
	return result;
}

uint32_t lpddr4_readphyconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result;
	uint32_t aindex;

	result = lpddr4_readphyconfigsf(pd);
	if ((regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL))
		result = -EINVAL;
	if (result == (uint32_t)0) {
		for (aindex = 0; aindex < regcount; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)regnum[aindex],
							  (uint32_t *)(&regvalues[aindex]));
	}
	return result;
}

uint32_t lpddr4_getphyindepinterruptmask(const lpddr4_privatedata *pd, uint32_t *mask)
{
	uint32_t result;

	result = lpddr4_getphyindepinterruptmsf(pd, mask);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		*mask = CPS_FLD_READ(LPDDR4__PI_INT_MASK__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INT_MASK__REG)));
	}
	return result;
}

uint32_t lpddr4_setphyindepinterruptmask(const lpddr4_privatedata *pd, const uint32_t *mask)
{
	uint32_t result;
	uint32_t regval = 0;
	const uint32_t ui32irqcount = (uint32_t)LPDDR4_INTR_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT + 1U;

	result = lpddr4_setphyindepinterruptmsf(pd, mask);
	if ((result == (uint32_t)0) && (ui32irqcount < WORD_SHIFT)) {
		if (*mask >= (1U << ui32irqcount))
			result = (uint32_t)-EINVAL;
	}
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		regval = CPS_FLD_WRITE(LPDDR4__PI_INT_MASK__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INT_MASK__REG)), *mask);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PI_INT_MASK__REG), regval);
	}
	return result;
}

uint32_t lpddr4_checkphyindepinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr, bool *irqstatus)
{
	uint32_t result = 0;
	uint32_t phyindepirqstatus = 0;

	result = LPDDR4_INTR_CheckPhyIndepIntSF(pd, intr, irqstatus);
	if ((result == (uint32_t)0) && ((uint32_t)intr < WORD_SHIFT)) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		phyindepirqstatus = CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INT_STATUS__REG));
		*irqstatus = (bool)(((phyindepirqstatus >> (uint32_t)intr) & LPDDR4_BIT_MASK) > 0U);
	}
	return result;
}

uint32_t lpddr4_ackphyindepinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	result = LPDDR4_INTR_AckPhyIndepIntSF(pd, intr);
	if ((result == (uint32_t)0) && ((uint32_t)intr < WORD_SHIFT)) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		regval = ((uint32_t)LPDDR4_BIT_MASK << (uint32_t)intr);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PI_INT_ACK__REG), regval);
	}

	return result;
}

static void lpddr4_checkcatrainingerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr)
{
	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress = (volatile uint32_t *)(&(ctlregbase->LPDDR4__PHY_ADR_CALVL_OBS1_0__REG));
	errbitmask = (CA_TRAIN_RL) | (NIBBLE_MASK);
	for (snum = 0U; snum < ASLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != CA_TRAIN_RL) {
			debuginfo->catraingerror = CDN_TRUE;
			*errfoundptr = true;
		}
		regaddress = lpddr4_addoffset(regaddress, (uint32_t)SLICE_WIDTH);
	}
}

static void lpddr4_checkgatelvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr)
{
	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress = (volatile uint32_t *)(&(ctlregbase->LPDDR4__PHY_GTLVL_STATUS_OBS_0__REG));
	errbitmask = GATE_LVL_ERROR_FIELDS;
	for (snum = (uint32_t)0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->gatelvlerror = CDN_TRUE;
			*errfoundptr = true;
		}
		regaddress = lpddr4_addoffset(regaddress, (uint32_t)SLICE_WIDTH);
	}
}

static void lpddr4_checkreadlvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr)
{
	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress = (volatile uint32_t *)(&(ctlregbase->LPDDR4__PHY_RDLVL_STATUS_OBS_0__REG));
	errbitmask = READ_LVL_ERROR_FIELDS;
	for (snum = (uint32_t)0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->readlvlerror = CDN_TRUE;
			*errfoundptr = true;
		}
		regaddress = lpddr4_addoffset(regaddress, (uint32_t)SLICE_WIDTH);
	}
}

static void lpddr4_checkdqtrainingerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr)
{
	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress = (volatile uint32_t *)(&(ctlregbase->LPDDR4__PHY_WDQLVL_STATUS_OBS_0__REG));
	errbitmask = DQ_LVL_STATUS;
	for (snum = (uint32_t)0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->dqtrainingerror = CDN_TRUE;
			*errfoundptr = true;
		}
		regaddress = lpddr4_addoffset(regaddress, (uint32_t)SLICE_WIDTH);
	}
}

bool lpddr4_checklvlerrors(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo, bool errfound)
{
	bool localerrfound = errfound;

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	if (localerrfound == (bool)false)
		lpddr4_checkcatrainingerror(ctlregbase, debuginfo, &localerrfound);

	if (localerrfound == (bool)false)
		lpddr4_checkwrlvlerror(ctlregbase, debuginfo, &localerrfound);

	if (localerrfound == (bool)false)
		lpddr4_checkgatelvlerror(ctlregbase, debuginfo, &localerrfound);

	if (localerrfound == (bool)false)
		lpddr4_checkreadlvlerror(ctlregbase, debuginfo, &localerrfound);

	if (localerrfound == (bool)false)
		lpddr4_checkdqtrainingerror(ctlregbase, debuginfo, &localerrfound);
	return localerrfound;
}

static uint8_t lpddr4_seterror(volatile uint32_t *reg, uint32_t errbitmask, uint8_t *errfoundptr, const uint32_t errorinfobits)
{
	uint32_t regval = 0U;

	regval = CPS_REG_READ(reg);
	if ((regval & errbitmask) != errorinfobits)
		*errfoundptr = CDN_TRUE;
	return *errfoundptr;
}

void lpddr4_seterrors(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, uint8_t *errfoundptr)
{
	uint32_t errbitmask = (LPDDR4_BIT_MASK << 0x1U) | (LPDDR4_BIT_MASK);

	debuginfo->pllerror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_PLL_OBS_0__REG),
					      errbitmask, errfoundptr, PLL_READY);
	if (*errfoundptr == CDN_FALSE)
		debuginfo->pllerror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_PLL_OBS_1__REG),
						      errbitmask, errfoundptr, PLL_READY);

	if (*errfoundptr == CDN_FALSE)
		debuginfo->iocaliberror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_CAL_RESULT_OBS_0__REG),
							  IO_CALIB_DONE, errfoundptr, IO_CALIB_DONE);
	if (*errfoundptr == CDN_FALSE)
		debuginfo->iocaliberror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_CAL_RESULT2_OBS_0__REG),
							  IO_CALIB_DONE, errfoundptr, IO_CALIB_DONE);
	if (*errfoundptr == CDN_FALSE)
		debuginfo->iocaliberror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_CAL_RESULT3_OBS_0__REG),
							  IO_CALIB_FIELD, errfoundptr, IO_CALIB_STATE);
}

static void lpddr4_setphysnapsettings(lpddr4_ctlregs *ctlregbase, const bool errorfound)
{
	uint32_t snum = 0U;
	volatile uint32_t *regaddress;
	uint32_t regval = 0U;

	if (errorfound == (bool)false) {
		regaddress = (volatile uint32_t *)(&(ctlregbase->LPDDR4__SC_PHY_SNAP_OBS_REGS_0__REG));
		for (snum = (uint32_t)0U; snum < DSLICE_NUM; snum++) {
			regval = CPS_FLD_SET(LPDDR4__SC_PHY_SNAP_OBS_REGS_0__FLD, CPS_REG_READ(regaddress));
			CPS_REG_WRITE(regaddress, regval);
			regaddress = lpddr4_addoffset(regaddress, (uint32_t)SLICE_WIDTH);
		}
	}
}

static void lpddr4_setphyadrsnapsettings(lpddr4_ctlregs *ctlregbase, const bool errorfound)
{
	uint32_t snum = 0U;
	volatile uint32_t *regaddress;
	uint32_t regval = 0U;

	if (errorfound == (bool)false) {
		regaddress = (volatile uint32_t *)(&(ctlregbase->LPDDR4__SC_PHY_ADR_SNAP_OBS_REGS_0__REG));
		for (snum = (uint32_t)0U; snum < ASLICE_NUM; snum++) {
			regval = CPS_FLD_SET(LPDDR4__SC_PHY_ADR_SNAP_OBS_REGS_0__FLD, CPS_REG_READ(regaddress));
			CPS_REG_WRITE(regaddress, regval);
			regaddress = lpddr4_addoffset(regaddress, (uint32_t)SLICE_WIDTH);
		}
	}
}

void lpddr4_setsettings(lpddr4_ctlregs *ctlregbase, const bool errorfound)
{
	lpddr4_setphysnapsettings(ctlregbase, errorfound);
	lpddr4_setphyadrsnapsettings(ctlregbase, errorfound);
}

static void readpdwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles)
{
	if (*fspnum == LPDDR4_FSP_0)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_PD_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F0__REG)));
	else if (*fspnum == LPDDR4_FSP_1)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_PD_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F1__REG)));
	else
		*cycles = CPS_FLD_READ(LPDDR4__LPI_PD_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F2__REG)));
}

static void readsrshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles)
{
	if (*fspnum == LPDDR4_FSP_0)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_SHORT_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F0__REG)));
	else if (*fspnum == LPDDR4_FSP_1)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_SHORT_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F1__REG)));
	else
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_SHORT_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F2__REG)));
}

static void readsrlongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles)
{
	if (*fspnum == LPDDR4_FSP_0)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_LONG_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F0__REG)));
	else if (*fspnum == LPDDR4_FSP_1)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_LONG_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F1__REG)));
	else
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_LONG_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F2__REG)));
}

static void readsrlonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles)
{
	if (*fspnum == LPDDR4_FSP_0)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__REG)));
	else if (*fspnum == LPDDR4_FSP_1)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__REG)));
	else
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__REG)));
}

static void readsrdpshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles)
{
	if (*fspnum == LPDDR4_FSP_0)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__REG)));
	else if (*fspnum == LPDDR4_FSP_1)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__REG)));
	else
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__REG)));
}

static void readsrdplongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles)
{
	if (*fspnum == LPDDR4_FSP_0)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__REG)));
	else if (*fspnum == LPDDR4_FSP_1)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__REG)));
	else
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__REG)));
}

static void readsrdplonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, uint32_t *cycles)
{
	if (*fspnum == LPDDR4_FSP_0)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__REG)));
	else if (*fspnum == LPDDR4_FSP_1)
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__REG)));
	else
		*cycles = CPS_FLD_READ(LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__REG)));

}

static void lpddr4_readlpiwakeuptime(lpddr4_ctlregs *ctlregbase, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, uint32_t *cycles)
{
	if (*lpiwakeupparam == LPDDR4_LPI_PD_WAKEUP_FN)
		readpdwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SR_SHORT_WAKEUP_FN)
		readsrshortwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_WAKEUP_FN)
		readsrlongwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN)
		readsrlonggatewakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN)
		readsrdpshortwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_LONG_WAKEUP_FN)
		readsrdplongwakeup(fspnum, ctlregbase, cycles);
	else
		readsrdplonggatewakeup(fspnum, ctlregbase, cycles);
}

uint32_t lpddr4_getlpiwakeuptime(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, uint32_t *cycles)
{
	uint32_t result = 0U;

	result = lpddr4_getlpiwakeuptimesf(pd, lpiwakeupparam, fspnum, cycles);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		lpddr4_readlpiwakeuptime(ctlregbase, lpiwakeupparam, fspnum, cycles);
	}
	return result;
}

static void writepdwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles)
{
	uint32_t regval = 0U;

	if (*fspnum == LPDDR4_FSP_0) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_PD_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F0__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_PD_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F1__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F1__REG), regval);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_PD_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F2__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_PD_WAKEUP_F2__REG), regval);
	}
}

static void writesrshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles)
{
	uint32_t regval = 0U;

	if (*fspnum == LPDDR4_FSP_0) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_SHORT_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F0__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_SHORT_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F1__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F1__REG), regval);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_SHORT_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F2__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_SHORT_WAKEUP_F2__REG), regval);
	}
}

static void writesrlongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles)
{
	uint32_t regval = 0U;

	if (*fspnum == LPDDR4_FSP_0) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F0__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F1__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F1__REG), regval);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F2__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_WAKEUP_F2__REG), regval);
	}
}

static void writesrlonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles)
{
	uint32_t regval = 0U;

	if (*fspnum == LPDDR4_FSP_0) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1__REG), regval);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2__REG), regval);
	}
}

static void writesrdpshortwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles)
{
	uint32_t regval = 0U;

	if (*fspnum == LPDDR4_FSP_0) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F1__REG), regval);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_SHORT_WAKEUP_F2__REG), regval);
	}
}

static void writesrdplongwakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles)
{
	uint32_t regval = 0U;

	if (*fspnum == LPDDR4_FSP_0) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F1__REG), regval);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_WAKEUP_F2__REG), regval);
	}
}
static void writesrdplonggatewakeup(const lpddr4_ctlfspnum *fspnum, lpddr4_ctlregs *ctlregbase, const uint32_t *cycles)
{
	uint32_t regval = 0U;

	if (*fspnum == LPDDR4_FSP_0) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0__REG), regval);
	} else if (*fspnum == LPDDR4_FSP_1) {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1__REG), regval);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__REG)), *cycles);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2__REG), regval);
	}
}

static void lpddr4_writelpiwakeuptime(lpddr4_ctlregs *ctlregbase, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const uint32_t *cycles)
{
	if (*lpiwakeupparam == LPDDR4_LPI_PD_WAKEUP_FN)
		writepdwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SR_SHORT_WAKEUP_FN)
		writesrshortwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_WAKEUP_FN)
		writesrlongwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN)
		writesrlonggatewakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN)
		writesrdpshortwakeup(fspnum, ctlregbase, cycles);
	else if (*lpiwakeupparam == LPDDR4_LPI_SRPD_LONG_WAKEUP_FN)
		writesrdplongwakeup(fspnum, ctlregbase, cycles);
	else
		writesrdplonggatewakeup(fspnum, ctlregbase, cycles);
}

uint32_t lpddr4_setlpiwakeuptime(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const uint32_t *cycles)
{
	uint32_t result = 0U;

	result = lpddr4_setlpiwakeuptimesf(pd, lpiwakeupparam, fspnum, cycles);
	if (result == (uint32_t)0) {
		if (*cycles > NIBBLE_MASK)
			result = (uint32_t)-EINVAL;
	}
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		lpddr4_writelpiwakeuptime(ctlregbase, lpiwakeupparam, fspnum, cycles);
	}
	return result;
}

uint32_t lpddr4_getdbireadmode(const lpddr4_privatedata *pd, bool *on_off)
{
	uint32_t result = 0U;

	result = lpddr4_getdbireadmodesf(pd, on_off);

	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		if (CPS_FLD_READ(LPDDR4__RD_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__RD_DBI_EN__REG))) == 0U)
			*on_off = false;
		else
			*on_off = true;
	}
	return result;
}

uint32_t lpddr4_getdbiwritemode(const lpddr4_privatedata *pd, bool *on_off)
{
	uint32_t result = 0U;

	result = lpddr4_getdbireadmodesf(pd, on_off);

	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		if (CPS_FLD_READ(LPDDR4__WR_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__WR_DBI_EN__REG))) == 0U)
			*on_off = false;
		else
			*on_off = true;
	}
	return result;
}

uint32_t lpddr4_setdbimode(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	result = lpddr4_setdbimodesf(pd, mode);

	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		if (*mode == LPDDR4_DBI_RD_ON)
			regval = CPS_FLD_WRITE(LPDDR4__RD_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__RD_DBI_EN__REG)), 1U);
		else if (*mode == LPDDR4_DBI_RD_OFF)
			regval = CPS_FLD_WRITE(LPDDR4__RD_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__RD_DBI_EN__REG)), 0U);
		else if (*mode == LPDDR4_DBI_WR_ON)
			regval = CPS_FLD_WRITE(LPDDR4__WR_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__WR_DBI_EN__REG)), 1U);
		else
			regval = CPS_FLD_WRITE(LPDDR4__WR_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__WR_DBI_EN__REG)), 0U);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__RD_DBI_EN__REG), regval);
	}
	return result;
}

uint32_t lpddr4_getrefreshrate(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, uint32_t *tref, uint32_t *tras_max)
{
	uint32_t result = 0U;

	result = lpddr4_getrefreshratesf(pd, fspnum, tref, tras_max);

	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		switch (*fspnum) {
		case LPDDR4_FSP_2:
			*tref = CPS_FLD_READ(LPDDR4__TREF_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_F2__REG)));
			*tras_max = CPS_FLD_READ(LPDDR4__TRAS_MAX_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TRAS_MAX_F2__REG)));
			break;
		case LPDDR4_FSP_1:
			*tref = CPS_FLD_READ(LPDDR4__TREF_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_F1__REG)));
			*tras_max = CPS_FLD_READ(LPDDR4__TRAS_MAX_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TRAS_MAX_F1__REG)));
			break;
		default:
			*tref = CPS_FLD_READ(LPDDR4__TREF_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_F0__REG)));
			*tras_max = CPS_FLD_READ(LPDDR4__TRAS_MAX_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TRAS_MAX_F0__REG)));
			break;
		}
	}
	return result;
}

static void lpddr4_updatefsp2refrateparams(const lpddr4_privatedata *pd, const uint32_t *tref, const uint32_t *tras_max)
{
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_WRITE(LPDDR4__TREF_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_F2__REG)), *tref);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_F2__REG), regval);
	regval = CPS_FLD_WRITE(LPDDR4__TRAS_MAX_F2__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TRAS_MAX_F2__REG)), *tras_max);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__TRAS_MAX_F2__REG), regval);
}

static void lpddr4_updatefsp1refrateparams(const lpddr4_privatedata *pd, const uint32_t *tref, const uint32_t *tras_max)
{
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_WRITE(LPDDR4__TREF_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_F1__REG)), *tref);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_F1__REG), regval);
	regval = CPS_FLD_WRITE(LPDDR4__TRAS_MAX_F1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TRAS_MAX_F1__REG)), *tras_max);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__TRAS_MAX_F1__REG), regval);;
}

static void lpddr4_updatefsp0refrateparams(const lpddr4_privatedata *pd, const uint32_t *tref, const uint32_t *tras_max)
{
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_WRITE(LPDDR4__TREF_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_F0__REG)), *tref);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_F0__REG), regval);
	regval = CPS_FLD_WRITE(LPDDR4__TRAS_MAX_F0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TRAS_MAX_F0__REG)), *tras_max);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__TRAS_MAX_F0__REG), regval);
}

uint32_t lpddr4_setrefreshrate(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const uint32_t *tref, const uint32_t *tras_max)
{
	uint32_t result = 0U;

	result = lpddr4_setrefreshratesf(pd, fspnum, tref, tras_max);

	if (result == (uint32_t)0) {
		switch (*fspnum) {
		case LPDDR4_FSP_2:
			lpddr4_updatefsp2refrateparams(pd, tref, tras_max);
			break;
		case LPDDR4_FSP_1:
			lpddr4_updatefsp1refrateparams(pd, tref, tras_max);
			break;
		default:
			lpddr4_updatefsp0refrateparams(pd, tref, tras_max);
			break;
		}
	}
	return result;
}

uint32_t lpddr4_refreshperchipselect(const lpddr4_privatedata *pd, const uint32_t trefinterval)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	result = lpddr4_refreshperchipselectsf(pd);

	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		regval = CPS_FLD_WRITE(LPDDR4__TREF_INTERVAL__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_INTERVAL__REG)), trefinterval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_INTERVAL__REG), regval);
	}
	return result;
}
