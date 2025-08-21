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
#include <mtk_mmap_pool.h>
#include <soc_temp_lvts_interface.h>
#include <thermal_lvts.h>

static void lvts_write_device_nodelay(struct lvts_data *lvts_data,
				      uint32_t data, unsigned int tc_id)
{
	uintptr_t base;

	base = GET_BASE_ADDR(lvts_data, tc_id);

	mmio_write_32(LVTS_CONFIG_0 + base, data);
}

static void lvts_write_data_check(struct lvts_data *lvts_data, uint32_t data,
				  unsigned int tc_id)
{
	uintptr_t base;
	int ret;
	uint32_t temp;

	base = GET_BASE_ADDR(lvts_data, tc_id);

	ret = mmio_read_32_poll_timeout(
		LVTS_CONFIG_0 + base,
		temp,
		((temp & (0x1 << DEVICE_ACCESS_START_BIT)) == 0),
		20);

	if (ret)
		INFO("write device err: LVTS %d didn't ready, data 0x%x\n",
		     tc_id, data);
}

void lvts_write_all_device(struct lvts_data *lvts_data, uint32_t data)
{
	unsigned int tc_id;
	struct tc_settings *tc = lvts_data->tc;

	for (tc_id = 0; tc_id < lvts_data->num_tc; tc_id++) {
		if (tc[tc_id].ctrl_on_off == CTRL_OFF)
			continue;

		lvts_write_device_nodelay(lvts_data, data, tc_id);
	}
	dmbsy();
	for (tc_id = 0; tc_id < lvts_data->num_tc; tc_id++) {
		if (tc[tc_id].ctrl_on_off == CTRL_OFF)
			continue;

		lvts_write_data_check(lvts_data, data, tc_id);
	}
}

void lvts_write_device(struct lvts_data *lvts_data, uint32_t data,
		       unsigned int tc_id)
{
	uintptr_t base;
	int ret;
	uint32_t temp;

	base = GET_BASE_ADDR(lvts_data, tc_id);

	mmio_write_32(LVTS_CONFIG_0 + base, data);
	dmbsy();

	udelay(5);

	ret = mmio_read_32_poll_timeout(
		LVTS_CONFIG_0 + base,
		temp,
		((temp & (0x1 << DEVICE_ACCESS_START_BIT)) == 0),
		20);

	if (ret)
		INFO("write device err: LVTS %d didn't ready, data 0x%x\n",
		     tc_id, data);

}

uint32_t lvts_read_device(struct lvts_data *lvts_data, uint32_t reg_idx,
			  unsigned int tc_id)
{
	uintptr_t base;
	uint32_t temp, data;
	uint32_t write_val = READ_DEVICE_REG(reg_idx);
	int ret;

	base = GET_BASE_ADDR(lvts_data, tc_id);
	mmio_write_32(LVTS_CONFIG_0 + base, write_val);
	dmbsy();

	ret = mmio_read_32_poll_timeout(
		LVTS_CONFIG_0 + base,
		temp,
		((temp & (0x1 << DEVICE_ACCESS_START_BIT)) == 0),
		20);
	if (ret)
		INFO("read device err: LVTS %d didn't ready, reg_idx 0x%x\n",
		     tc_id, reg_idx);

	data = mmio_read_32(LVTSRDATA0_0 + base);

	return data;
}

#define THERMAL_SENSOR_IDLE_MASK (BIT(10) | BIT(7) | BIT(0))
#define HALF_WORD (16)
#define IDLE_CHECK_BIT1_MASK (BIT(10))
#define IDLE_CHECK_BIT2_MASK (BIT(7))
#define IDLE_CHECK_BIT3_MASK (BIT(0))
#define SHIFT_8 (8)
#define SHIFT_6 (6)

static uint32_t lvts_thermal_check_all_sensing_point_idle(
	struct lvts_data *lvts_data)
{
	unsigned int i;
	uintptr_t base;
	uint32_t temp, mask, error_code;
	struct tc_settings *tc = lvts_data->tc;

	mask = THERMAL_SENSOR_IDLE_MASK;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);
		temp = mmio_read_32(base + LVTSMSRCTL1_0);
		/* Check if bit10=bit7=bit0=0 */
		if ((temp & mask) != 0) {
			error_code = (i << HALF_WORD)
				+ ((temp & IDLE_CHECK_BIT1_MASK) >> SHIFT_8)
				+ ((temp & IDLE_CHECK_BIT2_MASK) >> SHIFT_6)
				+ (temp & IDLE_CHECK_BIT3_MASK);

			return error_code;
		}
	}

	return 0;
}

#define ITERATION_TIME (80)
#define DELAY_US       (2)

