/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <mtk_dcm_utils.h>

#define MP_CPUSYS_TOP_ADB_DCM_REG0_MASK (BIT(17))
#define MP_CPUSYS_TOP_ADB_DCM_REG1_MASK (BIT(15) | \
			BIT(16) | \
			BIT(17) | \
			BIT(18) | \
			BIT(21))
#define MP_CPUSYS_TOP_ADB_DCM_REG2_MASK (BIT(15) | \
			BIT(16) | \
			BIT(17) | \
			BIT(18))
#define MP_CPUSYS_TOP_ADB_DCM_REG0_ON (BIT(17))
#define MP_CPUSYS_TOP_ADB_DCM_REG1_ON (BIT(15) | \
			BIT(16) | \
			BIT(17) | \
			BIT(18) | \
			BIT(21))
#define MP_CPUSYS_TOP_ADB_DCM_REG2_ON (BIT(15) | \
			BIT(16) | \
			BIT(17) | \
			BIT(18))
#define MP_CPUSYS_TOP_ADB_DCM_REG0_OFF ((0x0 << 17))
#define MP_CPUSYS_TOP_ADB_DCM_REG1_OFF ((0x0 << 15) | \
			(0x0 << 16) | \
			(0x0 << 17) | \
			(0x0 << 18) | \
			(0x0 << 21))
#define MP_CPUSYS_TOP_ADB_DCM_REG2_OFF ((0x0 << 15) | \
			(0x0 << 16) | \
			(0x0 << 17) | \
			(0x0 << 18))

bool dcm_mp_cpusys_top_adb_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(MP_ADB_DCM_CFG0) &
		MP_CPUSYS_TOP_ADB_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_ADB_DCM_REG0_ON);
	ret &= ((mmio_read_32(MP_ADB_DCM_CFG4) &
		MP_CPUSYS_TOP_ADB_DCM_REG1_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_ADB_DCM_REG1_ON);
	ret &= ((mmio_read_32(MCUSYS_DCM_CFG0) &
		MP_CPUSYS_TOP_ADB_DCM_REG2_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_ADB_DCM_REG2_ON);

	return ret;
}

void dcm_mp_cpusys_top_adb_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_adb_dcm'" */
		mmio_clrsetbits_32(MP_ADB_DCM_CFG0,
			MP_CPUSYS_TOP_ADB_DCM_REG0_MASK,
			MP_CPUSYS_TOP_ADB_DCM_REG0_ON);
		mmio_clrsetbits_32(MP_ADB_DCM_CFG4,
			MP_CPUSYS_TOP_ADB_DCM_REG1_MASK,
			MP_CPUSYS_TOP_ADB_DCM_REG1_ON);
		mmio_clrsetbits_32(MCUSYS_DCM_CFG0,
			MP_CPUSYS_TOP_ADB_DCM_REG2_MASK,
			MP_CPUSYS_TOP_ADB_DCM_REG2_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_adb_dcm'" */
		mmio_clrsetbits_32(MP_ADB_DCM_CFG0,
			MP_CPUSYS_TOP_ADB_DCM_REG0_MASK,
			MP_CPUSYS_TOP_ADB_DCM_REG0_OFF);
		mmio_clrsetbits_32(MP_ADB_DCM_CFG4,
			MP_CPUSYS_TOP_ADB_DCM_REG1_MASK,
			MP_CPUSYS_TOP_ADB_DCM_REG1_OFF);
		mmio_clrsetbits_32(MCUSYS_DCM_CFG0,
			MP_CPUSYS_TOP_ADB_DCM_REG2_MASK,
			MP_CPUSYS_TOP_ADB_DCM_REG2_OFF);
	}
}

#define MP_CPUSYS_TOP_APB_DCM_REG0_MASK (BIT(5))
#define MP_CPUSYS_TOP_APB_DCM_REG1_MASK (BIT(8))
#define MP_CPUSYS_TOP_APB_DCM_REG2_MASK (BIT(16))
#define MP_CPUSYS_TOP_APB_DCM_REG0_ON (BIT(5))
#define MP_CPUSYS_TOP_APB_DCM_REG1_ON (BIT(8))
#define MP_CPUSYS_TOP_APB_DCM_REG2_ON (BIT(16))
#define MP_CPUSYS_TOP_APB_DCM_REG0_OFF ((0x0 << 5))
#define MP_CPUSYS_TOP_APB_DCM_REG1_OFF ((0x0 << 8))
#define MP_CPUSYS_TOP_APB_DCM_REG2_OFF ((0x0 << 16))

