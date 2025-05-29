// SPDX-License-Identifier: BSD-3-Clause
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2026 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <errno.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <libfdt.h>

#include "cps_drv_lpddr4.h"
#include "lpddr4.h"
#include "lpddr4_if.h"
#include "lpddr4_structs_if.h"

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
	if (result != (uint32_t)0) {
		return result;
	}

	result = lpddr4_ackphyindepinterrupt(pd, LPDDR4_INTR_PHY_INDEP_INIT_DONE_BIT);
	if (result != (uint32_t)0) {
		return result;
	}

	result = lpddr4_pollctlirq(pd, LPDDR4_INTR_MC_INIT_DONE, LPDDR4_CUSTOM_TIMEOUT_DELAY);
	if (result != (uint32_t)0) {
		return result;
	}

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
	if ((configsize == NULL) || (config == NULL)) {
		return EINVAL;
	}

	*configsize = (uint16_t)(sizeof(lpddr4_privatedata));
	return 0U;
}

uint32_t lpddr4_init(lpddr4_privatedata *pd, const lpddr4_config *cfg)
{
	if ((pd == NULL) || (cfg == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)cfg->ctlbase;
	pd->ctlbase = ctlregbase;
	pd->infohandler = (lpddr4_infocallback)cfg->infohandler;
	pd->ctlinterrupthandler = (lpddr4_ctlcallback)cfg->ctlinterrupthandler;
	pd->phyindepinterrupthandler = (lpddr4_phyindepcallback)cfg->phyindepinterrupthandler;
	return 0U;
}

uint32_t lpddr4_start(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;

	if (pd == NULL) {
		return EINVAL;
	}

	result = lpddr4_enablepiinitiator(pd);
	if (result != (uint32_t)0) {
		return result;
	}

	result = lpddr4_startsequencecontroller(pd);

	return result;
}

uint32_t lpddr4_readreg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t *regvalue)
{
	if ((pd == NULL) || (regvalue == NULL)) {
		return EINVAL;
	} else if ((cpp != LPDDR4_CTL_REGS) &&
			(cpp != LPDDR4_PHY_REGS) &&
			(cpp != LPDDR4_PHY_INDEP_REGS)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	if (cpp == LPDDR4_CTL_REGS) {
		if (regoffset >= LPDDR4_INTR_CTL_REG_COUNT) {
			return EINVAL;
		} else {
			*regvalue = CPS_REG_READ(lpddr4_addoffset(&(ctlregbase->DENALI_CTL_0), regoffset));
		}
	} else if (cpp == LPDDR4_PHY_REGS) {
		if (regoffset >= LPDDR4_INTR_PHY_REG_COUNT) {
			return EINVAL;
		} else {
			*regvalue = CPS_REG_READ(lpddr4_addoffset(&(ctlregbase->DENALI_PHY_0), regoffset));
		}

	} else {
		if (regoffset >= LPDDR4_INTR_PHY_INDEP_REG_COUNT) {
			return EINVAL;
		} else {
			*regvalue = CPS_REG_READ(lpddr4_addoffset(&(ctlregbase->DENALI_PI_0), regoffset));
		}
	}

	return 0U;
}

uint32_t lpddr4_writereg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, uint32_t regoffset, uint32_t regvalue)
{
	if (pd == NULL) {
		return EINVAL;
	} else if ((cpp != LPDDR4_CTL_REGS) &&
			(cpp != LPDDR4_PHY_REGS) &&
			(cpp != LPDDR4_PHY_INDEP_REGS)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	if (cpp == LPDDR4_CTL_REGS) {
		if (regoffset >= LPDDR4_INTR_CTL_REG_COUNT) {
			return EINVAL;
		} else {
			CPS_REG_WRITE(lpddr4_addoffset(&(ctlregbase->DENALI_CTL_0), regoffset), regvalue);
		}
	} else if (cpp == LPDDR4_PHY_REGS) {
		if (regoffset >= LPDDR4_INTR_PHY_REG_COUNT) {
			return EINVAL;
		} else {
			CPS_REG_WRITE(lpddr4_addoffset(&(ctlregbase->DENALI_PHY_0), regoffset), regvalue);
		}
	} else {
		if (regoffset >= LPDDR4_INTR_PHY_INDEP_REG_COUNT) {
			return EINVAL;
		} else {
			CPS_REG_WRITE(lpddr4_addoffset(&(ctlregbase->DENALI_PI_0), regoffset), regvalue);
		}
	}

	return 0U;
}

uint32_t lpddr4_getmmrregister(const lpddr4_privatedata *pd, uint32_t readmoderegval, uint64_t *mmrvalue, uint8_t *mmrstatus)
{
	uint32_t result = 0U;
	uint32_t tdelay = 1000U;
	uint32_t regval = 0U;

	if ((pd == NULL) || (mmrvalue == NULL) || (mmrstatus == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_WRITE(LPDDR4__READ_MODEREG__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__READ_MODEREG__REG)), readmoderegval);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__READ_MODEREG__REG), regval);

	result = lpddr4_pollctlirq(pd, LPDDR4_INTR_MR_READ_DONE, tdelay);
	if (result != (uint32_t)0) {
		return result;
	}

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

	if ((pd == NULL) || (mrwstatus == NULL)) {
		return EINVAL;
	}

	result = lpddr4_writemmrregister(pd, writemoderegval);
	if (result != (uint32_t)0) {
		return result;
	}

	result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_MR_WRITE_DONE);
	if (result != (uint32_t)0) {
		return result;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	*mrwstatus = (uint8_t)CPS_FLD_READ(LPDDR4__MRW_STATUS__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MRW_STATUS__REG)));
	if ((*mrwstatus) != 0U) {
		result = (uint32_t)EIO;
	}

	return result;
}

