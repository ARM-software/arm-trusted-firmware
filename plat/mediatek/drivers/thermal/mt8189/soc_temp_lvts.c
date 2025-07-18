/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/mmio_poll.h>
#include <lib/utils_def.h>

#include <lib/mtk_init/mtk_init.h>
#include <lpm/mt_lp_api.h>
#include <mt_spm_reg.h>
#include <mtk_mmap_pool.h>
#include <soc_temp_lvts.h>
#include <soc_temp_lvts_interface.h>
#include <thermal_lvts.h>

enum mt8189_lvts_domain {
	MT8189_AP_DOMAIN,
	MT8189_MCU_DOMAIN,
	MT8189_NUM_DOMAIN
};

enum mt8189_lvts_controller_enum {
	MT8189_LVTS_MCU_CTRL0,
	MT8189_LVTS_MCU_CTRL1,
	MT8189_LVTS_MCU_CTRL2,
	MT8189_LVTS_AP_CTRL0,
	MT8189_LVTS_GPU_CTRL0,
	MT8189_LVTS_CTRL_NUM
};

#define THERMAL_CSRAM_BASE_MAP_ADDR  (THERMAL_CSRAM_BASE & ~(PAGE_SIZE - 1))
static const mmap_region_t thermal_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(THERM_CTRL_AP_BASE, THERM_AP_REG_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(THERM_CTRL_MCU_BASE, THERM_MCU_REG_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(THERMAL_CSRAM_BASE_MAP_ADDR, PAGE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(INFRACFG_BASE, PAGE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(thermal_mmap);

void disable_hw_reboot_interrupt(struct lvts_data *lvts_data,
				 unsigned int tc_id)
{
	uint32_t temp;
	uintptr_t base;

	base = GET_BASE_ADDR(lvts_data, tc_id);

	/* LVTS thermal controller has two interrupts for thermal HW reboot
	 * One is for AP SW and the other is for RGU
	 * The interrupt of AP SW can turn off by a bit of a register, but
	 * the other for RGU cannot.
	 * To prevent rebooting device accidentally, we are going to add
	 * a huge offset to LVTS and make LVTS always report extremely low
	 * temperature.
	 */

	/* After adding the huge offset 0x3FFF, LVTS alawys adds the
	 * offset to MSR_RAW.
	 * When MSR_RAW is larger, SW will convert lower temperature/
	 */
	mmio_setbits_32(LVTSPROTCTL_0 + base, 0x3FFF);

	/* Disable the interrupt of AP SW */
	temp = mmio_read_32(LVTSMONINT_0 + base);

	temp = temp & ~(STAGE3_INT_EN);

	if (lvts_data->enable_dump_log) {
		temp = temp & ~(HIGH_OFFSET3_INT_EN |
				HIGH_OFFSET2_INT_EN |
				HIGH_OFFSET1_INT_EN |
				HIGH_OFFSET0_INT_EN);

		temp = temp & ~(LOW_OFFSET3_INT_EN |
				LOW_OFFSET2_INT_EN |
				LOW_OFFSET1_INT_EN |
				LOW_OFFSET0_INT_EN);
	}

	mmio_write_32(LVTSMONINT_0 + base, temp);
	dmbsy();
}

void enable_hw_reboot_interrupt(struct lvts_data *lvts_data,
				unsigned int tc_id)
{
	uint32_t temp;
	uintptr_t base;

	base = GET_BASE_ADDR(lvts_data, tc_id);

	/* Enable the interrupt of AP SW */
	temp = mmio_read_32(LVTSMONINT_0 + base);

	if (lvts_data->enable_dump_log) {
		temp = temp | HIGH_OFFSET3_INT_EN |
			      HIGH_OFFSET2_INT_EN |
			      HIGH_OFFSET1_INT_EN |
			      HIGH_OFFSET0_INT_EN;

		temp = temp | LOW_OFFSET3_INT_EN |
			      LOW_OFFSET2_INT_EN |
			      LOW_OFFSET1_INT_EN |
			      LOW_OFFSET0_INT_EN;
	} else {
		temp = temp | STAGE3_INT_EN;
	}

	mmio_write_32(LVTSMONINT_0 + base, temp);

	/* Clear the offset */
	mmio_clrbits_32(LVTSPROTCTL_0 + base, PROTOFFSET);

	dmbsy();
}

void set_tc_hw_reboot_threshold(struct lvts_data *lvts_data, int trip_point,
				unsigned int tc_id)
{
	struct tc_settings *tc = lvts_data->tc;
	unsigned int msr_raw, cur_msr_raw, d_index, i;
	uint32_t temp, config;
	uintptr_t base;
	struct platform_ops *ops = &lvts_data->ops;

	msr_raw = 0;
	base = GET_BASE_ADDR(lvts_data, tc_id);
	d_index = get_dominator_index(lvts_data, tc_id);

	disable_hw_reboot_interrupt(lvts_data, tc_id);

	if (tc_id < MT8189_LVTS_CTRL_NUM) {
		temp = mmio_read_32(LVTSPROTCTL_0 + base);
		if (d_index == ALL_SENSING_POINTS) {
			/* Maximum of 4 sensing points */
			config = (0x1 << 16);
			mmio_write_32(LVTSPROTCTL_0 + base, config | temp);
			msr_raw = 0;
			for (i = 0; i < tc[tc_id].num_sensor; i++) {
				cur_msr_raw = ops->lvts_temp_to_raw(
					&(tc[tc_id].coeff), i, trip_point);
				if (msr_raw < cur_msr_raw)
					msr_raw = cur_msr_raw;
			}
		} else {
			/* Select protection sensor */
			config = ((d_index << 2) + 0x2) << 16;
			mmio_write_32(LVTSPROTCTL_0 + base, config | temp);
			msr_raw = ops->lvts_temp_to_raw(
				&(tc[tc_id].coeff), d_index, trip_point);
		}
	} else {
		INFO("Error: tc_id %d over %d\n", tc_id, MT8189_LVTS_CTRL_NUM);
	}

	if (lvts_data->enable_dump_log) {
		/* high offset INT */
		mmio_write_32(LVTSOFFSETH_0 + base, msr_raw);

		/*
		 * lowoffset INT
		 * set a big msr_raw = 0xffff(very low temperature)
		 * to let lowoffset INT not be triggered
		 */
		mmio_write_32(LVTSOFFSETL_0 + base, 0xffff);
	} else {
		mmio_write_32(LVTSPROTTC_0 + base, msr_raw);
	}

	dmbsy();

	enable_hw_reboot_interrupt(lvts_data, tc_id);
}

#define LVTS_COEFF_A_X_1000         (-250460)
#define LVTS_COEFF_B_X_1000          (250460)

static int lvts_raw_to_temp_v1(const struct formula_coeff *co,
			       uint32_t sensor_id, uint32_t msr_raw)
{
	/* This function returns degree mC
	 * temp[i] = a * MSR_RAW/16384 + GOLDEN_TEMP/2 + b
	 * a = -250.46
	 * b =  250.46
	 */
	(void)sensor_id;
	int temp_mC = 0;
	int temp1 = 0;
	int golden_temp = co ? (int)co->golden_temp : DEFAULT_EFUSE_GOLDEN_TEMP;

	temp1 = ((int)LVTS_COEFF_A_X_1000 * ((int)msr_raw)) >> 14;

	temp_mC = temp1 + golden_temp * 500 + LVTS_COEFF_B_X_1000;

	return temp_mC;
}

static uint32_t lvts_temp_to_raw_v1(const struct formula_coeff *co,
				    uint32_t sensor_id, int temp)
{
	/* MSR_RAW = ((temp[i] - GOLDEN_TEMP/2 - b) * 16384) / a
	 * a = -250.46
	 * b =  250.46
	 */
	(void)sensor_id;
	uint32_t msr_raw = 0;
	int golden_temp = co ? (int)co->golden_temp : DEFAULT_EFUSE_GOLDEN_TEMP;

	msr_raw = (uint32_t)(
		((long long)(
		(golden_temp * 500) + LVTS_COEFF_B_X_1000 - temp) << 14) /
		(-1 * LVTS_COEFF_A_X_1000));
	msr_raw = msr_raw & 0xFFFF;

	return msr_raw;
}

static void mt8189_get_calibration_data(struct lvts_data *lvts_data)
{
	lvts_data->efuse[0] = mmio_read_32(EFUSEC_BASE + 0x01A4);
	lvts_data->efuse[1] = mmio_read_32(EFUSEC_BASE + 0x01A8);
	lvts_data->efuse[2] = mmio_read_32(EFUSEC_BASE + 0x01AC);
	lvts_data->efuse[3] = mmio_read_32(EFUSEC_BASE + 0x01B0);
	lvts_data->efuse[4] = mmio_read_32(EFUSEC_BASE + 0x01B4);
	lvts_data->efuse[5] = mmio_read_32(EFUSEC_BASE + 0x01B8);
	lvts_data->efuse[6] = mmio_read_32(EFUSEC_BASE + 0x01BC);
	lvts_data->efuse[7] = mmio_read_32(EFUSEC_BASE + 0x01C0);
	lvts_data->efuse[8] = mmio_read_32(EFUSEC_BASE + 0x01C4);
	lvts_data->efuse[9] = mmio_read_32(EFUSEC_BASE + 0x01C8);
	lvts_data->efuse[10] = mmio_read_32(EFUSEC_BASE + 0x01CC);
	lvts_data->efuse[11] = mmio_read_32(EFUSEC_BASE + 0x01D0);
	lvts_data->efuse[12] = mmio_read_32(EFUSEC_BASE + 0x01D4);
	lvts_data->efuse[13] = mmio_read_32(EFUSEC_BASE + 0x01D8);
	lvts_data->efuse[14] = mmio_read_32(EFUSEC_BASE + 0x01DC);

	lvts_data->efuse[15] = mmio_read_32(EFUSEC_BASE + 0x01E0);
	lvts_data->efuse[16] = mmio_read_32(EFUSEC_BASE + 0x01E4);
	lvts_data->efuse[17] = mmio_read_32(EFUSEC_BASE + 0x01E8);
	lvts_data->efuse[18] = mmio_read_32(EFUSEC_BASE + 0x01EC);
	lvts_data->efuse[19] = mmio_read_32(EFUSEC_BASE + 0x01F0);
	lvts_data->efuse[20] = mmio_read_32(EFUSEC_BASE + 0x01F4);
	lvts_data->efuse[21] = mmio_read_32(EFUSEC_BASE + 0x01F8);
	lvts_data->efuse[22] = mmio_read_32(EFUSEC_BASE + 0x01FC);
}

static void init_controller_v1(struct lvts_data *lvts_data)
{
	unsigned int i;
	uintptr_t base;
	struct tc_settings *tc = lvts_data->tc;

	assert(lvts_data->num_tc <= MT8189_LVTS_CTRL_NUM);
	lvts_write_all_device(lvts_data, SET_DEVICE_LOW_POWER_SINGLE_MODE_V1);
	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);

		mmio_write_32(LVTSTSSEL_0 + base, 0x13121110);
		mmio_write_32(LVTSCALSCALE_0 + base, SET_CALC_SCALE_RULES);
		dmbsy();

		set_polling_speed(lvts_data, i);
		set_hw_filter(lvts_data, i);
	}
}

static void mt8189_device_enable_and_init(struct lvts_data *lvts_data)
{
	unsigned int i;
	struct tc_settings *tc = lvts_data->tc;

	assert(lvts_data->num_tc <= MT8189_LVTS_CTRL_NUM);
	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		lvts_write_device(lvts_data, 0xC1030300, i);
		lvts_write_device(lvts_data, 0xC10307A6, i);
		lvts_write_device(lvts_data, 0xC1030500, i);
		lvts_write_device(lvts_data, 0xC1030420, i);
		lvts_write_device(lvts_data, 0xC1030A8C, i);
		lvts_write_device(lvts_data, 0xC1030CFC, i);
		lvts_write_device(lvts_data, 0xC103098D, i);
		lvts_write_device(lvts_data, 0xC10308F1, i);
	}
}

static int mt8189_device_read_count_rc_n(struct lvts_data *lvts_data)
{
	/* Resistor-Capacitor Calibration */
	/* count_RC_N: count RC now */
	struct tc_settings *tc = lvts_data->tc;
	struct sensor_cal_data *cal_data = &lvts_data->cal_data;
	unsigned int i, j, s_index;
	uintptr_t base;
	int ret;
	uint32_t temp, data, rc_data;
	uint32_t refine_data_idx[4] = {0};
	uint32_t count_rc_delta = 0;

	lvts_write_all_device(lvts_data, SET_LVTS_MANUAL_RCK_V1);
	if (lvts_data->init_done == true) {
		lvts_write_all_device(lvts_data, SET_SENSOR_NO_RCK_V1);
		lvts_write_all_device(lvts_data,
				      SET_DEVICE_LOW_POWER_SINGLE_MODE_V1);
		return 0;
	}

	assert(lvts_data->num_tc <= MT8189_LVTS_CTRL_NUM);
	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);
		lvts_write_device(lvts_data, SET_LVTS_MANUAL_RCK_OPERATION_8189,
				  i);

		for (j = 0; j < tc[i].num_sensor; j++) {
			if (j >= ALL_SENSING_POINTS)
				break;

			if (tc[i].sensor_on_off[j] != SEN_ON)
				continue;

			refine_data_idx[j] = 0xff;
			s_index = tc[i].sensor_map[j];

			lvts_write_device(lvts_data, SELECT_SENSOR_RCK_V1(j),
					  i);
			lvts_write_device(lvts_data,
					  SET_DEVICE_SINGLE_MODE_8189, i);
			lvts_write_device(lvts_data, SET_TS_DIV_EN_8189, i);
			lvts_write_device(lvts_data, SET_VCO_RST_8189, i);
			lvts_write_device(lvts_data, SET_TS_DIV_EN_8189, i);
			udelay(20);

			lvts_write_device(lvts_data, KICK_OFF_RCK_COUNTING_V1,
					  i);

			ret = mmio_read_32_poll_timeout(
				LVTS_CONFIG_0 + base,
				temp,
				((temp & DEVICE_SENSING_STATUS) == 0),
				20);
			if (ret)
				INFO("Error: %d DEVICE_SENSING_STATUS timeout\n"
				     , i);

			lvts_write_device(lvts_data, SET_TS_DIS_8189, i);

			/* wait for sensor 0*/
			udelay(50 * (j == 0));

			data = lvts_read_device(lvts_data, 0x00, i);
			/* wait 5us buffer */
			udelay(5);

			rc_data = (data & GENMASK(23, 0));

			if (rc_data == 0) {
				refine_data_idx[j] = s_index;
			} else {
				if (cal_data->count_rc[i] > rc_data)
					count_rc_delta = (cal_data->count_rc[i]
						* 1000) / rc_data;
				else
					count_rc_delta = (rc_data * 1000)
						/ cal_data->count_rc[i];

				if (count_rc_delta > 1061) {
					refine_data_idx[j] = s_index;
				}
			}

			ret = mmio_read_32_poll_timeout(
				LVTS_CONFIG_0 + base,
				temp,
				((temp & DEVICE_ACCESS_STARTUS) == 0),
				20);
			if (ret)
				INFO("Error: %d DEVICE_ACCESS_START timeout\n",
				     i);

			if (s_index < MT8189_NUM_TS) {
				if (refine_data_idx[j] != 0xff) {
					cal_data->count_rc_now[s_index] =
						cal_data->count_rc[i];
				} else {
					cal_data->count_rc_now[s_index] =
						rc_data & GENMASK(23, 0);
				}
			}

			/* count data here that want to set to efuse later */
			cal_data->efuse_data[s_index] = (uint32_t)(((
				(unsigned long long)
				cal_data->count_rc_now[s_index]) *
				cal_data->count_r[s_index]) >> 14);
			lvts_write_device(lvts_data,
					  (0x10 | SELECT_SENSOR_RCK_V1(j)), i);
		}

		/* Recover Setting for Normal Access on
		 * temperature fetch
		 */
		lvts_write_device(lvts_data, SET_SENSOR_NO_RCK_V1, i);
		lvts_write_device(lvts_data,
				  SET_DEVICE_LOW_POWER_SINGLE_MODE_V1, i);
	}

	return 0;
}

