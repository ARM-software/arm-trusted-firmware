/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEF_H
#define DEF_H

#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

#define MAX_INTR_EL3			2

/* List all consoles */
#define CONSOLE_ID_pl011	U(1)
#define CONSOLE_ID_pl011_0	U(1)
#define CONSOLE_ID_pl011_1	U(2)
#define CONSOLE_ID_dcc	U(3)

#define CONSOLE_IS(con)	(CONSOLE_ID_ ## con == CONSOLE)

/* List all platforms */
#define SILICON		U(0)
#define SPP			U(1)
#define EMU			U(2)
#define QEMU			U(3)
#define SPP_MMD			U(5)
#define EMU_MMD			U(6)
#define QEMU_COSIM		U(7)

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
#define PRIMARY_CPU		U(0)

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
#define CRL					U(0xEB5E0000)
#define CRL_TIMESTAMP_REF_CTRL_OFFSET	U(0x14C)
#define CRL_RST_TIMESTAMP_OFFSET		U(0x348)

#define CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	(1U << 25U)

/* IOU SCNTRS */
#define IOU_SCNTRS					U(0xEC920000)
#define IOU_SCNTRS_COUNTER_CONTROL_REG_OFFSET	U(0)
#define IOU_SCNTRS_BASE_FREQ_OFFSET			U(0x20)

#define IOU_SCNTRS_CONTROL_EN	U(1)

#define APU_CLUSTER0		U(0xECC00000)
#define APU_RVBAR_L_0		U(0x40)
#define APU_RVBAR_H_0		U(0x44)
#define APU_CLUSTER_STEP	U(0x100000)

#define SLCR_OSPI_QSPI_IOU_AXI_MUX_SEL	U(0xF1060504)

/*******************************************************************************
 * IRQ constants
 ******************************************************************************/
#define IRQ_SEC_PHY_TIMER	U(29)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE		U(0xF1920000)
#define UART1_BASE		U(0xF1930000)

#define UART_BAUDRATE	115200

#if CONSOLE_IS(pl011_1)
#define UART_BASE		UART1_BASE
#else
/* Default console is UART0 */
#define UART_BASE            UART0_BASE
#endif

#endif /* DEF_H */
