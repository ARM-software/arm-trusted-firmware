// SPDX-License-Identifier: BSD-3-Clause
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <errno.h>

#include "cps_drv_lpddr4.h"
#include "lpddr4_ctl_regs.h"
#include "lpddr4_if.h"
#include "lpddr4.h"
#include "lpddr4_structs_if.h"

static uint32_t ctlintmap[51][3] = {
	{ 0,  0,  7  },
	{ 1,  0,  8  },
	{ 2,  0,  9  },
	{ 3,  0,  14 },
	{ 4,  0,  15 },
	{ 5,  0,  16 },
	{ 6,  0,  17 },
	{ 7,  0,  19 },
	{ 8,  1,  0  },
	{ 9,  2,  0  },
	{ 10, 2,  3  },
	{ 11, 3,  0  },
	{ 12, 4,  0  },
	{ 13, 5,  11 },
	{ 14, 5,  12 },
	{ 15, 5,  13 },
	{ 16, 5,  14 },
	{ 17, 5,  15 },
	{ 18, 6,  0  },
	{ 19, 6,  1  },
	{ 20, 6,  2  },
	{ 21, 6,  6  },
	{ 22, 6,  7  },
	{ 23, 7,  3  },
	{ 24, 7,  4  },
	{ 25, 7,  5  },
	{ 26, 7,  6  },
	{ 27, 7,  7  },
	{ 28, 8,  0  },
	{ 29, 9,  0  },
	{ 30, 10, 0  },
	{ 31, 10, 1  },
	{ 32, 10, 2  },
	{ 33, 10, 3  },
	{ 34, 10, 4  },
	{ 35, 10, 5  },
	{ 36, 11, 0  },
	{ 37, 12, 0  },
	{ 38, 12, 1  },
	{ 39, 12, 2  },
	{ 40, 12, 3  },
	{ 41, 12, 4  },
	{ 42, 12, 5  },
	{ 43, 13, 0  },
	{ 44, 13, 1  },
	{ 45, 13, 3  },
	{ 46, 14, 0  },
	{ 47, 14, 2  },
	{ 48, 14, 3  },
	{ 49, 15, 2  },
	{ 50, 16, 0  },
};

static void lpddr4_checkctlinterrupt_4(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr, uint32_t *ctlgrpirqstatus, uint32_t *ctlmasterintflag);
static void lpddr4_checkctlinterrupt_3(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr, uint32_t *ctlgrpirqstatus, uint32_t *ctlmasterintflag);
static void lpddr4_checkctlinterrupt_2(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr, uint32_t *ctlgrpirqstatus, uint32_t *ctlmasterintflag);
static void lpddr4_ackctlinterrupt_4(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr);
static void lpddr4_ackctlinterrupt_3(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr);
static void lpddr4_ackctlinterrupt_2(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr);

uint32_t lpddr4_enablepiinitiator(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_SET(LPDDR4__PI_NORMAL_LVL_SEQ__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_NORMAL_LVL_SEQ__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_NORMAL_LVL_SEQ__REG)), regval);
	regval = CPS_FLD_SET(LPDDR4__PI_INIT_LVL_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)), regval);

	return result;
}

uint32_t lpddr4_getctlinterruptmask(const lpddr4_privatedata *pd, uint64_t *mask)
{
	uint32_t result = 0U;

	result = lpddr4_getctlinterruptmasksf(pd, mask);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		*mask = (uint64_t)(CPS_FLD_READ(LPDDR4__INT_MASK_MASTER__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_MASTER__REG))));
	}
	return result;
}

uint32_t lpddr4_setctlinterruptmask(const lpddr4_privatedata *pd, const uint64_t *mask)
{
	uint32_t result;
	uint32_t regval = 0;
	const uint64_t ui64one = 1ULL;
	const uint32_t ui32irqcount = (uint32_t)32U;

	result = lpddr4_setctlinterruptmasksf(pd, mask);
	if ((result == (uint32_t)0) && (ui32irqcount < 64U)) {
		if (*mask >= (ui64one << ui32irqcount))
			result = (uint32_t)-EINVAL;
	}

	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		regval = CPS_FLD_WRITE(LPDDR4__INT_MASK_MASTER__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_MASTER__REG)), *mask);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_MASK_MASTER__REG), regval);
	}
	return result;
}

