/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VERSAL_NET_DEF_H
#define VERSAL_NET_DEF_H

#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

#define MAX_INTR_EL3			2
/* This part is taken from U-Boot project under GPL that's why dual license above */
#define __bf_shf(x) (__builtin_ffsll(x) - 1U)
#define FIELD_GET(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

/* List all consoles */
#define VERSAL_NET_CONSOLE_ID_pl011	U(1)
#define VERSAL_NET_CONSOLE_ID_pl011_0	U(1)
#define VERSAL_NET_CONSOLE_ID_pl011_1	U(2)

#define VERSAL_NET_CONSOLE_IS(con)	(VERSAL_NET_CONSOLE_ID_ ## con == VERSAL_NET_CONSOLE)

/* List all platforms */
#define VERSAL_NET_SILICON		U(0)
#define VERSAL_NET_SPP			U(1)
#define VERSAL_NET_EMU			U(2)
#define VERSAL_NET_QEMU			U(3)
#define VERSAL_NET_QEMU_COSIM		U(7)

/* For platform detection */
#define PMC_TAP				U(0xF11A0000)
#define PMC_TAP_VERSION			(PMC_TAP + 0x4U)
# define PLATFORM_MASK			GENMASK(27U, 24U)
# define PLATFORM_VERSION_MASK		GENMASK(31U, 28U)

/* Global timer reset */
#define PSX_CRF			U(0xEC200000)
#define ACPU0_CLK_CTRL		U(0x10C)
#define ACPU_CLK_CTRL_CLKACT	BIT(25)

#define RST_APU0_OFFSET		U(0x300)
#define RST_APU_COLD_RESET	BIT(0)
#define RST_APU_WARN_RESET	BIT(4)
#define RST_APU_CLUSTER_COLD_RESET	BIT(8)
#define RST_APU_CLUSTER_WARM_RESET	BIT(9)

#define PSX_CRF_RST_TIMESTAMP_OFFSET	U(0x33C)

#define APU_PCLI			(0xECB10000ULL)
#define APU_PCLI_CPU_STEP		(0x30ULL)
#define APU_PCLI_CLUSTER_CPU_STEP	(4ULL * APU_PCLI_CPU_STEP)
#define APU_PCLI_CLUSTER_OFFSET		U(0x8000)
#define APU_PCLI_CLUSTER_STEP		U(0x1000)
#define PCLI_PREQ_OFFSET		U(0x4)
#define PREQ_CHANGE_REQUEST		BIT(0)
#define PCLI_PSTATE_OFFSET		U(0x8)
#define PCLI_PSTATE_VAL_SET		U(0x48)
#define PCLI_PSTATE_VAL_CLEAR		U(0x38)

/* Firmware Image Package */
#define VERSAL_NET_PRIMARY_CPU		U(0)

#define CORE_0_ISR_WAKE_OFFSET			(0x00000020ULL)
#define APU_PCIL_CORE_X_ISR_WAKE_REG(cpu_id)	(APU_PCLI + (CORE_0_ISR_WAKE_OFFSET + \
						 (APU_PCLI_CPU_STEP * (cpu_id))))
#define APU_PCIL_CORE_X_ISR_WAKE_MASK		(0x00000001U)
#define CORE_0_IEN_WAKE_OFFSET			(0x00000028ULL)
#define APU_PCIL_CORE_X_IEN_WAKE_REG(cpu_id)	(APU_PCLI + (CORE_0_IEN_WAKE_OFFSET + \
						 (APU_PCLI_CPU_STEP * (cpu_id))))
#define APU_PCIL_CORE_X_IEN_WAKE_MASK		(0x00000001U)
#define CORE_0_IDS_WAKE_OFFSET			(0x0000002CULL)
#define APU_PCIL_CORE_X_IDS_WAKE_REG(cpu_id)	(APU_PCLI + (CORE_0_IDS_WAKE_OFFSET + \
						 (APU_PCLI_CPU_STEP * (cpu_id))))
#define APU_PCIL_CORE_X_IDS_WAKE_MASK		(0x00000001U)
#define CORE_0_ISR_POWER_OFFSET			(0x00000010ULL)
#define APU_PCIL_CORE_X_ISR_POWER_REG(cpu_id)	(APU_PCLI + (CORE_0_ISR_POWER_OFFSET + \
						 (APU_PCLI_CPU_STEP * (cpu_id))))
#define APU_PCIL_CORE_X_ISR_POWER_MASK		U(0x00000001)
#define CORE_0_IEN_POWER_OFFSET			(0x00000018ULL)
#define APU_PCIL_CORE_X_IEN_POWER_REG(cpu_id)	(APU_PCLI + (CORE_0_IEN_POWER_OFFSET + \
						 (APU_PCLI_CPU_STEP * (cpu_id))))