static void mt8189_lvts_reset(struct lvts_data *lvts_data)
{
	unsigned int i;
	uint32_t temp;

	for (i = 0; i < lvts_data->num_domain; i++) {
		if (i < MT8189_NUM_DOMAIN) {
			temp = (uint32_t)BIT(
				lvts_data->domain[i].reset_set_bitnum);
			mmio_write_32((lvts_data->domain[i].reset_base +
				      lvts_data->domain[i].reset_set_offset),
				      temp);

			udelay(1);

			temp = (uint32_t)BIT(
				lvts_data->domain[i].reset_clr_bitnum);
			mmio_write_32((lvts_data->domain[i].reset_base +
				      lvts_data->domain[i].reset_clr_offset),
				      temp);
		}
	}

}

static void mt8189_device_identification(struct lvts_data *lvts_data)
{
	unsigned int i;
	uintptr_t base;
	uint32_t data, lvts_dev_id;
	struct tc_settings *tc = lvts_data->tc;

	assert(lvts_data->num_tc <= MT8189_LVTS_CTRL_NUM);
	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);

		mmio_write_32(LVTSCLKEN_0 + base, ENABLE_LVTS_CTRL_CLK);
	}

	lvts_write_all_device(lvts_data, RESET_ALL_DEVICES);
	lvts_write_all_device(lvts_data, READ_BACK_DEVICE_ID);

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);
		/* Check LVTS device ID */
		data = mmio_read_32(LVTS_ID_0 + base) & GENMASK(7, 0);

		lvts_dev_id = 0x81 + i;

		if (data != lvts_dev_id) {
			INFO("LVTS_TC_%d, Device ID should be 0x%x, but 0x%x\n",
			     i, lvts_dev_id, data);
		}
	}
}