static void lpddr4_checkctlinterrupt_4(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr,
				       uint32_t *ctlgrpirqstatus, uint32_t *ctlmasterintflag)
{
	if ((intr >= LPDDR4_INTR_INIT_MEM_RESET_DONE) && (intr <= LPDDR4_INTR_INIT_POWER_ON_STATE))
		*ctlgrpirqstatus = CPS_FLD_READ(LPDDR4__INT_STATUS_INIT__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_INIT__REG)));
	else if ((intr >= LPDDR4_INTR_MRR_ERROR) && (intr <= LPDDR4_INTR_MR_WRITE_DONE))
		*ctlgrpirqstatus = CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_MODE__REG));
	else if (intr == LPDDR4_INTR_BIST_DONE)
		*ctlgrpirqstatus = CPS_FLD_READ(LPDDR4__INT_STATUS_BIST__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_BIST__REG)));
	else if (intr == LPDDR4_INTR_PARITY_ERROR)
		*ctlgrpirqstatus = CPS_FLD_READ(LPDDR4__INT_STATUS_PARITY__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_PARITY__REG)));
	else
		*ctlmasterintflag = (uint32_t)1U;
}

static void lpddr4_checkctlinterrupt_3(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr,
				       uint32_t *ctlgrpirqstatus, uint32_t *ctlmasterintflag)
{
	if ((intr >= LPDDR4_INTR_FREQ_DFS_REQ_HW_IGNORE) && (intr <= LPDDR4_INTR_FREQ_DFS_SW_DONE))
		*ctlgrpirqstatus = CPS_FLD_READ(LPDDR4__INT_STATUS_FREQ__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_FREQ__REG)));
	else if ((intr >= LPDDR4_INTR_LP_DONE) && (intr <= LPDDR4_INTR_LP_TIMEOUT))
		*ctlgrpirqstatus = CPS_FLD_READ(LPDDR4__INT_STATUS_LOWPOWER__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_LOWPOWER__REG)));
	else
		lpddr4_checkctlinterrupt_4(ctlregbase, intr, ctlgrpirqstatus, ctlmasterintflag);
}

static void lpddr4_checkctlinterrupt_2(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr,
				       uint32_t *ctlgrpirqstatus, uint32_t *ctlmasterintflag)
{
	if (intr <= LPDDR4_INTR_TIMEOUT_AUTO_REFRESH_MAX)
		*ctlgrpirqstatus = CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_TIMEOUT__REG));
	else if ((intr >= LPDDR4_INTR_TRAINING_ZQ_STATUS) && (intr <= LPDDR4_INTR_TRAINING_DQS_OSC_VAR_OUT))
		*ctlgrpirqstatus = CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_TRAINING__REG));
	else if ((intr >= LPDDR4_INTR_USERIF_OUTSIDE_MEM_ACCESS) && (intr <= LPDDR4_INTR_USERIF_INVAL_SETTING))
		*ctlgrpirqstatus = CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_USERIF__REG));
	else if ((intr >= LPDDR4_INTR_MISC_MRR_TRAFFIC) && (intr <= LPDDR4_INTR_MISC_REFRESH_STATUS))
		*ctlgrpirqstatus = CPS_FLD_READ(LPDDR4__INT_STATUS_MISC__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_MISC__REG)));
	else if ((intr >= LPDDR4_INTR_DFI_UPDATE_ERROR) && (intr <= LPDDR4_INTR_DFI_TIMEOUT))
		*ctlgrpirqstatus = CPS_FLD_READ(LPDDR4__INT_STATUS_DFI__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_DFI__REG)));
	else
		lpddr4_checkctlinterrupt_3(ctlregbase, intr, ctlgrpirqstatus, ctlmasterintflag);
}

uint32_t lpddr4_checkctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr, bool *irqstatus)
{
	uint32_t result;
	uint32_t ctlmasterirqstatus = 0U;
	uint32_t ctlgrpirqstatus = 0U;
	uint32_t ctlmasterintflag = 0U;

	result = LPDDR4_INTR_CheckCtlIntSF(pd, intr, irqstatus);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		ctlmasterirqstatus = (CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_MASTER__REG)) & (~((uint32_t)1 << 31)));

		lpddr4_checkctlinterrupt_2(ctlregbase, intr, &ctlgrpirqstatus, &ctlmasterintflag);

		if ((ctlintmap[intr][INT_SHIFT] < WORD_SHIFT) && (ctlintmap[intr][GRP_SHIFT] < WORD_SHIFT)) {
			if ((((ctlmasterirqstatus >> ctlintmap[intr][GRP_SHIFT]) & LPDDR4_BIT_MASK) > 0U) &&
			    (((ctlgrpirqstatus >> ctlintmap[intr][INT_SHIFT]) & LPDDR4_BIT_MASK) > 0U) &&
			    (ctlmasterintflag == (uint32_t)0))
				*irqstatus = true;
			else if ((((ctlmasterirqstatus >> ctlintmap[intr][GRP_SHIFT]) & LPDDR4_BIT_MASK) > 0U) &&
				 (ctlmasterintflag == (uint32_t)1U))
				*irqstatus = true;
			else
				*irqstatus = false;
		}
	}
	return result;
}

