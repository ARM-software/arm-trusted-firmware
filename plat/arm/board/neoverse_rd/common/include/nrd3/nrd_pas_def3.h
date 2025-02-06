/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NRD_PAS_DEF3_H
#define NRD_PAS_DEF3_H

#ifndef __ASSEMBLER__
#include <stddef.h>
#include <lib/gpt_rme/gpt_rme.h>
#endif

#include <nrd_css_def3.h>

/*****************************************************************************
 * PAS regions used to initialize the Granule Protection Table (GPT)
 ****************************************************************************/

/*
 * =====================================================================
 * Base Addr        |Size    |L? GPT |PAS    |Content                  |
 * =====================================================================
 * 0x00000000       |256MB   |L0 GPT |ANY    |SHARED RAM               |
 * 0x0FFFFFFF       |        |       |       |AP EXPANSION             |
 * ---------------------------------------------------------------------
 * 0x20000000       |64MB    |L1 GPT |ROOT   |SYSTEM NCI               |
 * 0x23FFFFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x28000000       |16MB    |L1 GPT |SECURE |DEBUG NIC                |
 * 0x28FFFFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A400000       |64KB    |L1 GPT |NS     |NS UART                  |
 * 0x2A40FFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A410000       |64KB    |L1 GPT |SECURE |SECURE UART              |
 * 0x2A41FFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A420000       |64KB    |L1 GPT |REALM  |REALM UART               |
 * 0x2A42FFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A430000       |64KB    |L1 GPT |SECURE |GENERIC REFCLK           |
 * 0x2A43FFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A440000       |128KB   |L1 GPT |NS     |AP NS WDOG               |
 * 0x2A45FFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A460000       |128KB   |L1 GPT |ROOT   |AP ROOT WDOG             |
 * 0x2A47FFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A480000       |128KB   |L1 GPT |SECURE |AP SECURE WDOG           |
 * 0x2A49FFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A4A0000       |64KB    |L1 GPT |NS     |SID                      |
 * 0x2A4AFFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2A4B0000       |64KB    |L1 GPT |SECURE |SECURE SRAM ERROR        |
 * 0x2A4BFFFF       |        |       |       |RECORD BLOCK - AP        |
 * ---------------------------------------------------------------------
 * 0x2A4C0000       |64KB    |L1 GPT |NS     |NS SRAM ERROR            |
 * 0x2A4CFFFF       |        |       |       |RECORD BLOCK - AP        |
 * ---------------------------------------------------------------------
 * 0x2A4D0000       |64KB    |L1 GPT |ROOT   |ROOT SRAM ERROR          |
 * 0x2A4DFFFF       |        |       |       |RECORD BLOCK - AP        |
 * ---------------------------------------------------------------------
 * 0x2A4E0000       |64KB    |L1 GPT |REALM  |REALM SRAM ERROR         |
 * 0x2A4EFFFF       |        |       |       |RECORD BLOCK - AP        |
 * ---------------------------------------------------------------------
 * 0x2A4F0000       |64KB    |L1 GPT |SECURE |SECURE SRAM ERROR        |
 * 0x2A4FFFFF       |        |       |       |RECORD BLOCK - SCP       |
 * ---------------------------------------------------------------------
 * 0x2A500000       |64KB    |L1 GPT |NS     |NS SRAM ERROR            |
 * 0x2A50FFFF       |        |       |       |RECORD BLOCK - SCP       |
 * ---------------------------------------------------------------------
 * 0x2A510000       |64KB    |L1 GPT |ROOT   |ROOT SRAM ERROR          |
 * 0x2A51FFFF       |        |       |       |RECORD BLOCK - SCP       |
 * ---------------------------------------------------------------------
 * 0x2A520000       |64KB    |L1 GPT |REALM  |REALM SRAM ERROR         |
 * 0x2A52FFFF       |        |       |       |RECORD BLOCK - SCP       |
 * ---------------------------------------------------------------------
 * 0x2A530000       |64KB    |L1 GPT |SECURE |SECURE SRAM ERROR        |
 * 0x2A53FFFF       |        |       |       |RECORD BLOCK - MCP       |
 * ---------------------------------------------------------------------
 * 0x2A540000       |64KB    |L1 GPT |NS     |NS SRAM ERROR            |
 * 0x2A54FFFF       |        |       |       |RECORD BLOCK - MCP       |
 * ---------------------------------------------------------------------
 * 0x2A550000       |64KB    |L1 GPT |ROOT   |ROOT SRAM ERROR          |
 * 0x2A55FFFF       |        |       |       |RECORD BLOCK - MCP       |
 * ---------------------------------------------------------------------
 * 0x2A560000       |64KB    |L1 GPT |REALM  |REALM SRAM ERROR         |
 * 0x2A56FFFF       |        |       |       |RECORD BLOCK - MCP       |
 * ---------------------------------------------------------------------
 * 0x2A570000       |64KB    |L1 GPT |SECURE |SECURE SRAM ERROR        |
 * 0x2A57FFFF       |        |       |       |RECORD BLOCK - RSE       |
 * ---------------------------------------------------------------------
 * 0x2A580000       |64KB    |L1 GPT |NS     |NS SRAM ERROR            |
 * 0x2A58FFFF       |        |       |       |RECORD BLOCK - RSE       |
 * ---------------------------------------------------------------------
 * 0x2A590000       |64KB    |L1 GPT |ROOT   |ROOT SRAM ERROR          |
 * 0x2A59FFFF       |        |       |       |RECORD BLOCK - RSE       |
 * ---------------------------------------------------------------------
 * 0x2A5A0000       |64KB    |L1 GPT |REALM  |REALM SRAM ERROR         |
 * 0x2A5AFFFF       |        |       |       |RECORD BLOCK - RSE       |
 * ---------------------------------------------------------------------
 * 0x2A5B0000       |64KB    |L1 GPT |SECURE |RSE SECURE SRAM ERROR    |
 * 0x2A5BFFFF       |        |       |       |RECORD BLOCK - RSM       |
 * ---------------------------------------------------------------------
 * 0x2A5C0000       |64KB    |L1 GPT |NS     |RSE NS SRAM ERROR        |
 * 0x2A5CFFFF       |        |       |       |RECORD BLOCK - RSM       |
 * ---------------------------------------------------------------------
 * 0x2A5D0000       |64KB    |L1 GPT |SECURE |SCP SECURE SRAM ERROR    |
 * 0x2A5DFFFF       |        |       |       |RECORD BLOCK - RSM       |
 * ---------------------------------------------------------------------
 * 0x2A5E0000       |64KB    |L1 GPT |NS     |SCP NS SRAM ERROR        |
 * 0x2A5EFFFF       |        |       |       |RECORD BLOCK - RSM       |
 * ---------------------------------------------------------------------
 * 0x2A5F0000       |64KB    |L1 GPT |SECURE |MCP SECURE SRAM ERROR    |
 * 0x2A5FFFFF       |        |       |       |RECORD BLOCK - RSM       |
 * ---------------------------------------------------------------------
 * 0x2A600000       |64KB    |L1 GPT |NS     |MCP NS SRAM ERROR        |
 * 0x2A60FFFF       |        |       |       |RECORD BLOCK - RSM       |
 * ---------------------------------------------------------------------
 * 0x2A800000       |128KB   |L1 GPT |NS     |CNTCTL REFCLK            |
 * 0x2A81FFFF       |        |       |       |READ FRAME               |
 * ---------------------------------------------------------------------
 * 0x2A820000       |64KB    |L1 GPT |SECURE |SECURE TIMER CTL         |
 * 0x2A82FFFF       |        |       |       |BASE FRAME               |
 * ---------------------------------------------------------------------
 * 0x2A830000       |64KB    |L1 GPT |NS     |NS TIMER CTL             |
 * 0x2A83FFFF       |        |       |       |BASE FRAME               |
 * ---------------------------------------------------------------------
 * 0x2A900000       |128KB   |L1 GPT |NS     |AP-SCP NS                |
 * 0x2A91FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2A920000       |128KB   |L1 GPT |SECURE |AP-SCP SECURE            |
 * 0x2A93FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2A940000       |128KB   |L1 GPT |ROOT   |AP-SCP ROOT              |
 * 0x2A95FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AA00000       |128KB   |L1 GPT |NS     |AP-MCP NS                |
 * 0x2AA1FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AA20000       |128KB   |L1 GPT |SECURE |AP-MCP SECURE            |
 * 0x2AA3FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AA40000       |128KB   |L1 GPT |ROOT   |AP-MCP ROOT              |
 * 0x2AA5FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AB00000       |128KB   |L1 GPT |NS     |AP-MCP NS                |
 * 0x2AB1FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AB20000       |128KB   |L1 GPT |SECURE |AP-RSE SECURE            |
 * 0x2AB3FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AB40000       |128KB   |L1 GPT |ROOT   |AP-RSE ROOT              |
 * 0x2AB5FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AB60000       |128KB   |L1 GPT |REALM  |AP-RSE REALM             |
 * 0x2AB7FFFF       |        |       |       |MHU                      |
 * ---------------------------------------------------------------------
 * 0x2AC00000       |1152KB  |L1 GPT |ROOT   |SCP MCP RSE              |
 * 0x2ACEFFFF       |        |       |       |CROSS CHIP MHU           |
 * ---------------------------------------------------------------------
 * 0x2B100000       |192KB   |L1 GPT |SECURE |SYNCNT                   |
 * 0x2B12FFFF       |        |       |       |MSTUPDTVAL_ADDR          |
 * ---------------------------------------------------------------------
 * 0x2CF00000       |~33MB   |L1 GPT |NS     |STM SYSTEM ITS           |
 * 0x2EFFFFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x2F000000       |4MB     |L1 GPT |ANY    |SHARED SRAM              |
 * 0x2F3FFFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x30000000       |128MB   |L1 GPT |ANY    |GIC CLAYTON              |
 * 0x37FFFFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x80000000       |2GB -   |L1 GPT |NS     |NS DRAM                  |
 * 0xF3FFFFFF       |192MB   |       |       |                         |
 * --------------------------------------------------------------------|
 * 0xF4000000       |9692KB  |L1 GPT |SECURE |BL32                     |
 * 0xFB200000       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x80000000       |26MB    |L1 GPT |REALM  |RMM                      |
 * 0x37FFFFFF       |        |       |       |TF-A SHARED              |
 * ---------------------------------------------------------------------
 * 0x80000000       |2MB     |L1 GPT |ROOT   |L1GPT                    |
 * 0x37FFFFFF       |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x100080000000   |2GB     |L1 GPT |NS     |DRAM 1 CHIP 3            |
 * 0x1000FFFFFFFF   |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x200080000000   |2GB     |L1 GPT |NS     |DRAM 1 CHIP 2            |
 * 0x2000FFFFFFFF   |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x300080000000   |2GB     |L1 GPT |NS     |DRAM 1 CHIP 1            |
 * 0x3000FFFFFFFF   |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x100000000      |1GB     |L1 GPT |ANY    |CMN                      |
 * 0x13FFFFFFF      |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x200000000      |1GB     |L1 GPT |ANY    |LCP PERIPHERALS          |
 * 0x23FFFFFFF      |        |       |       |DDR                      |
 * ---------------------------------------------------------------------
 * 0x240000000      |1GB     |L1 GPT |ANY    |DDR IO                   |
 * 0x27FFFFFFF      |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x280000000      |1.5GB   |L1 GPT |ANY    |SMMU & NCI IO            |
 * 0x2DFFFFFFF      |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x300000000      |128MB   |L1 GPT |ROOT   |GPC SMMU                 |
 * 0x308000000      |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x8080000000     |6GB     |L1 GPT |ANY    |DRAM 2 CHIP 0            |
 * 0x81FFFFFFFF     |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x108080000000   |6GB     |L1 GPT |NS     |DRAM 2 CHIP 1            |
 * 0x1081FFFFFFFF   |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x208080000000   |6GB     |L1 GPT |NS     |DRAM 2 CHIP 2            |
 * 0x2081FFFFFFFF   |        |       |       |                         |
 * ---------------------------------------------------------------------
 * 0x308080000000   |6GB     |L1 GPT |NS     |DRAM 2 CHIP 3            |
 * 0x3081FFFFFFFF   |        |       |       |                         |
 * =====================================================================
 */

