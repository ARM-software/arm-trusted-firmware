/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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
#ifndef __PLAT_CONFIG_H__
#define __PLAT_CONFIG_H__

#define CONFIG_GICC_BASE_OFFSET		0x4


#ifndef __ASSEMBLY__

#include <cassert.h>


enum plat_config_flags {
	/* Whether Base FVP memory map is in use */
	CONFIG_BASE_MMAP		= 0x1,
	/* Whether CCI should be enabled */
	CONFIG_HAS_CCI			= 0x2,
	/* Whether TZC should be configured */
	CONFIG_HAS_TZC			= 0x4
};

typedef struct plat_config {
	unsigned int gicd_base;
	unsigned int gicc_base;
	unsigned int gich_base;
	unsigned int gicv_base;
	unsigned int max_aff0;
	unsigned int max_aff1;
	unsigned long flags;
} plat_config_t;

inline const plat_config_t *get_plat_config();


CASSERT(CONFIG_GICC_BASE_OFFSET == __builtin_offsetof(
	plat_config_t, gicc_base),
	assert_gicc_base_offset_mismatch);

/* If used, plat_config must be defined and populated in the platform port*/
extern plat_config_t plat_config;

inline const plat_config_t *get_plat_config()
{
	return &plat_config;
}


#endif /* __ASSEMBLY__ */

#endif /* __PLAT_CONFIG_H__ */
