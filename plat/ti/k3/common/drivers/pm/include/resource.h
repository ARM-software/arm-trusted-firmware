/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <types/short_types.h>
#include <lib/io.h>
#include <pm_types.h>
#include <soc_device.h>
#include <device_clk.h>
#include <common/debug.h>
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

#define RESOURCE_IRQ    0x00U
#define RESOURCE_CLK    0x40U
#define RESOURCE_MEM    0x80U
#define RESOURCE_RST    0xc0U

/*
 * Each resource set includes a set of resources including a resource
 * header followed by count resources. The last resource header has
 * a it's last flag set. Resource types can be defined in any order.
 */
#define RESOURCE_TYPE_MASK      0xc0U
#define RESOURCE_LAST           0x20U
#define RESOURCE_COUNT_MASK     0x1fU

#define RESOURCE_CLK_NONE       DEV_CLK_ID_NONE

struct resource_clk {
	dev_clk_idx_t clk_id;
} __packed;

#define RESOURCE_MEM_NONE       0xffffffffU

struct resource_mem {
	uint32_t addr;
} __packed;

#define RESOURCE_RST_NONE       255U

struct resource_rst {
	/**
	 * Bitfield index of device reset for this reset resource.
	 * Set to 255 if this reset is not present.
	 */
	uint8_t bit;
} __packed;

#define RDAT_HDR(type, count, last) ((uint8_t) (type) | (uint8_t) (count) | \
				     (uint8_t) ((last) ? RESOURCE_LAST : 0U))
#define RDAT_CLK(clk)           (clk)

#define RDAT_MEM(mem)           (uint8_t) ((uint32_t) (mem) & 0xffU),	\
	(uint8_t) (((uint32_t) (mem) >> 8) & 0xffU),		\
	(uint8_t) (((uint32_t) (mem) >> 16) & 0xffU),		\
	(uint8_t) ((uint32_t) (mem) >> 24)

#define RDAT_RST(rst)           (rst)

/*
 * The stucts these return are not aligned and packed. They return NULL if
 * no such resource is present, never resources with the RESOURCE_xxx_NONE
 * value.
 */
struct device;

/**
 * \brief Return a clock device resource.
 *
 * \param dev
 * Which device the resource table is attached to.
 *
 * \param idx
 * There can be multiple resources of each type. This indicates which one
 * to return.
 *
 * \return
 * The desired clock resource, or NULL if the device has no such resource.
 * Device resources are packed data and the alignment of the returned data
 * will not be natural.
 */
const struct resource_clk *device_resource_clk(struct device *dev, uint8_t idx);

/**
 * \brief Return a memory device resource.
 *
 * \param dev
 * Which device the resource table is attached to.
 *
 * \param idx
 * There can be multiple resources of each type. This indicates which one
 * to return.
 *
 * \return
 * The desired memory resource, or NULL if the device has no such resource.
 * Device resources are packed data and the alignment of the returned data
 * will not be natural.
 */
const struct resource_mem *device_resource_mem(struct device *dev, uint8_t idx);

/**
 * \brief Return a reset device resource.
 *
 * \param dev
 * Which device the resource table is attached to.
 *
 * \param idx
 * There can be multiple resources of each type. This indicates which one
 * to return.
 *
 * \return
 * The desired reset resource, or NULL if the device has no such resource.
 * Device resources are packed data and the alignment of the returned data
 * will not be natural.
 */
const struct resource_rst *device_resource_rst(struct device *dev, uint8_t idx);

static inline uint32_t mem_readl(const struct resource_mem *mem, uint32_t off)
{
	return readl(mem->addr + off);
}

// CHANGE ME:
static inline void mem_ti_clk_writel(const struct resource_mem *mem, uint32_t a, uint32_t off)
{
	ti_clk_writel(a, mem->addr + off);
}

static inline uint16_t mem_readw(const struct resource_mem *mem, uint32_t off)
{
	return readw(mem->addr + off);
}

static inline void mem_writew(const struct resource_mem *mem, uint16_t a, uint32_t off)
{
	writew(a, mem->addr + off);
}

static inline uint8_t mem_readb(const struct resource_mem *mem, uint32_t off)
{
	return readb(mem->addr + off);
}

static inline void mem_writeb(const struct resource_mem *mem, uint8_t a, uint32_t off)
{
	writeb(a, mem->addr + off);
}

#endif
