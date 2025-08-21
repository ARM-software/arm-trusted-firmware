/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef THERMAL_LVTS_H
#define THERMAL_LVTS_H

/* Definition or macro function */
#define LK_LVTS_MAGIC	(0x0000555)
#define TFA_LVTS_MAGIC	(0x0000777)

#define THERMAL_TEMP_INVALID	(-274000)

#define DEVICE_ACCESS_START_BIT	(24)

#define DISABLE_THERMAL_HW_REBOOT	(-274000)

#define CLOCK_26MHZ_CYCLE_NS	(38)

#define FEATURE_DEVICE_AUTO_RCK	(BIT(0))

#define IS_ENABLE(lvts_data, feature) \
	(lvts_data->feature_bitmap & feature)

#define GET_BASE_ADDR(lvts_data, tc_id) \
	(lvts_data->domain[lvts_data->tc[tc_id].domain_index].base \
	+ lvts_data->tc[tc_id].addr_offset)

#define SET_TC_SPEED_IN_US(pu, gd, fd, sd) \
	{ \
		.period_unit = ((pu * 1000) / (256 * CLOCK_26MHZ_CYCLE_NS)), \
		.group_interval_delay = (gd / pu), \
		.filter_interval_delay = (fd / pu), \
		.sensor_interval_delay = (sd / pu), \
	}

#define GET_CAL_BITMASK(lvts_data, index, h, l) \
	((index < lvts_data->num_efuse_addr) \
	? ((lvts_data->efuse[index] & GENMASK(h, l)) >> l) \
	: 0)


/* LVTS HW filter settings
 * 000: Get one sample
 * 001: Get 2 samples and average them
 * 010: Get 4 samples, drop max and min, then average the rest of 2 samples
 * 011: Get 6 samples, drop max and min, then average the rest of 4 samples
 * 100: Get 10 samples, drop max and min, then average the rest of 8 samples
 * 101: Get 18 samples, drop max and min, then average the rest of 16 samples
 */
enum lvts_hw_filter {
	LVTS_FILTER_1,
	LVTS_FILTER_2,
	LVTS_FILTER_2_OF_4,
	LVTS_FILTER_4_OF_6,
	LVTS_FILTER_8_OF_10,
	LVTS_FILTER_16_OF_18
};

enum lvts_sensing_point {
	SENSING_POINT0,
	SENSING_POINT1,
	SENSING_POINT2,
	SENSING_POINT3,
	ALL_SENSING_POINTS
};

enum calibration_mode {
	CALI_NT,
	CALI_HT,
	ALL_CALI_MODES
};

/* Data structure */
struct lvts_data;

struct speed_settings {
	unsigned int period_unit;
	unsigned int group_interval_delay;
	unsigned int filter_interval_delay;
	unsigned int sensor_interval_delay;
};

struct formula_coeff {
	int a[ALL_SENSING_POINTS];
	unsigned int golden_temp;
	enum calibration_mode cali_mode;
};

enum sensor_switch_status {
	SEN_OFF,
	SEN_ON
};

enum controller_switch_status {
	CTRL_OFF,
	CTRL_ON
};

struct tc_settings {
	unsigned int domain_index;
	uintptr_t addr_offset;
	unsigned int num_sensor;
	unsigned int sensor_map[ALL_SENSING_POINTS]; /* In sensor ID */
	enum controller_switch_status ctrl_on_off;
	enum sensor_switch_status sensor_on_off[ALL_SENSING_POINTS];
	struct speed_settings tc_speed;
	/* HW filter setting
	 * 000: Get one sample
	 * 001: Get 2 samples and average them
	 * 010: Get 4 samples, drop max&min and average the rest of 2 samples
	 * 011: Get 6 samples, drop max&min and average the rest of 4 samples
	 * 100: Get 10 samples, drop max&min and average the rest of 8 samples
	 * 101: Get 18 samples, drop max&min and average the rest of 16 samples
	 */
	uint32_t hw_filter;
	/* Dominator_sensing point is used to select a sensing point
	 * and reference its temperature to trigger Thermal HW Reboot
	 * When it is ALL_SENSING_POINTS, it will select all sensing points
	 */
	enum lvts_sensing_point dominator_sensing_point;
	int hw_reboot_trip_point; /* -274000: Disable HW reboot */
	uint32_t irq_bit;
	struct formula_coeff coeff;
};