/*******************************************************************************
 * Multichip config
 ******************************************************************************/

#define NRD_MC_BASE(base, n)		(NRD_REMOTE_CHIP_MEM_OFFSET(n) + base)

/*******************************************************************************
 * PAS mappings
 ******************************************************************************/

#define NRD_PAS_SHARED_SRAM						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SHARED_SRAM_BASE,			\
			NRD_CSS_SHARED_SRAM_SIZE,			\
			GPT_GPI_ANY)

#define NRD_PAS_SYSTEM_NCI						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SYSTEM_NCI_BASE,			\
			NRD_CSS_SYSTEM_NCI_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_DEBUG_NIC						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_DEBUG_NIC_BASE,				\
			NRD_CSS_DEBUG_NIC_SIZE,				\
			GPT_GPI_SECURE)

#define NRD_PAS_NS_UART							\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_NS_UART_BASE,				\
			NRD_CSS_NS_UART_SIZE,				\
			GPT_GPI_NS)

#define NRD_PAS_REALM_UART						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_REALM_UART_BASE,			\
			NRD_CSS_REALM_UART_SIZE,			\
			GPT_GPI_REALM)

#define NRD_PAS_AP_NS_WDOG						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_NS_WDOG_BASE,			\
			NRD_CSS_AP_NS_WDOG_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_AP_ROOT_WDOG						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_ROOT_WDOG_BASE,			\
			NRD_CSS_AP_ROOT_WDOG_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_AP_SECURE_WDOG						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_SECURE_WDOG_BASE,			\
			NRD_CSS_AP_SECURE_WDOG_SIZE,			\
			GPT_GPI_SECURE)