uint32_t lpddr4_writectlconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL)) {
		return EINVAL;
	}

	for (aindex = 0; aindex < regcount; aindex++) {
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_CTL_REGS, (uint32_t)regnum[aindex],
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}

uint32_t lpddr4_writectlconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;
	uint32_t reg_val;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL)) {
		return EINVAL;
	}

	/* Iterate over the config data to convert from device node
	 * format to CPU format.
	 */
	for (aindex = 0; aindex < regcount; aindex++) {
		reg_val = fdt32_to_cpu(regvalues[aindex]);
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_CTL_REGS, aindex,
							   reg_val);
	}

	return result;
}

uint32_t lpddr4_writephyindepconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL)) {
		return EINVAL;
	}

	for (aindex = 0; aindex < regcount; aindex++) {
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_INDEP_REGS, (uint32_t)regnum[aindex],
				   (uint32_t)regvalues[aindex]);
	}
	return result;
}

uint32_t lpddr4_writephyindepconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;
	uint32_t reg_val;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL)) {
		return EINVAL;
	}

	/* Iterate over the config data to convert from device node
	 * format to CPU format.
	 */
	for (aindex = 0; aindex < regcount; aindex++) {
		reg_val = fdt32_to_cpu(regvalues[aindex]);
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_INDEP_REGS, aindex,
							   reg_val);
	}

	return result;
}

uint32_t lpddr4_writephyconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL)) {
		return EINVAL;
	}

	for (aindex = 0; aindex < regcount; aindex++) {
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)regnum[aindex],
							   (uint32_t)regvalues[aindex]);
	}
	return result;
}

uint32_t lpddr4_writephyconfigex(const lpddr4_privatedata *pd, const uint32_t regvalues[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;
	uint32_t reg_val;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL)) {
		return EINVAL;
	}

	/* Iterate over the config data to convert from device node
	 * format to CPU format.
	 */
	for (aindex = 0; aindex < regcount; aindex++) {
		reg_val = fdt32_to_cpu(regvalues[aindex]);
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, aindex,
							   reg_val);
	}

	return result;
}

uint32_t lpddr4_readctlconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL)) {
		return EINVAL;
	}

	for (aindex = 0; aindex < regcount; aindex++) {
		result = (uint32_t)lpddr4_readreg(pd, LPDDR4_CTL_REGS, (uint32_t)regnum[aindex],
						  (uint32_t *)(&regvalues[aindex]));
	}
	return result;
}