bool dcm_mp_cpusys_top_apb_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(MP_MISC_DCM_CFG0) &
		MP_CPUSYS_TOP_APB_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_APB_DCM_REG0_ON);
	ret &= ((mmio_read_32(MCUSYS_DCM_CFG0) &
		MP_CPUSYS_TOP_APB_DCM_REG1_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_APB_DCM_REG1_ON);
	ret &= ((mmio_read_32(MP0_DCM_CFG0) &
		MP_CPUSYS_TOP_APB_DCM_REG2_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_APB_DCM_REG2_ON);

	return ret;
}

void dcm_mp_cpusys_top_apb_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_apb_dcm'" */
		mmio_clrsetbits_32(MP_MISC_DCM_CFG0,
			MP_CPUSYS_TOP_APB_DCM_REG0_MASK,
			MP_CPUSYS_TOP_APB_DCM_REG0_ON);
		mmio_clrsetbits_32(MCUSYS_DCM_CFG0,
			MP_CPUSYS_TOP_APB_DCM_REG1_MASK,
			MP_CPUSYS_TOP_APB_DCM_REG1_ON);
		mmio_clrsetbits_32(MP0_DCM_CFG0,
			MP_CPUSYS_TOP_APB_DCM_REG2_MASK,
			MP_CPUSYS_TOP_APB_DCM_REG2_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_apb_dcm'" */
		mmio_clrsetbits_32(MP_MISC_DCM_CFG0,
			MP_CPUSYS_TOP_APB_DCM_REG0_MASK,
			MP_CPUSYS_TOP_APB_DCM_REG0_OFF);
		mmio_clrsetbits_32(MCUSYS_DCM_CFG0,
			MP_CPUSYS_TOP_APB_DCM_REG1_MASK,
			MP_CPUSYS_TOP_APB_DCM_REG1_OFF);
		mmio_clrsetbits_32(MP0_DCM_CFG0,
			MP_CPUSYS_TOP_APB_DCM_REG2_MASK,
			MP_CPUSYS_TOP_APB_DCM_REG2_OFF);
	}
}

#define MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_MASK (BIT(11))
#define MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_ON (BIT(11))
#define MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_OFF ((0x0 << 11))