#define NRD_PAS_SECURE_SRAM_ERB_AP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SECURE_SRAM_ERB_AP_BASE,		\
			NRD_CSS_SECURE_SRAM_ERB_AP_SIZE,		\
			GPT_GPI_SECURE)

#define NRD_PAS_NS_SRAM_ERB_AP						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_NS_SRAM_ERB_AP_BASE,			\
			NRD_CSS_NS_SRAM_ERB_AP_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_ROOT_SRAM_ERB_AP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_ROOT_SRAM_ERB_AP_BASE,			\
			NRD_CSS_ROOT_SRAM_ERB_AP_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_REALM_SRAM_ERB_AP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_REALM_SRAM_ERB_AP_BASE,			\
			NRD_CSS_REALM_SRAM_ERB_AP_SIZE,			\
			GPT_GPI_REALM)

#define NRD_PAS_SECURE_SRAM_ERB_SCP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SECURE_SRAM_ERB_SCP_BASE,		\
			NRD_CSS_SECURE_SRAM_ERB_SCP_SIZE,		\
			GPT_GPI_SECURE)

#define NRD_PAS_NS_SRAM_ERB_SCP						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_NS_SRAM_ERB_SCP_BASE,			\
			NRD_CSS_NS_SRAM_ERB_SCP_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_ROOT_SRAM_ERB_SCP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_ROOT_SRAM_ERB_SCP_BASE,			\
			NRD_CSS_ROOT_SRAM_ERB_SCP_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_REALM_SRAM_ERB_SCP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_REALM_SRAM_ERB_SCP_BASE,		\
			NRD_CSS_REALM_SRAM_ERB_SCP_SIZE,		\
			GPT_GPI_REALM)