struct sensor_cal_data {
	unsigned int golden_temp;
	unsigned int golden_temp_ht;
	uint32_t cali_mode;
	uint32_t *count_r;
	uint32_t *count_rc;
	uint32_t *count_rc_now;
	uint32_t *efuse_data;

	unsigned int default_golden_temp;
	unsigned int default_golden_temp_ht;
	uint32_t default_count_r;
	uint32_t default_count_rc;
};

struct platform_ops {
	void (*lvts_reset)(struct lvts_data *lvts_data);
	void (*device_identification)(struct lvts_data *lvts_data);
	void (*get_calibration_data)(struct lvts_data *lvts_data);
	void (*efuse_to_cal_data)(struct lvts_data *lvts_data);
	void (*device_enable_and_init)(struct lvts_data *lvts_data);
	void (*device_enable_auto_rck)(struct lvts_data *lvts_data);
	int (*device_read_count_rc_n)(struct lvts_data *lvts_data);
	void (*set_cal_data)(struct lvts_data *lvts_data);
	void (*init_controller)(struct lvts_data *lvts_data);
	int (*lvts_raw_to_temp)(const struct formula_coeff *co,
				unsigned int id, unsigned int msr_raw);
	unsigned int (*lvts_temp_to_raw)(const struct formula_coeff *co,
					 unsigned int id, int temp);
	void (*check_cal_data)(struct lvts_data *lvts_data);
	void (*update_coef_data)(struct lvts_data *lvts_data);
};

struct power_domain {
	uintptr_t base;	/* LVTS base addresses */
	uintptr_t reset_base;
	uintptr_t reset_set_offset;
	uintptr_t reset_clr_offset;
	uint32_t reset_set_bitnum;
	uint32_t reset_clr_bitnum;
};

struct sensor_data {
	int temp;		/* Current temperature */
	unsigned int msr_raw;	/* MSR raw data from LVTS */
};

struct lvts_data {
	unsigned int num_domain;
	struct power_domain *domain;

	unsigned int num_tc;		/* Number of LVTS thermal controllers */
	struct tc_settings *tc;
	int counting_window_us;		/* LVTS device counting window */

	unsigned int num_sensor;	/* Number of sensors in this platform */
	struct sensor_data *sen_data;

	struct platform_ops ops;
	int feature_bitmap;		/* Show what features are enabled */

	unsigned int num_efuse_addr;
	uint32_t *efuse;

	uint32_t *irq_bitmap;

	struct sensor_cal_data cal_data;
	int enable_dump_log;
	bool init_done;
};

/* LVTS device register */
#define RG_TSFM_CTRL_0		0x03
#define RG_TSFM_CTRL_3		0x06
#define RG_TSV2F_CTRL_0		0x08
#define RG_TSV2F_CTRL_1		0x09
#define RG_TSV2F_CTRL_2		0x0A
#define RG_TSV2F_CTRL_3		0x0B
#define RG_TSV2F_CTRL_4		0x0C
#define RG_TSV2F_CTRL_5		0x0D
#define RG_TSV2F_CTRL_6		0x0E
#define RG_DID_LVTS		0xFC
#define RG_TSFM_RST		0xFF

/* LVTS controller register */
#define LVTSMONCTL0_0	0x000
#define LVTS_SINGLE_SENSE	(1UL << 9)
#define DISABLE_SENSING_POINT	(LVTS_SINGLE_SENSE | 0x0)
#define LVTSMONCTL1_0		0x004
#define LVTSMONCTL2_0		0x008
#define LVTSMONINT_0		0x00C
#define STAGE3_INT_EN		(1UL << 31)