#define APU_PCIL_CORE_X_IEN_POWER_MASK		(0x00000001U)
#define CORE_0_IDS_POWER_OFFSET			(0x0000001CULL)
#define APU_PCIL_CORE_X_IDS_POWER_REG(cpu_id)	(APU_PCLI + (CORE_0_IDS_POWER_OFFSET + \
						 (APU_PCLI_CPU_STEP * (cpu_id))))
#define APU_PCIL_CORE_X_IDS_POWER_MASK		(0x00000001U)
#define CORE_PWRDN_EN_BIT_MASK			(0x1U)

/*******************************************************************************
 * memory map related constants
 ******************************************************************************/
/* IPP 1.2/SPP 0.9 mapping */
#define DEVICE0_BASE		U(0xE8000000) /* psx, crl, iou */
#define DEVICE0_SIZE		U(0x08000000)
#define DEVICE1_BASE		U(0xE2000000) /* gic */
#define DEVICE1_SIZE		U(0x00800000)
#define DEVICE2_BASE		U(0xF1000000) /* uart, pmc_tap */
#define DEVICE2_SIZE		U(0x01000000)
#define CRF_BASE		U(0xFD1A0000)
#define CRF_SIZE		U(0x00600000)
#define IPI_BASE		U(0xEB300000)
#define IPI_SIZE		U(0x00100000)

/* CRL */
#define VERSAL_NET_CRL					U(0xEB5E0000)
#define VERSAL_NET_CRL_TIMESTAMP_REF_CTRL_OFFSET	U(0x14C)
#define VERSAL_NET_CRL_RST_TIMESTAMP_OFFSET		U(0x348)

#define VERSAL_NET_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	(1U << 25U)

/* IOU SCNTRS */
#define VERSAL_NET_IOU_SCNTRS					U(0xEC920000)
#define VERSAL_NET_IOU_SCNTRS_COUNTER_CONTROL_REG_OFFSET	U(0)
#define VERSAL_NET_IOU_SCNTRS_BASE_FREQ_OFFSET			U(0x20)

#define VERSAL_NET_IOU_SCNTRS_CONTROL_EN	U(1)

#define APU_CLUSTER0		U(0xECC00000)
#define APU_RVBAR_L_0		U(0x40)
#define APU_RVBAR_H_0		U(0x44)
#define APU_CLUSTER_STEP	U(0x100000)

#define SLCR_OSPI_QSPI_IOU_AXI_MUX_SEL	U(0xF1060504)

/*******************************************************************************
 * IRQ constants
 ******************************************************************************/
#define VERSAL_NET_IRQ_SEC_PHY_TIMER	U(29)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define VERSAL_NET_UART0_BASE		U(0xF1920000)
#define VERSAL_NET_UART_BAUDRATE	115200

#define VERSAL_NET_UART_BASE		VERSAL_NET_UART0_BASE

#define PLAT_VERSAL_NET_CRASH_UART_BASE		VERSAL_NET_UART_BASE
#define PLAT_VERSAL_NET_CRASH_UART_CLK_IN_HZ	VERSAL_NET_UART_CLOCK
#define VERSAL_NET_CONSOLE_BAUDRATE		VERSAL_NET_UART_BAUDRATE

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

/* Processor core device IDs */
#define PM_DEV_CLUSTER0_ACPU_0	(0x1810C0AFU)
#define PM_DEV_CLUSTER0_ACPU_1	(0x1810C0B0U)
#define PM_DEV_CLUSTER0_ACPU_2	(0x1810C0B1U)
#define PM_DEV_CLUSTER0_ACPU_3	(0x1810C0B2U)

#define PM_DEV_CLUSTER1_ACPU_0	(0x1810C0B3U)
#define PM_DEV_CLUSTER1_ACPU_1	(0x1810C0B4U)
#define PM_DEV_CLUSTER1_ACPU_2	(0x1810C0B5U)
#define PM_DEV_CLUSTER1_ACPU_3	(0x1810C0B6U)

#define PM_DEV_CLUSTER2_ACPU_0	(0x1810C0B7U)
#define PM_DEV_CLUSTER2_ACPU_1	(0x1810C0B8U)
#define PM_DEV_CLUSTER2_ACPU_2	(0x1810C0B9U)
#define PM_DEV_CLUSTER2_ACPU_3	(0x1810C0BAU)

#define PM_DEV_CLUSTER3_ACPU_0	(0x1810C0BBU)
#define PM_DEV_CLUSTER3_ACPU_1	(0x1810C0BCU)
#define PM_DEV_CLUSTER3_ACPU_2	(0x1810C0BDU)
#define PM_DEV_CLUSTER3_ACPU_3	(0x1810C0BEU)

#endif /* VERSAL_NET_DEF_H */
