/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STM32_CRYP2_SAES_REG_H
#define STM32_CRYP2_SAES_REG_H

/* SAES control register */
#define _SAES_CR			0x0U
/* SAES status register */
#define _SAES_SR			0x04U
/* SAES data input register */
#define _SAES_DINR			0x08U
/* SAES data output register */
#define _SAES_DOUTR			0x0CU
/* SAES key registers [0-7] */
#define _SAES_KEYR0			0x20U
#define _SAES_KEYR1			0x24U
#define _SAES_KEYR2			0x28U
#define _SAES_KEYR3			0x2CU
#define _SAES_KEYR4			0x30U
#define _SAES_KEYR5			0x34U
#define _SAES_KEYR6			0x38U
#define _SAES_KEYR7			0x3CU
/* SAES initialization vector registers [0-3] */
#define _SAES_IVR0			0x40U
#define _SAES_IVR1			0x44U
#define _SAES_IVR2			0x48U
#define _SAES_IVR3			0x4CU
/* SAES context swap registers [0-7] */
#define _SAES_CSR0			0x50U
#define _SAES_CSR1			0x54U
#define _SAES_CSR2			0x58U
#define _SAES_CSR3			0x5CU
#define _SAES_CSR4			0x50U
#define _SAES_CSR5			0x54U
#define _SAES_CSR6			0x58U
#define _SAES_CSR7			0x5CU
/* SAES Interrupt Enable Register */
#define _SAES_IER			0x300U
/* SAES Interrupt Status Register */
#define _SAES_ISR			0x304U
/* SAES Interrupt Clear Register */
#define _SAES_ICR			0x308U

/* SAES control register fields */
#define _SAES_CR_RESET_VALUE		0x0U
#define _SAES_CR_IPRST			BIT(31)
#define _SAES_CR_KEYSEL_MASK		GENMASK_32(30, 28)
#define _SAES_CR_KEYSEL_SHIFT		28U
#define _SAES_CR_KEYSEL_SOFT		0x0U
#define _SAES_CR_KEYSEL_DHUK		0x1U
#define _SAES_CR_KEYSEL_BHK		0x2U
#define _SAES_CR_KEYSEL_BHU_XOR_BH_K	0x4U
#define _SAES_CR_WRAPID_MASK		GENMASK_32(27, 25)
#define _SAES_CR_WRAPID_SHIFT		25U
#define _SAES_CR_WRAPID_SAES		0x0U
#define _SAES_CR_WRAPID_CRYP1		0x1U
#define _SAES_CR_WRAPID_CRYP2		0x2U
#define _SAES_CR_WRAPEN			BIT(24)
#define _SAES_CR_NPBLB_MASK		GENMASK_32(23, 20)
#define _SAES_CR_NPBLB_SHIFT		20U
#define _SAES_CR_KEYPROT		BIT(19)
#define _SAES_CR_KEYSIZE_MASK		GENMASK_32(18, 17)
#define _SAES_CR_KEYSIZE_SHIFT		17U
#define _SAES_CR_KEYSIZE_128		0x0U
#define _SAES_CR_KEYSIZE_192		0x1U
#define _SAES_CR_KEYSIZE_256		0x2U
#define _SAES_CR_CPHASE_MASK		GENMASK_32(14, 13)
#define _SAES_CR_CPHASE_SHIFT		13U
#define _SAES_CR_CPHASE_INIT		0U
#define _SAES_CR_CPHASE_HEADER		1U
#define _SAES_CR_CPHASE_PAYLOAD		2U
#define _SAES_CR_CPHASE_FINAL		3U
#define _SAES_CR_DMAOUTEN		BIT(12)
#define _SAES_CR_DMAINEN		BIT(11)
#define _SAES_CR_CHMOD_MASK		GENMASK_32(8, 5)
#define _SAES_CR_CHMOD_SHIFT		5U
#define _SAES_CR_CHMOD_ECB		0x0U
#define _SAES_CR_CHMOD_CBC		0x1U
#define _SAES_CR_CHMOD_CTR		0x2U
#define _SAES_CR_CHMOD_GCM		0x3U
#define _SAES_CR_CHMOD_GMAC		0x3U
#define _SAES_CR_CHMOD_CCM		0x4U
#define _SAES_CR_OPMODE_MASK		GENMASK_32(4, 3)
#define _SAES_CR_OPMODE_SHIFT		3U
#define _SAES_CR_OPMODE_ENC		0U
#define _SAES_CR_OPMODE_KEYPREP		1U
#define _SAES_CR_OPMODE_DEC		2U
#define _SAES_CR_DATATYPE_MASK		GENMASK_32(2, 1)
#define _SAES_CR_DATATYPE_SHIFT		1U
#define _SAES_CR_DATATYPE_NONE		0U
#define _SAES_CR_DATATYPE_HALF_WORD	1U
#define _SAES_CR_DATATYPE_BYTE		2U
#define _SAES_CR_DATATYPE_BIT		3U
#define _SAES_CR_EN			BIT(0)

/* SAES status register fields */
#define _SAES_SR_KEYVALID		BIT(7)
#define _SAES_SR_WRERR			BIT(6)
#define _SAES_SR_RDERR			BIT(5)
#define _SAES_SR_BUSY			BIT(4)

/* SAES interrupt registers fields */
#define _SAES_I_RNG_ERR			BIT(5)
#define _SAES_I_KEY_ERR			BIT(4)
#define _SAES_I_RW_ERR			BIT(3)
#define _SAES_I_CCF			BIT(2)

/* Correspondence between registers fields and the used define in the driver */
#define _SAES_CR_GCMPH_MASK		_SAES_CR_CPHASE_MASK
#define _SAES_CR_GCMPH_SHIFT		_SAES_CR_CPHASE_SHIFT
#define _SAES_CR_GCMPH_INIT		_SAES_CR_CPHASE_INIT
#define _SAES_CR_GCMPH_HEADER		_SAES_CR_CPHASE_HEADER
#define _SAES_CR_GCMPH_PAYLOAD		_SAES_CR_CPHASE_PAYLOAD
#define _SAES_CR_GCMPH_FINAL		_SAES_CR_CPHASE_FINAL

#define _SAES_CR_MODE_MASK		_SAES_CR_OPMODE_MASK
#define _SAES_CR_MODE_SHIFT		_SAES_CR_OPMODE_SHIFT
#define _SAES_CR_MODE_ENC		_SAES_CR_OPMODE_ENC
#define _SAES_CR_MODE_KEYPREP		_SAES_CR_OPMODE_KEYPREP
#define _SAES_CR_MODE_DEC		_SAES_CR_OPMODE_DEC

#endif /* STM32_CRYP2_SAES_REG_H */