static void lpddr4_ackctlinterrupt_4(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr)
{
	uint32_t regval = 0;

	if ((intr >= LPDDR4_INTR_MRR_ERROR) && (intr <= LPDDR4_INTR_MR_WRITE_DONE) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_MODE__REG), (uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]);
	} else if ((intr == LPDDR4_INTR_BIST_DONE) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		regval = CPS_FLD_WRITE(LPDDR4__INT_ACK_BIST__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_ACK_BIST__REG)),
				       (uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_BIST__REG), regval);
	} else if ((intr == LPDDR4_INTR_PARITY_ERROR) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		regval = CPS_FLD_WRITE(LPDDR4__INT_ACK_PARITY__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_ACK_PARITY__REG)),
				       (uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_PARITY__REG), regval);
	} else {
	}
}

static void lpddr4_ackctlinterrupt_3(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr)
{
	uint32_t regval = 0;

	if ((intr >= LPDDR4_INTR_LP_DONE) && (intr <= LPDDR4_INTR_LP_TIMEOUT) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		regval = CPS_FLD_WRITE(LPDDR4__INT_ACK_LOWPOWER__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_ACK_LOWPOWER__REG)),
				       (uint32_t)((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_LOWPOWER__REG), regval);
	} else if ((intr >= LPDDR4_INTR_INIT_MEM_RESET_DONE) && (intr <= LPDDR4_INTR_INIT_POWER_ON_STATE) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		regval = CPS_FLD_WRITE(LPDDR4__INT_ACK_INIT__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_ACK_INIT__REG)),
				       (uint32_t)((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_INIT__REG), regval);
	} else {
		lpddr4_ackctlinterrupt_4(ctlregbase, intr);
	}
}

static void  lpddr4_ackctlinterrupt_2(lpddr4_ctlregs *ctlregbase, lpddr4_intr_ctlinterrupt intr)
{
	uint32_t regval = 0;

	if ((intr >= LPDDR4_INTR_DFI_UPDATE_ERROR) && (intr <= LPDDR4_INTR_DFI_TIMEOUT) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_DFI__REG), (uint32_t)((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
	} else if ((intr >= LPDDR4_INTR_FREQ_DFS_REQ_HW_IGNORE) && (intr <= LPDDR4_INTR_FREQ_DFS_SW_DONE) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		regval = CPS_FLD_WRITE(LPDDR4__INT_ACK_FREQ__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_ACK_FREQ__REG)),
				       (uint32_t)((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_FREQ__REG), regval);
	} else {
		lpddr4_ackctlinterrupt_3(ctlregbase, intr);
	}
}

uint32_t lpddr4_ackctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr)
{
	uint32_t result;

	result = LPDDR4_INTR_AckCtlIntSF(pd, intr);
	if ((result == (uint32_t)0) && ((uint32_t)ctlintmap[intr][INT_SHIFT] < WORD_SHIFT)) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		if (intr <= LPDDR4_INTR_TIMEOUT_AUTO_REFRESH_MAX)
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_TIMEOUT__REG), ((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
		else if ((intr >= LPDDR4_INTR_TRAINING_ZQ_STATUS) && (intr <= LPDDR4_INTR_TRAINING_DQS_OSC_VAR_OUT))
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_TRAINING__REG), ((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
		else if ((intr >= LPDDR4_INTR_USERIF_OUTSIDE_MEM_ACCESS) && (intr <= LPDDR4_INTR_USERIF_INVAL_SETTING))
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_USERIF__REG), ((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
		else if ((intr >= LPDDR4_INTR_MISC_MRR_TRAFFIC) && (intr <= LPDDR4_INTR_MISC_REFRESH_STATUS))
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_MISC__REG), ((uint32_t)LPDDR4_BIT_MASK << (uint32_t)ctlintmap[intr][INT_SHIFT]));
		else
			lpddr4_ackctlinterrupt_2(ctlregbase, intr);
	}

	return result;
}