static void mt8189_efuse_to_cal_data(struct lvts_data *lvts_data)
{
	struct sensor_cal_data *cal_data = &lvts_data->cal_data;
	struct tc_settings *tc = lvts_data->tc;
	unsigned int i;

	cal_data->golden_temp = GET_CAL_BITMASK(lvts_data, 0, 7, 0);

	assert(lvts_data->num_tc <= MT8189_LVTS_CTRL_NUM);
	for (i = 0; i < lvts_data->num_tc; i++)
		tc[i].coeff.golden_temp = cal_data->golden_temp;

	cal_data->count_r[MT8189_TS1_0] = GET_CAL_BITMASK(lvts_data, 1, 23, 0);
	cal_data->count_r[MT8189_TS1_1] = GET_CAL_BITMASK(lvts_data, 2, 23, 0);
	cal_data->count_r[MT8189_TS1_2] = GET_CAL_BITMASK(lvts_data, 3, 23, 0);
	cal_data->count_r[MT8189_TS1_3] = GET_CAL_BITMASK(lvts_data, 4, 23, 0);

	cal_data->count_r[MT8189_TS2_0] = GET_CAL_BITMASK(lvts_data, 6, 23, 0);
	cal_data->count_r[MT8189_TS2_1] = GET_CAL_BITMASK(lvts_data, 7, 23, 0);
	cal_data->count_r[MT8189_TS2_2] = GET_CAL_BITMASK(lvts_data, 8, 23, 0);
	cal_data->count_r[MT8189_TS2_3] = GET_CAL_BITMASK(lvts_data, 9, 23, 0);

	cal_data->count_r[MT8189_TS3_0] = GET_CAL_BITMASK(lvts_data, 11, 23, 0);
	cal_data->count_r[MT8189_TS3_1] = GET_CAL_BITMASK(lvts_data, 12, 23, 0);
	cal_data->count_r[MT8189_TS3_2] = GET_CAL_BITMASK(lvts_data, 13, 23, 0);
	cal_data->count_r[MT8189_TS3_3] = GET_CAL_BITMASK(lvts_data, 14, 23, 0);

	cal_data->count_r[MT8189_TS4_0] = GET_CAL_BITMASK(lvts_data, 16, 23, 0);
	cal_data->count_r[MT8189_TS4_1] = GET_CAL_BITMASK(lvts_data, 17, 23, 0);
	cal_data->count_r[MT8189_TS4_2] = GET_CAL_BITMASK(lvts_data, 18, 23, 0);
	cal_data->count_r[MT8189_TS4_3] = GET_CAL_BITMASK(lvts_data, 19, 23, 0);

	cal_data->count_r[MT8189_TS5_0] = GET_CAL_BITMASK(lvts_data, 21, 23, 0);
	cal_data->count_r[MT8189_TS5_1] = GET_CAL_BITMASK(lvts_data, 22, 23, 0);

	cal_data->count_rc[MT8189_LVTS_MCU_CTRL0] =
		GET_CAL_BITMASK(lvts_data, 0, 31, 8);
	cal_data->count_rc[MT8189_LVTS_MCU_CTRL1] =
		GET_CAL_BITMASK(lvts_data, 5, 23, 0);
	cal_data->count_rc[MT8189_LVTS_MCU_CTRL2] =
		GET_CAL_BITMASK(lvts_data, 10, 23, 0);

	cal_data->count_rc[MT8189_LVTS_AP_CTRL0] =
		GET_CAL_BITMASK(lvts_data, 15, 23, 0);
	cal_data->count_rc[MT8189_LVTS_GPU_CTRL0] =
		GET_CAL_BITMASK(lvts_data, 20, 23, 0);
}

