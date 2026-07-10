/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PDC register definitions for lemans APSS.
 */

#ifndef PDC_REGS_H
#define PDC_REGS_H

#include <lib/mmio.h>

#include <platform_def.h>

/* Base addresses */
#define PDC_BASE		(QTI_AOSS_BASE + 0x200000U)
#define PDC_SEQ_MEM_BASE	(QTI_AOSS_BASE + 0x400000U)

/* Register address helpers */
#define PDC_REG(base, off)		((uintptr_t)(base) + (uint32_t)(off))
#define PDC_REG_DRV(base, off, d)	((uintptr_t)(base) + (uint32_t)(off) + \
					 0x10000U * (uint32_t)(d))

/* --------------------------------------------------------------------------
 * ENABLE_PDC  (base + 0x4500)
 * --------------------------------------------------------------------------
 */
#define PDC_ENABLE_PDC_OFF		0x4500U
#define PDC_ENABLE_PDC_BMSK		0x1U

#define PDC_ENABLE_PDC_RMW(base, val) \
	mmio_write_32(PDC_REG(base, PDC_ENABLE_PDC_OFF), \
		      (mmio_read_32(PDC_REG(base, PDC_ENABLE_PDC_OFF)) & \
		       ~PDC_ENABLE_PDC_BMSK) | \
		      ((uint32_t)(val) & PDC_ENABLE_PDC_BMSK))

/* --------------------------------------------------------------------------
 * PDC_PARAM_RESOURCE  (base + 0x1004 + 0x10000*d)
 * --------------------------------------------------------------------------
 */
#define PDC_PARAM_RESOURCE_OFF			0x1004U
#define PDC_PARAM_RESOURCE_PROFILING_UNIT_BMSK	0xf000U
#define PDC_PARAM_RESOURCE_PROFILING_UNIT_SHFT	12U
#define PDC_PARAM_RESOURCE_TCS_BMSK		0xf00U
#define PDC_PARAM_RESOURCE_TCS_SHFT		8U
#define PDC_PARAM_RESOURCE_TCS_CMDS_BMSK	0xe0U
#define PDC_PARAM_RESOURCE_TCS_CMDS_SHFT	5U

#define PDC_PARAM_RESOURCE_READ(base, d) \
	mmio_read_32(PDC_REG_DRV(base, PDC_PARAM_RESOURCE_OFF, d))

#define PDC_PARAM_PROFILING_UNIT(base) \
	((PDC_PARAM_RESOURCE_READ(base, 0) & \
	  PDC_PARAM_RESOURCE_PROFILING_UNIT_BMSK) >> \
	 PDC_PARAM_RESOURCE_PROFILING_UNIT_SHFT)

#define PDC_PARAM_TCS_COUNT(base) \
	((PDC_PARAM_RESOURCE_READ(base, 0) & PDC_PARAM_RESOURCE_TCS_BMSK) >> \
	 PDC_PARAM_RESOURCE_TCS_SHFT)

#define PDC_PARAM_TCS_CMDS(base) \
	((PDC_PARAM_RESOURCE_READ(base, 0) & PDC_PARAM_RESOURCE_TCS_CMDS_BMSK) >> \
	 PDC_PARAM_RESOURCE_TCS_CMDS_SHFT)

/* --------------------------------------------------------------------------
 * PDC_PARAM_SEQ_CONFIG  (base + 0x1008 + 0x10000*d)
 * --------------------------------------------------------------------------
 */
#define PDC_PARAM_SEQ_CONFIG_OFF	0x1008U
#define PDC_PARAM_SEQ_CMD_WORDS_BMSK	0xff0000U
#define PDC_PARAM_SEQ_CMD_WORDS_SHFT	16U

#define PDC_PARAM_SEQ_CMD_WORDS(base) \
	((mmio_read_32(PDC_REG_DRV(base, PDC_PARAM_SEQ_CONFIG_OFF, 0)) & \
	  PDC_PARAM_SEQ_CMD_WORDS_BMSK) >> PDC_PARAM_SEQ_CMD_WORDS_SHFT)

/* --------------------------------------------------------------------------
 * SEQ_CFG_BR_ADDR  (base + 0x4560 + 0x4*b)
 * --------------------------------------------------------------------------
 */
#define PDC_SEQ_BR_ADDR_OFF(b)		(0x4560U + 0x4U * (uint32_t)(b))

#define PDC_SEQ_BR_ADDR_WRITE(base, b, val) \
	mmio_write_32(PDC_REG(base, PDC_SEQ_BR_ADDR_OFF(b)), (uint32_t)(val))

