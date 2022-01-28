/*
 * Copyright (c) 2021-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/regulator.h>
#include <libfdt.h>

#define MAX_PROPERTY_LEN 64

static struct rdev rdev_array[PLAT_NB_RDEVS];

#define for_each_rdev(rdev) \
	for (rdev = rdev_array; rdev < (rdev_array + PLAT_NB_RDEVS); rdev++)

#define for_each_registered_rdev(rdev) \
	for (rdev = rdev_array; \
	     (rdev < (rdev_array + PLAT_NB_RDEVS)) && (rdev->desc != NULL); rdev++)

static void lock_driver(const struct rdev *rdev)
{
	if (rdev->desc->ops->lock != NULL) {
		rdev->desc->ops->lock(rdev->desc);
	}
}

static void unlock_driver(const struct rdev *rdev)
{
	if (rdev->desc->ops->unlock != NULL) {
		rdev->desc->ops->unlock(rdev->desc);
	}
}

static struct rdev *regulator_get_by_phandle(int32_t phandle)
{
	struct rdev *rdev;

	for_each_registered_rdev(rdev) {
		if (rdev->phandle == phandle) {
			return rdev;
		}
	}

	WARN("%s: phandle %d not found\n", __func__, phandle);
	return NULL;
}

/*
 * Get a regulator from its node name
 *
 * @fdt - pointer to device tree memory
 * @node_name - name of the node "ldo1"
 * Return pointer to rdev if succeed, NULL else.
 */
struct rdev *regulator_get_by_name(const char *node_name)
{
	struct rdev *rdev;

	assert(node_name != NULL);
	VERBOSE("get %s\n", node_name);

	for_each_registered_rdev(rdev) {
		if (strcmp(rdev->desc->node_name, node_name) == 0) {
			return rdev;
		}
	}

	WARN("%s: %s not found\n", __func__, node_name);
	return NULL;
}

static int32_t get_supply_phandle(const void *fdt, int node, const char *name)
{
	const fdt32_t *cuint;
	int len __unused;
	int supply_phandle = -FDT_ERR_NOTFOUND;
	char prop_name[MAX_PROPERTY_LEN];

	len = snprintf(prop_name, MAX_PROPERTY_LEN - 1, "%s-supply", name);
	assert((len >= 0) && (len < MAX_PROPERTY_LEN - 1));

	cuint = fdt_getprop(fdt, node, prop_name, NULL);
	if (cuint != NULL) {
		supply_phandle = fdt32_to_cpu(*cuint);
		VERBOSE("%s: supplied by %d\n", name, supply_phandle);
	}

	return supply_phandle;
}

/*
 * Get a regulator from a supply name
 *
 * @fdt - pointer to device tree memory
 * @node - offset of the node that contains the supply description
 * @name - name of the supply "vdd" for "vdd-supply'
 * Return pointer to rdev if succeed, NULL else.
 */
struct rdev *regulator_get_by_supply_name(const void *fdt, int node, const char *name)
{
	const int p = get_supply_phandle(fdt, node, name);

	if (p < 0) {
		return NULL;
	}

	return regulator_get_by_phandle(p);
}

static int __regulator_set_state(struct rdev *rdev, bool state)
{
	if (rdev->desc->ops->set_state == NULL) {
		return -ENODEV;
	}

	return rdev->desc->ops->set_state(rdev->desc, state);
}

/*
 * Enable regulator
 *
 * @rdev - pointer to rdev struct
 * Return 0 if succeed, non 0 else.
 */
int regulator_enable(struct rdev *rdev)
{
	int ret;

	assert(rdev != NULL);

	ret = __regulator_set_state(rdev, STATE_ENABLE);

	udelay(rdev->enable_ramp_delay);

	return ret;
}

/*
 * Disable regulator
 *
 * @rdev - pointer to rdev struct
 * Return 0 if succeed, non 0 else.
 */
int regulator_disable(struct rdev *rdev)
{
	int ret;

	assert(rdev != NULL);

	if (rdev->flags & REGUL_ALWAYS_ON) {
		return 0;
	}

	ret = __regulator_set_state(rdev, STATE_DISABLE);

	udelay(rdev->enable_ramp_delay);

	return ret;
}

/*
 * Regulator enabled query
 *
 * @rdev - pointer to rdev struct
 * Return 0 if disabled, 1 if enabled, <0 else.
 */
int regulator_is_enabled(const struct rdev *rdev)
{
	int ret;

	assert(rdev != NULL);

	VERBOSE("%s: is en\n", rdev->desc->node_name);

	if (rdev->desc->ops->get_state == NULL) {
		return -ENODEV;
	}

	lock_driver(rdev);

	ret = rdev->desc->ops->get_state(rdev->desc);
	if (ret < 0) {
		ERROR("regul %s get state failed: err:%d\n",
		      rdev->desc->node_name, ret);
	}

	unlock_driver(rdev);

	return ret;
}

