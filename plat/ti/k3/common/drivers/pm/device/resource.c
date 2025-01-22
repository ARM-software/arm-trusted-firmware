/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <resource.h>
#include <device.h>
#include <stddef.h>
#include <limits.h>

#define RESOURCE_MAX2(a, b) ((a) > (b) ? (a) : (b))
#define RESOURCE_MAX3(A, B, C) RESOURCE_MAX2(A, (RESOURCE_MAX2(B, C)))

/**
 * \brief Lookup a resource table entry.
 *
 * This function walks the resource table, first finding the array of correct
 * resource type, and then returning the resource at the appropriate index.
 *
 * \param dev
 * The device to which the resource table belongs.
 *
 * \param type
 * The type of resource table entry (clock, mem, reset).
 *
 * \param idx
 * Returns the nth entry of the desired type, where n is specified by idx.
 *
 * \return
 * The desired resource entry, or NULL if no such entry exists.
 */
static const void *resource_get(struct device *dev, uint8_t type, uint8_t idx)
{
	/*
	 * The downshift here is just because the macro that defines the type
	 * starts at bit 6.
	 */
	static const uint8_t sizes[RESOURCE_MAX3((RESOURCE_CLK >> 6U),
						 (RESOURCE_MEM >> 6U),
						 (RESOURCE_RST >> 6U)) + 1U] = {
		[RESOURCE_IRQ >> 6U] = 0,
		[RESOURCE_CLK >> 6U] = (uint8_t) sizeof(struct resource_clk),
		[RESOURCE_MEM >> 6U] = (uint8_t) sizeof(struct resource_mem),
		[RESOURCE_RST >> 6U] = (uint8_t) sizeof(struct resource_rst),
	};
	const struct dev_data *ddata = get_dev_data(dev);
	const uint8_t *data;
	const uint8_t *r;
	uint8_t hdr;

	BUILD_ASSERT((((sizeof(struct resource_clk)) <= ((size_t) UCHAR_MAX)) &&
		      ((sizeof(struct resource_mem)) <= ((size_t) UCHAR_MAX))) &&
		     ((sizeof(struct resource_rst)) <= ((size_t) UCHAR_MAX)));
	/*
	 * If the device does not have drv_data, it does not have resources.
	 * Return NULL
	 */
	r = (((uint32_t) (ddata->flags) & DEVD_FLAG_DRV_DATA) ==
	     DEVD_FLAG_DRV_DATA) ? to_drv_data(ddata)->r : NULL;

	while (r != NULL) {
		hdr = r[0];
		data = r + 1;
		/* Is this the right resource type? */
		if ((hdr & RESOURCE_TYPE_MASK) == type) {
			/* Is the array long enough? */
			if (idx < (hdr & RESOURCE_COUNT_MASK)) {
				/* Return our resource */
				r = &data[idx * sizes[type >> 6]];
				break;
			} else {
				r = NULL;
			}
		} else if ((hdr & RESOURCE_LAST) != 0U) {
			/* We've reached the end of the table, stop walking */
			r = NULL;
		} else {
			/* Walk to the next entry */
			r = &data[(hdr & RESOURCE_COUNT_MASK) *
				  sizes[hdr >> 6]];
		}
	}

	return r;
}

const struct resource_clk *device_resource_clk(struct device *dev, uint8_t idx)
{
	const struct resource_clk *ret =
		(const struct resource_clk *) resource_get(dev, RESOURCE_CLK, idx);

	return (ret && (ret->clk_id != RESOURCE_CLK_NONE)) ? ret : NULL;
}

const struct resource_mem *device_resource_mem(struct device *dev, uint8_t idx)
{
	const struct resource_mem *ret =
		(const struct resource_mem *) resource_get(dev, RESOURCE_MEM, idx);

	return (ret && (ret->addr != RESOURCE_MEM_NONE)) ? ret : NULL;
}

const struct resource_rst *device_resource_rst(struct device *dev, uint8_t idx)
{
	const struct resource_rst *ret =
		(const struct resource_rst *) resource_get(dev, RESOURCE_RST, idx);

	return (ret && (ret->bit != RESOURCE_RST_NONE)) ? ret : NULL;
}
