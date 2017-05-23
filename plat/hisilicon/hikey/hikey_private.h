/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __HIKEY_PRIVATE_H__
#define __HIKEY_PRIVATE_H__

#include <bl_common.h>

#define RANDOM_MAX		0x7fffffffffffffff
#define RANDOM_MAGIC		0x9a4dbeaf

struct random_serial_num {
	uint64_t	magic;
	uint64_t	data;
	char		serialno[32];
};

/*
 * Function and variable prototypes
 */
void hikey_init_mmu_el1(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);
void hikey_init_mmu_el3(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);

void hikey_ddr_init(void);
void hikey_io_setup(void);

int hikey_get_partition_size(const char *arg, int left, char *response);
int hikey_get_partition_type(const char *arg, int left, char *response);

int hikey_erase(const char *arg);
int hikey_flash(const char *arg);
int hikey_oem(const char *arg);
int hikey_reboot(const char *arg);

const char *hikey_init_serialno(void);
int hikey_read_serialno(struct random_serial_num *serialno);
int hikey_write_serialno(struct random_serial_num *serialno);

void init_acpu_dvfs(void);

#endif /* __HIKEY_PRIVATE_H__ */
