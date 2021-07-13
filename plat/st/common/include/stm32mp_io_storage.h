/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STM32MP_IO_STORAGE_H
#define STM32MP_IO_STORAGE_H

#include <stdint.h>

#include <drivers/io/io_storage.h>

/* IO devices handle */
extern uintptr_t storage_dev_handle;
extern uintptr_t fip_dev_handle;

extern io_block_spec_t image_block_spec;

/* Function declarations */
int open_fip(const uintptr_t spec);
int open_storage(const uintptr_t spec);

#endif /* STM32MP_IO_STORAGE_H */
