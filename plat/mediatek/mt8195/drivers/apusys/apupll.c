/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/spinlock.h>

#include <apupwr_clkctl.h>
#include <apupwr_clkctl_def.h>
#include <mtk_plat_common.h>
#include <platform_def.h>

uint32_t mixed_con0_addr[APUPLL_MAX] = {
	APU_PLL4H_PLL1_CON0,
	APU_PLL4H_PLL2_CON0,
	APU_PLL4H_PLL3_CON0,
	APU_PLL4H_PLL4_CON0,
};

uint32_t mixed_con1_addr[APUPLL_MAX] = {
	APU_PLL4H_PLL1_CON1,
	APU_PLL4H_PLL2_CON1,
	APU_PLL4H_PLL3_CON1,
	APU_PLL4H_PLL4_CON1,
};

uint32_t mixed_con3_addr[APUPLL_MAX] = {
	APU_PLL4H_PLL1_CON3,
	APU_PLL4H_PLL2_CON3,
	APU_PLL4H_PLL3_CON3,
	APU_PLL4H_PLL4_CON3,
};

uint32_t fhctl_dds_addr[APUPLL_MAX] = {
	APU_PLL4H_FHCTL0_DDS,
	APU_PLL4H_FHCTL1_DDS,
	APU_PLL4H_FHCTL2_DDS,
	APU_PLL4H_FHCTL3_DDS,
};

uint32_t fhctl_dvfs_addr[APUPLL_MAX] = {
	APU_PLL4H_FHCTL0_DVFS,
	APU_PLL4H_FHCTL1_DVFS,
	APU_PLL4H_FHCTL2_DVFS,
	APU_PLL4H_FHCTL3_DVFS,
};

uint32_t fhctl_mon_addr[APUPLL_MAX] = {
	APU_PLL4H_FHCTL0_MON,
	APU_PLL4H_FHCTL1_MON,
	APU_PLL4H_FHCTL2_MON,
	APU_PLL4H_FHCTL3_MON,
};

uint32_t fhctl_cfg_addr[APUPLL_MAX] = {
	APU_PLL4H_FHCTL0_CFG,
	APU_PLL4H_FHCTL1_CFG,
	APU_PLL4H_FHCTL2_CFG,
	APU_PLL4H_FHCTL3_CFG,
};

static spinlock_t apupll_lock;
static spinlock_t npupll_lock;
static spinlock_t apupll_1_lock;
static spinlock_t apupll_2_lock;
static uint32_t pll_cnt[APUPLL_MAX];
/**
 * vd2pllidx() - voltage domain to pll idx.
 * @domain: the voltage domain for getting pll index.
 *
 * Caller will get correspond pll index by different voltage domain.
 * pll_idx[0] --> APUPLL (MDLA0/1)
 * pll_idx[1] --> NPUPLL (VPU0/1)
 * pll_idx[2] --> APUPLL1(CONN)
 * pll_idx[3] --> APUPLL2(IOMMU)
 * The longer description may have multiple paragraphs.
 *
 * Context: Any context.
 * Return:
 * * 0 ~ 3	- return the corresponding pll index
 * * -EEXIST	- cannot find pll idex of the specific voltage domain
 *
 */
static int32_t vd2pllidx(enum dvfs_voltage_domain domain)
{
	int32_t ret;

	switch (domain) {
	case V_VPU0:
	case V_VPU1:
		ret = NPUPLL;
		break;
	case V_MDLA0:
	case V_MDLA1:
		ret = APUPLL;
		break;
	case V_TOP_IOMMU:
		ret = APUPLL2;
		break;
	case V_APU_CONN:
		ret = APUPLL1;
		break;
	default:
		ERROR("%s wrong voltage domain: %d\n", __func__, domain);
		ret = -EEXIST; /* non-exist */
		break;
	}

	return ret;
}

/**
 * pllidx2name() - return names of specific pll index.
 * @pll_idx: input for specific pll index.
 *
 * Given pll index, this function will return name of it.
 *
 * Context: Any context.
 * Return: Names of pll_idx, if found, otherwise will return "NULL"
 */
