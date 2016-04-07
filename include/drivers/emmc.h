/*
 *    Copyright 2016 Broadcom
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __EMMC_H__
#define __EMMC_H__

#include <stdint.h>

uint32_t emmc_init(void);

uint32_t emmc_deinit(void);

uint32_t emmc_partition_select(uint32_t partition);

uint32_t emmc_read(uintptr_t mem_addr, uintptr_t storage_addr,
		   size_t storage_size, size_t bytes_to_read);

uint32_t emmc_write(uintptr_t mem_addr, uintptr_t data_addr,
		    size_t bytes_to_write);

#endif