void lpddr4_checkwrlvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr)
{
	uint32_t regval;
	uint32_t errbitmask = 0U;
	uint32_t snum;
	volatile uint32_t *regaddress;

	regaddress = (volatile uint32_t *)(&(ctlregbase->LPDDR4__PHY_WRLVL_STATUS_OBS_0__REG));
	errbitmask = ((uint32_t)LPDDR4_BIT_MASK << (uint32_t)12U);
	for (snum = 0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->wrlvlerror = CDN_TRUE;
			*errfoundptr = true;
		}
		regaddress = lpddr4_addoffset(regaddress, (uint32_t)SLICE_WIDTH);
	}
}

uint32_t lpddr4_getdebuginitinfo(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo)
{
	uint32_t result = 0U;
	bool errorfound = false;

	result = lpddr4_getdebuginitinfosf(pd, debuginfo);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		lpddr4_seterrors(ctlregbase, debuginfo, (uint8_t *)&errorfound);
		lpddr4_setsettings(ctlregbase, errorfound);
		errorfound = (bool)lpddr4_checklvlerrors(pd, debuginfo, errorfound);
	}

	if (errorfound == (bool)true)
		result = (uint32_t)EPROTO;

	return result;
}

uint32_t lpddr4_getreducmode(const lpddr4_privatedata *pd, lpddr4_reducmode *mode)
{
	uint32_t result = 0U;

	result = lpddr4_getreducmodesf(pd, mode);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		if (CPS_FLD_READ(LPDDR4__MEM_DP_REDUCTION__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MEM_DP_REDUCTION__REG))) == 0U)
			*mode = LPDDR4_REDUC_ON;
		else
			*mode = LPDDR4_REDUC_OFF;
	}
	return result;
}

uint32_t lpddr4_setreducmode(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	result = lpddr4_setreducmodesf(pd, mode);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		regval = (uint32_t)CPS_FLD_WRITE(LPDDR4__MEM_DP_REDUCTION__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MEM_DP_REDUCTION__REG)), *mode);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__MEM_DP_REDUCTION__REG), regval);
	}
	return result;
}

uint32_t lpddr4_checkmmrreaderror(const lpddr4_privatedata *pd, uint64_t *mmrvalue, uint8_t *mrrstatus)
{
	uint32_t lowerdata;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t result = (uint32_t)0;

	if (lpddr4_pollctlirq(pd, LPDDR4_INTR_MRR_ERROR, 100) == 0U) {
		*mrrstatus = (uint8_t)CPS_FLD_READ(LPDDR4__MRR_ERROR_STATUS__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MRR_ERROR_STATUS__REG)));
		*mmrvalue = (uint64_t)0;
		result = (uint32_t)EIO;
	} else {
		*mrrstatus = (uint8_t)0;
		lowerdata = CPS_REG_READ(&(ctlregbase->LPDDR4__PERIPHERAL_MRR_DATA__REG));
		*mmrvalue = (uint64_t)((*mmrvalue << WORD_SHIFT) | lowerdata);
		result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_MR_READ_DONE);
	}
	return result;
}

uint32_t lpddr4_getdslicemask(uint32_t dslicenum, uint32_t arrayoffset)
{
	uint32_t rwmask = 0U;

	switch (dslicenum) {
	case 0:
		if (arrayoffset < DSLICE0_REG_COUNT)
			rwmask = g_lpddr4_data_slice_0_rw_mask[arrayoffset];
		break;
	default:
		if (arrayoffset < DSLICE1_REG_COUNT)
			rwmask = g_lpddr4_data_slice_1_rw_mask[arrayoffset];
		break;
	}
	return rwmask;
}

uint32_t lpddr4_geteccenable(const lpddr4_privatedata *pd, lpddr4_eccenable *eccparam)
{
	uint32_t result = 0U;

	result = lpddr4_geteccenablesf(pd, eccparam);
	if (result == (uint32_t)0) {
		*eccparam = LPDDR4_ECC_DISABLED;
		result = (uint32_t)EOPNOTSUPP;
	}

	return result;
}
uint32_t lpddr4_seteccenable(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam)
{
	uint32_t result = 0U;

	result = lpddr4_seteccenablesf(pd, eccparam);
	if (result == (uint32_t)0)
		result = (uint32_t)EOPNOTSUPP;

	return result;
}

#if 0
void lpddr4_enablelowpower(void)
{
	populatelowpowerconfigarray();
}

static uint32_t lpddr4_store_dsregsfreq2(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t DSNum = 0U;
	uint32_t regval = 0;
	uint32_t StartVal = 0;
	uint32_t EndVal = 0;

	for (DSNum = 0U; DSNum < (uint32_t)DSLICE_NUM; DSNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x2);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		StartVal = (uint32_t)LPDDR4_PHY_DS_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum);
		EndVal = (uint32_t)LPDDR4_PHY_DS_LP_REGS_SET + ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum);
		for (aindex = StartVal; aindex < EndVal; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpdsregnumretention[aindex],
							  (uint32_t *)(&phylpdsregvalretention[aindex]));
	}
	return result;
}