/*
 * Set regulator voltage
 *
 * @rdev - pointer to rdev struct
 * @mvolt - Target voltage level in millivolt
 * Return 0 if succeed, non 0 else.
 */
int regulator_set_voltage(struct rdev *rdev, uint16_t mvolt)
{
	int ret;

	assert(rdev != NULL);

	VERBOSE("%s: set mvolt\n", rdev->desc->node_name);

	if (rdev->desc->ops->set_voltage == NULL) {
		return -ENODEV;
	}

	if ((mvolt < rdev->min_mv) || (mvolt > rdev->max_mv)) {
		return -EPERM;
	}

	lock_driver(rdev);

	ret = rdev->desc->ops->set_voltage(rdev->desc, mvolt);
	if (ret < 0) {
		ERROR("regul %s set volt failed: err:%d\n",
		      rdev->desc->node_name, ret);
	}

	unlock_driver(rdev);

	return ret;
}

/*
 * Set regulator min voltage
 *
 * @rdev - pointer to rdev struct
 * Return 0 if succeed, non 0 else.
 */
int regulator_set_min_voltage(struct rdev *rdev)
{
	return regulator_set_voltage(rdev, rdev->min_mv);
}

/*
 * Get regulator voltage
 *
 * @rdev - pointer to rdev struct
 * Return milli volts if succeed, <0 else.
 */
int regulator_get_voltage(const struct rdev *rdev)
{
	int ret;

	assert(rdev != NULL);

	VERBOSE("%s: get volt\n", rdev->desc->node_name);

	if (rdev->desc->ops->get_voltage == NULL) {
		return rdev->min_mv;
	}

	lock_driver(rdev);

	ret = rdev->desc->ops->get_voltage(rdev->desc);
	if (ret < 0) {
		ERROR("regul %s get voltage failed: err:%d\n",
		      rdev->desc->node_name, ret);
	}

	unlock_driver(rdev);

	return ret;
}

/*
 * List regulator voltages
 *
 * @rdev - pointer to rdev struct
 * @levels - out: array of supported millitvolt levels from min to max value
 * @count - out: number of possible millivolt values
 * Return 0 if succeed, non 0 else.
 */
int regulator_list_voltages(const struct rdev *rdev, const uint16_t **levels, size_t *count)
{
	int ret;
	size_t n;

	assert(rdev != NULL);
	assert(levels != NULL);
	assert(count != NULL);

	VERBOSE("%s: list volt\n", rdev->desc->node_name);

	if (rdev->desc->ops->list_voltages == NULL) {
		return -ENODEV;
	}

	lock_driver(rdev);

	ret = rdev->desc->ops->list_voltages(rdev->desc, levels, count);

	unlock_driver(rdev);

	if (ret < 0) {
		ERROR("regul %s list_voltages failed: err: %d\n",
		      rdev->desc->node_name, ret);
		return ret;
	}

	/*
	 * Reduce the possible values depending on min and max from device-tree
	 */
	n = *count;
	while ((n > 1U) && ((*levels)[n - 1U] > rdev->max_mv)) {
		n--;
	}

	/* Verify that max val is a valid value */
	if (rdev->max_mv != (*levels)[n - 1]) {
		ERROR("regul %s: max value %u is invalid\n",
		      rdev->desc->node_name, rdev->max_mv);
		return -EINVAL;
	}

	while ((n > 1U) && ((*levels[0U]) < rdev->min_mv)) {
		(*levels)++;
		n--;
	}

	/* Verify that min is not too high */
	if (n == 0U) {
		ERROR("regul %s set min voltage is too high\n",
		      rdev->desc->node_name);
		return -EINVAL;
	}

	/* Verify that min val is a valid vlue */
	if (rdev->min_mv != (*levels)[0U]) {
		ERROR("regul %s: min value %u is invalid\n",
		      rdev->desc->node_name, rdev->min_mv);
		return -EINVAL;
	}

	*count = n;

	VERBOSE("rdev->min_mv=%u rdev->max_mv=%u\n", rdev->min_mv, rdev->max_mv);

	return 0;
}

/*
 * Get regulator voltages range
 *
 * @rdev - pointer to rdev struct
 * @min_mv - out: min possible millivolt value
 * @max_mv - out: max possible millivolt value
 * Return 0 if succeed, non 0 else.
 */
void regulator_get_range(const struct rdev *rdev, uint16_t *min_mv, uint16_t *max_mv)
{
	assert(rdev != NULL);

	if (min_mv != NULL) {
		*min_mv = rdev->min_mv;
	}
	if (max_mv != NULL) {
		*max_mv = rdev->max_mv;
	}
}

/*
 * Set regulator flag
 *
 * @rdev - pointer to rdev struct
 * @flag - flag value to set (eg: REGUL_OCP)
 * Return 0 if succeed, non 0 else.
 */