/* --------------------------------------------------------------------------
 * SEQ_CFG_DELAY_VAL  (base + 0x45A0 + 0x4*v)
 * --------------------------------------------------------------------------
 */
#define PDC_SEQ_DELAY_VAL_OFF(v)	(0x45a0U + 0x4U * (uint32_t)(v))

#define PDC_SEQ_DELAY_WRITE(base, v, val) \
	mmio_write_32(PDC_REG(base, PDC_SEQ_DELAY_VAL_OFF(v)), (uint32_t)(val))

/* --------------------------------------------------------------------------
 * SEQ_MEM  (PDC_SEQ_MEM_BASE + 0x4*m)
 * --------------------------------------------------------------------------
 */
#define PDC_SEQ_MEM_OFF(m)		(0x4U * (uint32_t)(m))

#define PDC_SEQ_MEM_RMW(m, mask, val) \
	mmio_write_32(PDC_REG(PDC_SEQ_MEM_BASE, PDC_SEQ_MEM_OFF(m)), \
		      (mmio_read_32(PDC_REG(PDC_SEQ_MEM_BASE, \
					    PDC_SEQ_MEM_OFF(m))) & \
		       ~(uint32_t)(mask)) | \
		      ((uint32_t)(val) & (uint32_t)(mask)))

/* --------------------------------------------------------------------------
 * TCS_CMD_ENABLE_BANK  (base + 0x5504 + 0xC8*t)
 * --------------------------------------------------------------------------
 */
#define PDC_TCS_CMD_ENABLE_OFF(t)	(0x5504U + 0xc8U * (uint32_t)(t))

#define PDC_TCS_CMD_ENABLE_WRITE(base, t, val) \
	mmio_write_32(PDC_REG(base, PDC_TCS_CMD_ENABLE_OFF(t)), (uint32_t)(val))

/* --------------------------------------------------------------------------
 * TCS_CMD_WAIT_FOR_CMPL_BANK  (base + 0x5508 + 0xC8*t)
 * --------------------------------------------------------------------------
 */
#define PDC_TCS_WAIT_CMPL_OFF(t)	(0x5508U + 0xc8U * (uint32_t)(t))

#define PDC_TCS_WAIT_CMPL_READ(base, t) \
	mmio_read_32(PDC_REG(base, PDC_TCS_WAIT_CMPL_OFF(t)))

#define PDC_TCS_WAIT_CMPL_WRITE(base, t, val) \
	mmio_write_32(PDC_REG(base, PDC_TCS_WAIT_CMPL_OFF(t)), (uint32_t)(val))

/* --------------------------------------------------------------------------
 * TCSt_CMDn registers  (base + offset + 0xC8*t + 0x10*n)
 * --------------------------------------------------------------------------
 */
#define PDC_TCS_MSGID_OFF(t, n)	(0x550cU + 0xc8U * (uint32_t)(t) + \
				 0x10U * (uint32_t)(n))
#define PDC_TCS_ADDR_OFF(t, n)	(0x5510U + 0xc8U * (uint32_t)(t) + \
				 0x10U * (uint32_t)(n))
#define PDC_TCS_DATA_OFF(t, n)	(0x5514U + 0xc8U * (uint32_t)(t) + \
				 0x10U * (uint32_t)(n))

#define PDC_TCS_MSGID_WRITE_BMSK	0x10000U	/* READ_OR_WRITE */
#define PDC_TCS_MSGID_RES_REQ_BMSK	0x100U

#define PDC_TCS_MSGID_WRITE(base, t, n, val) \
	mmio_write_32(PDC_REG(base, PDC_TCS_MSGID_OFF(t, n)), (uint32_t)(val))

#define PDC_TCS_ADDR_WRITE(base, t, n, val) \
	mmio_write_32(PDC_REG(base, PDC_TCS_ADDR_OFF(t, n)), (uint32_t)(val))

#define PDC_TCS_DATA_WRITE(base, t, n, val) \
	mmio_write_32(PDC_REG(base, PDC_TCS_DATA_OFF(t, n)), (uint32_t)(val))

/* --------------------------------------------------------------------------
 * IRQ_i_OWNER  (PDC_BASE + 0x4600 + 0x4*i)
 * --------------------------------------------------------------------------
 */
#define PDC_IRQ_OWNER_OFF(i)	(0x4600U + 0x4U * (uint32_t)(i))
#define PDC_IRQ_OWNER_BMSK	0x7U

#define PDC_IRQ_OWNER_WRITE(i, val) \
	mmio_write_32(PDC_REG(PDC_BASE, PDC_IRQ_OWNER_OFF(i)), \
		      (uint32_t)(val) & PDC_IRQ_OWNER_BMSK)

#endif /* PDC_REGS_H */