static uint32_t lpddr4_store_dsregs(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t DSNum = 0U;
	uint32_t regval = 0;
	uint32_t StartVal = 0;
	uint32_t EndVal = 0;

	for (DSNum = 0U; DSNum < (uint32_t)DSLICE_NUM; DSNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x1);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		StartVal = 0U + ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum);
		EndVal = (uint32_t)LPDDR4_PHY_DS_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum);
		for (aindex = StartVal; aindex < EndVal; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpdsregnumretention[aindex],
							  (uint32_t *)(&phylpdsregvalretention[aindex]));
	}
	if (result == (uint32_t)0)
		result = lpddr4_store_dsregsfreq2(pd);
	return result;
}

static uint32_t lpddr4_store_adrtopregsfreq2(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t regval = 0;

	regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x2);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
	for (aindex = (uint32_t)LPDDR4_PHY_TOP_FREQ_INDEX_2_REG_OFFSET; aindex < (uint32_t)LPDDR4_PHY_TOP_LP_REGS_SET; aindex++)
		result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)phylptopregnumretention[aindex],
						  (uint32_t *)(&phylptopregvalretention[aindex]));

	return result;
}

static uint32_t lpddr4_store_adrtopregsfreq1(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t regval = 0;

	regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x1);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
	for (aindex = 0U; aindex < (uint32_t)LPDDR4_PHY_TOP_FREQ_INDEX_2_REG_OFFSET; aindex++)
		result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)phylptopregnumretention[aindex],
						  (uint32_t *)(&phylptopregvalretention[aindex]));

	if (result == (uint32_t)0)
		result = lpddr4_store_adrtopregsfreq2(pd);
	return result;
}

static uint32_t lpddr4_store_adrregsfreq2(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t AdrNum = 0U;
	uint32_t regval = 0;
	uint32_t StartVal = 0;
	uint32_t EndVal = 0;

	for (AdrNum = 0; AdrNum < (uint32_t)ASLICE_NUM; AdrNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x2);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		StartVal = (uint32_t)LPDDR4_PHY_ADR_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum);
		EndVal = (uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum);
		for (aindex = StartVal; aindex < EndVal; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpadrregnumretention[aindex],
							  (uint32_t *)(&phylpadrregvalretention[aindex]));
	}

	return result;
}

static uint32_t lpddr4_store_adrregs(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t AdrNum = 0U;
	uint32_t regval = 0;
	uint32_t StartVal = 0;
	uint32_t EndVal = 0;

	for (AdrNum = 0; AdrNum < (uint32_t)ASLICE_NUM; AdrNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x1);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		StartVal = 0U + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum);
		EndVal = (uint32_t)LPDDR4_PHY_ADR_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum);
		for (aindex = StartVal; aindex < EndVal; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpadrregnumretention[aindex],
							  (uint32_t *)(&phylpadrregvalretention[aindex]));
	}
	if (result == (uint32_t)0)
		result = lpddr4_store_adrregsfreq2(pd);
	if (result == (uint32_t)0)
		result = lpddr4_store_adrtopregsfreq1(pd);
	return result;
}

static uint32_t lpddr4_store_lpconfig(const lpddr4_privatedata *pd)
{
	uint32_t aindex = 0U;
	uint32_t result = 0U;

	for (aindex = 0U; aindex < (uint32_t)LPDDR4_CTL_LP_REGS_SET; aindex++)
		result = (uint32_t)lpddr4_readreg(pd, LPDDR4_CTL_REGS, (uint32_t)ctllpregnumretention[aindex],
						  (uint32_t *)(&ctllpregvalretention[aindex]));

	if (result == (uint32_t)0)
		result = lpddr4_store_dsregs(pd);
	if (result == (uint32_t)0)
		result = lpddr4_store_adrregs(pd);
	if (result == (uint32_t)0) {
		for (aindex = (uint32_t)0U; aindex < (uint32_t)LPDDR4_PI_LP_REGS_SET; aindex++)
			result = (uint32_t)lpddr4_readreg(pd, LPDDR4_PHY_INDEP_REGS, (uint32_t)pilpregnumretention[aindex],
							  (uint32_t *)(&pilpregvalretention[aindex]));
	}
	return result;
}