static void mt8189_check_cal_data(struct lvts_data *lvts_data)
{
	struct sensor_cal_data *cal_data = &lvts_data->cal_data;
	struct tc_settings *tc = lvts_data->tc;
	unsigned int i, j;
	bool efuse_calibrated = false;

	assert(lvts_data->num_sensor <= MT8189_NUM_TS);
	assert(lvts_data->num_tc <= MT8189_LVTS_CTRL_NUM);
	if ((cal_data->golden_temp != 0) || (cal_data->golden_temp_ht != 0)) {
		efuse_calibrated = true;
	} else {
		for (i = 0; i < lvts_data->num_sensor; i++) {
			if (cal_data->count_r[i] != 0) {
				efuse_calibrated = true;
				break;
			}
		}
		if (!efuse_calibrated) {
			for (j = 0; j < lvts_data->num_tc; j++) {
				if (cal_data->count_rc[j] != 0) {
					efuse_calibrated = true;
					break;
				}
			}
		}
	}

	if (!efuse_calibrated) {
		/* It means all efuse data are equal to 0 */
		INFO("[lvts_cal] This sample is not calibrated, fake !!\n");
		for (i = 0; i < lvts_data->num_sensor; i++)
			cal_data->count_r[i] = cal_data->default_count_r;

		for (i = 0; i < lvts_data->num_tc; i++)
			cal_data->count_rc[i] = cal_data->default_count_rc;

		cal_data->golden_temp = cal_data->default_golden_temp;

		for (i = 0; i < lvts_data->num_tc; i++)
			tc[i].coeff.golden_temp = cal_data->default_golden_temp;
	}
}