uint32_t lpddr4_readphyindepconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL)) {
		return EINVAL;
	}

	for (aindex = 0; aindex < regcount; aindex++) {
		result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_INDEP_REGS, (uint32_t)regnum[aindex],
						  (uint32_t *)(&regvalues[aindex]));
	}
	return result;
}

uint32_t lpddr4_readphyconfig(const lpddr4_privatedata *pd, uint32_t regvalues[], uint16_t regnum[], uint16_t regcount)
{
	uint32_t result = 0U;
	uint32_t aindex;

	if ((pd == NULL) || (regvalues == (uint32_t *)NULL) || (regnum == (uint16_t *)NULL)) {
		return EINVAL;
	}

	for (aindex = 0; aindex < regcount; aindex++) {
		result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)regnum[aindex],
						  (uint32_t *)(&regvalues[aindex]));
	}
	return result;
}

uint32_t lpddr4_getphyindepinterruptmask(const lpddr4_privatedata *pd, uint32_t *mask)
{
	if ((pd == NULL) || (mask == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	*mask = CPS_FLD_READ(LPDDR4__PI_INT_MASK__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INT_MASK__REG)));

	return 0U;
}

uint32_t lpddr4_setphyindepinterruptmask(const lpddr4_privatedata *pd, const uint32_t *mask)
{
	uint32_t regval = 0;
	const uint32_t ui32irqcount = (uint32_t)LPDDR4_INTR_PHY_INDEP_DLL_LOCK_STATE_CHANGE_BIT + 1U;

	if ((pd == NULL) || (mask == NULL)) {
		return EINVAL;
	}

	if (ui32irqcount < WORD_SHIFT) {
		if (*mask >= (1U << ui32irqcount))
			return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_WRITE(LPDDR4__PI_INT_MASK__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INT_MASK__REG)), *mask);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__PI_INT_MASK__REG), regval);

	return 0U;
}

uint32_t lpddr4_checkphyindepinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr, bool *irqstatus)
{
	uint32_t result = 0;
	uint32_t phyindepirqstatus = 0;

	result = lpddr4_intr_phyint_sf(pd, intr, irqstatus);
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

	result = lpddr4_intr_ack_phyint_sf(pd, intr);
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

	if (localerrfound == (bool)false) {
		lpddr4_checkcatrainingerror(ctlregbase, debuginfo, &localerrfound);
	}

	if (localerrfound == (bool)false) {
		lpddr4_checkwrlvlerror(ctlregbase, debuginfo, &localerrfound);
	}

	if (localerrfound == (bool)false) {
		lpddr4_checkgatelvlerror(ctlregbase, debuginfo, &localerrfound);
	}

	if (localerrfound == (bool)false) {
		lpddr4_checkreadlvlerror(ctlregbase, debuginfo, &localerrfound);
	}

	if (localerrfound == (bool)false) {
		lpddr4_checkdqtrainingerror(ctlregbase, debuginfo, &localerrfound);
	}
	return localerrfound;
}

static uint8_t lpddr4_seterror(volatile uint32_t *reg, uint32_t errbitmask, uint8_t *errfoundptr, const uint32_t errorinfobits)
{
	uint32_t regval = 0U;

	regval = CPS_REG_READ(reg);
	if ((regval & errbitmask) != errorinfobits) {
		*errfoundptr = CDN_TRUE;
	}
	return *errfoundptr;
}

