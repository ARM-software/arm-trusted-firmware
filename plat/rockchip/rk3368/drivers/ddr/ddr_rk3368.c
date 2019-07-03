/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <ddr_rk3368.h>
#include <pmu.h>
#include <rk3368_def.h>
#include <soc.h>

/* GRF_SOC_STATUS0 */
#define DPLL_LOCK		(0x1 << 2)

/* GRF_DDRC0_CON0 */
#define GRF_DDR_16BIT_EN	(((0x1 << 3) << 16) | (0x1 << 3))
#define GRF_DDR_32BIT_EN	(((0x1 << 3) << 16) | (0x0 << 3))
#define GRF_MOBILE_DDR_EN	(((0x1 << 4) << 16) | (0x1 << 4))
#define GRF_MOBILE_DDR_DISB	(((0x1 << 4) << 16) | (0x0 << 4))
#define GRF_DDR3_EN		(((0x1 << 2) << 16) | (0x1 << 2))
#define GRF_LPDDR2_3_EN		(((0x1 << 2) << 16) | (0x0 << 2))

/* PMUGRF_SOC_CON0 */
#define ddrphy_bufferen_io_en(n)	((0x1 << (9 + 16)) | (n << 9))
#define ddrphy_bufferen_core_en(n)	((0x1 << (8 + 16)) | (n << 8))

struct PCTRL_TIMING_TAG {
	uint32_t ddrfreq;
	uint32_t TOGCNT1U;
	uint32_t TINIT;
	uint32_t TRSTH;
	uint32_t TOGCNT100N;
	uint32_t TREFI;
	uint32_t TMRD;
	uint32_t TRFC;
	uint32_t TRP;
	uint32_t TRTW;
	uint32_t TAL;
	uint32_t TCL;
	uint32_t TCWL;
	uint32_t TRAS;
	uint32_t TRC;
	uint32_t TRCD;
	uint32_t TRRD;
	uint32_t TRTP;
	uint32_t TWR;
	uint32_t TWTR;
	uint32_t TEXSR;
	uint32_t TXP;
	uint32_t TXPDLL;
	uint32_t TZQCS;
	uint32_t TZQCSI;
	uint32_t TDQS;
	uint32_t TCKSRE;
	uint32_t TCKSRX;
	uint32_t TCKE;
	uint32_t TMOD;
	uint32_t TRSTL;
	uint32_t TZQCL;
	uint32_t TMRR;
	uint32_t TCKESR;
	uint32_t TDPD;
	uint32_t TREFI_MEM_DDR3;
};

struct MSCH_SAVE_REG_TAG {
	uint32_t ddrconf;
	uint32_t ddrtiming;
	uint32_t ddrmode;
	uint32_t readlatency;
	uint32_t activate;
	uint32_t devtodev;
};

/* ddr suspend need save reg */
struct PCTL_SAVE_REG_TAG {
	uint32_t SCFG;
	uint32_t CMDTSTATEN;
	uint32_t MCFG1;
	uint32_t MCFG;
	uint32_t PPCFG;
	struct PCTRL_TIMING_TAG pctl_timing;
	/* DFI Control Registers */
	uint32_t DFITCTRLDELAY;
	uint32_t DFIODTCFG;
	uint32_t DFIODTCFG1;
	uint32_t DFIODTRANKMAP;
	/* DFI Write Data Registers */
	uint32_t DFITPHYWRDATA;
	uint32_t DFITPHYWRLAT;
	uint32_t DFITPHYWRDATALAT;
	/* DFI Read Data Registers */
	uint32_t DFITRDDATAEN;
	uint32_t DFITPHYRDLAT;
	/* DFI Update Registers */
	uint32_t DFITPHYUPDTYPE0;
	uint32_t DFITPHYUPDTYPE1;
	uint32_t DFITPHYUPDTYPE2;
	uint32_t DFITPHYUPDTYPE3;
	uint32_t DFITCTRLUPDMIN;
	uint32_t DFITCTRLUPDMAX;
	uint32_t DFITCTRLUPDDLY;
	uint32_t DFIUPDCFG;
	uint32_t DFITREFMSKI;
	uint32_t DFITCTRLUPDI;
	/* DFI Status Registers */
	uint32_t DFISTCFG0;
	uint32_t DFISTCFG1;
	uint32_t DFITDRAMCLKEN;
	uint32_t DFITDRAMCLKDIS;
	uint32_t DFISTCFG2;
	/* DFI Low Power Register */
	uint32_t DFILPCFG0;
};

