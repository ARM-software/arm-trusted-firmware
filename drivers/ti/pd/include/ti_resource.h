/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Resource Definitions
 *
 * This header defines compact device resource storage and access mechanisms
 * for SoC-independent device interaction, including memory resource data
 * structures and resource lookup functions.
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <common/debug.h>

#include <ti_device_clk.h>
#include <ti_io.h>
#include <ti_pm_types.h>
#include <ti_soc_device.h>
/*
 * A compact method of storing device resources. These are meant to allow
 * SoC independent interaction with devices. For instance, for device foo,
 * clk 0 is always the functional clock and clk 1 is always the interface
 * clock, even though their device/clock framework IDs may differ. Only
 * resources utilized by drivers within the System Firmware are included.
 *
 * Not all devices will have the full set of resources supported by a driver,
 * if the missing resources are at the end of the resource list, the
 * resoure table can just define fewer resources. If the resource is in
 * the middle of the resource list, the resource table should include
 * RESOURCE_xxx_NONE for the missing resources.
 */

/**
 * \brief Resource type flag for memory resources
 *
 * Identifies a resource descriptor as containing memory address information.
 * Used in resource headers to indicate the following data describes memory
 * resources for device access.
 */
#define RESOURCE_MEM	0x80U

/*
 * Each resource set includes a set of resources including a resource
 * header followed by count resources. The last resource header has
 * a it's last flag set. Resource types can be defined in any order.
 */

/**
 * \brief Flag indicating the last resource header in a resource set
 *
 * When set in a resource header byte, indicates this is the final resource
 * descriptor in the device's resource list. Used for resource parsing
 * termination.
 */
#define RESOURCE_LAST		0x20U

/**
 * \brief Mask for extracting resource count from header byte
 *
 * Applied to the resource header byte to extract the number of resources
 * of this type that follow. Supports up to 31 resources of each type.
 */
#define RESOURCE_COUNT_MASK	0x1fU

/**
 * \brief Sentinel value indicating no memory resource
 *
 * Used in resource tables to mark a missing or invalid memory resource entry.
 * Allows gaps in resource lists without requiring complete table
 * reorganization.
 */
#define RESOURCE_MEM_NONE	0xffffffffU

/**
 * \brief Memory resource descriptor
 *
 * Contains a single 32-bit physical address for a device memory resource.
 * Packed structure for space-efficient storage in resource tables.
 */
struct ti_resource_mem {
	/** Physical base address of the memory resource */
	uint32_t addr;
} __packed;

/**
 * \brief Construct a resource header byte
 *
 * Combines resource type, count, and last-flag into a single byte for compact
 * resource table encoding.
 *
 * \param type Resource type (e.g., RESOURCE_MEM)
 * \param count Number of resources of this type (0-31)
 * \param last Non-zero if this is the last resource header, 0 otherwise
 *
 * \return Encoded resource header byte
 */
#define RDAT_HDR(type, count, last) ((uint8_t) (type) | (uint8_t) (count) | \
				     (uint8_t) ((last) ? RESOURCE_LAST : 0U))

/**
 * \brief Encode a 32-bit memory address as four bytes
 *
 * Converts a memory address into a comma-separated sequence of four byte
 * values in little-endian order for direct inclusion in static resource
 * table initializers.
 *
 * \param mem 32-bit physical memory address
 *
 * \return Comma-separated byte sequence (for use in array initializers)
 */
#define RDAT_MEM(mem)		(uint8_t) ((uint32_t) (mem) & 0xffU),	     \
				(uint8_t) (((uint32_t) (mem) >> 8) & 0xffU), \
				(uint8_t) (((uint32_t) (mem) >> 16) & 0xffU),\
				(uint8_t) ((uint32_t) (mem) >> 24)

struct ti_device;

/**
 * \brief Retrieve a specific resource from a device
 *
 * Searches the device's resource table for a resource of the specified type
 * and index, returning a pointer to the resource data.
 *
 * \param dev Pointer to the device structure
 * \param type Resource type to search for (e.g., RESOURCE_MEM)
 * \param idx Zero-based index of the resource within its type
 *
 * \return Pointer to the resource data, or NULL if not found
 */
const void *resource_get(struct ti_device *dev, uint8_t type, uint8_t idx);

#endif