bool dcm_mp_cpusys_top_bus_pll_div_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(BUS_PLLDIV_CFG) &
		MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_bus_pll_div_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_bus_pll_div_dcm'" */
		mmio_clrsetbits_32(BUS_PLLDIV_CFG,
			MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_MASK,
			MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_bus_pll_div_dcm'" */
		mmio_clrsetbits_32(BUS_PLLDIV_CFG,
			MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_MASK,
			MP_CPUSYS_TOP_BUS_PLL_DIV_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_MASK (BIT(0))
#define MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_ON (BIT(0))
#define MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_OFF ((0x0 << 0))

bool dcm_mp_cpusys_top_core_stall_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(MP0_DCM_CFG7) &
		MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_core_stall_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_core_stall_dcm'" */
		mmio_clrsetbits_32(MP0_DCM_CFG7,
			MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_core_stall_dcm'" */
		mmio_clrsetbits_32(MP0_DCM_CFG7,
			MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CORE_STALL_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_CPUBIU_DCM_REG0_MASK ((0xffff << 0))
#define MP_CPUSYS_TOP_CPUBIU_DCM_REG0_ON ((0xffff << 0))
#define MP_CPUSYS_TOP_CPUBIU_DCM_REG0_OFF ((0x0 << 0))

bool dcm_mp_cpusys_top_cpubiu_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(MCSI_DCM0) &
		MP_CPUSYS_TOP_CPUBIU_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_CPUBIU_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_cpubiu_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_cpubiu_dcm'" */
		mmio_clrsetbits_32(MCSI_DCM0,
			MP_CPUSYS_TOP_CPUBIU_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPUBIU_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_cpubiu_dcm'" */
		mmio_clrsetbits_32(MCSI_DCM0,
			MP_CPUSYS_TOP_CPUBIU_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPUBIU_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_MASK (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_ON (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_OFF ((0x0 << 11))

bool dcm_mp_cpusys_top_cpu_pll_div_0_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(CPU_PLLDIV_CFG0) &
		MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_cpu_pll_div_0_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_cpu_pll_div_0_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG0,
			MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_cpu_pll_div_0_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG0,
			MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_0_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_MASK (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_ON (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_OFF ((0x0 << 11))

bool dcm_mp_cpusys_top_cpu_pll_div_1_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(CPU_PLLDIV_CFG1) &
		MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_cpu_pll_div_1_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_cpu_pll_div_1_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG1,
			MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_cpu_pll_div_1_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG1,
			MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_1_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_MASK (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_ON (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_OFF ((0x0 << 11))

bool dcm_mp_cpusys_top_cpu_pll_div_2_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(CPU_PLLDIV_CFG2) &
		MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_cpu_pll_div_2_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_cpu_pll_div_2_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG2,
			MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_cpu_pll_div_2_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG2,
			MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_2_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_MASK (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_ON (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_OFF ((0x0 << 11))

bool dcm_mp_cpusys_top_cpu_pll_div_3_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(CPU_PLLDIV_CFG3) &
		MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_cpu_pll_div_3_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_cpu_pll_div_3_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG3,
			MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_cpu_pll_div_3_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG3,
			MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_3_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_MASK (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_ON (BIT(11))
#define MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_OFF ((0x0 << 11))

bool dcm_mp_cpusys_top_cpu_pll_div_4_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(CPU_PLLDIV_CFG4) &
		MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_cpu_pll_div_4_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_cpu_pll_div_4_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG4,
			MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_cpu_pll_div_4_dcm'" */
		mmio_clrsetbits_32(CPU_PLLDIV_CFG4,
			MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_MASK,
			MP_CPUSYS_TOP_CPU_PLL_DIV_4_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_MASK (BIT(4))
#define MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_ON (BIT(4))
#define MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_OFF ((0x0 << 4))

bool dcm_mp_cpusys_top_fcm_stall_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(MP0_DCM_CFG7) &
		MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_fcm_stall_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_fcm_stall_dcm'" */
		mmio_clrsetbits_32(MP0_DCM_CFG7,
			MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_MASK,
			MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_fcm_stall_dcm'" */
		mmio_clrsetbits_32(MP0_DCM_CFG7,
			MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_MASK,
			MP_CPUSYS_TOP_FCM_STALL_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_MASK ((0x1U << 31))
#define MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_ON ((0x1U << 31))
#define MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_OFF ((0x0U << 31))

bool dcm_mp_cpusys_top_last_cor_idle_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(BUS_PLLDIV_CFG) &
		MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_last_cor_idle_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_last_cor_idle_dcm'" */
		mmio_clrsetbits_32(BUS_PLLDIV_CFG,
			MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_MASK,
			MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_last_cor_idle_dcm'" */
		mmio_clrsetbits_32(BUS_PLLDIV_CFG,
			MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_MASK,
			MP_CPUSYS_TOP_LAST_COR_IDLE_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_MISC_DCM_REG0_MASK (BIT(1) | \
			BIT(4))
#define MP_CPUSYS_TOP_MISC_DCM_REG0_ON (BIT(1) | \
			BIT(4))
#define MP_CPUSYS_TOP_MISC_DCM_REG0_OFF ((0x0 << 1) | \
			(0x0 << 4))

bool dcm_mp_cpusys_top_misc_dcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(MP_MISC_DCM_CFG0) &
		MP_CPUSYS_TOP_MISC_DCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_MISC_DCM_REG0_ON);

	return ret;
}

void dcm_mp_cpusys_top_misc_dcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_misc_dcm'" */
		mmio_clrsetbits_32(MP_MISC_DCM_CFG0,
			MP_CPUSYS_TOP_MISC_DCM_REG0_MASK,
			MP_CPUSYS_TOP_MISC_DCM_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_misc_dcm'" */
		mmio_clrsetbits_32(MP_MISC_DCM_CFG0,
			MP_CPUSYS_TOP_MISC_DCM_REG0_MASK,
			MP_CPUSYS_TOP_MISC_DCM_REG0_OFF);
	}
}

#define MP_CPUSYS_TOP_MP0_QDCM_REG0_MASK (BIT(3))
#define MP_CPUSYS_TOP_MP0_QDCM_REG1_MASK (BIT(0) | \
			BIT(1) | \
			BIT(2) | \
			BIT(3))
#define MP_CPUSYS_TOP_MP0_QDCM_REG0_ON (BIT(3))
#define MP_CPUSYS_TOP_MP0_QDCM_REG1_ON (BIT(0) | \
			BIT(1) | \
			BIT(2) | \
			BIT(3))
#define MP_CPUSYS_TOP_MP0_QDCM_REG0_OFF ((0x0 << 3))
#define MP_CPUSYS_TOP_MP0_QDCM_REG1_OFF ((0x0 << 0) | \
			(0x0 << 1) | \
			(0x0 << 2) | \
			(0x0 << 3))

bool dcm_mp_cpusys_top_mp0_qdcm_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(MP_MISC_DCM_CFG0) &
		MP_CPUSYS_TOP_MP0_QDCM_REG0_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_MP0_QDCM_REG0_ON);
	ret &= ((mmio_read_32(MP0_DCM_CFG0) &
		MP_CPUSYS_TOP_MP0_QDCM_REG1_MASK) ==
		(unsigned int) MP_CPUSYS_TOP_MP0_QDCM_REG1_ON);

	return ret;
}

void dcm_mp_cpusys_top_mp0_qdcm(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'mp_cpusys_top_mp0_qdcm'" */
		mmio_clrsetbits_32(MP_MISC_DCM_CFG0,
			MP_CPUSYS_TOP_MP0_QDCM_REG0_MASK,
			MP_CPUSYS_TOP_MP0_QDCM_REG0_ON);
		mmio_clrsetbits_32(MP0_DCM_CFG0,
			MP_CPUSYS_TOP_MP0_QDCM_REG1_MASK,
			MP_CPUSYS_TOP_MP0_QDCM_REG1_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'mp_cpusys_top_mp0_qdcm'" */
		mmio_clrsetbits_32(MP_MISC_DCM_CFG0,
			MP_CPUSYS_TOP_MP0_QDCM_REG0_MASK,
			MP_CPUSYS_TOP_MP0_QDCM_REG0_OFF);
		mmio_clrsetbits_32(MP0_DCM_CFG0,
			MP_CPUSYS_TOP_MP0_QDCM_REG1_MASK,
			MP_CPUSYS_TOP_MP0_QDCM_REG1_OFF);
	}
}

#define CPCCFG_REG_EMI_WFIFO_REG0_MASK (BIT(0) | \
			BIT(1) | \
			BIT(2) | \
			BIT(3))
#define CPCCFG_REG_EMI_WFIFO_REG0_ON (BIT(0) | \
			BIT(1) | \
			BIT(2) | \
			BIT(3))
#define CPCCFG_REG_EMI_WFIFO_REG0_OFF ((0x0 << 0) | \
			(0x0 << 1) | \
			(0x0 << 2) | \
			(0x0 << 3))

bool dcm_cpccfg_reg_emi_wfifo_is_on(void)
{
	bool ret = true;

	ret &= ((mmio_read_32(EMI_WFIFO) &
		CPCCFG_REG_EMI_WFIFO_REG0_MASK) ==
		(unsigned int) CPCCFG_REG_EMI_WFIFO_REG0_ON);

	return ret;
}

void dcm_cpccfg_reg_emi_wfifo(bool on)
{
	if (on) {
		/* TINFO = "Turn ON DCM 'cpccfg_reg_emi_wfifo'" */
		mmio_clrsetbits_32(EMI_WFIFO,
			CPCCFG_REG_EMI_WFIFO_REG0_MASK,
			CPCCFG_REG_EMI_WFIFO_REG0_ON);
	} else {
		/* TINFO = "Turn OFF DCM 'cpccfg_reg_emi_wfifo'" */
		mmio_clrsetbits_32(EMI_WFIFO,
			CPCCFG_REG_EMI_WFIFO_REG0_MASK,
			CPCCFG_REG_EMI_WFIFO_REG0_OFF);
	}
}

