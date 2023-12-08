/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Versal IPI management enums and defines */

#ifndef PLAT_IPI_H
#define PLAT_IPI_H

#include <stdint.h>

#include <ipi.h>

/*********************************************************************
 * IPI agent IDs macros
 ********************************************************************/
#define IPI_ID_PMC	1U
#define IPI_ID_APU	2U
#define IPI_ID_RPU0	3U
#define IPI_ID_RPU1	4U
#define IPI_ID_3	5U
#define IPI_ID_4	6U
#define IPI_ID_5	7U
#define IPI_ID_PMC_NOBUF	8U
#define IPI_ID_6_NOBUF_95	9U
#define IPI_ID_1_NOBUF	10U
#define IPI_ID_2_NOBUF	11U
#define IPI_ID_3_NOBUF	12U
#define IPI_ID_4_NOBUF	13U
#define IPI_ID_5_NOBUF	14U
#define IPI_ID_6_NOBUF_101	15U
#define IPI_ID_MAX	16U

/*********************************************************************
 * IPI message buffers
 ********************************************************************/
#define IPI_BUFFER_BASEADDR	(0xEB3F0000U)

#define IPI_LOCAL_ID		IPI_ID_APU
#define IPI_REMOTE_ID		IPI_ID_PMC

#define IPI_BUFFER_LOCAL_BASE	(IPI_BUFFER_BASEADDR + (IPI_LOCAL_ID * 0x200U))
#define IPI_BUFFER_REMOTE_BASE	(IPI_BUFFER_BASEADDR + (IPI_REMOTE_ID * 0x200U))

#define IPI_BUFFER_TARGET_LOCAL_OFFSET	(IPI_LOCAL_ID * 0x40U)
#define IPI_BUFFER_TARGET_REMOTE_OFFSET	(IPI_REMOTE_ID * 0x40U)

#define IPI_BUFFER_MAX_WORDS	8

#define IPI_BUFFER_REQ_OFFSET	0x0U
#define IPI_BUFFER_RESP_OFFSET	0x20U

/*********************************************************************
 * Platform specific IPI API declarations
 ********************************************************************/

/* Configure IPI table for versal_net */
void versal_net_ipi_config_table_init(void);

/*******************************************************************************
 * IPI registers and bitfields
 ******************************************************************************/
#define IPI0_REG_BASE		(0xEB330000U)
#define IPI0_TRIG_BIT		(1 << 2)
#define PMC_IPI_TRIG_BIT	(1 << 1)
#define IPI1_REG_BASE		(0xEB340000U)
#define IPI1_TRIG_BIT		(1 << 3)
#define IPI2_REG_BASE		(0xEB350000U)
#define IPI2_TRIG_BIT		(1 << 4)
#define IPI3_REG_BASE		(0xEB360000U)
#define IPI3_TRIG_BIT		(1 << 5)
#define IPI4_REG_BASE		(0xEB370000U)
#define IPI4_TRIG_BIT		(1 << 6)
#define IPI5_REG_BASE		(0xEB380000U)
#define IPI5_TRIG_BIT		(1 << 7)
#define PMC_NOBUF_REG_BASE	(0xEB390000U)
#define PMC_NOBUF_TRIG_BIT	(1 << 8)
#define IPI6_NOBUF_95_REG_BASE	(0xEB3A0000U)
#define IPI6_NOBUF_95_TRIG_BIT	(1 << 9)
#define IPI1_NOBUF_REG_BASE	(0xEB3B0000U)
#define IPI1_NOBUF_TRIG_BIT	(1 << 10)
#define IPI2_NOBUF_REG_BASE	(0xEB3B1000U)
#define IPI2_NOBUF_TRIG_BIT	(1 << 11)
#define IPI3_NOBUF_REG_BASE	(0xEB3B2000U)
#define IPI3_NOBUF_TRIG_BIT	(1 << 12)
#define IPI4_NOBUF_REG_BASE	(0xEB3B3000U)
#define IPI4_NOBUF_TRIG_BIT	(1 << 13)
#define IPI5_NOBUF_REG_BASE	(0xEB3B4000U)
#define IPI5_NOBUF_TRIG_BIT	(1 << 14)
#define IPI6_NOBUF_101_REG_BASE	(0xEB3B5000U)
#define IPI6_NOBUF_101_TRIG_BIT	(1 << 15)

#endif /* PLAT_IPI_H */
