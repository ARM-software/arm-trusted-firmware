/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_H
#define LPDDR4_H

#include "lpddr4_ctl_regs.h"
#include "lpddr4_sanity.h"
#ifdef DDR_16BIT
#include "lpddr4_16bit.h"
#include "lpddr4_16bit_sanity.h"
#else
#include "lpddr4_32bit.h"
#include "lpddr4_32bit_sanity.h"
#endif

#define PRODUCT_ID (0x1046U)

#define LPDDR4_BIT_MASK    (0x1U)
#define BYTE_MASK   (0xffU)
#define NIBBLE_MASK (0xfU)

#define WORD_SHIFT (32U)
#define WORD_MASK (0xffffffffU)
#define SLICE_WIDTH (0x100)

#define CTL_OFFSET 0
#define PI_OFFSET (((uint32_t)1) << 11)
#define PHY_OFFSET (((uint32_t)1) << 12)

#define CTL_INT_MASK_ALL ((uint32_t)LPDDR4_LOR_BITS - WORD_SHIFT)

#define PLL_READY (0x3U)
#define IO_CALIB_DONE ((uint32_t)0x1U << 23U)
#define IO_CALIB_FIELD ((uint32_t)NIBBLE_MASK << 28U)
#define IO_CALIB_STATE ((uint32_t)0xBU << 28U)
#define RX_CAL_DONE ((uint32_t)LPDDR4_BIT_MASK << 4U)
#define CA_TRAIN_RL (((uint32_t)LPDDR4_BIT_MASK << 5U) | ((uint32_t)LPDDR4_BIT_MASK << 4U))
#define WR_LVL_STATE (((uint32_t)NIBBLE_MASK) << 13U)
#define GATE_LVL_ERROR_FIELDS (((uint32_t)LPDDR4_BIT_MASK << 7U) | ((uint32_t)LPDDR4_BIT_MASK << 6U))
#define READ_LVL_ERROR_FIELDS ((((uint32_t)NIBBLE_MASK) << 28U) | (((uint32_t)BYTE_MASK) << 16U))
#define DQ_LVL_STATUS (((uint32_t)LPDDR4_BIT_MASK << 26U) | (((uint32_t)BYTE_MASK) << 18U))

#define CDN_TRUE  1U
#define CDN_FALSE 0U

#ifndef LPDDR4_CUSTOM_TIMEOUT_DELAY
#define LPDDR4_CUSTOM_TIMEOUT_DELAY 100000000U
#endif

#ifndef LPDDR4_CPS_NS_DELAY_TIME
#define LPDDR4_CPS_NS_DELAY_TIME 10000000U
#endif

void lpddr4_setsettings(lpddr4_ctlregs *ctlregbase, const bool errorfound);
volatile uint32_t *lpddr4_addoffset(volatile uint32_t *addr, uint32_t regoffset);
uint32_t lpddr4_pollctlirq(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt irqbit, uint32_t delay);
bool lpddr4_checklvlerrors(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo, bool errfound);
void lpddr4_seterrors(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, uint8_t *errfoundptr);

uint32_t lpddr4_enablepiinitiator(const lpddr4_privatedata *pd);
void lpddr4_checkwrlvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr);
uint32_t lpddr4_checkmmrreaderror(const lpddr4_privatedata *pd, uint64_t *mmrvalue, uint8_t *mrrstatus);
uint32_t lpddr4_getdslicemask(uint32_t dslicenum, uint32_t arrayoffset);
void lpddr4_enablelowpower(void);

#endif  /* LPDDR4_H */