static void mt8189_update_coef_data(struct lvts_data *lvts_data)
{
	struct sensor_cal_data *cal_data = &lvts_data->cal_data;
	struct tc_settings *tc = lvts_data->tc;
	unsigned int i, j, s_index;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		for  (j = 0; j < tc[i].num_sensor; j++) {
			if (tc[i].sensor_on_off[j] != SEN_ON)
				continue;

			s_index = tc[i].sensor_map[j];
			tc[i].coeff.a[j] = COF_A_OFS + (COF_A_CONST_OFS *
					(int)(cal_data->count_r[s_index])
					/ COF_A_COUNT_R_GLD);
		}
	}
}

static struct power_domain mt8189_domain_settings[MT8189_NUM_DOMAIN] = {
	[MT8189_AP_DOMAIN] = {
		.base = THERM_CTRL_AP_BASE,
		.reset_base = INFRACFG_BASE,
		.reset_set_offset = THERM_AP_RESET_SET_OFFSET,
		.reset_clr_offset = THERM_AP_RESET_CLR_OFFSET,
		.reset_set_bitnum = THERM_AP_RESET_SET_BITNUM,
		.reset_clr_bitnum = THERM_AP_RESET_CLR_BITNUM,
	},
	[MT8189_MCU_DOMAIN] = {
		.base = THERM_CTRL_MCU_BASE,
		.reset_base = INFRACFG_BASE,
		.reset_set_offset = THERM_MCU_RESET_SET_OFFSET,
		.reset_clr_offset = THERM_MCU_RESET_CLR_OFFSET,
		.reset_set_bitnum = THERM_MCU_RESET_SET_BITNUM,
		.reset_clr_bitnum = THERM_MCU_RESET_CLR_BITNUM,
	},
};