struct DDRPHY_SAVE_REG_TAG {
	uint32_t PHY_REG0;
	uint32_t PHY_REG1;
	uint32_t PHY_REGB;
	uint32_t PHY_REGC;
	uint32_t PHY_REG11;
	uint32_t PHY_REG13;
	uint32_t PHY_REG14;
	uint32_t PHY_REG16;
	uint32_t PHY_REG20;
	uint32_t PHY_REG21;
	uint32_t PHY_REG26;
	uint32_t PHY_REG27;
	uint32_t PHY_REG28;
	uint32_t PHY_REG30;
	uint32_t PHY_REG31;
	uint32_t PHY_REG36;
	uint32_t PHY_REG37;
	uint32_t PHY_REG38;
	uint32_t PHY_REG40;
	uint32_t PHY_REG41;
	uint32_t PHY_REG46;
	uint32_t PHY_REG47;
	uint32_t PHY_REG48;
	uint32_t PHY_REG50;
	uint32_t PHY_REG51;
	uint32_t PHY_REG56;
	uint32_t PHY_REG57;
	uint32_t PHY_REG58;
	uint32_t PHY_REGDLL;
	uint32_t PHY_REGEC;
	uint32_t PHY_REGED;
	uint32_t PHY_REGEE;
	uint32_t PHY_REGEF;
	uint32_t PHY_REGFB;
	uint32_t PHY_REGFC;
	uint32_t PHY_REGFD;
	uint32_t PHY_REGFE;
};

struct BACKUP_REG_TAG {
	uint32_t tag;
	uint32_t pctladdr;
	struct PCTL_SAVE_REG_TAG pctl;
	uint32_t phyaddr;
	struct DDRPHY_SAVE_REG_TAG phy;
	uint32_t nocaddr;
	struct MSCH_SAVE_REG_TAG noc;
	uint32_t pllselect;
	uint32_t phypllockaddr;
	uint32_t phyplllockmask;
	uint32_t phyplllockval;
	uint32_t pllpdstat;
	uint32_t dpllmodeaddr;
	uint32_t dpllslowmode;
	uint32_t dpllnormalmode;
	uint32_t dpllresetaddr;
	uint32_t dpllreset;
	uint32_t dplldereset;
	uint32_t dpllconaddr;
	uint32_t dpllcon[4];
	uint32_t dplllockaddr;
	uint32_t dplllockmask;
	uint32_t dplllockval;
	uint32_t ddrpllsrcdivaddr;
	uint32_t ddrpllsrcdiv;
	uint32_t retendisaddr;
	uint32_t retendisval;
	uint32_t grfregaddr;
	uint32_t grfddrcreg;
	uint32_t crupctlphysoftrstaddr;
	uint32_t cruresetpctlphy;
	uint32_t cruderesetphy;
	uint32_t cruderesetpctlphy;
	uint32_t physoftrstaddr;
	uint32_t endtag;
};

static uint32_t ddr_get_phy_pll_freq(void)
{
	uint32_t ret = 0;
	uint32_t fb_div, pre_div;

	fb_div = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGEC);
	fb_div |= (mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGED) & 0x1) << 8;

	pre_div = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGEE) & 0xff;
	ret = 2 * 24 * fb_div / (4 * pre_div);

	return ret;
}

