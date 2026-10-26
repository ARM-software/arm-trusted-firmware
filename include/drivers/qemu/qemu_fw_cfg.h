/*
 * Copyright (c) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QEMU_FW_CFG_H
#define QEMU_FW_CFG_H

#include <stddef.h>
#include <stdint.h>

int qemu_fw_cfg_init(uintptr_t base);
int qemu_fw_cfg_find_file(const char *name, uint16_t *select, size_t *size);
void qemu_fw_cfg_select(uint16_t key);
void qemu_fw_cfg_read_bytes(uint8_t *buf, size_t len);
void qemu_fw_cfg_skip_bytes(size_t len);

#endif /* QEMU_FW_CFG_H */