static struct tc_settings mt8189_tc_settings[MT8189_LVTS_CTRL_NUM] = {
	[MT8189_LVTS_MCU_CTRL0] = {
		.domain_index = MT8189_MCU_DOMAIN,
		.addr_offset = 0x0,
		.num_sensor = 4,
		.sensor_map = {MT8189_TS1_0, MT8189_TS1_1, MT8189_TS1_2,
			MT8189_TS1_3},
		.ctrl_on_off = CTRL_ON,
		.sensor_on_off = {SEN_ON, SEN_ON, SEN_ON, SEN_ON},
		.tc_speed = SET_TC_SPEED_IN_US(10, 2460, 10, 10),
		.hw_filter = LVTS_FILTER_1,
		.dominator_sensing_point = ALL_SENSING_POINTS,
		.hw_reboot_trip_point = THERMAL_REBOOT_TEMPERATURE,
		.irq_bit = BIT(1),
		.coeff = {
			.cali_mode = CALI_NT,
		},
	},
	[MT8189_LVTS_MCU_CTRL1] = {
		.domain_index = MT8189_MCU_DOMAIN,
		.addr_offset = 0x100,
		.num_sensor = 4,
		.sensor_map = {MT8189_TS2_0, MT8189_TS2_1, MT8189_TS2_2,
			MT8189_TS2_3},
		.ctrl_on_off = CTRL_ON,
		.sensor_on_off = {SEN_ON, SEN_ON, SEN_ON, SEN_ON},
		.tc_speed = SET_TC_SPEED_IN_US(10, 2460, 10, 10),
		.hw_filter = LVTS_FILTER_1,
		.dominator_sensing_point = ALL_SENSING_POINTS,
		.hw_reboot_trip_point = THERMAL_REBOOT_TEMPERATURE,
		.irq_bit = BIT(2),
		.coeff = {
			.cali_mode = CALI_NT,
		},
	},
	[MT8189_LVTS_MCU_CTRL2] = {
		.domain_index = MT8189_MCU_DOMAIN,
		.addr_offset = 0x200,
		.num_sensor = 4,
		.sensor_map = {MT8189_TS3_0, MT8189_TS3_1, MT8189_TS3_2,
			MT8189_TS3_3},
		.ctrl_on_off = CTRL_ON,
		.sensor_on_off = {SEN_ON, SEN_ON, SEN_ON, SEN_ON},
		.tc_speed = SET_TC_SPEED_IN_US(10, 4490, 10, 10),
		.hw_filter = LVTS_FILTER_1,
		.dominator_sensing_point = ALL_SENSING_POINTS,
		.hw_reboot_trip_point = THERMAL_REBOOT_TEMPERATURE,
		.irq_bit = BIT(3),
		.coeff = {
			.cali_mode = CALI_NT,
		},
	},
	[MT8189_LVTS_AP_CTRL0] = {
		.domain_index = MT8189_AP_DOMAIN,
		.addr_offset = 0x0,
		.num_sensor = 4,
		.sensor_map = {MT8189_TS4_0, MT8189_TS4_1, MT8189_TS4_2,
			MT8189_TS4_3},
		.ctrl_on_off = CTRL_ON,
		.sensor_on_off = {SEN_ON, SEN_ON, SEN_ON, SEN_ON},
		.tc_speed = SET_TC_SPEED_IN_US(10, 2100, 10, 10),
		.hw_filter = LVTS_FILTER_1,
		.dominator_sensing_point = ALL_SENSING_POINTS,
		.hw_reboot_trip_point = THERMAL_REBOOT_TEMPERATURE,
		.irq_bit = BIT(1),
		.coeff = {
			.cali_mode = CALI_NT,
		},
	},
	[MT8189_LVTS_GPU_CTRL0] = {
		.domain_index = MT8189_AP_DOMAIN,
		.addr_offset = 0x100,
		.num_sensor = 2,
		.sensor_map = {MT8189_TS5_0, MT8189_TS5_1},
		.ctrl_on_off = CTRL_ON,
		.sensor_on_off = {SEN_ON, SEN_ON},
		.tc_speed = SET_TC_SPEED_IN_US(10, 1380, 10, 10),
		.hw_filter = LVTS_FILTER_1,
		.dominator_sensing_point = ALL_SENSING_POINTS,
		.hw_reboot_trip_point = THERMAL_REBOOT_TEMPERATURE,
		.irq_bit = BIT(2),
		.coeff = {
			.cali_mode = CALI_NT,
		},
	},
};

