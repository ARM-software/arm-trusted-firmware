/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "ncore_ccu.h"
#include "socfpga_mailbox.h"
#include "socfpga_plat_def.h"
#include "socfpga_system_manager.h"

uint32_t poll_active_bit(uint32_t dir);

#define SMMU_DMI					1
#define CCU_DMI0_DMIUSMCMCR				SOCFPGA_CCU_NOC_REG_BASE + 0x7340
#define CCU_DMI0_DMIUSMCMAR				SOCFPGA_CCU_NOC_REG_BASE + 0x7344
#define CCU_DMI0_DMIUSMCMCR_MNTOP			GENMASK(3, 0)
#define MAX_DISTRIBUTED_MEM_INTERFACE			2
#define FLUSH_ALL_ENTRIES				0x4
#define CCU_DMI0_DMIUSMCMCR_ARRAY_ID			GENMASK(21, 16)
#define ARRAY_ID_TAG					0x0
#define ARRAY_ID_DATA					0x1
#define CACHE_OPERATION_DONE				BIT(0)
#define TIMEOUT_200MS					200

#define __bf_shf(x)					(__builtin_ffsll(x) - 1)

#define FIELD_PREP(_mask, _val)						\
	({ \
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
ncore_ccu_reg_t ncore_ccu_modules[] = {
				{"caiu0@1c000000",             0x1C000000, 0x00001000},
				{"ncaiu0@1c001000",            0x1C001000, 0x00001000},
				{"ncaiu1@1c002000",            0x1C002000, 0x00001000},
				{"ncaiu2@1c003000",            0x1C003000, 0x00001000},
				{"ncaiu3@1c004000",            0x1C004000, 0x00001000},
				{"dce0@1c005000",              0x1C005000, 0x00001000},
				{"dce1@1c006000",              0x1C006000, 0x00001000},
				{"dmi0@1c007000",              0x1C007000, 0x00001000},
				{"dmi1@1c008000",              0x1C008000, 0x00001000},
				{"noc_fw_l4_per@10d21000",     0x10D21000, 0x0000008C},
				{"noc_fw_l4_sys@10d21100",     0x10D21100, 0x00000098},
				{"noc_fw_lwsoc2fpga@10d21300", 0x10D21300, 0x00000004},
				{"noc_fw_soc2fpga@10d21200",   0x10D21200, 0x00000004},
				{"noc_fw_tcu@10d21400",        0x10D21400, 0x00000004}
				};

ncore_ccu_t ccu_caiu0[] = {
				/* CAIUAMIGR */
				{0x000003C0, 0x00000003, 0x0000001F},
				/* CAIUMIFSR */
				{0x000003C4, 0x00000000, 0x07070777},
				/* DII1_MPFEREGS */
				{0x00000414, 0x00018000, 0xFFFFFFFF},
				{0x00000418, 0x00000000, 0x000000FF},
				{0x00000410, 0xC0E00200, 0xC1F03E1F},
				/* DII2_GICREGS */
				{0x00000424, 0x0001D000, 0xFFFFFFFF},
				{0x00000428, 0x00000000, 0x000000FF},
				{0x00000420, 0xC0800400, 0xC1F03E1F},
				/* NCAIU0_LWSOC2FPGA */
				{0x00000444, 0x00020000, 0xFFFFFFFF},
				{0x00000448, 0x00000000, 0x000000FF},
				{0x00000440, 0xC1100006, 0xC1F03E1F},
				/* NCAIU0_SOC2FPGA_1G */
				{0x00000454, 0x00040000, 0xFFFFFFFF},
				{0x00000458, 0x00000000, 0x000000FF},
				{0x00000450, 0xC1200006, 0xC1F03E1F},
				/* DMI_SDRAM_2G */
				{0x00000464, 0x00080000, 0xFFFFFFFF},
				{0x00000468, 0x00000000, 0x000000FF},
				{0x00000460, 0x81300006, 0xC1F03E1F},
				/* NCAIU0_SOC2FPGA_16G */
				{0x00000474, 0x00400000, 0xFFFFFFFF},
				{0x00000478, 0x00000000, 0x000000FF},
				{0x00000470, 0xC1600006, 0xC1F03E1F},
				/* DMI_SDRAM_30G */
				{0x00000484, 0x00800000, 0xFFFFFFFF},
				{0x00000488, 0x00000000, 0x000000FF},
				{0x00000480, 0x81700006, 0xC1F03E1F},
				/* NCAIU0_SOC2FPGA_256G */
				{0x00000494, 0x04000000, 0xFFFFFFFF},
				{0x00000498, 0x00000000, 0x000000FF},
				{0x00000490, 0xC1A00006, 0xC1F03E1F},
				/* DMI_SDRAM_480G */
				{0x000004A4, 0x08000000, 0xFFFFFFFF},
				{0x000004A8, 0x00000000, 0x000000FF},
				{0x000004A0, 0x81B00006, 0xC1F03E1F}
			};

ncore_ccu_t ccu_ncaiu0[] = {
				/* NCAIU0AMIGR */
				{0x000003C0, 0x00000003, 0x0000001F},
				/* NCAIU0MIFSR */
				{0x000003C4, 0x00000000, 0x07070777},
				/* PSS */
				{0x00000404, 0x00010000, 0xFFFFFFFF},
				{0x00000408, 0x00000000, 0x000000FF},
				{0x00000400, 0xC0F00000, 0xC1F03E1F},
				/* DII1_MPFEREGS */
				{0x00000414, 0x00018000, 0xFFFFFFFF},
				{0x00000418, 0x00000000, 0x000000FF},
				{0x00000410, 0xC0E00200, 0xC1F03E1F},
				/* NCAIU0_LWSOC2FPGA */
				{0x00000444, 0x00020000, 0xFFFFFFFF},
				{0x00000448, 0x00000000, 0x000000FF},
				{0x00000440, 0xC1100006, 0xC1F03E1F},
				/* NCAIU0_SOC2FPGA_1G */
				{0x00000454, 0x00040000, 0xFFFFFFFF},
				{0x00000458, 0x00000000, 0x000000FF},
				{0x00000450, 0xC1200006, 0xC1F03E1F},
				/* DMI_SDRAM_2G */
				{0x00000464, 0x00080000, 0xFFFFFFFF},
				{0x00000468, 0x00000000, 0x000000FF},
				{0x00000460, 0x81300006, 0xC1F03E1F},
				/* NCAIU0_SOC2FPGA_16G */
				{0x00000474, 0x00400000, 0xFFFFFFFF},
				{0x00000478, 0x00000000, 0x000000FF},
				{0x00000470, 0xC1600006, 0xC1F03E1F},
				/* DMI_SDRAM_30G */
				{0x00000484, 0x00800000, 0xFFFFFFFF},
				{0x00000488, 0x00000000, 0x000000FF},
				{0x00000480, 0x81700006, 0xC1F03E1F},
				/* NCAIU0_SOC2FPGA_256G */
				{0x00000494, 0x04000000, 0xFFFFFFFF},
				{0x00000498, 0x00000000, 0x000000FF},
				{0x00000490, 0xC1A00006, 0xC1F03E1F},
				/* DMI_SDRAM_480G */
				{0x000004A4, 0x08000000, 0xFFFFFFFF},
				{0x000004A8, 0x00000000, 0x000000FF},
				{0x000004A0, 0x81B00006, 0xC1F03E1F}
			};

ncore_ccu_t ccu_ncaiu1[] = {
				/* NCAIU1AMIGR */
				{0x000003C0, 0x00000003, 0x0000001F},
				/* NCAIU1MIFSR */
				{0x000003C4, 0x00000000, 0x07070777},
				/* DMI_SDRAM_2G */
				{0x00000464, 0x00080000, 0xFFFFFFFF},
				{0x00000468, 0x00000000, 0x000000FF},
				{0x00000460, 0x81300006, 0xC1F03E1F},
				/* DMI_SDRAM_30G */
				{0x00000484, 0x00800000, 0xFFFFFFFF},
				{0x00000488, 0x00000000, 0x000000FF},
				{0x00000480, 0x81700006, 0xC1F03E1F},
				/* DMI_SDRAM_480G */
				{0x000004A4, 0x08000000, 0xFFFFFFFF},
				{0x000004A8, 0x00000000, 0x000000FF},
				{0x000004A0, 0x81B00006, 0xC1F03E1F}
			};

ncore_ccu_t ccu_ncaiu2[] = {
				/* NCAIU2AMIGR */
				{0x000003C0, 0x00000003, 0x0000001F},
				/* NCAIU2MIFSR */
				{0x000003C4, 0x00000000, 0x07070777},
				/* DMI_SDRAM_2G */
				{0x00000464, 0x00080000, 0xFFFFFFFF},
				{0x00000468, 0x00000000, 0x000000FF},
				{0x00000460, 0x81300006, 0xC1F03E1F},
				/* DMI_SDRAM_30G */
				{0x00000484, 0x00800000, 0xFFFFFFFF},
				{0x00000488, 0x00000000, 0x000000FF},
				{0x00000480, 0x81700006, 0xC1F03E1F},
				/* DMI_SDRAM_480G */
				{0x000004A4, 0x08000000, 0xFFFFFFFF},
				{0x000004A8, 0x00000000, 0x000000FF},
				{0x000004A0, 0x81B00006, 0xC1F03E1F}
			};

ncore_ccu_t ccu_ncaiu3[] = {
				/* NCAIU3AMIGR */
				{0x000003C0, 0x00000003, 0x0000001F},
				/* NCAIU3MIFSR */
				{0x000003C4, 0x00000000, 0x07070777},
				/* DII1_MPFEREGS */
				{0x00000414, 0x00018000, 0xFFFFFFFF},
				{0x00000418, 0x00000000, 0x000000FF},
				{0x00000410, 0xC0E00200, 0xC1F03E1F},
				/* DMI_SDRAM_2G */
				{0x00000464, 0x00080000, 0xFFFFFFFF},
				{0x00000468, 0x00000000, 0x000000FF},
				{0x00000460, 0x81300006, 0xC1F03E1F},
				/* DMI_SDRAM_30G */
				{0x00000484, 0x00800000, 0xFFFFFFFF},
				{0x00000488, 0x00000000, 0x000000FF},
				{0x00000480, 0x81700006, 0xC1F03E1F},
				/* DMI_SDRAM_480G */
				{0x000004A4, 0x08000000, 0xFFFFFFFF},
				{0x000004A8, 0x00000000, 0x000000FF},
				{0x000004A0, 0x81B00006, 0xC1F03E1F}
			};

ncore_ccu_t ccu_dce0[] = {
				/* DCEUAMIGR0 */
				{0x000003C0, 0x00000003, 0x0000001F},
				/* DCEUMIFSR0 */
				{0x000003C4, 0x00000000, 0x07070777},
				/* DMI_SDRAM_2G */
				{0x00000464, 0x00080000, 0xFFFFFFFF},
				{0x00000468, 0x00000000, 0x000000FF},
				{0x00000460, 0x81300006, 0xC1F03E1F},
				/* DMI_SDRAM_30G */
				{0x00000484, 0x00800000, 0xFFFFFFFF},
				{0x00000488, 0x00000000, 0x000000FF},
				{0x00000480, 0x81700006, 0xC1F03E1F},
				/* DMI_SDRAM_480G */
				{0x000004A4, 0x08000000, 0xFFFFFFFF},
				{0x000004A8, 0x00000000, 0x000000FF},
				{0x000004A0, 0x81B00006, 0xC1F03E1F}
			};

ncore_ccu_t ccu_dce1[] = {
				/* DCEUAMIGR1 */
				{0x000003C0, 0x00000003, 0x0000001F},
				/* DCEUMIFSR1 */
				{0x000003C4, 0x00000000, 0x07070777},
				/* DMI_SDRAM_2G */
				{0x00000464, 0x00080000, 0xFFFFFFFF},
				{0x00000468, 0x00000000, 0x000000FF},
				{0x00000460, 0x81300006, 0xC1F03E1F},
				/* DMI_SDRAM_30G */
				{0x00000484, 0x00800000, 0xFFFFFFFF},
				{0x00000488, 0x00000000, 0x000000FF},
				{0x00000480, 0x81700006, 0xC1F03E1F},
				/* DMI_SDRAM_480G */
				{0x000004A4, 0x08000000, 0xFFFFFFFF},
				{0x000004A8, 0x00000000, 0x000000FF},
				{0x000004A0, 0x81B00006, 0xC1F03E1F}
			};

ncore_ccu_t ccu_dmi0[] = {
				/* DMIUSMCTCR */
				{0x00000300, 0x00000001, 0x00000003},
				{0x00000300, 0x00000003, 0x00000003}
			};

ncore_ccu_t ccu_dmi1[] = {
				/* DMIUSMCTCR */
				{0x00000300, 0x00000001, 0x00000003},
				{0x00000300, 0x00000003, 0x00000003}
			};

ncore_ccu_t ccu_noc_fw_l4_per[] = {
				/* NAND */
				{0x00000000, 0x01010001, 0x01010001},
				/* USB0 */
				{0x0000000C, 0x01010001, 0x01010001},
				/* USB1 */
				{0x00000010, 0x01010001, 0x01010001},
				/* SPI_MAIN0 */
				{0x0000001C, 0x01010301, 0x01010301},
				/* SPI_MAIN1 */
				{0x00000020, 0x01010301, 0x01010301},
				/* SPI_SECONDARY0 */
				{0x00000024, 0x01010301, 0x01010301},
				/* SPI_SECONDARY1 */
				{0x00000028, 0x01010301, 0x01010301},
				/* EMAC0 */
				{0x0000002C, 0x01010001, 0x01010001},
				/* EMAC1 */
				{0x00000030, 0x01010001, 0x01010001},
				/* EMAC2 */
				{0x00000034, 0x01010001, 0x01010001},
				/* SDMMC */
				{0x00000040, 0x01010001, 0x01010001},
				/* GPIO0 */
				{0x00000044, 0x01010301, 0x01010301},
				/* GPIO1 */
				{0x00000048, 0x01010301, 0x01010301},
				/* I2C0 */
				{0x00000050, 0x01010301, 0x01010301},
				/* I2C1 */
				{0x00000054, 0x01010301, 0x01010301},
				/* I2C2 */
				{0x00000058, 0x01010301, 0x01010301},
				/* I2C3 */
				{0x0000005C, 0x01010301, 0x01010301},
				/* I2C4 */
				{0x00000060, 0x01010301, 0x01010301},
				/* SP_TIMER0 */
				{0x00000064, 0x01010301, 0x01010301},
				/* SP_TIMER1 */
				{0x00000068, 0x01010301, 0x01010301},
				/* UART0 */
				{0x0000006C, 0x01010301, 0x01010301},
				/* UART1 */
				{0x00000070, 0x01010301, 0x01010301},
				/* I3C0 */
				{0x00000074, 0x01010301, 0x01010301},
				/* I3C1 */
				{0x00000078, 0x01010301, 0x01010301},
				/* DMA0 */
				{0x0000007C, 0x01010001, 0x01010001},
				/* DMA1 */
				{0x00000080, 0x01010001, 0x01010001},
				/* COMBO_PHY */
				{0x00000084, 0x01010001, 0x01010001},
				/* NAND_SDMA */
				{0x00000088, 0x01010301, 0x01010301}
			};

ncore_ccu_t ccu_noc_fw_l4_sys[] = {
				/* DMA_ECC */
				{0x00000008, 0x01010001, 0x01010001},
				/* EMAC0RX_ECC */
				{0x0000000C, 0x01010001, 0x01010001},
				/* EMAC0TX_ECC */
				{0x00000010, 0x01010001, 0x01010001},
				/* EMAC1RX_ECC */
				{0x00000014, 0x01010001, 0x01010001},
				/* EMAC1TX_ECC */
				{0x00000018, 0x01010001, 0x01010001},
				/* EMAC2RX_ECC */
				{0x0000001C, 0x01010001, 0x01010001},
				/* EMAC2TX_ECC */
				{0x00000020, 0x01010001, 0x01010001},
				/* NAND_ECC */
				{0x0000002C, 0x01010001, 0x01010001},
				/* NAND_READ_ECC */
				{0x00000030, 0x01010001, 0x01010001},
				/* NAND_WRITE_ECC */
				{0x00000034, 0x01010001, 0x01010001},
				/* OCRAM_ECC */
				{0x00000038, 0x01010001, 0x01010001},
				/* SDMMC_ECC */
				{0x00000040, 0x01010001, 0x01010001},
				/* USB0_ECC */
				{0x00000044, 0x01010001, 0x01010001},
				/* USB1_CACHEECC */
				{0x00000048, 0x01010001, 0x01010001},
				/* CLOCK_MANAGER */
				{0x0000004C, 0x01010001, 0x01010001},
				/* IO_MANAGER */
				{0x00000054, 0x01010001, 0x01010001},
				/* RESET_MANAGER */
				{0x00000058, 0x01010001, 0x01010001},
				/* SYSTEM_MANAGER */
				{0x0000005C, 0x01010001, 0x01010001},
				/* OSC0_TIMER */
				{0x00000060, 0x01010301, 0x01010301},
				/* OSC1_TIMER0*/
				{0x00000064, 0x01010301, 0x01010301},
				/* WATCHDOG0 */
				{0x00000068, 0x01010301, 0x01010301},
				/* WATCHDOG1 */
				{0x0000006C, 0x01010301, 0x01010301},
				/* WATCHDOG2 */
				{0x00000070, 0x01010301, 0x01010301},
				/* WATCHDOG3 */
				{0x00000074, 0x01010301, 0x01010301},
				/* DAP */
				{0x00000078, 0x03010001, 0x03010001},
				/* WATCHDOG4 */
				{0x0000007C, 0x01010301, 0x01010301},
				/* POWER_MANAGER */
				{0x00000080, 0x01010001, 0x01010001},
				/* USB1_RXECC */
				{0x00000084, 0x01010001, 0x01010001},
				/* USB1_TXECC */
				{0x00000088, 0x01010001, 0x01010001},
				/* L4_NOC_PROBES */
				{0x00000090, 0x01010001, 0x01010001},
				/* L4_NOC_QOS */
				{0x00000094, 0x01010001, 0x01010001}
			};

ncore_ccu_t ccu_noc_fw_lwsoc2fpga[] = {
				/* LWSOC2FPGA_CSR */
				{0x00000000, 0x0FFE0301, 0x0FFE0301}
			};

ncore_ccu_t ccu_noc_fw_soc2fpga[] = {
				/* SOC2FPGA_CSR */
				{0x00000000, 0x0FFE0301, 0x0FFE0301}
			};

ncore_ccu_t ccu_noc_fw_tcu[] = {
				/* TCU_CSR */
				{0x00000000, 0x01010001, 0x01010001}
			};

uint32_t init_ncore_ccu(void)
{
	ncore_ccu_t *ccu_module_table = NULL;
	uint32_t base;
	uint32_t size;
	uint32_t val;
	uint32_t offset;
	uint32_t mask;
	uint32_t set_mask = 0U;
	uint32_t reg = 0U;

	for (int index = 0; index < ARRAY_SIZE(ncore_ccu_modules); index++) {
		base = ncore_ccu_modules[index].base;
		size = ncore_ccu_modules[index].size;

		switch (index) {
		case 0:
			ccu_module_table = ccu_caiu0;
			size = (sizeof(ccu_caiu0) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 1:
			ccu_module_table = ccu_ncaiu0;
			size = (sizeof(ccu_ncaiu0) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 2:
			ccu_module_table = ccu_ncaiu1;
			size = (sizeof(ccu_ncaiu1) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 3:
			ccu_module_table = ccu_ncaiu2;
			size = (sizeof(ccu_ncaiu2) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 4:
			ccu_module_table = ccu_ncaiu3;
			size = (sizeof(ccu_ncaiu3) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 5:
			ccu_module_table = ccu_dce0;
			size = (sizeof(ccu_dce0) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 6:
			ccu_module_table = ccu_dce1;
			size = (sizeof(ccu_dce1) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 7:
			ccu_module_table = ccu_dmi0;
			size = (sizeof(ccu_dmi0) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 8:
			ccu_module_table = ccu_dmi1;
			size = (sizeof(ccu_dmi1) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 9:
			ccu_module_table = ccu_noc_fw_l4_per;
			size = (sizeof(ccu_noc_fw_l4_per) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 10:
			ccu_module_table = ccu_noc_fw_l4_sys;
			size = (sizeof(ccu_noc_fw_l4_sys) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 11:
			ccu_module_table = ccu_noc_fw_lwsoc2fpga;
			size = (sizeof(ccu_noc_fw_lwsoc2fpga) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 12:
			ccu_module_table = ccu_noc_fw_soc2fpga;
			size = (sizeof(ccu_noc_fw_soc2fpga) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		case 13:
			ccu_module_table = ccu_noc_fw_tcu;
			size = (sizeof(ccu_noc_fw_tcu) / CCU_WORD_BYTE) / CCU_OFFSET_VAL_MASK;
			break;

		default:
			break;
		}

		VERBOSE("CCU node base addr 0x%x, name %s, size 0x%x and module table %p\n",
			base, ncore_ccu_modules[index].name, size, (uint32_t *)ccu_module_table);

		/*
		 * First element: offset
		 * Second element: val
		 * Third element: mask
		 */
		for (int i = 0; i < size; i++) {
			offset = ccu_module_table[i].offset;
			val = ccu_module_table[i].val;

			/* Reads the masking bit value from the list */
			mask = ccu_module_table[i].mask;

			if (mask != 0) {
				if (mask == 0xFFFFFFFF) {
					reg = base + offset;
					mmio_write_32((uintptr_t)reg, val);
				} else {
					/* Mask the value with the masking bits */
					set_mask = val & mask;
					reg = base + offset;

					/* Clears and sets specific bits in the register */
					mmio_clrsetbits_32((uintptr_t)reg, mask, set_mask);
				}
			}

		}

	}

	return 0;
}
#endif

static coh_ss_id_t subsystem_id;
void get_subsystem_id(void)
{
	uint32_t snoop_filter, directory, coh_agent;
	snoop_filter = CSIDR_NUM_SF(mmio_read_32(NCORE_CCU_CSR(NCORE_CSIDR)));
	directory = CSUIDR_NUM_DIR(mmio_read_32(NCORE_CCU_CSR(NCORE_CSUIDR)));
	coh_agent = CSUIDR_NUM_CAI(mmio_read_32(NCORE_CCU_CSR(NCORE_CSUIDR)));
	subsystem_id.num_snoop_filter = snoop_filter + 1;
	subsystem_id.num_directory = directory;
	subsystem_id.num_coh_agent = coh_agent;
}

uint32_t directory_init(void)
{
	uint32_t dir_sf_mtn, dir_sf_en;
	uint32_t dir, sf, ret;
	for (dir = 0; dir < subsystem_id.num_directory; dir++) {
		for (sf = 0; sf < subsystem_id.num_snoop_filter; sf++) {
			dir_sf_mtn = DIRECTORY_UNIT(dir, NCORE_DIRUSFMCR);
			dir_sf_en = DIRECTORY_UNIT(dir, NCORE_DIRUSFER);
			/* Initialize All Entries */
			mmio_write_32(dir_sf_mtn, SNOOP_FILTER_ID(sf));
			/* Poll Active Bit */
			ret = poll_active_bit(dir);
			if (ret != 0) {
				ERROR("Timeout during active bit polling\n");
				return -ETIMEDOUT;
			}
			/* Disable snoop filter, a bit per snoop filter */
			mmio_clrbits_32(dir_sf_en, BIT(sf));
		}
	}
	return 0;
}

uint32_t coherent_agent_intfc_init(void)
{
	uint32_t dir, ca, ca_id, ca_type, ca_snoop_en;
	for (dir = 0; dir < subsystem_id.num_directory; dir++) {
		for (ca = 0; ca < subsystem_id.num_coh_agent; ca++) {
			ca_snoop_en = DIRECTORY_UNIT(ca, NCORE_DIRUCASER0);
			ca_id = mmio_read_32(COH_AGENT_UNIT(ca, NCORE_CAIUIDR));
			/* Coh Agent Snoop Enable */
			if (CACHING_AGENT_BIT(ca_id))
				mmio_setbits_32(ca_snoop_en, BIT(ca));
			/* Coh Agent Snoop DVM Enable */
			ca_type = CACHING_AGENT_TYPE(ca_id);
			if (ca_type == ACE_W_DVM || ca_type == ACE_L_W_DVM)
				mmio_setbits_32(NCORE_CCU_CSR(NCORE_CSADSER0),
						BIT(ca));
		}
	}
	return 0;
}

uint32_t poll_active_bit(uint32_t dir)
{
	uint32_t timeout = 80000;
	uint32_t poll_dir =  DIRECTORY_UNIT(dir, NCORE_DIRUSFMAR);
	while (timeout > 0) {
		if (mmio_read_32(poll_dir) == 0)
			return 0;
		timeout--;
	}
	return -1;
}

void bypass_ocram_firewall(void)
{
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF1),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF2),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF3),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF4),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
}

void ncore_enable_ocram_firewall(void)
{
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF1),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF2),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF3),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF4),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
}

#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
uint32_t init_ncore_ccu(void)
{
	uint32_t status;
	get_subsystem_id();
	status = directory_init();
	status = coherent_agent_intfc_init();
	bypass_ocram_firewall();
	return status;
}
#endif

void setup_smmu_stream_id(void)
{
	/* Configure Stream ID for Agilex5 */
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_ID_AX_REG_0_DMA0), DMA0);
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_ID_AX_REG_0_DMA1), DMA1);
	mmio_write_32(SOCFPGA_SYSMGR(SDM_TBU_STREAM_ID_AX_REG_1_SDM), SDM);
	/* Reg map showing USB2 but Linux USB0? */
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_USB2), USB0);
	/* Reg map showing USB3 but Linux USB1? */
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_USB3), USB1);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_SDMMC), SDMMC);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_NAND), NAND);
	/* To confirm ETR - core sight debug*/
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_ETR), CORE_SIGHT_DEBUG);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_TSN0), TSN0);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_TSN1), TSN1);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_TSN2), TSN2);
	/* Enabled Stream ctrl register for Agilex5 */
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_CTRL_REG_0_DMA0), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_CTRL_REG_0_DMA1), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(SDM_TBU_STREAM_CTRL_REG_1_SDM), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_USB2), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_USB3), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_SDMMC), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_NAND), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_ETR), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN0), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN1), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN2), ENABLE_STREAMID);
}

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
/* TODO: Temp added this here*/
static int poll_idle_status(uint32_t addr, uint32_t mask, uint32_t match, uint32_t delay_ms)
{
	int time_out = delay_ms;

	while (time_out-- > 0) {

		if ((mmio_read_32(addr) & mask) == match) {
			return 0;
		}
		udelay(1000);
	}

	return -ETIMEDOUT;
}