static const char *pllidx2name(int32_t pll_idx)
{
	static const char *const names[] = {
		[APUPLL] = "PLL4H_PLL1",
		[NPUPLL] = "PLL4H_PLL2",
		[APUPLL1] = "PLL4H_PLL3",
		[APUPLL2] = "PLL4H_PLL4",
		[APUPLL_MAX] = "NULL",
	};

	if (pll_idx >= APUPLL_MAX) {
		pll_idx = APUPLL_MAX;
	}

	return names[pll_idx];
}

/**
 * _fhctl_mon_done() - poll whether fhctl HW mode is done.
 * @pll_idx: input for specific pll index.
 * @tar_dds: target dds for fhctl_mon to be.
 *
 * Given pll index, this function will continue to poll whether fhctl_mon
 * has reached the expected value within 80us.
 *
 * Context: Any context.
 * Return:
 * * 0 - OK for fhctl_mon == tar_dds
 * * -ETIMEDOUT - fhctl_mon not reach tar_dds
 */
static int32_t _fhctl_mon_done(uint32_t pll_idx, unsigned long tar_dds)
{
	unsigned long mon_dds;
	uint64_t timeout = timeout_init_us(PLL_READY_TIME_20US);
	int32_t ret = 0;

	tar_dds &= DDS_MASK;
	do {
		mon_dds = apupwr_readl(fhctl_mon_addr[pll_idx]) & DDS_MASK;
		if (mon_dds == tar_dds) {
			break;
		}

		if (timeout_elapsed(timeout)) {
			ERROR("%s monitor DDS 0x%08lx != expect 0x%08lx\n",
				  pllidx2name(pll_idx), mon_dds, tar_dds);
			ret = -ETIMEDOUT;
			break;
		}
	} while (mon_dds != tar_dds);

	return ret;
}

/**
 * _pll_get_postdiv_reg() - return current post dividor of pll_idx
 * @pll_idx: input for specific pll index.
 *
 * Given pll index, this function will return its current post dividor.
 *
 * Context: Any context.
 * Return: post dividor of current pll_idx.
 *
 */
static uint32_t _pll_get_postdiv_reg(uint32_t pll_idx)
{
	int32_t pll_postdiv_reg = 0;
	uint32_t val;

	val = apupwr_readl(mixed_con1_addr[pll_idx]);
	pll_postdiv_reg = (val >> POSDIV_SHIFT) & POSDIV_MASK;
	return pll_postdiv_reg;
}

/**
 * _set_postdiv_reg() - set pll_idx's post dividor.
 * @pll_idx: Which PLL to enable/disable
 * @post_div: the register value of post dividor to be wrtten.
 *
 * Below are lists of post dividor register value and its meaning:
 * [31]     APUPLL_SDM_PCW_CHG
 * [26:24]  APUPLL_POSDIV
 * [21:0]   APUPLL_SDM_PCW (8bit integer + 14bit fraction)
 * expected freq range ----- divider-------post divider in reg:
 * >1500M   (1500/ 1)         -> 1        -> 0(2 to the zero power)
 * > 750M   (1500/ 2)         -> 2        -> 1(2 to the 1st  power)
 * > 375M   (1500/ 4)         -> 4        -> 2(2 to the 2nd  power)
 * > 187.5M (1500/ 8)         -> 8        -> 3(2 to the 3rd  power)
 * > 93.75M (1500/16)         -> 16       -> 4(2 to the 4th  power)
 *
 * Context: Any context.
 */
static void _set_postdiv_reg(uint32_t pll_idx, uint32_t post_div)
{
	apupwr_clrbits(POSDIV_MASK << POSDIV_SHIFT, mixed_con1_addr[pll_idx]);
	apupwr_setbits((post_div & POSDIV_MASK) << POSDIV_SHIFT,
			mixed_con1_addr[pll_idx]);
}

/**
 * _cal_pll_data() - input freq, calculate correspond post dividor and dds.
 * @pd: address of output post dividor.
 * @dds: address of output dds.
 * @freq: input frequency.
 *
 * Given freq, this function will calculate correspond post dividor and dds.
 *
 * Context: Any context.
 * Return:
 * * 0 - done for calculating post dividor and dds.
 */
