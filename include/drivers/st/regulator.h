/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef REGULATOR_H
#define REGULATOR_H

#include <platform_def.h>

#ifndef PLAT_NB_RDEVS
#error "Missing PLAT_NB_RDEVS"
#endif

/*
 * Consumer interface
 */

/* regulator-always-on : regulator should never be disabled */
#define REGUL_ALWAYS_ON		BIT(0)
/*
 * regulator-boot-on:
 * It's expected that this regulator was left on by the bootloader.
 * The core shouldn't prevent it from being turned off later.
 * The regulator is needed to exit from suspend so it is turned on during suspend entry.
 */
#define REGUL_BOOT_ON		BIT(1)
/* regulator-over-current-protection: Enable over current protection. */
#define REGUL_OCP		BIT(2)
/* regulator-active-discharge: enable active discharge. */
#define REGUL_ACTIVE_DISCHARGE	BIT(3)
/* regulator-pull-down: Enable pull down resistor when the regulator is disabled. */
#define REGUL_PULL_DOWN		BIT(4)
/*
 * st,mask-reset: set mask reset for the regulator, meaning that the regulator
 * setting is maintained during pmic reset.
 */
#define REGUL_MASK_RESET	BIT(5)
/* st,regulator-sink-source: set the regulator in sink source mode */
#define REGUL_SINK_SOURCE	BIT(6)
/* st,regulator-bypass: set the regulator in bypass mode */
#define REGUL_ENABLE_BYPASS	BIT(7)

struct rdev *regulator_get_by_name(const char *node_name);

struct rdev *regulator_get_by_supply_name(const void *fdt, int node, const char *name);

int regulator_enable(struct rdev *rdev);
int regulator_disable(struct rdev *rdev);
int regulator_is_enabled(const struct rdev *rdev);

int regulator_set_voltage(struct rdev *rdev, uint16_t volt);
int regulator_set_min_voltage(struct rdev *rdev);
int regulator_get_voltage(const struct rdev *rdev);

int regulator_list_voltages(const struct rdev *rdev, const uint16_t **levels, size_t *count);
void regulator_get_range(const struct rdev *rdev, uint16_t *min_mv, uint16_t *max_mv);
int regulator_set_flag(struct rdev *rdev, uint16_t flag);

/*
 * Driver Interface
 */

/* set_state() arguments */
#define STATE_DISABLE		false
#define STATE_ENABLE		true

struct regul_description {
	const char *node_name;
	const struct regul_ops *ops;
	const void *driver_data;
	const char *supply_name;
	const uint32_t enable_ramp_delay;
};

struct regul_ops {
	int (*set_state)(const struct regul_description *desc, bool state);
	int (*get_state)(const struct regul_description *desc);
	int (*set_voltage)(const struct regul_description *desc, uint16_t mv);
	int (*get_voltage)(const struct regul_description *desc);
	int (*list_voltages)(const struct regul_description *desc,
			     const uint16_t **levels, size_t *count);
	int (*set_flag)(const struct regul_description *desc, uint16_t flag);
	void (*lock)(const struct regul_description *desc);
	void (*unlock)(const struct regul_description *desc);
};

int regulator_register(const struct regul_description *desc, int node);

/*
 * Internal regulator structure
 * The structure is internal to the core, and the content should not be used
 * by a consumer nor a driver.
 */
struct rdev {
	const struct regul_description *desc;

	int32_t phandle;

	uint16_t min_mv;
	uint16_t max_mv;

	uint16_t flags;

	uint32_t enable_ramp_delay;
};

#endif /* REGULATOR_H */