int flush_l3_dcache(void)
{
	int i;
	int ret = 0;

	/* Flushing all entries in CCU system memory cache */
	for (i = 0; i < MAX_DISTRIBUTED_MEM_INTERFACE; i++) {
		mmio_write_32(FIELD_PREP(CCU_DMI0_DMIUSMCMCR_MNTOP, FLUSH_ALL_ENTRIES) |
			   FIELD_PREP(CCU_DMI0_DMIUSMCMCR_ARRAY_ID, ARRAY_ID_TAG),
			   (uintptr_t)(CCU_DMI0_DMIUSMCMCR + (i * 0x1000)));

		/* Wait for cache maintenance operation done */
		ret = poll_idle_status((CCU_DMI0_DMIUSMCMAR +
				(i * 0x1000)), CACHE_OPERATION_DONE,
				CACHE_OPERATION_DONE, TIMEOUT_200MS);

		if (ret != 0) {
			VERBOSE("%s: Timeout while waiting for flushing tag in DMI%d done\n",
					__func__, i);
			return ret;
		}

		mmio_write_32(FIELD_PREP(CCU_DMI0_DMIUSMCMCR_MNTOP, FLUSH_ALL_ENTRIES) |
			   FIELD_PREP(CCU_DMI0_DMIUSMCMCR_ARRAY_ID, ARRAY_ID_DATA),
			   (uintptr_t)(CCU_DMI0_DMIUSMCMCR + (i * 0x1000)));

		/* Wait for cache maintenance operation done */
		ret = poll_idle_status((CCU_DMI0_DMIUSMCMAR +
				(i * 0x1000)), CACHE_OPERATION_DONE,
				CACHE_OPERATION_DONE, TIMEOUT_200MS);

		if (ret != 0) {
			VERBOSE("%s: Timeout while waiting for flushing data in DMI%d done\n",
					__func__, i);
		}
	}

	return ret;
}
#endif