void lpddr4_seterrors(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, uint8_t *errfoundptr)
{
	uint32_t errbitmask = (LPDDR4_BIT_MASK << 0x1U) | (LPDDR4_BIT_MASK);

	debuginfo->pllerror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_PLL_OBS_0__REG),
					      errbitmask, errfoundptr, PLL_READY);
	if (*errfoundptr == CDN_FALSE) {
		debuginfo->pllerror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_PLL_OBS_1__REG),
						      errbitmask, errfoundptr, PLL_READY);
	}

	if (*errfoundptr == CDN_FALSE) {
		debuginfo->iocaliberror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_CAL_RESULT_OBS_0__REG),
							  IO_CALIB_DONE, errfoundptr, IO_CALIB_DONE);
	}
	if (*errfoundptr == CDN_FALSE) {
		debuginfo->iocaliberror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_CAL_RESULT2_OBS_0__REG),
							  IO_CALIB_DONE, errfoundptr, IO_CALIB_DONE);
	}
	if (*errfoundptr == CDN_FALSE) {
		debuginfo->iocaliberror = lpddr4_seterror(&(ctlregbase->LPDDR4__PHY_CAL_RESULT3_OBS_0__REG),
							  IO_CALIB_FIELD, errfoundptr, IO_CALIB_STATE);
	}
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

uint32_t lpddr4_getdbireadmode(const lpddr4_privatedata *pd, bool *on_off)
{
	if ((pd == NULL) || (on_off == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	if (CPS_FLD_READ(LPDDR4__RD_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__RD_DBI_EN__REG))) == 0U) {
		*on_off = false;
	} else {
		*on_off = true;
	}

	return 0U;
}

uint32_t lpddr4_getdbiwritemode(const lpddr4_privatedata *pd, bool *on_off)
{
	if ((pd == NULL) || (on_off == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	if (CPS_FLD_READ(LPDDR4__WR_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__WR_DBI_EN__REG))) == 0U) {
		*on_off = false;
	} else {
		*on_off = true;
	}

	return 0U;
}

uint32_t lpddr4_setdbimode(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode)
{
	uint32_t regval = 0U;

	if ((pd  == NULL) || (mode == NULL)) {
		return EINVAL;
	} else if ((*mode != LPDDR4_DBI_RD_ON) &&
			(*mode != LPDDR4_DBI_RD_OFF) &&
			(*mode != LPDDR4_DBI_WR_ON) &&
			(*mode != LPDDR4_DBI_WR_OFF)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	if (*mode == LPDDR4_DBI_RD_ON) {
		regval = CPS_FLD_WRITE(LPDDR4__RD_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__RD_DBI_EN__REG)), 1U);
	} else if (*mode == LPDDR4_DBI_RD_OFF) {
		regval = CPS_FLD_WRITE(LPDDR4__RD_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__RD_DBI_EN__REG)), 0U);
	} else if (*mode == LPDDR4_DBI_WR_ON) {
		regval = CPS_FLD_WRITE(LPDDR4__WR_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__WR_DBI_EN__REG)), 1U);
	} else {
		regval = CPS_FLD_WRITE(LPDDR4__WR_DBI_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__WR_DBI_EN__REG)), 0U);
	}
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__RD_DBI_EN__REG), regval);

	return 0U;
}

uint32_t lpddr4_getrefreshrate(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, uint32_t *tref, uint32_t *tras_max)
{
	if ((pd == NULL) || (tref == NULL) || (tras_max == NULL)) {
		return EINVAL;
	} else if ((*fspnum != LPDDR4_FSP_0) &&
			(*fspnum != LPDDR4_FSP_1) &&
			(*fspnum != LPDDR4_FSP_2)) {
		return EINVAL;
	}

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

	return 0U;
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
	if ((pd == NULL) || (fspnum != NULL) || (tref ==  NULL) || (tras_max == NULL)) {
		return EINVAL;
	} else if ((*fspnum != LPDDR4_FSP_0) &&
			(*fspnum != LPDDR4_FSP_1) &&
			(*fspnum != LPDDR4_FSP_2)) {
		return EINVAL;
	}

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

	return 0U;
}

uint32_t lpddr4_refreshperchipselect(const lpddr4_privatedata *pd, const uint32_t trefinterval)
{
	uint32_t regval = 0U;

	if (pd == NULL) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	regval = CPS_FLD_WRITE(LPDDR4__TREF_INTERVAL__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__TREF_INTERVAL__REG)), trefinterval);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__TREF_INTERVAL__REG), regval);

	return 0U;
}