int regulator_set_flag(struct rdev *rdev, uint16_t flag)
{
	int ret;

	/* check that only one bit is set on flag */
	if (__builtin_popcount(flag) != 1) {
		return -EINVAL;
	}

	/* REGUL_ALWAYS_ON and REGUL_BOOT_ON are internal properties of the core */
	if ((flag == REGUL_ALWAYS_ON) || (flag == REGUL_BOOT_ON)) {
		rdev->flags |= flag;
		return 0;
	}

	if (rdev->desc->ops->set_flag == NULL) {
		ERROR("%s can not set any flag\n", rdev->desc->node_name);
		return -ENODEV;
	}

	lock_driver(rdev);

	ret = rdev->desc->ops->set_flag(rdev->desc, flag);

	unlock_driver(rdev);

	if (ret != 0) {
		ERROR("%s: could not set flag %d ret=%d\n",
		      rdev->desc->node_name, flag, ret);
		return ret;
	}

	rdev->flags |= flag;

	return 0;
}

static int parse_properties(const void *fdt, struct rdev *rdev, int node)
{
	int ret;

	if (fdt_getprop(fdt, node, "regulator-always-on", NULL) != NULL) {
		VERBOSE("%s: set regulator-always-on\n", rdev->desc->node_name);
		ret = regulator_set_flag(rdev, REGUL_ALWAYS_ON);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

/*
 * Parse the device-tree for a regulator
 *
 * Read min/max voltage from dt and check its validity
 * Read the properties, and call the driver to set flags
 * Read power supply phandle
 * Read and store low power mode states
 *
 * @rdev - pointer to rdev struct
 * @node - device-tree node offset of the regulator
 * Return 0 if disabled, 1 if enabled, <0 else.
 */
static int parse_dt(struct rdev *rdev, int node)
{
	void *fdt;
	const fdt32_t *cuint;
	const uint16_t *levels;
	size_t size;
	int ret;

	VERBOSE("%s: parse dt\n", rdev->desc->node_name);

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	rdev->phandle = fdt_get_phandle(fdt, node);

	cuint = fdt_getprop(fdt, node, "regulator-min-microvolt", NULL);
	if (cuint != NULL) {
		uint16_t min_mv;

		min_mv = (uint16_t)(fdt32_to_cpu(*cuint) / 1000U);
		VERBOSE("%s: min_mv=%d\n", rdev->desc->node_name, (int)min_mv);
		if (min_mv <= rdev->max_mv) {
			rdev->min_mv = min_mv;
		} else {
			ERROR("%s: min_mv=%d is too high\n",
			      rdev->desc->node_name, (int)min_mv);
			return -EINVAL;
		}
	}

	cuint = fdt_getprop(fdt, node, "regulator-max-microvolt", NULL);
	if (cuint != NULL) {
		uint16_t max_mv;

		max_mv = (uint16_t)(fdt32_to_cpu(*cuint) / 1000U);
		VERBOSE("%s: max_mv=%d\n", rdev->desc->node_name, (int)max_mv);
		if (max_mv >= rdev->min_mv) {
			rdev->max_mv = max_mv;
		} else {
			ERROR("%s: max_mv=%d is too low\n",
			      rdev->desc->node_name, (int)max_mv);
			return -EINVAL;
		}
	}

	/* validate that min and max values can be used */
	ret = regulator_list_voltages(rdev, &levels, &size);
	if ((ret != 0) && (ret != -ENODEV)) {
		return ret;
	}

	ret = parse_properties(fdt, rdev, node);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

/*
 * Register a regulator driver in regulator framework.
 * Initialize voltage range from driver description
 *
 * @desc - pointer to the regulator description
 * @node - device-tree node offset of the regulator
 * Return 0 if succeed, non 0 else.
 */
int regulator_register(const struct regul_description *desc, int node)
{
	struct rdev *rdev;

	assert(desc != NULL);

	VERBOSE("register %s\n", desc->node_name);

	for_each_rdev(rdev) {
		if (rdev->desc == NULL) {
			break;
		}
	}

	if (rdev == rdev_array + PLAT_NB_RDEVS) {
		WARN("Not enough place for regulators, PLAT_NB_RDEVS should be increased.\n");
		return -ENOMEM;
	}

	rdev->desc = desc;
	rdev->enable_ramp_delay = rdev->desc->enable_ramp_delay;

	if (rdev->desc->ops->list_voltages != NULL) {
		int ret;
		const uint16_t *levels;
		size_t count;

		lock_driver(rdev);

		ret = rdev->desc->ops->list_voltages(rdev->desc, &levels, &count);

		unlock_driver(rdev);

		if (ret < 0) {
			ERROR("regul %s set state failed: err:%d\n",
			      rdev->desc->node_name, ret);
			return ret;
		}

		rdev->min_mv = levels[0];
		rdev->max_mv = levels[count - 1U];
	} else {
		rdev->max_mv = UINT16_MAX;
	}

	return parse_dt(rdev, node);
}
