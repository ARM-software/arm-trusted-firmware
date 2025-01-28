/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_API_H
#define SPMI_API_H

#include <stdint.h>

#include <drivers/spmi/spmi_common.h>

/* external API */
int spmi_register_zero_write(struct spmi_device *dev, uint8_t data);
int spmi_register_read(struct spmi_device *dev, uint8_t addr, uint8_t *buf);
int spmi_register_write(struct spmi_device *dev, uint8_t addr, uint8_t data);
int spmi_ext_register_read(struct spmi_device *dev, uint8_t addr, uint8_t *buf,
			   uint8_t len);
int spmi_ext_register_write(struct spmi_device *dev, uint8_t addr,
			    const uint8_t *buf, uint8_t len);
int spmi_ext_register_readl(struct spmi_device *dev, uint16_t addr,
			    uint8_t *buf, uint8_t len);
int spmi_ext_register_writel(struct spmi_device *dev, uint16_t addr,
			     const uint8_t *buf, uint8_t len);
int spmi_ext_register_readl_field(struct spmi_device *dev, uint16_t addr,
				  uint8_t *buf, uint16_t mask, uint16_t shift);
int spmi_ext_register_writel_field(struct spmi_device *dev, uint16_t addr,
				   uint8_t data, uint16_t mask, uint16_t shift);
struct spmi_device *get_spmi_device(int mstid, int slvid);
int spmi_device_register(struct spmi_device *platform_spmi_dev, int num_devs);

#endif