static void wait_all_tc_sensing_point_idle(struct lvts_data *lvts_data)
{
	unsigned int i;
	uintptr_t base;
	uint32_t temp, mask, error_code;
	int cnt = 0;
	struct tc_settings *tc = lvts_data->tc;
	static unsigned int no_idle_count;

	mask = THERMAL_SENSOR_IDLE_MASK;

	while (cnt < ITERATION_TIME) {
		temp = lvts_thermal_check_all_sensing_point_idle(lvts_data);
		if (temp == 0)
			goto TAIL;

		udelay(DELAY_US);
		cnt++;

	}
	no_idle_count++;
	mmio_write_32(THERMAL_CSRAM_BASE + NO_IDLE_COUNT_ADDR_OFFSET,
		no_idle_count);

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);
		temp = mmio_read_32(LVTSMSRCTL1_0 + base);
		/*
		 * Error code
		 * 000: IDLE
		 * 001: Write transaction
		 * 010: Waiting for read after Write
		 * 011: Disable Continue fetching on Device
		 * 100: Read transaction
		 * 101: Set Device special Register for Voltage threshold
		 * 111: Set TSMCU number for Fetch
		 */
		if ((temp & mask) != 0) {
			error_code = ((temp & IDLE_CHECK_BIT1_MASK) >> SHIFT_8)
				+ ((temp & IDLE_CHECK_BIT2_MASK) >> SHIFT_6)
				+ (temp & IDLE_CHECK_BIT3_MASK);

			INFO("LVTS %d sensing points not idle, error_code %d\n",
			     i, error_code);
		}
	}
TAIL:
	return;
}

static void disable_all_sensing_points(struct lvts_data *lvts_data)
{
	unsigned int i;
	uintptr_t base;
	struct tc_settings *tc = lvts_data->tc;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);
		mmio_write_32(LVTSMONCTL0_0 + base, DISABLE_SENSING_POINT);
	}
	dmbsy();
}

static void enable_all_sensing_points(struct lvts_data *lvts_data)
{
	struct tc_settings *tc = lvts_data->tc;
	unsigned int i, j, num;
	uintptr_t base;
	uint32_t flag;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);
		num = tc[i].num_sensor;

		if (num > ALL_SENSING_POINTS) {
			INFO("%s, LVTS%d, illegal number of sensors: %d\n",
			     __func__, i, tc[i].num_sensor);
			continue;
		}

		flag = LVTS_SINGLE_SENSE;
		for (j = 0; j < tc[i].num_sensor; j++) {
			if (tc[i].sensor_on_off[j] != SEN_ON)
				continue;

			flag = flag | (0x1 << j);
		}
		mmio_write_32(LVTSMONCTL0_0 + base, flag);
	}
	dmbsy();
}

void set_polling_speed(struct lvts_data *lvts_data, unsigned int tc_id)
{
	struct tc_settings *tc = lvts_data->tc;
	uint32_t lvts_mon_ctrl1, lvts_mon_ctrl2;
	uintptr_t base;

	base = GET_BASE_ADDR(lvts_data, tc_id);

	lvts_mon_ctrl1 = (((tc[tc_id].tc_speed.group_interval_delay
			<< 20) & GENMASK(29, 20)) |
			(tc[tc_id].tc_speed.period_unit &
			GENMASK(9, 0)));
	lvts_mon_ctrl2 = (((tc[tc_id].tc_speed.filter_interval_delay
			<< 16) & GENMASK(25, 16)) |
			(tc[tc_id].tc_speed.sensor_interval_delay
			& GENMASK(9, 0)));
	/*
	 * Clock source of LVTS thermal controller is 26MHz.
	 * Period unit is a base for all interval delays
	 * All interval delays must multiply it to convert a setting to time.
	 * Filter interval delay is a delay between two samples of the same
	 * sensor
	 * Sensor interval delay is a delay between two samples of differnet
	 * sensors
	 * Group interval delay is a delay between different rounds.
	 * For example:
	 *     If Period unit = C, filter delay = 1, sensor delay = 2, group
	 *     delay = 1,
	 *     and two sensors, TS1 and TS2, are in a LVTS thermal controller
	 *     and then
	 *     Period unit = C * 1/26M * 256 = 12 * 38.46ns * 256 = 118.149us
	 *     Filter interval delay = 1 * Period unit = 118.149us
	 *     Sensor interval delay = 2 * Period unit = 236.298us
	 *     Group interval delay = 1 * Period unit = 118.149us
	 *
	 *     TS1    TS1 ... TS1    TS2    TS2 ... TS2    TS1...
	 *        <--> Filter interval delay
	 *                       <--> Sensor interval delay
	 *                                             <--> Group interval delay
	 */
	mmio_write_32(LVTSMONCTL1_0 + base, lvts_mon_ctrl1);
	mmio_write_32(LVTSMONCTL2_0 + base, lvts_mon_ctrl2);
	dmbsy();
}