static uint32_t lpddr4_restore_dsregsfreq2(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t DSNum = 0U;
	uint32_t regval = 0U;

	for (DSNum = 0U; DSNum < (uint32_t)DSLICE_NUM; DSNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x2);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		for (aindex = ((uint32_t)LPDDR4_PHY_DS_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum)); aindex < ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET + ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum)); aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpdsregnumretention[aindex],
							   (uint32_t)phylpdsregvalretention[aindex]);
	}
	return result;
}

static uint32_t lpddr4_restore_dsregs(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t DSNum = 0U;
	uint32_t regval = 0U;

	for (DSNum = 0U; DSNum < (uint32_t)DSLICE_NUM; DSNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x1);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		for (aindex = ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum); aindex < ((uint32_t)LPDDR4_PHY_DS_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_DS_LP_REGS_SET * DSNum)); aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpdsregnumretention[aindex],
							   (uint32_t)phylpdsregvalretention[aindex]);
	}

	if (result == (uint32_t)0)
		result = lpddr4_restore_dsregsfreq2(pd);
	return result;
}

static uint32_t lpddr4_restore_adrtopregsfreq2(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t regval = 0;

	regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x2);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
	for (aindex = (uint32_t)LPDDR4_PHY_TOP_FREQ_INDEX_2_REG_OFFSET; aindex < (uint32_t)LPDDR4_PHY_TOP_LP_REGS_SET; aindex++)
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)phylptopregnumretention[aindex],
						   (uint32_t)phylptopregvalretention[aindex]);

	return result;
}

static uint32_t lpddr4_restore_adrtopregsfreq1(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t regval = 0;

	regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x1);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
	for (aindex = 0U; aindex < (uint32_t)LPDDR4_PHY_TOP_FREQ_INDEX_2_REG_OFFSET; aindex++)
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)phylptopregnumretention[aindex],
						   (uint32_t)phylptopregvalretention[aindex]);

	result = lpddr4_restore_adrtopregsfreq2(pd);
	return result;
}

static uint32_t lpddr4_restore_adrregsfreq2(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t AdrNum = 0U;
	uint32_t regval = 0;

	for (AdrNum = 0U; AdrNum < (uint32_t)ASLICE_NUM; AdrNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x2);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		for (aindex = ((uint32_t)LPDDR4_PHY_ADR_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum)); aindex < ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum)); aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpadrregnumretention[aindex],
							   (uint32_t)(phylpadrregvalretention[aindex]));
	}
	return result;
}

static uint32_t lpddr4_restore_adrregsfreq1(const lpddr4_privatedata *pd)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t aindex = 0U;
	uint32_t result = 0U;
	uint32_t AdrNum = 0U;
	uint32_t regval = 0;

	for (AdrNum = 0U; AdrNum < (uint32_t)ASLICE_NUM; AdrNum++) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_INDEX__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG)), 0x1);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_INDEX__REG), regval);
		for (aindex = (0U + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum)); aindex < ((uint32_t)LPDDR4_PHY_ADR_FREQ_INDEX_2_REG_OFFSET + ((uint32_t)LPDDR4_PHY_ADR_LP_REGS_SET * AdrNum)); aindex++)
			result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_REGS, (uint32_t)phylpadrregnumretention[aindex],
							   (uint32_t)(phylpadrregvalretention[aindex]));
	}

	if (result == (uint32_t)0)
		result = lpddr4_restore_adrregsfreq2(pd);
	return result;
}

static uint32_t lpddr4_restore_adrregs(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;

	result = lpddr4_restore_adrregsfreq1(pd);
	if (result == (uint32_t)0) {
		result = lpddr4_restore_adrtopregsfreq1(pd);
	} else {
	}
	return result;
}

