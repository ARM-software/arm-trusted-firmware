/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#define MU_ATR0_OFFSET1		0x0
#define MU_ARR0_OFFSET1		0x10
#define MU_ASR_OFFSET1		0x20
#define MU_ACR_OFFSET1		0x24
#define MU_TR_COUNT1		4
#define MU_RR_COUNT1		4

#define MU_CR_GIEn_MASK1	(0xF << 28)
#define MU_CR_RIEn_MASK1	(0xF << 24)
#define MU_CR_TIEn_MASK1	(0xF << 20)
#define MU_CR_GIRn_MASK1	(0xF << 16)
#define MU_CR_NMI_MASK1		(1 << 3)
#define MU_CR_Fn_MASK1		0x7

#define MU_SR_TE0_MASK1		(1 << 23)
#define MU_SR_RF0_MASK1		(1 << 27)
#define MU_CR_RIE0_MASK1	(1 << 27)
#define MU_CR_GIE0_MASK1	(1 << 31)

#define MU_TR_COUNT			4
#define MU_RR_COUNT			4

void MU_Init(uint32_t base);
void MU_SendMessage(uint32_t base, uint32_t regIndex, uint32_t msg);
void MU_ReceiveMsg(uint32_t base, uint32_t regIndex, uint32_t *msg);
void MU_EnableGeneralInt(uint32_t base, uint32_t index);
void MU_EnableRxFullInt(uint32_t base, uint32_t index);
void MU_Resume(uint32_t base);
