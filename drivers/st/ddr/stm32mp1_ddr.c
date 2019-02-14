/*
 * Copyright (C) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp_pmic.h>
#include <drivers/st/stm32mp1_ddr.h>
#include <drivers/st/stm32mp1_ddr_regs.h>
#include <drivers/st/stm32mp1_pwr.h>
#include <drivers/st/stm32mp1_ram.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

struct reg_desc {
	const char *name;
	uint16_t offset;	/* Offset for base address */
	uint8_t par_offset;	/* Offset for parameter array */
};

#define INVALID_OFFSET	0xFFU

#define TIMEOUT_US_1S	1000000U

#define DDRCTL_REG(x, y)					\
	{							\
		.name = #x,					\
		.offset = offsetof(struct stm32mp1_ddrctl, x),	\
		.par_offset = offsetof(struct y, x)		\
	}

#define DDRPHY_REG(x, y)					\
	{							\
		.name = #x,					\
		.offset = offsetof(struct stm32mp1_ddrphy, x),	\
		.par_offset = offsetof(struct y, x)		\
	}

#define DDRCTL_REG_REG(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_reg)
static const struct reg_desc ddr_reg[] = {
	DDRCTL_REG_REG(mstr),
	DDRCTL_REG_REG(mrctrl0),
	DDRCTL_REG_REG(mrctrl1),
	DDRCTL_REG_REG(derateen),
	DDRCTL_REG_REG(derateint),
	DDRCTL_REG_REG(pwrctl),
	DDRCTL_REG_REG(pwrtmg),
	DDRCTL_REG_REG(hwlpctl),
	DDRCTL_REG_REG(rfshctl0),
	DDRCTL_REG_REG(rfshctl3),
	DDRCTL_REG_REG(crcparctl0),
	DDRCTL_REG_REG(zqctl0),
	DDRCTL_REG_REG(dfitmg0),
	DDRCTL_REG_REG(dfitmg1),
	DDRCTL_REG_REG(dfilpcfg0),
	DDRCTL_REG_REG(dfiupd0),
	DDRCTL_REG_REG(dfiupd1),
	DDRCTL_REG_REG(dfiupd2),
	DDRCTL_REG_REG(dfiphymstr),
	DDRCTL_REG_REG(odtmap),
	DDRCTL_REG_REG(dbg0),
	DDRCTL_REG_REG(dbg1),
	DDRCTL_REG_REG(dbgcmd),
	DDRCTL_REG_REG(poisoncfg),
	DDRCTL_REG_REG(pccfg),
};

#define DDRCTL_REG_TIMING(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_timing)
static const struct reg_desc ddr_timing[] = {
	DDRCTL_REG_TIMING(rfshtmg),
	DDRCTL_REG_TIMING(dramtmg0),
	DDRCTL_REG_TIMING(dramtmg1),
	DDRCTL_REG_TIMING(dramtmg2),
	DDRCTL_REG_TIMING(dramtmg3),
	DDRCTL_REG_TIMING(dramtmg4),
	DDRCTL_REG_TIMING(dramtmg5),
	DDRCTL_REG_TIMING(dramtmg6),
	DDRCTL_REG_TIMING(dramtmg7),
	DDRCTL_REG_TIMING(dramtmg8),
	DDRCTL_REG_TIMING(dramtmg14),
	DDRCTL_REG_TIMING(odtcfg),
};

#define DDRCTL_REG_MAP(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_map)
static const struct reg_desc ddr_map[] = {
	DDRCTL_REG_MAP(addrmap1),
	DDRCTL_REG_MAP(addrmap2),
	DDRCTL_REG_MAP(addrmap3),
	DDRCTL_REG_MAP(addrmap4),
	DDRCTL_REG_MAP(addrmap5),
	DDRCTL_REG_MAP(addrmap6),
	DDRCTL_REG_MAP(addrmap9),
	DDRCTL_REG_MAP(addrmap10),
	DDRCTL_REG_MAP(addrmap11),
};

#define DDRCTL_REG_PERF(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_perf)
static const struct reg_desc ddr_perf[] = {
	DDRCTL_REG_PERF(sched),
	DDRCTL_REG_PERF(sched1),
	DDRCTL_REG_PERF(perfhpr1),
	DDRCTL_REG_PERF(perflpr1),
	DDRCTL_REG_PERF(perfwr1),
	DDRCTL_REG_PERF(pcfgr_0),
	DDRCTL_REG_PERF(pcfgw_0),
	DDRCTL_REG_PERF(pcfgqos0_0),
	DDRCTL_REG_PERF(pcfgqos1_0),
	DDRCTL_REG_PERF(pcfgwqos0_0),
	DDRCTL_REG_PERF(pcfgwqos1_0),
	DDRCTL_REG_PERF(pcfgr_1),
	DDRCTL_REG_PERF(pcfgw_1),
	DDRCTL_REG_PERF(pcfgqos0_1),
	DDRCTL_REG_PERF(pcfgqos1_1),
	DDRCTL_REG_PERF(pcfgwqos0_1),
	DDRCTL_REG_PERF(pcfgwqos1_1),
};