static void ddr_copy(uint32_t *pdest, uint32_t *psrc, uint32_t words)
{
	uint32_t i;

	for (i = 0; i < words; i++)
		pdest[i] = psrc[i];
}

static void ddr_get_dpll_cfg(uint32_t *p)
{
	uint32_t nmhz, NO, NF, NR;

	nmhz = ddr_get_phy_pll_freq();
	if (nmhz <= 150)
		NO = 6;
	else if (nmhz <= 250)
		NO = 4;
	else if (nmhz <= 500)
		NO = 2;
	else
		NO = 1;

	NR = 1;
	NF = 2 * nmhz * NR * NO / 24;

	p[0] = SET_NR(NR) | SET_NO(NO);
	p[1] = SET_NF(NF);
	p[2] = SET_NB(NF / 2);
}

void ddr_reg_save(uint32_t pllpdstat, uint64_t base_addr)
{
	struct BACKUP_REG_TAG *p_ddr_reg = (struct BACKUP_REG_TAG *)base_addr;
	struct PCTL_SAVE_REG_TAG *pctl_tim = &p_ddr_reg->pctl;

	p_ddr_reg->tag = 0x56313031;
	p_ddr_reg->pctladdr = DDR_PCTL_BASE;
	p_ddr_reg->phyaddr = DDR_PHY_BASE;
	p_ddr_reg->nocaddr = SERVICE_BUS_BASE;

	/* PCTLR */
	ddr_copy((uint32_t *)&pctl_tim->pctl_timing.TOGCNT1U,
		 (uint32_t *)(DDR_PCTL_BASE + DDR_PCTL_TOGCNT1U), 35);
	pctl_tim->pctl_timing.TREFI |= DDR_UPD_REF_ENABLE;
	pctl_tim->SCFG = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_SCFG);
	pctl_tim->CMDTSTATEN = mmio_read_32(DDR_PCTL_BASE +
					    DDR_PCTL_CMDTSTATEN);
	pctl_tim->MCFG1 = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_MCFG1);
	pctl_tim->MCFG = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_MCFG);
	pctl_tim->PPCFG = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_PPCFG);
	pctl_tim->pctl_timing.ddrfreq = mmio_read_32(DDR_PCTL_BASE +
						     DDR_PCTL_TOGCNT1U * 2);
	pctl_tim->DFITCTRLDELAY = mmio_read_32(DDR_PCTL_BASE +
					       DDR_PCTL_DFITCTRLDELAY);
	pctl_tim->DFIODTCFG = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_DFIODTCFG);
	pctl_tim->DFIODTCFG1 = mmio_read_32(DDR_PCTL_BASE +
					    DDR_PCTL_DFIODTCFG1);
	pctl_tim->DFIODTRANKMAP = mmio_read_32(DDR_PCTL_BASE +
					       DDR_PCTL_DFIODTRANKMAP);
	pctl_tim->DFITPHYWRDATA = mmio_read_32(DDR_PCTL_BASE +
					       DDR_PCTL_DFITPHYWRDATA);
	pctl_tim->DFITPHYWRLAT = mmio_read_32(DDR_PCTL_BASE +
					      DDR_PCTL_DFITPHYWRLAT);
	pctl_tim->DFITPHYWRDATALAT = mmio_read_32(DDR_PCTL_BASE +
						  DDR_PCTL_DFITPHYWRDATALAT);
	pctl_tim->DFITRDDATAEN = mmio_read_32(DDR_PCTL_BASE +
					      DDR_PCTL_DFITRDDATAEN);
	pctl_tim->DFITPHYRDLAT = mmio_read_32(DDR_PCTL_BASE +
					      DDR_PCTL_DFITPHYRDLAT);
	pctl_tim->DFITPHYUPDTYPE0 = mmio_read_32(DDR_PCTL_BASE +
						 DDR_PCTL_DFITPHYUPDTYPE0);
	pctl_tim->DFITPHYUPDTYPE1 = mmio_read_32(DDR_PCTL_BASE +
						 DDR_PCTL_DFITPHYUPDTYPE1);
	pctl_tim->DFITPHYUPDTYPE2 = mmio_read_32(DDR_PCTL_BASE +
						 DDR_PCTL_DFITPHYUPDTYPE2);
	pctl_tim->DFITPHYUPDTYPE3 = mmio_read_32(DDR_PCTL_BASE +
						 DDR_PCTL_DFITPHYUPDTYPE3);
	pctl_tim->DFITCTRLUPDMIN = mmio_read_32(DDR_PCTL_BASE +
						DDR_PCTL_DFITCTRLUPDMIN);
	pctl_tim->DFITCTRLUPDMAX = mmio_read_32(DDR_PCTL_BASE +
						DDR_PCTL_DFITCTRLUPDMAX);
	pctl_tim->DFITCTRLUPDDLY = mmio_read_32(DDR_PCTL_BASE +
						DDR_PCTL_DFITCTRLUPDDLY);

	pctl_tim->DFIUPDCFG = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_DFIUPDCFG);
	pctl_tim->DFITREFMSKI = mmio_read_32(DDR_PCTL_BASE +
					     DDR_PCTL_DFITREFMSKI);
	pctl_tim->DFITCTRLUPDI = mmio_read_32(DDR_PCTL_BASE +
					      DDR_PCTL_DFITCTRLUPDI);
	pctl_tim->DFISTCFG0 = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_DFISTCFG0);
	pctl_tim->DFISTCFG1 = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_DFISTCFG1);
	pctl_tim->DFITDRAMCLKEN = mmio_read_32(DDR_PCTL_BASE +
					       DDR_PCTL_DFITDRAMCLKEN);
	pctl_tim->DFITDRAMCLKDIS = mmio_read_32(DDR_PCTL_BASE +
						DDR_PCTL_DFITDRAMCLKDIS);
	pctl_tim->DFISTCFG2 = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_DFISTCFG2);
	pctl_tim->DFILPCFG0 = mmio_read_32(DDR_PCTL_BASE + DDR_PCTL_DFILPCFG0);

	/* PHY */
	p_ddr_reg->phy.PHY_REG0 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG0);
	p_ddr_reg->phy.PHY_REG1 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG1);
	p_ddr_reg->phy.PHY_REGB = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGB);
	p_ddr_reg->phy.PHY_REGC = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGC);
	p_ddr_reg->phy.PHY_REG11 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG11);
	p_ddr_reg->phy.PHY_REG13 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG13);
	p_ddr_reg->phy.PHY_REG14 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG14);
	p_ddr_reg->phy.PHY_REG16 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG16);
	p_ddr_reg->phy.PHY_REG20 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG20);
	p_ddr_reg->phy.PHY_REG21 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG21);
	p_ddr_reg->phy.PHY_REG26 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG26);
	p_ddr_reg->phy.PHY_REG27 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG27);
	p_ddr_reg->phy.PHY_REG28 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG28);
	p_ddr_reg->phy.PHY_REG30 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG30);
	p_ddr_reg->phy.PHY_REG31 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG31);
	p_ddr_reg->phy.PHY_REG36 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG36);
	p_ddr_reg->phy.PHY_REG37 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG37);
	p_ddr_reg->phy.PHY_REG38 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG38);
	p_ddr_reg->phy.PHY_REG40 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG40);
	p_ddr_reg->phy.PHY_REG41 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG41);
	p_ddr_reg->phy.PHY_REG46 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG46);
	p_ddr_reg->phy.PHY_REG47 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG47);
	p_ddr_reg->phy.PHY_REG48 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG48);
	p_ddr_reg->phy.PHY_REG50 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG50);
	p_ddr_reg->phy.PHY_REG51 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG51);
	p_ddr_reg->phy.PHY_REG56 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG56);
	p_ddr_reg->phy.PHY_REG57 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG57);
	p_ddr_reg->phy.PHY_REG58 = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG58);
	p_ddr_reg->phy.PHY_REGDLL = mmio_read_32(DDR_PHY_BASE +
						 DDR_PHY_REGDLL);
	p_ddr_reg->phy.PHY_REGEC = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGEC);
	p_ddr_reg->phy.PHY_REGED = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGED);
	p_ddr_reg->phy.PHY_REGEE = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGEE);
	p_ddr_reg->phy.PHY_REGEF = 0;

	if (mmio_read_32(DDR_PHY_BASE + DDR_PHY_REG2) & 0x2) {
		p_ddr_reg->phy.PHY_REGFB = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REG2C);
		p_ddr_reg->phy.PHY_REGFC = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REG3C);
		p_ddr_reg->phy.PHY_REGFD = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REG4C);
		p_ddr_reg->phy.PHY_REGFE = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REG5C);
	} else {
		p_ddr_reg->phy.PHY_REGFB = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REGFB);
		p_ddr_reg->phy.PHY_REGFC = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REGFC);
		p_ddr_reg->phy.PHY_REGFD = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REGFD);
		p_ddr_reg->phy.PHY_REGFE = mmio_read_32(DDR_PHY_BASE +
							DDR_PHY_REGFE);
	}

	/* NOC */
	p_ddr_reg->noc.ddrconf = mmio_read_32(SERVICE_BUS_BASE + MSCH_DDRCONF);
	p_ddr_reg->noc.ddrtiming = mmio_read_32(SERVICE_BUS_BASE +
						MSCH_DDRTIMING);
	p_ddr_reg->noc.ddrmode = mmio_read_32(SERVICE_BUS_BASE + MSCH_DDRMODE);
	p_ddr_reg->noc.readlatency = mmio_read_32(SERVICE_BUS_BASE +
						  MSCH_READLATENCY);
	p_ddr_reg->noc.activate = mmio_read_32(SERVICE_BUS_BASE +
					       MSCH_ACTIVATE);
	p_ddr_reg->noc.devtodev = mmio_read_32(SERVICE_BUS_BASE +
					       MSCH_DEVTODEV);

	p_ddr_reg->pllselect = mmio_read_32(DDR_PHY_BASE + DDR_PHY_REGEE) * 0x1;
	p_ddr_reg->phypllockaddr = GRF_BASE + GRF_SOC_STATUS0;
	p_ddr_reg->phyplllockmask = GRF_DDRPHY_LOCK;
	p_ddr_reg->phyplllockval = 0;

	/* PLLPD */
	p_ddr_reg->pllpdstat = pllpdstat;
	/* DPLL */
	p_ddr_reg->dpllmodeaddr = CRU_BASE + PLL_CONS(DPLL_ID, 3);
	/* slow mode and power on */
	p_ddr_reg->dpllslowmode = DPLL_WORK_SLOW_MODE | DPLL_POWER_DOWN;
	p_ddr_reg->dpllnormalmode = DPLL_WORK_NORMAL_MODE;
	p_ddr_reg->dpllresetaddr = CRU_BASE + PLL_CONS(DPLL_ID, 3);
	p_ddr_reg->dpllreset = DPLL_RESET_CONTROL_NORMAL;
	p_ddr_reg->dplldereset = DPLL_RESET_CONTROL_RESET;
	p_ddr_reg->dpllconaddr = CRU_BASE + PLL_CONS(DPLL_ID, 0);

	if (p_ddr_reg->pllselect == 0) {
		p_ddr_reg->dpllcon[0] = (mmio_read_32(CRU_BASE +
						      PLL_CONS(DPLL_ID, 0))
							& 0xffff) |
					(0xFFFFu << 16);
		p_ddr_reg->dpllcon[1] = (mmio_read_32(CRU_BASE +
						      PLL_CONS(DPLL_ID, 1))
							& 0xffff);
		p_ddr_reg->dpllcon[2] = (mmio_read_32(CRU_BASE +
						      PLL_CONS(DPLL_ID, 2))
							& 0xffff);
		p_ddr_reg->dpllcon[3] = (mmio_read_32(CRU_BASE +
						      PLL_CONS(DPLL_ID, 3))
							& 0xffff) |
					(0xFFFFu << 16);
	} else {
		ddr_get_dpll_cfg(&p_ddr_reg->dpllcon[0]);
	}

	p_ddr_reg->pllselect = 0;
	p_ddr_reg->dplllockaddr = CRU_BASE + PLL_CONS(DPLL_ID, 1);
	p_ddr_reg->dplllockmask = DPLL_STATUS_LOCK;
	p_ddr_reg->dplllockval = DPLL_STATUS_LOCK;

	/* SET_DDR_PLL_SRC */
	p_ddr_reg->ddrpllsrcdivaddr = CRU_BASE + CRU_CLKSELS_CON(13);
	p_ddr_reg->ddrpllsrcdiv = (mmio_read_32(CRU_BASE + CRU_CLKSELS_CON(13))
					& DDR_PLL_SRC_MASK)
					| (DDR_PLL_SRC_MASK << 16);
	p_ddr_reg->retendisaddr = PMU_BASE + PMU_PWRMD_COM;
	p_ddr_reg->retendisval = PD_PERI_PWRDN_ENABLE;
	p_ddr_reg->grfregaddr = GRF_BASE + GRF_DDRC0_CON0;
	p_ddr_reg->grfddrcreg = (mmio_read_32(GRF_BASE + GRF_DDRC0_CON0) &
					      DDR_PLL_SRC_MASK) |
				 (DDR_PLL_SRC_MASK << 16);

	/* pctl phy soft reset */
	p_ddr_reg->crupctlphysoftrstaddr = CRU_BASE + CRU_SOFTRSTS_CON(10);
	p_ddr_reg->cruresetpctlphy = DDRCTRL0_PSRSTN_REQ(1) |
				     DDRCTRL0_SRSTN_REQ(1) |
				     DDRPHY0_PSRSTN_REQ(1) |
				     DDRPHY0_SRSTN_REQ(1);
	p_ddr_reg->cruderesetphy = DDRCTRL0_PSRSTN_REQ(1) |
				   DDRCTRL0_SRSTN_REQ(1) |
				   DDRPHY0_PSRSTN_REQ(0) |
				   DDRPHY0_SRSTN_REQ(0);

	p_ddr_reg->cruderesetpctlphy = DDRCTRL0_PSRSTN_REQ(0) |
				       DDRCTRL0_SRSTN_REQ(0) |
				       DDRPHY0_PSRSTN_REQ(0) |
				       DDRPHY0_SRSTN_REQ(0);

	p_ddr_reg->physoftrstaddr = DDR_PHY_BASE + DDR_PHY_REG0;

	p_ddr_reg->endtag = 0xFFFFFFFF;
}

/*
 * "rk3368_ddr_reg_resume_V1.05.bin" is an executable bin which is generated
 * by ARM DS5 for resuming ddr controller. If the soc wakes up from system
 * suspend, ddr needs to be resumed and the resuming code needs to be run in
 * sram. But there is not a way to pointing the resuming code to the PMUSRAM
 * when linking .o files of bl31, so we use the
 * "rk3368_ddr_reg_resume_V1.05.bin" whose code is position-independent and
 * it can be loaded anywhere and run.
 */
static __aligned(4) unsigned int ddr_reg_resume[] = {
	#include "rk3368_ddr_reg_resume_V1.05.bin"
};

uint32_t ddr_get_resume_code_size(void)
{
	return sizeof(ddr_reg_resume);
}

uint32_t ddr_get_resume_data_size(void)
{
	return sizeof(struct BACKUP_REG_TAG);
}

uint32_t *ddr_get_resume_code_base(void)
{
	return (unsigned int *)ddr_reg_resume;
}