static void lpddr4_multicastcontrol(const lpddr4_privatedata *pd, uint32_t enable)
{
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	uint32_t regval = 0;
	static uint32_t multicast = 0;

	if (enable == 0U) {
		multicast = (uint8_t)CPS_FLD_READ(LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__REG)));
		if (multicast == 1U) {
			regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__REG)), enable);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__REG), regval);
		}
	} else if ((multicast == 1U) && (enable == 1U)) {
		regval = CPS_FLD_WRITE(LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__REG)), enable);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PHY_FREQ_SEL_MULTICAST_EN__REG), regval);
	} else {
	}
}
static uint32_t lpddr4_restore_lpconfig(const lpddr4_privatedata *pd)
{
	uint32_t aindex = 0U;
	uint32_t result = 0U;

	lpddr4_multicastcontrol(pd, 0U);
	for (aindex = 0U; aindex < (uint32_t)LPDDR4_CTL_LP_REGS_SET; aindex++)
		result = (uint32_t)lpddr4_writereg(pd, LPDDR4_CTL_REGS, (uint32_t)ctllpregnumretention[aindex],
						   (uint32_t)ctllpregvalretention[aindex]);

	if (result == (uint32_t)0) {
		result = lpddr4_restore_dsregs(pd);
		if (result == (uint32_t)0) {
			result = lpddr4_restore_adrregs(pd);
			if (result == (uint32_t)0) {
				for (aindex = 0U; aindex < (uint32_t)LPDDR4_PI_LP_REGS_SET; aindex++)
					result = (uint32_t)lpddr4_writereg(pd, LPDDR4_PHY_INDEP_REGS, (uint32_t)pilpregnumretention[aindex],
									   (uint32_t)pilpregvalretention[aindex]);
			}
		}
	}
	lpddr4_multicastcontrol(pd, 1U);
	return result;
}

static uint32_t lpddr4_deepsleepenter(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;
	lpddr4_infotype infotype;

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	result = lpddr4_store_lpconfig(pd);
	if (result == (uint32_t)0) {
		regval = CPS_FLD_WRITE(LPDDR4__LP_CMD__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_CMD__REG)), (uint32_t)0x31);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__LP_CMD__REG), regval);

		result = lpddr4_pollctlirq(pd, LPDDR4_INTR_LP_DONE, LPDDR4_CUSTOM_TIMEOUT_DELAY);
		if (result == (uint32_t)0)
			result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_LP_DONE);

		regval = (uint32_t)(CPS_FLD_READ(LPDDR4__LP_STATE__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_STATE__REG))));
		if ((regval & ((LPDDR4_BIT_MASK) << 6)) == 64U) {
			if ((regval & (uint8_t)(0x3f)) == (uint8_t)0x0E) {
				result = (uint32_t)0;;
			} else {
				result = (uint32_t)-EINVAL;
			}

			if (pd->infohandler != (lpddr4_infocallback)NULL) {
				infotype = LPDDR4_DRV_SOC_PHY_DATA_RET_ASSERT;
				pd->infohandler(pd, infotype);
			}
		} else {
			result = (uint32_t)-EINVAL;
		}
	}
	return result;
}

static void lpddr4_lpwakeupsubroutine(const lpddr4_privatedata *pd)
{
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_SET(LPDDR4__PI_INIT_LVL_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)), regval);

	regval = CPS_FLD_SET(LPDDR4__PI_START__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_START__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_START__REG)), regval);

	regval = CPS_FLD_SET(LPDDR4__START__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__START__REG)));
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__START__REG), regval);

	regval = CPS_FLD_WRITE(LPDDR4__LP_CMD__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_CMD__REG)), (uint32_t)0x2);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__LP_CMD__REG), regval);
}

static uint32_t lpddr4_deepsleepexit(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;
	lpddr4_infotype infotype;

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	result = lpddr4_restore_lpconfig(pd);
	if (result == (uint32_t)0) {
		regval = CPS_FLD_SET(LPDDR4__PI_PWRUP_SREFRESH_EXIT__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_PWRUP_SREFRESH_EXIT__REG)));
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PI_PWRUP_SREFRESH_EXIT__REG), regval);

		regval = CPS_FLD_SET(LPDDR4__PI_MC_PWRUP_SREFRESH_EXIT__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_MC_PWRUP_SREFRESH_EXIT__REG)));
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__PI_MC_PWRUP_SREFRESH_EXIT__REG), regval);

		if (pd->infohandler != (lpddr4_infocallback)NULL) {
			infotype = LPDDR4_DRV_SOC_PHY_DATA_RET_DEASSERT;
			pd->infohandler(pd, infotype);
		}

		lpddr4_lpwakeupsubroutine(pd);

		result = lpddr4_pollctlirq(pd, LPDDR4_INTR_LP_DONE, LPDDR4_CUSTOM_TIMEOUT_DELAY);
		if (result == (uint32_t)0)
			result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_LP_DONE);
	}
	return result;
}

static void lpddr4_automodecontrol(lpddr4_ctlregs *ctlregbase, uint32_t Enable)
{
	uint32_t regval = 0U;

	regval = CPS_FLD_WRITE(LPDDR4__LP_AUTO_ENTRY_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_AUTO_ENTRY_EN__REG)), Enable);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__LP_AUTO_ENTRY_EN__REG), regval);
	regval = CPS_FLD_WRITE(LPDDR4__LP_AUTO_EXIT_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_AUTO_EXIT_EN__REG)), Enable);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__LP_AUTO_EXIT_EN__REG), regval);
}