#define DDRPHY_REG_REG(x)	DDRPHY_REG(x, stm32mp1_ddrphy_reg)
static const struct reg_desc ddrphy_reg[] = {
	DDRPHY_REG_REG(pgcr),
	DDRPHY_REG_REG(aciocr),
	DDRPHY_REG_REG(dxccr),
	DDRPHY_REG_REG(dsgcr),
	DDRPHY_REG_REG(dcr),
	DDRPHY_REG_REG(odtcr),
	DDRPHY_REG_REG(zq0cr1),
	DDRPHY_REG_REG(dx0gcr),
	DDRPHY_REG_REG(dx1gcr),
	DDRPHY_REG_REG(dx2gcr),
	DDRPHY_REG_REG(dx3gcr),
};

#define DDRPHY_REG_TIMING(x)	DDRPHY_REG(x, stm32mp1_ddrphy_timing)
static const struct reg_desc ddrphy_timing[] = {
	DDRPHY_REG_TIMING(ptr0),
	DDRPHY_REG_TIMING(ptr1),
	DDRPHY_REG_TIMING(ptr2),
	DDRPHY_REG_TIMING(dtpr0),
	DDRPHY_REG_TIMING(dtpr1),
	DDRPHY_REG_TIMING(dtpr2),
	DDRPHY_REG_TIMING(mr0),
	DDRPHY_REG_TIMING(mr1),
	DDRPHY_REG_TIMING(mr2),
	DDRPHY_REG_TIMING(mr3),
};

#define DDRPHY_REG_CAL(x)	DDRPHY_REG(x, stm32mp1_ddrphy_cal)
static const struct reg_desc ddrphy_cal[] = {
	DDRPHY_REG_CAL(dx0dllcr),
	DDRPHY_REG_CAL(dx0dqtr),
	DDRPHY_REG_CAL(dx0dqstr),
	DDRPHY_REG_CAL(dx1dllcr),
	DDRPHY_REG_CAL(dx1dqtr),
	DDRPHY_REG_CAL(dx1dqstr),
	DDRPHY_REG_CAL(dx2dllcr),
	DDRPHY_REG_CAL(dx2dqtr),
	DDRPHY_REG_CAL(dx2dqstr),
	DDRPHY_REG_CAL(dx3dllcr),
	DDRPHY_REG_CAL(dx3dqtr),
	DDRPHY_REG_CAL(dx3dqstr),
};

#define DDR_REG_DYN(x)						\
	{							\
		.name = #x,					\
		.offset = offsetof(struct stm32mp1_ddrctl, x),	\
		.par_offset = INVALID_OFFSET \
	}

static const struct reg_desc ddr_dyn[] = {
	DDR_REG_DYN(stat),
	DDR_REG_DYN(init0),
	DDR_REG_DYN(dfimisc),
	DDR_REG_DYN(dfistat),
	DDR_REG_DYN(swctl),
	DDR_REG_DYN(swstat),
	DDR_REG_DYN(pctrl_0),
	DDR_REG_DYN(pctrl_1),
};

#define DDRPHY_REG_DYN(x)					\
	{							\
		.name = #x,					\
		.offset = offsetof(struct stm32mp1_ddrphy, x),	\
		.par_offset = INVALID_OFFSET			\
	}

static const struct reg_desc ddrphy_dyn[] = {
	DDRPHY_REG_DYN(pir),
	DDRPHY_REG_DYN(pgsr),
};

enum reg_type {
	REG_REG,
	REG_TIMING,
	REG_PERF,
	REG_MAP,
	REGPHY_REG,
	REGPHY_TIMING,
	REGPHY_CAL,
/*
 * Dynamic registers => managed in driver or not changed,
 * can be dumped in interactive mode.
 */
	REG_DYN,
	REGPHY_DYN,
	REG_TYPE_NB
};

enum base_type {
	DDR_BASE,
	DDRPHY_BASE,
	NONE_BASE
};

struct ddr_reg_info {
	const char *name;
	const struct reg_desc *desc;
	uint8_t size;
	enum base_type base;
};