#define HIGH_OFFSET3_INT_EN	(1UL << 25)
#define HIGH_OFFSET2_INT_EN	(1UL << 13)
#define HIGH_OFFSET1_INT_EN	(1UL << 8)
#define HIGH_OFFSET0_INT_EN	(1UL << 3)

#define LOW_OFFSET3_INT_EN	(1UL << 24)
#define LOW_OFFSET2_INT_EN	(1UL << 12)
#define LOW_OFFSET1_INT_EN	(1UL << 7)
#define LOW_OFFSET0_INT_EN	(1UL << 2)

#define LVTSOFFSETH_0		0x030
#define LVTSOFFSETL_0		0x034
#define LVTSMSRCTL0_0		0x038
#define LVTSMSRCTL1_0		0x03C
#define LVTSTSSEL_0		0x040
#define LVTSCALSCALE_0		0x048
#define SET_CALC_SCALE_RULES	0x00000300
#define LVTS_ID_0		0x04C
#define LVTS_CONFIG_0		0x050
#define LVTSEDATA00_0		0x054
#define MRS_RAW_MASK		GENMASK(15, 0)
#define LVTSRDATA0_0		0x0B0
#define LVTSPROTCTL_0		0x0C0
#define PROTOFFSET		GENMASK(15, 0)
#define LVTSPROTTC_0		0x0CC
#define LVTSCLKEN_0		0x0E4
#define ENABLE_LVTS_CTRL_CLK	(1)
#define LVTSSPARE0_0		0x0F0

#define BROADCAST_ID_UPDATE	(1UL << 26)
#define DEVICE_SENSING_STATUS	(1UL << 25)
#define DEVICE_ACCESS_STARTUS	(1UL << 24)
#define WRITE_ACCESS		(1UL << 16)

#define DEVICE_WRITE		(1UL << 31 | 1UL << 30 | DEVICE_ACCESS_STARTUS \
				| 1UL << 17 | WRITE_ACCESS)


#define DEVICE_READ		(1UL << 31 | 1UL << 30 | DEVICE_ACCESS_STARTUS \
				| 1UL << 17)
#define RESET_ALL_DEVICES	(DEVICE_WRITE | RG_TSFM_RST << 8 | 0xFF)
#define READ_BACK_DEVICE_ID	(1UL << 31 | 1UL << 30 | BROADCAST_ID_UPDATE \
				| DEVICE_ACCESS_STARTUS | 1UL << 17 \
				| RG_DID_LVTS << 8)

#define READ_DEVICE_REG(reg_idx)	(DEVICE_READ | reg_idx << 8 | 0x00)

/* LVTS register mask */
#define INIT_FOOTPRINT_ADDR_OFFSET	(0x1F8)
#define CLOSE_FOOTPRINT_ADDR_OFFSET	(0x1FC)
#define NO_IDLE_COUNT_ADDR_OFFSET	(0x1E4)

void thermal_clock_open(void);
void thermal_clock_close(void);
void set_polling_speed(struct lvts_data *lvts_data, unsigned int tc_id);
void set_hw_filter(struct lvts_data *lvts_data, unsigned int tc_id);
void lvts_write_all_device(struct lvts_data *lvts_data, uint32_t data);
void lvts_write_device(struct lvts_data *lvts_data, uint32_t data,
		       unsigned int tc_id);
uint32_t lvts_read_device(struct lvts_data *lvts_data, uint32_t reg_idx,
			  unsigned int tc_id);
void set_calibration_data_v1(struct lvts_data *lvts_data);
unsigned int get_dominator_index(struct lvts_data *lvts_data,
				 unsigned int tc_id);
void disable_hw_reboot_interrupt(struct lvts_data *lvts_data,
				 unsigned int tc_id);
void enable_hw_reboot_interrupt(struct lvts_data *lvts_data,
				unsigned int tc_id);
void set_tc_hw_reboot_threshold(struct lvts_data *lvts_data, int trip_point,
				unsigned int tc_id);

extern struct lvts_data lvts_data_instance;

#endif /* THERMAL_LVTS_H */