#define NRD_PAS_SECURE_SRAM_ERB_MCP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SECURE_SRAM_ERB_MCP_BASE,		\
			NRD_CSS_SECURE_SRAM_ERB_MCP_SIZE,		\
			GPT_GPI_SECURE)

#define NRD_PAS_NS_SRAM_ERB_MCP						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_NS_SRAM_ERB_MCP_BASE,			\
			NRD_CSS_NS_SRAM_ERB_MCP_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_ROOT_SRAM_ERB_MCP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_ROOT_SRAM_ERB_MCP_BASE,			\
			NRD_CSS_ROOT_SRAM_ERB_MCP_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_REALM_SRAM_ERB_MCP					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_REALM_SRAM_ERB_MCP_BASE,		\
			NRD_CSS_REALM_SRAM_ERB_MCP_SIZE,		\
			GPT_GPI_REALM)

#define NRD_PAS_SECURE_SRAM_ERB_RSE					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SECURE_SRAM_ERB_RSE_BASE,		\
			NRD_CSS_SECURE_SRAM_ERB_RSE_SIZE,		\
			GPT_GPI_SECURE)

#define NRD_PAS_NS_SRAM_ERB_RSE						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_NS_SRAM_ERB_RSE_BASE,			\
			NRD_CSS_NS_SRAM_ERB_RSE_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_ROOT_SRAM_ERB_RSE					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_ROOT_SRAM_ERB_RSE_BASE,			\
			NRD_CSS_ROOT_SRAM_ERB_RSE_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_REALM_SRAM_ERB_RSE					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_REALM_SRAM_ERB_RSE_BASE,		\
			NRD_CSS_REALM_SRAM_ERB_RSE_SIZE,		\
			GPT_GPI_REALM)

