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

#ifndef __FVP_PRIVATE_H__
#define __FVP_PRIVATE_H__

#include <platform_def.h>


typedef volatile struct mailbox {
	unsigned long value
	__attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));
} mailbox_t;


/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct meminfo;

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void configure_mmu_el1(struct meminfo *,
			unsigned long,
			unsigned long,
			unsigned long,
			unsigned long);
void configure_mmu_el3(struct meminfo *,
			unsigned long,
			unsigned long,
			unsigned long,
			unsigned long);
unsigned long platform_get_cfgvar(unsigned int);
int platform_config_setup(void);

/* Declarations for fvp_gic.c */
void gic_cpuif_deactivate(unsigned int);
void gic_cpuif_setup(unsigned int);
void gic_pcpu_distif_setup(unsigned int);
void gic_setup(void);

/* Declarations for fvp_topology.c */
int plat_setup_topology(void);

/* Declarations for plat_io_storage.c */
void io_setup(void);

/* Declarations for plat_security.c */
void plat_security_setup(void);


#endif /* __FVP_PRIVATE_H__ */