static int32_t _cal_pll_data(uint32_t *pd, uint32_t *dds, uint32_t freq)
{
	uint32_t vco, postdiv_val = 1, postdiv_reg = 0;
	uint32_t pcw_val;

	vco = freq;
	postdiv_val = 1;
	postdiv_reg = 0;
	while (vco <= FREQ_VCO_MIN) {
		postdiv_val = postdiv_val << 1;
		postdiv_reg = postdiv_reg + 1;
		vco = vco << 1;
	}

	pcw_val = vco * (1 << PCW_FRACTIONAL_SHIFT);
	pcw_val = pcw_val / FREQ_FIN;

	if (postdiv_reg == 0) { /* Fvco * 2 with post_divider = 2 */
		pcw_val = pcw_val * 2;
		postdiv_val = postdiv_val << 1;
		postdiv_reg = postdiv_reg + 1;
	} /* Post divider is 1 is not available */
	*pd = postdiv_reg;
	*dds = pcw_val | RG_PLL_SDM_PCW_CHG;

	return 0;
}

/**
 * _pll_en() - enable/disable RG_PLL_EN of CON1 for pll[pll_idx]
 * @pll_idx: Which PLL to enable/disable
 * @on: 1 -> enable, 0 -> disable.
 *
 * This funciton will only change RG_PLL_EN of CON1 for pll[pll_idx].
 *
 * Context: Any context.
 */
static void _pll_en(uint32_t pll_idx, bool on)
{
	if (on) {
		apupwr_setbits(RG_PLL_EN, mixed_con0_addr[pll_idx]);
	} else {
		apupwr_clrbits(RG_PLL_EN, mixed_con0_addr[pll_idx]);
	}
}

/**
 * _pll_pwr() - enable/disable PLL_SDM_PWR_ON of CON3 for pll[pll_idx]
 * @pll_idx: Which PLL to enable/disable
 * @on: 1 -> enable, 0 -> disable.
 *
 * This funciton will only change PLL_SDM_PWR_ON of CON3 for pll[pll_idx].
 *
 * Context: Any context.
 */
static void _pll_pwr(uint32_t pll_idx, bool on)
{
	if (on) {
		apupwr_setbits(DA_PLL_SDM_PWR_ON, mixed_con3_addr[pll_idx]);
	} else {
		apupwr_clrbits(DA_PLL_SDM_PWR_ON, mixed_con3_addr[pll_idx]);
	}
}

/**
 * _pll_iso() - enable/disable PLL_SDM_ISO_EN of CON3 for pll[pll_idx]
 * @pll_idx: Which PLL to enable/disable
 * @enable: 1 -> turn on isolation, 0 -> turn off isolation.
 *
 * This funciton will turn on/off pll isolation by
 * changing PLL_SDM_PWR_ON of CON3 for pll[pll_idx].
 *
 * Context: Any context.
 */
static void _pll_iso(uint32_t pll_idx, bool enable)
{
	if (enable) {
		apupwr_setbits(DA_PLL_SDM_ISO_EN, mixed_con3_addr[pll_idx]);
	} else {
		apupwr_clrbits(DA_PLL_SDM_ISO_EN, mixed_con3_addr[pll_idx]);
	}
}

/**
 * _pll_switch() - entry point to turn whole PLL on/off
 * @pll_idx: Which PLL to enable/disable
 * @on: 1 -> enable, 0 -> disable.
 * @fhctl_en: enable or disable fhctl function
 *
 * This is the entry poing for controlling pll and fhctl funciton on/off.
 * Caller can chose only enable pll instead of fhctl function.
 *
 * Context: Any context.
 * Return:
 * * 0		- done for enable pll or fhctl as well.
 */