#define NRD_PAS_RSE_SECURE_SRAM_ERB_RSM					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_RSE_SECURE_SRAM_ERB_RSM_BASE,		\
			NRD_CSS_RSE_SECURE_SRAM_ERB_RSM_SIZE,		\
			GPT_GPI_SECURE)

#define NRD_PAS_RSE_NS_SRAM_ERB_RSM					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_RSE_NS_SRAM_ERB_RSM_BASE,		\
			NRD_CSS_RSE_NS_SRAM_ERB_RSM_SIZE,		\
			GPT_GPI_NS)

#define NRD_PAS_SCP_SECURE_SRAM_ERB_RSM					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SCP_SECURE_SRAM_ERB_RSM_BASE,		\
			NRD_CSS_SCP_SECURE_SRAM_ERB_RSM_SIZE,		\
			GPT_GPI_SECURE)

#define NRD_PAS_SCP_NS_SRAM_ERB_RSM					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SCP_NS_SRAM_ERB_RSM_BASE,		\
			NRD_CSS_SCP_NS_SRAM_ERB_RSM_SIZE,		\
			GPT_GPI_NS)

#define NRD_PAS_MCP_SECURE_SRAM_ERB_RSM					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_MCP_SECURE_SRAM_ERB_RSM_BASE,		\
			NRD_CSS_MCP_SECURE_SRAM_ERB_RSM_SIZE,		\
			GPT_GPI_SECURE)

#define	NRD_PAS_MCP_NS_SRAM_ERB_RSM					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_MCP_NS_SRAM_ERB_RSM_BASE,		\
			NRD_CSS_MCP_NS_SRAM_ERB_RSM_SIZE,		\
			GPT_GPI_NS)

#define NRD_PAS_AP_SCP_ROOT_MHU						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_SCP_ROOT_MHU_BASE,			\
			NRD_CSS_AP_SCP_ROOT_MHU_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_AP_MCP_NS_MHU						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_MCP_NS_MHU_BASE,			\
			NRD_CSS_AP_MCP_NS_MHU_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_AP_MCP_SECURE_MHU					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_MCP_SECURE_MHU_BASE,			\
			NRD_CSS_AP_MCP_SECURE_MHU_SIZE,			\
			GPT_GPI_SECURE)

#define NRD_PAS_AP_MCP_ROOT_MHU						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_MCP_ROOT_MHU_BASE,			\
			NRD_CSS_AP_MCP_ROOT_MHU_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_AP_RSE_NS_MHU						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_RSE_NS_MHU_BASE,			\
			NRD_CSS_AP_RSE_NS_MHU_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_AP_RSE_SECURE_MHU					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_RSE_SECURE_MHU_BASE,			\
			NRD_CSS_AP_RSE_SECURE_MHU_SIZE,			\
			GPT_GPI_SECURE)

#define NRD_PAS_AP_RSE_ROOT_MHU						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_RSE_ROOT_MHU_BASE,			\
			NRD_CSS_AP_RSE_ROOT_MHU_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_AP_RSE_REALM_MHU					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_AP_RSE_REALM_MHU_BASE,			\
			NRD_CSS_AP_RSE_REALM_MHU_SIZE,			\
			GPT_GPI_REALM)

#define NRD_PAS_SCP_MCP_RSE_CROSS_CHIP_MHU				\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SCP_MCP_RSE_CROSS_CHIP_MHU_BASE,	\
			NRD_CSS_SCP_MCP_RSE_CROSS_CHIP_MHU_SIZE,	\
			GPT_GPI_ROOT)

#define NRD_PAS_SYNCNT_MSTUPDTVAL_ADDR					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SYNCNT_MSTUPDTVAL_ADDR_BASE,		\
			NRD_CSS_SYNCNT_MSTUPDTVAL_ADDR_SIZE,		\
			GPT_GPI_SECURE)

