/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDRPHY_PHYINIT_USERCUSTOM_H
#define DDRPHY_PHYINIT_USERCUSTOM_H

#include <stdbool.h>
#include <stdint.h>

#include <ddrphy_csr_all_cdefines.h>

#include <drivers/st/stm32mp2_ddr.h>

/* Message Block Structure Definitions */
#if STM32MP_DDR3_TYPE
#include <mnpmusrammsgblock_ddr3.h>
#elif STM32MP_DDR4_TYPE
#include <mnpmusrammsgblock_ddr4.h>
#else /* STM32MP_LPDDR4_TYPE */
#include <mnpmusrammsgblock_lpddr4.h>
#endif /* STM32MP_DDR3_TYPE */

/*
 * -------------------------------------------------------------
 * Defines for Firmware Images
 * - indicate IMEM/DMEM address, size (bytes) and offsets.
 * -------------------------------------------------------------
 *
 * IMEM_SIZE max size of instruction memory.
 * DMEM_SIZE max size of data memory.
 *
 * IMEM_ST_ADDR start of IMEM address in memory.
 * DMEM_ST_ADDR start of DMEM address in memory.
 * DMEM_BIN_OFFSET start offset in DMEM memory (message block).
 */
#if STM32MP_DDR3_TYPE
#define IMEM_SIZE			0x4C28U
#define DMEM_SIZE			0x6C8U
#elif STM32MP_DDR4_TYPE
#define IMEM_SIZE			0x6D24U
#define DMEM_SIZE			0x6CCU
#else /* STM32MP_LPDDR4_TYPE */
#define IMEM_SIZE			0x7E50U
#define DMEM_SIZE			0x67CU
#endif /* STM32MP_DDR3_TYPE */
#define IMEM_ST_ADDR			0x50000U
#define DMEM_ST_ADDR			0x54000U
#define DMEM_BIN_OFFSET			0x200U

/*
 * ------------------
 * Type definitions
 * ------------------
 */

/* A structure used to SRAM memory address space */
enum return_offset_lastaddr {
	RETURN_OFFSET,
	RETURN_LASTADDR
};

/* Enumeration of instructions for PhyInit Register Interface */
enum reginstr {
	STARTTRACK,	/* Start register tracking */
	STOPTRACK,	/* Stop register tracking */
	SAVEREGS,	/* Save(read) tracked register values */
	RESTOREREGS,	/* Restore (write) saved register values */
};

/* Data structure to store register address/value pairs */
struct reg_addr_val {
	uint32_t	address;	/* Register address */
	uint16_t	value;		/* Register value */
};

/* Target CSR for the impedance value for ddrphy_phyinit_mapdrvstren() */
enum drvtype {
	DRVSTRENFSDQP,
	DRVSTRENFSDQN,
	ODTSTRENP,
	ODTSTRENN,
	ADRVSTRENP,
	ADRVSTRENN
};

/*
 * -------------------------------------------------------------
 * Fixed Function prototypes
 * -------------------------------------------------------------
 */
int ddrphy_phyinit_sequence(struct stm32mp_ddr_config *config, bool skip_training, bool reten);
int ddrphy_phyinit_restore_sequence(void);
int ddrphy_phyinit_c_initphyconfig(struct stm32mp_ddr_config *config,
				   struct pmu_smb_ddr_1d *mb_ddr_1d, uint32_t *ardptrinitval);
void ddrphy_phyinit_d_loadimem(void);
void ddrphy_phyinit_progcsrskiptrain(struct stm32mp_ddr_config *config,
				     struct pmu_smb_ddr_1d *mb_ddr_1d, uint32_t ardptrinitval);
int ddrphy_phyinit_f_loaddmem(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d);
int ddrphy_phyinit_g_execfw(void);
void ddrphy_phyinit_i_loadpieimage(struct stm32mp_ddr_config *config, bool skip_training);
void ddrphy_phyinit_loadpieprodcode(void);
int ddrphy_phyinit_mapdrvstren(uint32_t drvstren_ohm, enum drvtype targetcsr);
int ddrphy_phyinit_calcmb(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d);
void ddrphy_phyinit_writeoutmem(uint32_t *mem, uint32_t mem_offset, uint32_t mem_size);
void ddrphy_phyinit_writeoutmsgblk(uint16_t *mem, uint32_t mem_offset, uint32_t mem_size);
int ddrphy_phyinit_isdbytedisabled(struct stm32mp_ddr_config *config,
				   struct pmu_smb_ddr_1d *mb_ddr_1d, uint32_t dbytenumber);
int ddrphy_phyinit_trackreg(uint32_t adr);
int ddrphy_phyinit_reginterface(enum reginstr myreginstr, uint32_t adr, uint16_t dat);

void ddrphy_phyinit_usercustom_custompretrain(struct stm32mp_ddr_config *config);
int ddrphy_phyinit_usercustom_g_waitfwdone(void);
int ddrphy_phyinit_usercustom_saveretregs(struct stm32mp_ddr_config *config);

#endif /* DDRPHY_PHYINIT_USERCUSTOM_H */