void set_hw_filter(struct lvts_data *lvts_data, unsigned int tc_id)
{
	struct tc_settings *tc = lvts_data->tc;
	uint32_t option;
	uintptr_t base;

	base = GET_BASE_ADDR(lvts_data, tc_id);
	option = tc[tc_id].hw_filter & 0x7;
	/* hw filter
	 * 000: Get one sample
	 * 001: Get 2 samples and average them
	 * 010: Get 4 samples, drop max&min and average the rest of 2 samples
	 * 011: Get 6 samples, drop max&min and average the rest of 4 samples
	 * 100: Get 10 samples, drop max&min and average the rest of 8 samples
	 * 101: Get 18 samples, drop max&min and average the rest of 16 samples
	 */
	option = (option << 9) | (option << 6) | (option << 3) | option;

	mmio_write_32(LVTSMSRCTL0_0 + base, option);
	dmbsy();
}

unsigned int get_dominator_index(struct lvts_data *lvts_data,
				 unsigned int tc_id)
{
	struct tc_settings *tc = lvts_data->tc;
	unsigned int d_index;

	if (tc[tc_id].dominator_sensing_point == ALL_SENSING_POINTS) {
		d_index = ALL_SENSING_POINTS;
	} else if (tc[tc_id].dominator_sensing_point <
		tc[tc_id].num_sensor){
		d_index = tc[tc_id].dominator_sensing_point;
	} else {
		INFO("Error: LVTS%d, dominator_sensing_point= %d over %d\n",
		     tc_id, tc[tc_id].dominator_sensing_point,
		     tc[tc_id].num_sensor);

		INFO("Use the sensing point 0 as the dominated sensor\n");
		d_index = SENSING_POINT0;
	}

	return d_index;
}

static void set_all_tc_hw_reboot(struct lvts_data *lvts_data)
{
	struct tc_settings *tc = lvts_data->tc;
	int trip_point;
	unsigned int i;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		trip_point = tc[i].hw_reboot_trip_point;

		if (tc[i].num_sensor == 0)
			continue;

		if (trip_point == DISABLE_THERMAL_HW_REBOOT) {
			disable_hw_reboot_interrupt(lvts_data, i);
			continue;
		}

		set_tc_hw_reboot_threshold(lvts_data, trip_point, i);
	}
}

static bool lvts_lk_init_check(struct lvts_data *lvts_data)
{
	unsigned int i;
	uint32_t data;
	uintptr_t base;
	bool ret = false;
	struct tc_settings *tc = lvts_data->tc;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);

		/* Check LVTS device ID */
		data = mmio_read_32(LVTSSPARE0_0 + base) & GENMASK(11, 0);

		if (data == LK_LVTS_MAGIC) {
			mmio_write_32(LVTSSPARE0_0 + base, 0x0);
			ret = true;
		} else {
			ret = false;
			break;
		}
	}
	dmbsy();
	return ret;
}

static void lvts_set_init_flag_tfa(struct lvts_data *lvts_data)
{
	uintptr_t base;
	unsigned int i;
	struct tc_settings *tc = lvts_data->tc;

	/* write TFA init done flag to inform kernel */

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);

		/* enable sensing point 0 */
		mmio_write_32(base + LVTSSPARE0_0, TFA_LVTS_MAGIC);
	}
	dmbsy();
}

static void lvts_clear_init_flag_tfa(struct lvts_data *lvts_data)
{
	uintptr_t base;
	unsigned int i;
	struct tc_settings *tc = lvts_data->tc;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);
		mmio_write_32(LVTSSPARE0_0 + base, 0);
	}
	dmbsy();
}

static int read_calibration_data(struct lvts_data *lvts_data)
{
	struct tc_settings *tc = lvts_data->tc;
	unsigned int i, j, s_index;
	uintptr_t base;
	struct sensor_cal_data *cal_data = &lvts_data->cal_data;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);

		for (j = 0; j < tc[i].num_sensor; j++) {
			if (tc[i].sensor_on_off[j] != SEN_ON)
				continue;

			s_index = tc[i].sensor_map[j];

			cal_data->efuse_data[s_index] =
				mmio_read_32(LVTSEDATA00_0 + base + 0x4 * j);

		}
	}

	return 0;
}