static const struct ddr_reg_info ddr_registers[REG_TYPE_NB] = {
	[REG_REG] = {
		.name = "static",
		.desc = ddr_reg,
		.size = ARRAY_SIZE(ddr_reg),
		.base = DDR_BASE
	},
	[REG_TIMING] = {
		.name = "timing",
		.desc = ddr_timing,
		.size = ARRAY_SIZE(ddr_timing),
		.base = DDR_BASE
	},
	[REG_PERF] = {
		.name = "perf",
		.desc = ddr_perf,
		.size = ARRAY_SIZE(ddr_perf),
		.base = DDR_BASE
	},
	[REG_MAP] = {
		.name = "map",
		.desc = ddr_map,
		.size = ARRAY_SIZE(ddr_map),
		.base = DDR_BASE
	},
	[REGPHY_REG] = {
		.name = "static",
		.desc = ddrphy_reg,
		.size = ARRAY_SIZE(ddrphy_reg),
		.base = DDRPHY_BASE
	},
	[REGPHY_TIMING] = {
		.name = "timing",
		.desc = ddrphy_timing,
		.size = ARRAY_SIZE(ddrphy_timing),
		.base = DDRPHY_BASE
	},
	[REGPHY_CAL] = {
		.name = "cal",
		.desc = ddrphy_cal,
		.size = ARRAY_SIZE(ddrphy_cal),
		.base = DDRPHY_BASE
	},
	[REG_DYN] = {
		.name = "dyn",
		.desc = ddr_dyn,
		.size = ARRAY_SIZE(ddr_dyn),
		.base = DDR_BASE
	},
	[REGPHY_DYN] = {
		.name = "dyn",
		.desc = ddrphy_dyn,
		.size = ARRAY_SIZE(ddrphy_dyn),
		.base = DDRPHY_BASE
	},
};

static uintptr_t get_base_addr(const struct ddr_info *priv, enum base_type base)
{
	if (base == DDRPHY_BASE) {
		return (uintptr_t)priv->phy;
	} else {
		return (uintptr_t)priv->ctl;
	}
}

static void set_reg(const struct ddr_info *priv,
		    enum reg_type type,
		    const void *param)
{
	unsigned int i;
	unsigned int value;
	enum base_type base = ddr_registers[type].base;
	uintptr_t base_addr = get_base_addr(priv, base);
	const struct reg_desc *desc = ddr_registers[type].desc;

	VERBOSE("init %s\n", ddr_registers[type].name);
	for (i = 0; i < ddr_registers[type].size; i++) {
		uintptr_t ptr = base_addr + desc[i].offset;

		if (desc[i].par_offset == INVALID_OFFSET) {
			ERROR("invalid parameter offset for %s", desc[i].name);
			panic();
		} else {
			value = *((uint32_t *)((uintptr_t)param +
					       desc[i].par_offset));
			mmio_write_32(ptr, value);
		}
	}
}

static void stm32mp1_ddrphy_idone_wait(struct stm32mp1_ddrphy *phy)
{
	uint32_t pgsr;
	int error = 0;
	uint64_t timeout = timeout_init_us(TIMEOUT_US_1S);

	do {
		pgsr = mmio_read_32((uintptr_t)&phy->pgsr);

		VERBOSE("  > [0x%lx] pgsr = 0x%x &\n",
			(uintptr_t)&phy->pgsr, pgsr);

		if (timeout_elapsed(timeout)) {
			panic();
		}

		if ((pgsr & DDRPHYC_PGSR_DTERR) != 0U) {
			VERBOSE("DQS Gate Trainig Error\n");
			error++;
		}

		if ((pgsr & DDRPHYC_PGSR_DTIERR) != 0U) {
			VERBOSE("DQS Gate Trainig Intermittent Error\n");
			error++;
		}

		if ((pgsr & DDRPHYC_PGSR_DFTERR) != 0U) {
			VERBOSE("DQS Drift Error\n");
			error++;
		}

		if ((pgsr & DDRPHYC_PGSR_RVERR) != 0U) {
			VERBOSE("Read Valid Training Error\n");
			error++;
		}

		if ((pgsr & DDRPHYC_PGSR_RVEIRR) != 0U) {
			VERBOSE("Read Valid Training Intermittent Error\n");
			error++;
		}
	} while (((pgsr & DDRPHYC_PGSR_IDONE) == 0U) && (error == 0));
	VERBOSE("\n[0x%lx] pgsr = 0x%x\n",
		(uintptr_t)&phy->pgsr, pgsr);
}

static void stm32mp1_ddrphy_init(struct stm32mp1_ddrphy *phy, uint32_t pir)
{
	uint32_t pir_init = pir | DDRPHYC_PIR_INIT;

	mmio_write_32((uintptr_t)&phy->pir, pir_init);
	VERBOSE("[0x%lx] pir = 0x%x -> 0x%x\n",
		(uintptr_t)&phy->pir, pir_init,
		mmio_read_32((uintptr_t)&phy->pir));

	/* Need to wait 10 configuration clock before start polling */
	udelay(10);

	/* Wait DRAM initialization and Gate Training Evaluation complete */
	stm32mp1_ddrphy_idone_wait(phy);
}

/* Start quasi dynamic register update */
static void stm32mp1_start_sw_done(struct stm32mp1_ddrctl *ctl)
{
	mmio_clrbits_32((uintptr_t)&ctl->swctl, DDRCTRL_SWCTL_SW_DONE);
	VERBOSE("[0x%lx] swctl = 0x%x\n",
		(uintptr_t)&ctl->swctl,  mmio_read_32((uintptr_t)&ctl->swctl));
}

