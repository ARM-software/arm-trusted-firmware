// SPDX-License-Identifier: BSD-3-Clause
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2026 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <errno.h>

#include "cps_drv_lpddr4.h"
#include "lpddr4.h"
#include "lpddr4_ctl_regs.h"
#include "lpddr4_if.h"
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

uint32_t lpddr4_enablepiinitiator(const lpddr4_privatedata *pd)
{
	uint32_t regval = 0U;

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_SET(LPDDR4__PI_NORMAL_LVL_SEQ__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_NORMAL_LVL_SEQ__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_NORMAL_LVL_SEQ__REG)), regval);
	regval = CPS_FLD_SET(LPDDR4__PI_INIT_LVL_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)));
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)), regval);

	return 0U;
}

uint32_t lpddr4_getctlinterruptmask(const lpddr4_privatedata *pd, uint64_t *mask)
{
	if ((pd == NULL) || (mask == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	*mask = (uint64_t)(CPS_FLD_READ(LPDDR4__INT_MASK_MASTER__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_MASTER__REG))));

	return 0U;
}

uint32_t lpddr4_setctlinterruptmask(const lpddr4_privatedata *pd, const uint64_t *mask)
{
	uint32_t regval = 0U;
	const uint64_t ui64one = 1ULL;
	const uint32_t ui32irqcount = (uint32_t)32U;

	if (pd == NULL) {
		return EINVAL;
	}

	if (*mask >= (ui64one << ui32irqcount)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	regval = CPS_FLD_WRITE(LPDDR4__INT_MASK_MASTER__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_MASTER__REG)), *mask);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_MASK_MASTER__REG), regval);

	return 0U;
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

	result = lpddr4_intr_ctlint_sf(pd, intr, irqstatus);
	if (result != (uint32_t)0) {
		return result;
	}

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

	result = lpddr4_intr_ack_ctlint_sf(pd, intr);
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
	bool errorfound = false;

	if ((pd == NULL) || (debuginfo == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	lpddr4_seterrors(ctlregbase, debuginfo, (uint8_t *)&errorfound);
	lpddr4_setsettings(ctlregbase, errorfound);
	errorfound = (bool)lpddr4_checklvlerrors(pd, debuginfo, errorfound);

	if (errorfound == (bool)true)
		return EPROTO;

	return 0U;
}

uint32_t lpddr4_getreducmode(const lpddr4_privatedata *pd, lpddr4_reducmode *mode)
{
	if ((pd == NULL) || (mode == NULL)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	if (CPS_FLD_READ(LPDDR4__MEM_DP_REDUCTION__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MEM_DP_REDUCTION__REG))) == 0U)
		*mode = LPDDR4_REDUC_ON;
	else
		*mode = LPDDR4_REDUC_OFF;

	return 0U;
}

uint32_t lpddr4_setreducmode(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode)
{
	uint32_t regval = 0U;

	if ((pd == NULL) || (mode == NULL)) {
		return EINVAL;
	} else if ((*mode != LPDDR4_REDUC_ON) &&
			(*mode != LPDDR4_REDUC_OFF)) {
		return EINVAL;
	}

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	regval = (uint32_t)CPS_FLD_WRITE(LPDDR4__MEM_DP_REDUCTION__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MEM_DP_REDUCTION__REG)), *mode);
	CPS_REG_WRITE(&(ctlregbase->LPDDR4__MEM_DP_REDUCTION__REG), regval);

	return 0U;
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
	if ((pd == NULL) || (eccparam == NULL)) {
		return EINVAL;
	}

	*eccparam = LPDDR4_ECC_DISABLED;

	return EOPNOTSUPP;
}
uint32_t lpddr4_seteccenable(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam)
{
	if ((pd == NULL) || (eccparam == NULL)) {
		return EINVAL;
	}

	return EOPNOTSUPP;
}
