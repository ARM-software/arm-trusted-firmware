/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Resource Management
 *
 * This module provides resource lookup and access functions for device
 * resources including memory, clocks, and resets stored in device tables.
 */

#include <limits.h>
#include <stddef.h>

#include <ti_device.h>
#include <ti_resource.h>

#define RESOURCE_TYPE_MASK	0xc0U

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
const void *resource_get(struct ti_device *dev, uint8_t type, uint8_t idx)
{
	/*
	 * The downshift here is just because the macro that defines the type
	 * starts at bit 6.
	 */
	static const uint8_t sizes[(RESOURCE_MEM >> 6U) + 1U] = {
		[RESOURCE_MEM >> 6U] = (uint8_t) sizeof(struct ti_resource_mem),
	};
	const struct ti_dev_data *ddata = get_dev_data(dev);
	const uint8_t *data;
	const uint8_t *r;
	uint8_t hdr;

	CASSERT(((sizeof(struct ti_resource_mem)) <= ((size_t) UCHAR_MAX)),
		     size_of_resource_mem);
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