#define NRD_PAS_STM_SYSTEM_ITS						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_STM_SYSTEM_ITS_BASE,			\
			NRD_CSS_STM_SYSTEM_ITS_SIZE,			\
			GPT_GPI_NS)

#define NRD_PAS_SCP_MCP_RSE_SHARED_SRAM					\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SCP_MCP_RSE_SHARED_SRAM_BASE,		\
			NRD_CSS_SCP_MCP_RSE_SHARED_SRAM_SIZE,		\
			GPT_GPI_ANY)

#define NRD_PAS_GIC							\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_GIC_BASE,				\
			NRD_CSS_GIC_SIZE,				\
			GPT_GPI_ANY)

#define NRD_PAS_NS_DRAM							\
		GPT_MAP_REGION_GRANULE(					\
			ARM_NS_DRAM1_BASE,				\
			ARM_NS_DRAM1_SIZE,				\
			GPT_GPI_NS)

#define NRD_PAS_DRAM1_CHIP1						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_MC_BASE(NRD_CSS_DRAM1_BASE, 1),		\
			ARM_DRAM1_SIZE,					\
			GPT_GPI_NS)

#define NRD_PAS_DRAM1_CHIP2						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_MC_BASE(NRD_CSS_DRAM1_BASE, 2),		\
			ARM_DRAM1_SIZE,					\
			GPT_GPI_NS)

#define NRD_PAS_DRAM1_CHIP3						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_MC_BASE(NRD_CSS_DRAM1_BASE, 3),		\
			ARM_DRAM1_SIZE,					\
			GPT_GPI_NS)

#if SPD_spmd && SPMD_SPM_AT_SEL2
#define NRD_PAS_BL32							\
		GPT_MAP_REGION_GRANULE(					\
			PLAT_ARM_SPMC_BASE,				\
			PLAT_ARM_SPMC_SIZE,				\
			GPT_GPI_SECURE)
#endif

#define NRD_PAS_RMM							\
		GPT_MAP_REGION_GRANULE(					\
			ARM_REALM_BASE,					\
			ARM_REALM_SIZE +				\
			ARM_EL3_RMM_SHARED_SIZE,			\
			GPT_GPI_REALM)

#define NRD_PAS_L1GPT							\
		GPT_MAP_REGION_GRANULE(					\
			ARM_L1_GPT_BASE,				\
			ARM_L1_GPT_SIZE,				\
			GPT_GPI_ROOT)

#define NRD_PAS_CMN							\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_CMN_BASE,				\
			NRD_CSS_CMN_SIZE,				\
			GPT_GPI_ANY)

#define NRD_PAS_LCP_PERIPHERAL						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_LCP_PERIPHERAL_BASE,			\
			NRD_CSS_LCP_PERIPHERAL_SIZE,			\
			GPT_GPI_ANY)

#define NRD_PAS_DDR_IO							\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_DDR_IO_BASE,				\
			NRD_CSS_DDR_IO_SIZE,				\
			GPT_GPI_ANY)

#define NRD_PAS_SMMU_NCI_IO						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_SMMU_NCI_IO_BASE,			\
			NRD_CSS_SMMU_NCI_IO_SIZE,			\
			GPT_GPI_ANY)

#define NRD_PAS_GPC_SMMUV3						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_CSS_GPC_SMMUV3_BASE,			\
			NRD_CSS_GPC_SMMUV3_SIZE,			\
			GPT_GPI_ROOT)

#define NRD_PAS_DRAM2_CHIP0						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_MC_BASE(NRD_CSS_DRAM2_BASE, 0),		\
			ARM_DRAM2_SIZE,					\
			GPT_GPI_NS)

#define NRD_PAS_DRAM2_CHIP1						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_MC_BASE(NRD_CSS_DRAM2_BASE, 1),		\
			ARM_DRAM2_SIZE,					\
			GPT_GPI_NS)

#define NRD_PAS_DRAM2_CHIP2						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_MC_BASE(NRD_CSS_DRAM2_BASE, 2),		\
			ARM_DRAM2_SIZE,					\
			GPT_GPI_NS)

#define NRD_PAS_DRAM2_CHIP3						\
		GPT_MAP_REGION_GRANULE(					\
			NRD_MC_BASE(NRD_CSS_DRAM2_BASE, 3),		\
			ARM_DRAM2_SIZE,					\
			GPT_GPI_NS)

#endif /* NRD_PAS_DEF3_H */