/* Wait quasi dynamic register update */
static void stm32mp1_wait_sw_done_ack(struct stm32mp1_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t swstat;

	mmio_setbits_32((uintptr_t)&ctl->swctl, DDRCTRL_SWCTL_SW_DONE);
	VERBOSE("[0x%lx] swctl = 0x%x\n",
		(uintptr_t)&ctl->swctl, mmio_read_32((uintptr_t)&ctl->swctl));

	timeout = timeout_init_us(TIMEOUT_US_1S);
	do {
		swstat = mmio_read_32((uintptr_t)&ctl->swstat);
		VERBOSE("[0x%lx] swstat = 0x%x ",
			(uintptr_t)&ctl->swstat, swstat);
		if (timeout_elapsed(timeout)) {
			panic();
		}
	} while ((swstat & DDRCTRL_SWSTAT_SW_DONE_ACK) == 0U);

	VERBOSE("[0x%lx] swstat = 0x%x\n",
		(uintptr_t)&ctl->swstat, swstat);
}

/* Wait quasi dynamic register update */
static void stm32mp1_wait_operating_mode(struct ddr_info *priv, uint32_t mode)
{
	uint64_t timeout;
	uint32_t stat;
	int break_loop = 0;

	timeout = timeout_init_us(TIMEOUT_US_1S);
	for ( ; ; ) {
		uint32_t operating_mode;
		uint32_t selref_type;

		stat = mmio_read_32((uintptr_t)&priv->ctl->stat);
		operating_mode = stat & DDRCTRL_STAT_OPERATING_MODE_MASK;
		selref_type = stat & DDRCTRL_STAT_SELFREF_TYPE_MASK;
		VERBOSE("[0x%lx] stat = 0x%x\n",
			(uintptr_t)&priv->ctl->stat, stat);
		if (timeout_elapsed(timeout)) {
			panic();
		}

		if (mode == DDRCTRL_STAT_OPERATING_MODE_SR) {
			/*
			 * Self-refresh due to software
			 * => checking also STAT.selfref_type.
			 */
			if ((operating_mode ==
			     DDRCTRL_STAT_OPERATING_MODE_SR) &&
			    (selref_type == DDRCTRL_STAT_SELFREF_TYPE_SR)) {
				break_loop = 1;
			}
		} else if (operating_mode == mode) {
			break_loop = 1;
		} else if ((mode == DDRCTRL_STAT_OPERATING_MODE_NORMAL) &&
			   (operating_mode == DDRCTRL_STAT_OPERATING_MODE_SR) &&
			   (selref_type == DDRCTRL_STAT_SELFREF_TYPE_ASR)) {
			/* Normal mode: handle also automatic self refresh */
			break_loop = 1;
		}

		if (break_loop == 1) {
			break;
		}
	}

	VERBOSE("[0x%lx] stat = 0x%x\n",
		(uintptr_t)&priv->ctl->stat, stat);
}

/* Mode Register Writes (MRW or MRS) */
static void stm32mp1_mode_register_write(struct ddr_info *priv, uint8_t addr,
					 uint32_t data)
{
	uint32_t mrctrl0;

	VERBOSE("MRS: %d = %x\n", addr, data);

	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is '0'.
	 *    This checks that there is no outstanding MR transaction.
	 *    No write should be performed to MRCTRL0 and MRCTRL1
	 *    if MRSTAT.mr_wr_busy = 1.
	 */
	while ((mmio_read_32((uintptr_t)&priv->ctl->mrstat) &
		DDRCTRL_MRSTAT_MR_WR_BUSY) != 0U) {
		;
	}

	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank
	 *    and (for MRWs) MRCTRL1.mr_data to define the MR transaction.
	 */
	mrctrl0 = DDRCTRL_MRCTRL0_MR_TYPE_WRITE |
		  DDRCTRL_MRCTRL0_MR_RANK_ALL |
		  (((uint32_t)addr << DDRCTRL_MRCTRL0_MR_ADDR_SHIFT) &
		   DDRCTRL_MRCTRL0_MR_ADDR_MASK);
	mmio_write_32((uintptr_t)&priv->ctl->mrctrl0, mrctrl0);
	VERBOSE("[0x%lx] mrctrl0 = 0x%x (0x%x)\n",
		(uintptr_t)&priv->ctl->mrctrl0,
		mmio_read_32((uintptr_t)&priv->ctl->mrctrl0), mrctrl0);
	mmio_write_32((uintptr_t)&priv->ctl->mrctrl1, data);
	VERBOSE("[0x%lx] mrctrl1 = 0x%x\n",
		(uintptr_t)&priv->ctl->mrctrl1,
		mmio_read_32((uintptr_t)&priv->ctl->mrctrl1));

	/*
	 * 3. In a separate APB transaction, write the MRCTRL0.mr_wr to 1. This
	 *    bit is self-clearing, and triggers the MR transaction.
	 *    The uMCTL2 then asserts the MRSTAT.mr_wr_busy while it performs
	 *    the MR transaction to SDRAM, and no further access can be
	 *    initiated until it is deasserted.
	 */
	mrctrl0 |= DDRCTRL_MRCTRL0_MR_WR;
	mmio_write_32((uintptr_t)&priv->ctl->mrctrl0, mrctrl0);

	while ((mmio_read_32((uintptr_t)&priv->ctl->mrstat) &
	       DDRCTRL_MRSTAT_MR_WR_BUSY) != 0U) {
		;
	}

	VERBOSE("[0x%lx] mrctrl0 = 0x%x\n",
		(uintptr_t)&priv->ctl->mrctrl0, mrctrl0);
}