static int32_t _pll_switch(uint32_t pll_idx, bool on, bool fhctl_en)
{
	int32_t ret = 0;

	if (pll_idx >= APUPLL_MAX) {
		ERROR("%s wrong pll_idx: %d\n", __func__, pll_idx);
		ret = -EINVAL;
		goto err;
	}

	if (on) {
		_pll_pwr(pll_idx, true);
		udelay(PLL_CMD_READY_TIME_1US);
		_pll_iso(pll_idx, false);
		udelay(PLL_CMD_READY_TIME_1US);
		_pll_en(pll_idx, true);
		udelay(PLL_READY_TIME_20US);
	} else {
		_pll_en(pll_idx, false);
		_pll_iso(pll_idx, true);
		_pll_pwr(pll_idx, false);
	}

err:
	return ret;
}

/**
 * apu_pll_enable() - API for smc function to enable/disable pll
 * @pll_idx: Which pll to enable/disable.
 * @enable: 1 -> enable, 0 -> disable.
 * @fhctl_en: enable or disable fhctl function
 *
 * pll_idx[0] --> APUPLL (MDLA0/1)
 * pll_idx[1] --> NPUPLL (VPU0/1)
 * pll_idx[2] --> APUPLL1(CONN)
 * pll_idx[3] --> APUPLL2(IOMMU)
 * The differences between _pll_switch are:
 * 1. Atomic update pll reference cnt to protect double enable pll &
 * close pll during user is not zero.
 *
 * Context: Any context.
 * Return:
 * * 0 - done for enable pll or fhctl as well.
 */
