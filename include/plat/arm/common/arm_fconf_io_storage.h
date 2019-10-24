/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_FCONF_IO_STORAGE_H
#define ARM_FCONF_IO_STORAGE_H

#include <stdint.h>

/* IO devices handle */
extern uintptr_t memmap_dev_handle;
extern uintptr_t fip_dev_handle;

/* Function declarations */
int open_fip(const uintptr_t spec);
int open_memmap(const uintptr_t spec);

#endif /* ARM_FCONF_IO_STORAGE_H */