/* Switch DDR3 from DLL-on to DLL-off */
static void stm32mp1_ddr3_dll_off(struct ddr_info *priv)
{
	uint32_t mr1 = mmio_read_32((uintptr_t)&priv->phy->mr1);
	uint32_t mr2 = mmio_read_32((uintptr_t)&priv->phy->mr2);
	uint32_t dbgcam;

	VERBOSE("mr1: 0x%x\n", mr1);
	VERBOSE("mr2: 0x%x\n", mr2);

	/*
	 * 1. Set the DBG1.dis_hif = 1.
	 *    This prevents further reads/writes being received on the HIF.
	 */
	mmio_setbits_32((uintptr_t)&priv->ctl->dbg1, DDRCTRL_DBG1_DIS_HIF);
	VERBOSE("[0x%lx] dbg1 = 0x%x\n",
		(uintptr_t)&priv->ctl->dbg1,
		mmio_read_32((uintptr_t)&priv->ctl->dbg1));

	/*
	 * 2. Ensure all commands have been flushed from the uMCTL2 by polling
	 *    DBGCAM.wr_data_pipeline_empty = 1,
	 *    DBGCAM.rd_data_pipeline_empty = 1,
	 *    DBGCAM.dbg_wr_q_depth = 0 ,
	 *    DBGCAM.dbg_lpr_q_depth = 0, and
	 *    DBGCAM.dbg_hpr_q_depth = 0.
	 */
	do {
		dbgcam = mmio_read_32((uintptr_t)&priv->ctl->dbgcam);
		VERBOSE("[0x%lx] dbgcam = 0x%x\n",
			(uintptr_t)&priv->ctl->dbgcam, dbgcam);
	} while ((((dbgcam & DDRCTRL_DBGCAM_DATA_PIPELINE_EMPTY) ==
		   DDRCTRL_DBGCAM_DATA_PIPELINE_EMPTY)) &&
		 ((dbgcam & DDRCTRL_DBGCAM_DBG_Q_DEPTH) == 0U));

	/*
	 * 3. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers)
	 *    to disable RTT_NOM:
	 *    a. DDR3: Write to MR1[9], MR1[6] and MR1[2]
	 *    b. DDR4: Write to MR1[10:8]
	 */
	mr1 &= ~(BIT(9) | BIT(6) | BIT(2));
	stm32mp1_mode_register_write(priv, 1, mr1);

	/*
	 * 4. For DDR4 only: Perform an MRS command
	 *    (using MRCTRL0 and MRCTRL1 registers) to write to MR5[8:6]
	 *    to disable RTT_PARK
	 */

	/*
	 * 5. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers)
	 *    to write to MR2[10:9], to disable RTT_WR
	 *    (and therefore disable dynamic ODT).
	 *    This applies for both DDR3 and DDR4.
	 */
	mr2 &= ~GENMASK(10, 9);
	stm32mp1_mode_register_write(priv, 2, mr2);

	/*
	 * 6. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers)
	 *    to disable the DLL. The timing of this MRS is automatically
	 *    handled by the uMCTL2.
	 *    a. DDR3: Write to MR1[0]
	 *    b. DDR4: Write to MR1[0]
	 */
	mr1 |= BIT(0);
	stm32mp1_mode_register_write(priv, 1, mr1);

	/*
	 * 7. Put the SDRAM into self-refresh mode by setting
	 *    PWRCTL.selfref_sw = 1, and polling STAT.operating_mode to ensure
	 *    the DDRC has entered self-refresh.
	 */
	mmio_setbits_32((uintptr_t)&priv->ctl->pwrctl,
			DDRCTRL_PWRCTL_SELFREF_SW);
	VERBOSE("[0x%lx] pwrctl = 0x%x\n",
		(uintptr_t)&priv->ctl->pwrctl,
		mmio_read_32((uintptr_t)&priv->ctl->pwrctl));

	/*
	 * 8. Wait until STAT.operating_mode[1:0]==11 indicating that the
	 *    DWC_ddr_umctl2 core is in self-refresh mode.
	 *    Ensure transition to self-refresh was due to software
	 *    by checking that STAT.selfref_type[1:0]=2.
	 */
	stm32mp1_wait_operating_mode(priv, DDRCTRL_STAT_OPERATING_MODE_SR);

	/*
	 * 9. Set the MSTR.dll_off_mode = 1.
	 *    warning: MSTR.dll_off_mode is a quasi-dynamic type 2 field
	 */
	stm32mp1_start_sw_done(priv->ctl);

	mmio_setbits_32((uintptr_t)&priv->ctl->mstr, DDRCTRL_MSTR_DLL_OFF_MODE);
	VERBOSE("[0x%lx] mstr = 0x%x\n",
		(uintptr_t)&priv->ctl->mstr,
		mmio_read_32((uintptr_t)&priv->ctl->mstr));

	stm32mp1_wait_sw_done_ack(priv->ctl);

	/* 10. Change the clock frequency to the desired value. */

	/*
	 * 11. Update any registers which may be required to change for the new
	 *     frequency. This includes static and dynamic registers.
	 *     This includes both uMCTL2 registers and PHY registers.
	 */

	/* Change Bypass Mode Frequency Range */
	if (stm32mp_clk_get_rate(DDRPHYC) < 100000000U) {
		mmio_clrbits_32((uintptr_t)&priv->phy->dllgcr,
				DDRPHYC_DLLGCR_BPS200);
	} else {
		mmio_setbits_32((uintptr_t)&priv->phy->dllgcr,
				DDRPHYC_DLLGCR_BPS200);
	}

	mmio_setbits_32((uintptr_t)&priv->phy->acdllcr, DDRPHYC_ACDLLCR_DLLDIS);

	mmio_setbits_32((uintptr_t)&priv->phy->dx0dllcr,
			DDRPHYC_DXNDLLCR_DLLDIS);
	mmio_setbits_32((uintptr_t)&priv->phy->dx1dllcr,
			DDRPHYC_DXNDLLCR_DLLDIS);
	mmio_setbits_32((uintptr_t)&priv->phy->dx2dllcr,
			DDRPHYC_DXNDLLCR_DLLDIS);
	mmio_setbits_32((uintptr_t)&priv->phy->dx3dllcr,
			DDRPHYC_DXNDLLCR_DLLDIS);

	/* 12. Exit the self-refresh state by setting PWRCTL.selfref_sw = 0. */
	mmio_clrbits_32((uintptr_t)&priv->ctl->pwrctl,
			DDRCTRL_PWRCTL_SELFREF_SW);
	stm32mp1_wait_operating_mode(priv, DDRCTRL_STAT_OPERATING_MODE_NORMAL);

	/*
	 * 13. If ZQCTL0.dis_srx_zqcl = 0, the uMCTL2 performs a ZQCL command
	 *     at this point.
	 */

	/*
	 * 14. Perform MRS commands as required to re-program timing registers
	 *     in the SDRAM for the new frequency
	 *     (in particular, CL, CWL and WR may need to be changed).
	 */

	/* 15. Write DBG1.dis_hif = 0 to re-enable reads and writes. */
	mmio_clrbits_32((uintptr_t)&priv->ctl->dbg1, DDRCTRL_DBG1_DIS_HIF);
	VERBOSE("[0x%lx] dbg1 = 0x%x\n",
		(uintptr_t)&priv->ctl->dbg1,
		mmio_read_32((uintptr_t)&priv->ctl->dbg1));
}