int32_t apu_pll_enable(int32_t pll_idx, bool enable, bool fhctl_en)
{
	int32_t ret = 0;

	if (pll_idx >= APUPLL_MAX) {
		ERROR("%s wrong pll_idx: %d\n", __func__, pll_idx);
		ret = -EINVAL;
		goto err;
	}

	if (enable) {
		switch (pll_idx) {
		case APUPLL:
			spin_lock(&apupll_lock);
			if (pll_cnt[APUPLL] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			pll_cnt[APUPLL]++;
			spin_unlock(&apupll_lock);
			break;
		case NPUPLL:
			spin_lock(&npupll_lock);
			if (pll_cnt[NPUPLL] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			pll_cnt[NPUPLL]++;
			spin_unlock(&npupll_lock);
			break;
		case APUPLL1:
			spin_lock(&apupll_1_lock);
			if (pll_cnt[APUPLL1] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			pll_cnt[APUPLL1]++;
			spin_unlock(&apupll_1_lock);
			break;
		case APUPLL2:
			spin_lock(&apupll_2_lock);
			if (pll_cnt[APUPLL2] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			pll_cnt[APUPLL2]++;
			spin_unlock(&apupll_2_lock);
			break;
		default:
			ERROR("%s invalid pll_idx: %d\n", __func__, pll_idx);
			ret = -EINVAL;
			break;
		}
	} else {
		switch (pll_idx) {
		case APUPLL:
			spin_lock(&apupll_lock);
			if (pll_cnt[APUPLL]) {
				pll_cnt[APUPLL]--;
			}
			if (pll_cnt[APUPLL] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			spin_unlock(&apupll_lock);
			break;
		case NPUPLL:
			spin_lock(&npupll_lock);
			if (pll_cnt[NPUPLL]) {
				pll_cnt[NPUPLL]--;
			}
			if (pll_cnt[NPUPLL] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			spin_unlock(&npupll_lock);
			break;
		case APUPLL1:
			spin_lock(&apupll_1_lock);
			if (pll_cnt[APUPLL1]) {
				pll_cnt[APUPLL1]--;
			}
			if (pll_cnt[APUPLL1] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			spin_unlock(&apupll_1_lock);
			break;
		case APUPLL2:
			spin_lock(&apupll_2_lock);
			if (pll_cnt[APUPLL2]) {
				pll_cnt[APUPLL2]--;
			}
			if (pll_cnt[APUPLL2] == 0) {
				_pll_switch(pll_idx, enable, fhctl_en);
			}
			spin_unlock(&apupll_2_lock);
			break;
		default:
			ERROR("%s invalid pll_idx: %d\n", __func__, pll_idx);
			ret = -EINVAL;
			break;
		}
	}

err:
	return ret;
}

/**
 * anpu_pll_set_rate() - API for smc function to set rate of voltage domain.
 * @domain: Which pll of correspond voltage domain to change rate.
 * @mode: which mode to use when set_rate
 * @freq: which frequency to set.
 *
 * For V_VPU0/1, it will only allow 1 of them to modify NPUPLL
 * such that there will be no race condition happen.
 *
 * For V_MDLA0/1, it will only allow 1 of them to modify APUPLL1
 * such that there will be no race condition happen.
 *
 * There are 3 kinds of modes to set pll's rate.
 * 1. pure sw mode: (CON0_PCW)
 *        fhctl function is off and change rate by programming CON1_PCW.
 * 2. fhctl sw mode: (FHCTL_SW)
 *        fhctl function is on and change rate by programming fhctl_dds.
 *        (post dividor is still need to program CON1_PCW)
 * 3. fhctl hw mode: (FHCTL_HW)
 *        fhctl function is on and change rate by programming fhctl_dvfs.
 *        (post dividor is still need to program CON1_PCW)
 *
 * Context: Any context.
 * Return:
 * * 0 - done for set rate of voltage domain.
 */
int32_t anpu_pll_set_rate(enum dvfs_voltage_domain domain,
		      enum pll_set_rate_mode mode, int32_t freq)
{
	uint32_t pd, old_pd, dds;
	int32_t pll_idx, ret = 0;

	pll_idx = vd2pllidx(domain);
	if (pll_idx < 0) {
		ret = pll_idx;
		goto err;
	}

	_cal_pll_data(&pd, &dds, freq / 1000);

	INFO("%s %s new post_div=%d, target dds=0x%08x(%dMhz) mode = %d\n",
	     __func__, pllidx2name(pll_idx), pd, dds, freq / 1000, mode);

	/* spin_lock for NPULL, since vpu0/1 share npupll */
	if (domain == V_VPU0 || domain == V_VPU1) {
		spin_lock(&npupll_lock);
	}

	/* spin_lock for APUPLL, since mdla0/1 shate apupll */
	if (domain == V_MDLA0 || domain == V_MDLA1) {
		spin_lock(&apupll_lock);
	}

	switch (mode) {
	case CON0_PCW:
		pd = RG_PLL_SDM_PCW_CHG |
		     (pd & POSDIV_MASK) << POSDIV_SHIFT | dds;
		apupwr_writel(pd, mixed_con1_addr[pll_idx]);
		udelay(PLL_READY_TIME_20US);
		break;
	case FHCTL_SW:
		/* pll con0 disable */
		_pll_en(pll_idx, false);
		apupwr_writel(dds, fhctl_dds_addr[pll_idx]);
		_set_postdiv_reg(pll_idx, pd);
		apupwr_setbits(PLL_TGL_ORG, fhctl_dds_addr[pll_idx]);
		udelay(PLL_CMD_READY_TIME_1US);
		/* pll con0 enable */
		_pll_en(pll_idx, true);
		udelay(PLL_READY_TIME_20US);
		break;
	case FHCTL_HW:
		old_pd = _pll_get_postdiv_reg(pll_idx);
		if (pd > old_pd) {
			_set_postdiv_reg(pll_idx, pd);
			apupwr_writel(dds, fhctl_dvfs_addr[pll_idx]);
		} else {
			apupwr_writel(dds, fhctl_dvfs_addr[pll_idx]);
			_set_postdiv_reg(pll_idx, pd);
		}
		ret = _fhctl_mon_done(pll_idx, dds);
		break;
	default:
		ERROR("%s input wrong mode: %d\n", __func__, mode);
		ret = -EINVAL;
		break;
	}

	/* spin_lock for NPULL, since vpu0/1 share npupll */
	if (domain == V_VPU0 || domain == V_VPU1) {
		spin_unlock(&npupll_lock);
	}

	/* spin_lock for APUPLL, since mdla0/1 share apupll */
	if (domain == V_MDLA0 || domain == V_MDLA1) {
		spin_unlock(&apupll_lock);
	}

err:
	return ret;
}