static uint32_t lpddr4_lpopsdone(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;

	result = lpddr4_pollctlirq(pd, LPDDR4_INTR_LP_DONE, LPDDR4_CUSTOM_TIMEOUT_DELAY);
	if (result == (uint32_t)0)
		result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_LP_DONE);
	return result;
}

static uint32_t lpddr4_lpidlemode(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = (uint32_t)(CPS_FLD_READ(LPDDR4__DRAM_CLASS__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__DRAM_CLASS__REG))));
	if ((regval == (uint32_t)0x0b))
		regval = CPS_FLD_WRITE(LPDDR4__LP_CMD__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_CMD__REG)), (uint32_t)0x01);  /* LPDDR4 */
	else
		regval = CPS_FLD_WRITE(LPDDR4__LP_CMD__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_CMD__REG)), (uint32_t)0x09);  /* Non LPDDR4 */
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__LP_CMD__REG), regval);

	result = lpddr4_lpopsdone(pd);

	regval = (uint32_t)(CPS_FLD_READ(LPDDR4__LP_STATE__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_STATE__REG))));
	if ((regval & ((LPDDR4_BIT_MASK) << 6)) == 64U) {
		if ((regval & (uint32_t)(0x3f)) == (uint32_t)0x5)
			result = (uint32_t)0;
	} else {
		result = (uint32_t)-EINVAL;
	}

	return result;
}

static uint32_t lpddr4_lpmodeexit(const lpddr4_privatedata *pd)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_WRITE(LPDDR4__LP_CMD__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_CMD__REG)), (uint32_t)0x2);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__LP_CMD__REG), regval);

	result = lpddr4_lpopsdone(pd);
	return result;
}
uint32_t lpddr4_setmclowpowermode(const lpddr4_privatedata *pd, lpddr4_ctllpstate LpState)
{
	uint32_t result = 0U;
	lpddr4_infotype infotype;

	result = lpddr4_setmclowpowermodesf(pd, LpState);

	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		lpddr4_automodecontrol(ctlregbase, 0U);

		switch (LpState) {
		case LPDDR4_LP_ACTIVE_REDUCED_FREQ:
			if (pd->infohandler != (lpddr4_infocallback)NULL) {
				infotype = LPDDR4_DRV_SOC_PHY_REDUCED_FREQ;
				pd->infohandler(pd, infotype);
			}

			break;
		case LPDDR4_LP_IDLE:
			result = lpddr4_lpidlemode(pd);
			break;

		case LPDDR4_LP_DEEP_SLEEP:
			result = lpddr4_deepsleepenter(pd);
			break;

		case LPDDR4_LP_DEEP_SLEEP_EXIT:
			result = lpddr4_deepsleepexit(pd);
			break;

		case LPDDR4_LP_EXIT:
			result = lpddr4_lpmodeexit(pd);
			break;

		default:
			result = (uint32_t)-EINVAL;
			break;
		}
		lpddr4_automodecontrol(ctlregbase, 0xF);
	}
	return result;
}

uint32_t lpddr4_getmclowpowermode(const lpddr4_privatedata *pd, lpddr4_ctllpstate *getlpstate)
{
	uint32_t result = 0U;
	uint32_t regval = 0U;

	result = lpddr4_getmclowpowermodesf(pd, getlpstate);
	if (result == (uint32_t)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		regval = (uint32_t)(CPS_FLD_READ(LPDDR4__LP_STATE__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__LP_STATE__REG))));
		if ((regval & ((LPDDR4_BIT_MASK) << 6)) == 64U) {
			if ((regval & (uint8_t)(0x3f)) == (uint8_t)0x00)
				*getlpstate = LPDDR4_LP_ACTIVE;
			else if ((regval & (uint8_t)(0x3f)) == (uint8_t)0x05)
				*getlpstate = LPDDR4_LP_IDLE;
			else if ((regval & (uint8_t)(0x3f)) == (uint8_t)0x0E)
				*getlpstate = LPDDR4_LP_DEEP_SLEEP;
			else
				result = (uint32_t)EAGAIN;
		} else {
			if ((CPS_FLD_READ(LPDDR4__FSP_OP_CURRENT__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__FSP_OP_CURRENT__REG))) == 1U))
				*getlpstate = LPDDR4_LP_ACTIVE_REDUCED_FREQ;
			else
				result = (uint32_t)EAGAIN;
		}
	}

	return result;
}
#endif