static struct sensor_data mt8189_sensor_data[MT8189_NUM_TS];
static uint32_t mt8189_efuse[NUM_EFUSE_ADDR];
static uint32_t count_r[MT8189_NUM_TS];
static uint32_t count_rc[MT8189_LVTS_CTRL_NUM];
static uint32_t count_rc_now[MT8189_NUM_TS];
static uint32_t efuse_data[MT8189_NUM_TS];
static uint32_t irq_bitmap[MT8189_NUM_DOMAIN];

struct lvts_data lvts_data_instance = {
	.num_domain = MT8189_NUM_DOMAIN,
	.domain = mt8189_domain_settings,
	.num_tc = MT8189_LVTS_CTRL_NUM,
	.tc = mt8189_tc_settings,
	.num_sensor = MT8189_NUM_TS,
	.sen_data = mt8189_sensor_data,
	.ops = {
		.lvts_reset = mt8189_lvts_reset,
		.device_identification = mt8189_device_identification,
		.get_calibration_data = mt8189_get_calibration_data,
		.efuse_to_cal_data = mt8189_efuse_to_cal_data,
		.device_enable_and_init = mt8189_device_enable_and_init,
		.device_enable_auto_rck = NULL,
		.device_read_count_rc_n = mt8189_device_read_count_rc_n,
		.set_cal_data = set_calibration_data_v1,
		.init_controller = init_controller_v1,
		.lvts_temp_to_raw = lvts_temp_to_raw_v1,
		.lvts_raw_to_temp = lvts_raw_to_temp_v1,
		.check_cal_data = mt8189_check_cal_data,
		.update_coef_data = mt8189_update_coef_data,
	},
	.feature_bitmap = 0,
	.num_efuse_addr = NUM_EFUSE_ADDR,
	.efuse = mt8189_efuse,
	.cal_data = {
		.default_golden_temp = DEFAULT_EFUSE_GOLDEN_TEMP,
		.default_golden_temp_ht = DEFAULT_EFUSE_GOLDEN_TEMP_HT,
		.default_count_r = DEFAULT_EFUSE_COUNT,
		.default_count_rc = DEFAULT_EFUSE_COUNT_RC,

		.count_r = count_r,
		.count_rc = count_rc,
		.count_rc_now = count_rc_now,
		.efuse_data = efuse_data,
	},
	.irq_bitmap = irq_bitmap,
	.enable_dump_log = 0,
};

void thermal_clock_open(void)
{
	mmio_setbits_32(THERM_MODULE_SW_CG_0_CLR, BIT(THERM_BIT));
	dmbsy();
}

void thermal_clock_close(void)
{
	mmio_setbits_32(THERM_MODULE_SW_CG_0_SET, BIT(THERM_BIT));
	dmbsy();
}