static void stm32mp1_refresh_disable(struct stm32mp1_ddrctl *ctl)
{
	stm32mp1_start_sw_done(ctl);
	/* Quasi-dynamic register update*/
	mmio_setbits_32((uintptr_t)&ctl->rfshctl3,
			DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH);
	mmio_clrbits_32((uintptr_t)&ctl->pwrctl, DDRCTRL_PWRCTL_POWERDOWN_EN);
	mmio_clrbits_32((uintptr_t)&ctl->dfimisc,
			DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	stm32mp1_wait_sw_done_ack(ctl);
}

static void stm32mp1_refresh_restore(struct stm32mp1_ddrctl *ctl,
				     uint32_t rfshctl3, uint32_t pwrctl)
{
	stm32mp1_start_sw_done(ctl);
	if ((rfshctl3 & DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH) == 0U) {
		mmio_clrbits_32((uintptr_t)&ctl->rfshctl3,
				DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH);
	}
	if ((pwrctl & DDRCTRL_PWRCTL_POWERDOWN_EN) != 0U) {
		mmio_setbits_32((uintptr_t)&ctl->pwrctl,
				DDRCTRL_PWRCTL_POWERDOWN_EN);
	}
	mmio_setbits_32((uintptr_t)&ctl->dfimisc,
			DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	stm32mp1_wait_sw_done_ack(ctl);
}

static int board_ddr_power_init(enum ddr_type ddr_type)
{
	if (dt_pmic_status() > 0) {
		return pmic_ddr_power_init(ddr_type);
	}

	return 0;
}

void stm32mp1_ddr_init(struct ddr_info *priv,
		       struct stm32mp1_ddr_config *config)
{
	uint32_t pir;
	int ret = -EINVAL;

	if ((config->c_reg.mstr & DDRCTRL_MSTR_DDR3) != 0U) {
		ret = board_ddr_power_init(STM32MP_DDR3);
	} else if ((config->c_reg.mstr & DDRCTRL_MSTR_LPDDR2) != 0U) {
		ret = board_ddr_power_init(STM32MP_LPDDR2);
	} else {
		ERROR("DDR type not supported\n");
	}

	if (ret != 0) {
		panic();
	}

	VERBOSE("name = %s\n", config->info.name);
	VERBOSE("speed = %d kHz\n", config->info.speed);
	VERBOSE("size  = 0x%x\n", config->info.size);

	/* DDR INIT SEQUENCE */

	/*
	 * 1. Program the DWC_ddr_umctl2 registers
	 *     nota: check DFIMISC.dfi_init_complete = 0
	 */

	/* 1.1 RESETS: presetn, core_ddrc_rstn, aresetn */
	mmio_setbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAPBRST);
	mmio_setbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAXIRST);
	mmio_setbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCORERST);
	mmio_setbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYAPBRST);
	mmio_setbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYRST);
	mmio_setbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYCTLRST);

	/* 1.2. start CLOCK */
	if (stm32mp1_ddr_clk_enable(priv, config->info.speed) != 0) {
		panic();
	}

	/* 1.3. deassert reset */
	/* De-assert PHY rstn and ctl_rstn via DPHYRST and DPHYCTLRST. */
	mmio_clrbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYRST);
	mmio_clrbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYCTLRST);
	/*
	 * De-assert presetn once the clocks are active
	 * and stable via DDRCAPBRST bit.
	 */
	mmio_clrbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAPBRST);

	/* 1.4. wait 128 cycles to permit initialization of end logic */
	udelay(2);
	/* For PCLK = 133MHz => 1 us is enough, 2 to allow lower frequency */

	/* 1.5. initialize registers ddr_umctl2 */
	/* Stop uMCTL2 before PHY is ready */
	mmio_clrbits_32((uintptr_t)&priv->ctl->dfimisc,
			DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	VERBOSE("[0x%lx] dfimisc = 0x%x\n",
		(uintptr_t)&priv->ctl->dfimisc,
		mmio_read_32((uintptr_t)&priv->ctl->dfimisc));

	set_reg(priv, REG_REG, &config->c_reg);

	/* DDR3 = don't set DLLOFF for init mode */
	if ((config->c_reg.mstr &
	     (DDRCTRL_MSTR_DDR3 | DDRCTRL_MSTR_DLL_OFF_MODE))
	    == (DDRCTRL_MSTR_DDR3 | DDRCTRL_MSTR_DLL_OFF_MODE)) {
		VERBOSE("deactivate DLL OFF in mstr\n");
		mmio_clrbits_32((uintptr_t)&priv->ctl->mstr,
				DDRCTRL_MSTR_DLL_OFF_MODE);
		VERBOSE("[0x%lx] mstr = 0x%x\n",
			(uintptr_t)&priv->ctl->mstr,
			mmio_read_32((uintptr_t)&priv->ctl->mstr));
	}

	set_reg(priv, REG_TIMING, &config->c_timing);
	set_reg(priv, REG_MAP, &config->c_map);

	/* Skip CTRL init, SDRAM init is done by PHY PUBL */
	mmio_clrsetbits_32((uintptr_t)&priv->ctl->init0,
			   DDRCTRL_INIT0_SKIP_DRAM_INIT_MASK,
			   DDRCTRL_INIT0_SKIP_DRAM_INIT_NORMAL);
	VERBOSE("[0x%lx] init0 = 0x%x\n",
		(uintptr_t)&priv->ctl->init0,
		mmio_read_32((uintptr_t)&priv->ctl->init0));

	set_reg(priv, REG_PERF, &config->c_perf);

	/*  2. deassert reset signal core_ddrc_rstn, aresetn and presetn */
	mmio_clrbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCORERST);
	mmio_clrbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAXIRST);
	mmio_clrbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYAPBRST);

	/*
	 * 3. start PHY init by accessing relevant PUBL registers
	 *    (DXGCR, DCR, PTR*, MR*, DTPR*)
	 */
	set_reg(priv, REGPHY_REG, &config->p_reg);
	set_reg(priv, REGPHY_TIMING, &config->p_timing);
	set_reg(priv, REGPHY_CAL, &config->p_cal);

	/* DDR3 = don't set DLLOFF for init mode */
	if ((config->c_reg.mstr &
	     (DDRCTRL_MSTR_DDR3 | DDRCTRL_MSTR_DLL_OFF_MODE))
	    == (DDRCTRL_MSTR_DDR3 | DDRCTRL_MSTR_DLL_OFF_MODE)) {
		VERBOSE("deactivate DLL OFF in mr1\n");
		mmio_clrbits_32((uintptr_t)&priv->phy->mr1, BIT(0));
		VERBOSE("[0x%lx] mr1 = 0x%x\n",
			(uintptr_t)&priv->phy->mr1,
			mmio_read_32((uintptr_t)&priv->phy->mr1));
	}

	/*
	 *  4. Monitor PHY init status by polling PUBL register PGSR.IDONE
	 *     Perform DDR PHY DRAM initialization and Gate Training Evaluation
	 */
	stm32mp1_ddrphy_idone_wait(priv->phy);

	/*
	 *  5. Indicate to PUBL that controller performs SDRAM initialization
	 *     by setting PIR.INIT and PIR CTLDINIT and pool PGSR.IDONE
	 *     DRAM init is done by PHY, init0.skip_dram.init = 1
	 */

	pir = DDRPHYC_PIR_DLLSRST | DDRPHYC_PIR_DLLLOCK | DDRPHYC_PIR_ZCAL |
	      DDRPHYC_PIR_ITMSRST | DDRPHYC_PIR_DRAMINIT | DDRPHYC_PIR_ICPC;

	if ((config->c_reg.mstr & DDRCTRL_MSTR_DDR3) != 0U) {
		pir |= DDRPHYC_PIR_DRAMRST; /* Only for DDR3 */
	}

	stm32mp1_ddrphy_init(priv->phy, pir);

	/*
	 *  6. SET DFIMISC.dfi_init_complete_en to 1
	 *  Enable quasi-dynamic register programming.
	 */
	stm32mp1_start_sw_done(priv->ctl);

	mmio_setbits_32((uintptr_t)&priv->ctl->dfimisc,
			DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	VERBOSE("[0x%lx] dfimisc = 0x%x\n",
		(uintptr_t)&priv->ctl->dfimisc,
		mmio_read_32((uintptr_t)&priv->ctl->dfimisc));

	stm32mp1_wait_sw_done_ack(priv->ctl);

	/*
	 *  7. Wait for DWC_ddr_umctl2 to move to normal operation mode
	 *     by monitoring STAT.operating_mode signal
	 */

	/* Wait uMCTL2 ready */
	stm32mp1_wait_operating_mode(priv, DDRCTRL_STAT_OPERATING_MODE_NORMAL);

	/* Switch to DLL OFF mode */
	if ((config->c_reg.mstr & DDRCTRL_MSTR_DLL_OFF_MODE) != 0U) {
		stm32mp1_ddr3_dll_off(priv);
	}

	VERBOSE("DDR DQS training : ");

	/*
	 *  8. Disable Auto refresh and power down by setting
	 *    - RFSHCTL3.dis_au_refresh = 1
	 *    - PWRCTL.powerdown_en = 0
	 *    - DFIMISC.dfiinit_complete_en = 0
	 */
	stm32mp1_refresh_disable(priv->ctl);

	/*
	 *  9. Program PUBL PGCR to enable refresh during training
	 *     and rank to train
	 *     not done => keep the programed value in PGCR
	 */

	/*
	 * 10. configure PUBL PIR register to specify which training step
	 * to run
	 * Warning : RVTRN  is not supported by this PUBL
	 */
	stm32mp1_ddrphy_init(priv->phy, DDRPHYC_PIR_QSTRN);

	/* 11. monitor PUB PGSR.IDONE to poll cpmpletion of training sequence */
	stm32mp1_ddrphy_idone_wait(priv->phy);

	/*
	 * 12. set back registers in step 8 to the orginal values if desidered
	 */
	stm32mp1_refresh_restore(priv->ctl, config->c_reg.rfshctl3,
				 config->c_reg.pwrctl);

	/* Enable uMCTL2 AXI port 0 */
	mmio_setbits_32((uintptr_t)&priv->ctl->pctrl_0,
			DDRCTRL_PCTRL_N_PORT_EN);
	VERBOSE("[0x%lx] pctrl_0 = 0x%x\n",
		(uintptr_t)&priv->ctl->pctrl_0,
		mmio_read_32((uintptr_t)&priv->ctl->pctrl_0));

	/* Enable uMCTL2 AXI port 1 */
	mmio_setbits_32((uintptr_t)&priv->ctl->pctrl_1,
			DDRCTRL_PCTRL_N_PORT_EN);
	VERBOSE("[0x%lx] pctrl_1 = 0x%x\n",
		(uintptr_t)&priv->ctl->pctrl_1,
		mmio_read_32((uintptr_t)&priv->ctl->pctrl_1));
}