static int lvts_init(struct lvts_data *lvts_data)
{
	struct platform_ops *ops = &lvts_data->ops;
	int ret;
	bool lk_init = false;

	mmio_write_32(THERMAL_CSRAM_BASE + CLOSE_FOOTPRINT_ADDR_OFFSET, 0x0);

	thermal_clock_open();

	lk_init = lvts_lk_init_check(lvts_data);
	if (lk_init == true) {
		ret = read_calibration_data(lvts_data);

		lvts_set_init_flag_tfa(lvts_data);
		lvts_data->init_done = true;
		INFO("%s, LK init LVTS\n", __func__);

		return ret;
	}

	ops->lvts_reset(lvts_data);

	if (ops->device_identification)
		ops->device_identification(lvts_data);

	if (ops->device_enable_and_init)
		ops->device_enable_and_init(lvts_data);

	if (IS_ENABLE(lvts_data, (int)FEATURE_DEVICE_AUTO_RCK)) {
		if (ops->device_enable_auto_rck)
			ops->device_enable_auto_rck(lvts_data);
	} else {
		if (ops->device_read_count_rc_n)
			ops->device_read_count_rc_n(lvts_data);
	}

	if (ops->set_cal_data)
		ops->set_cal_data(lvts_data);

	disable_all_sensing_points(lvts_data);
	wait_all_tc_sensing_point_idle(lvts_data);
	if (ops->init_controller)
		ops->init_controller(lvts_data);

	set_all_tc_hw_reboot(lvts_data);
	enable_all_sensing_points(lvts_data);
	lvts_set_init_flag_tfa(lvts_data);
	lvts_data->init_done = true;

	return 0;
}

static int update_lvts_data_from_efuse(struct lvts_data *lvts_data)
{
	struct platform_ops *ops = &lvts_data->ops;

	if (ops->get_calibration_data)
		ops->get_calibration_data(lvts_data);

	if (ops->efuse_to_cal_data)
		ops->efuse_to_cal_data(lvts_data);

	if (ops->check_cal_data)
		ops->check_cal_data(lvts_data);

	if (ops->update_coef_data)
		ops->update_coef_data(lvts_data);

	return 0;
}

static void lvts_close(struct lvts_data *lvts_data)
{
	mmio_write_32(THERMAL_CSRAM_BASE + INIT_FOOTPRINT_ADDR_OFFSET, 0x0);

	lvts_data->ops.lvts_reset(lvts_data);

	lvts_clear_init_flag_tfa(lvts_data);

	thermal_clock_close();
}

void set_calibration_data_v1(struct lvts_data *lvts_data)
{
	struct tc_settings *tc = lvts_data->tc;
	struct sensor_cal_data *cal_data = &lvts_data->cal_data;
	unsigned int i, j, s_index;
	uint32_t e_data;
	uintptr_t base;

	for (i = 0; i < lvts_data->num_tc; i++) {
		if (tc[i].ctrl_on_off == CTRL_OFF)
			continue;

		base = GET_BASE_ADDR(lvts_data, i);

		for (j = 0; j < tc[i].num_sensor; j++) {
			if (tc[i].sensor_on_off[j] != SEN_ON)
				continue;

			s_index = tc[i].sensor_map[j];
			if (IS_ENABLE(lvts_data, (int)FEATURE_DEVICE_AUTO_RCK))
				e_data = cal_data->count_r[s_index];
			else
				e_data = cal_data->efuse_data[s_index];

			mmio_write_32(LVTSEDATA00_0 + base + 0x4 * j, e_data);
		}
	}
	dmbsy();
}

static int lvts_thermal_init(void)
{
	struct lvts_data *lvts_data;
	int ret;

	lvts_data = &lvts_data_instance;

	ret = update_lvts_data_from_efuse(lvts_data);
	if (ret)
		return ret;

	ret = lvts_init(lvts_data);
	if (ret)
		return ret;

	return 0;
}

/* suspend/resume */
static int lvts_suspend_callback(struct lvts_data *lvts_data)
{
	lvts_close(lvts_data);

	return 0;
}

static int lvts_resume_callback(struct lvts_data *lvts_data)
{
	return lvts_init(lvts_data);
}

static void *lvts_pm_suspend_event_handler(const void *arg)
{
	struct mt_lp_publish_event const *event =
		(struct mt_lp_publish_event const *) arg;
	struct lvts_data *lvts_data = &lvts_data_instance;
	int ret;

	if (!event) {
		return NULL;
	}

	if (event->id == MT_LPM_PUBEVENTS_SYS_POWER_OFF) {
		ret = lvts_suspend_callback(lvts_data);
		if (ret) {
			INFO("%s, %d, ret = %d\n", __func__, __LINE__, ret);
		}
		return (void *)arg;

	} else if (event->id == MT_LPM_PUBEVENTS_SYS_POWER_ON) {
		ret = lvts_resume_callback(lvts_data);
		if (ret) {
			INFO("%s, %d, ret = %d\n", __func__, __LINE__, ret);
		}
		return (void *)arg;
	}

	return NULL;
}
MT_LP_SUBSCRIBE_SUSPEND(lvts_pm_suspend_event_handler);

MTK_PLAT_RUNTIME_INIT(lvts_thermal_init);
