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

#ifndef __PLAT_PRIVATE_H__
#define __PLAT_PRIVATE_H__
/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define DEVINFO_SIZE 4
#define LINUX_KERNEL_32 0

typedef struct {
	unsigned int atf_magic;
	unsigned int tee_support;
	unsigned int tee_entry;
	unsigned int tee_boot_arg_addr;
	unsigned int hwuid[4];     /* HW Unique id for t-base used */
	unsigned int HRID[2];      /* HW random id for t-base used */
	unsigned int atf_log_port;
	unsigned int atf_log_baudrate;
	unsigned int atf_log_buf_start;
	unsigned int atf_log_buf_size;
	unsigned int atf_irq_num;
	unsigned int devinfo[DEVINFO_SIZE];
	unsigned int atf_aee_debug_buf_start;
	unsigned int atf_aee_debug_buf_size;
} atf_arg_t, *atf_arg_t_ptr;

void plat_configure_mmu_el3(unsigned long total_base,
					unsigned long total_size,
					unsigned long,
					unsigned long,
					unsigned long,
					unsigned long);

void plat_cci_init(void);
void plat_cci_enable(void);
void plat_cci_disable(void);

/* Declarations for plat_mt_gic.c */
void plat_mt_gic_init(void);

/* Declarations for plat_topology.c */
int mt_setup_topology(void);

/* Declarations for mt_cpuxgpt.c */
void setup_syscnt(void);

/* Declarations for bl31_plat_setup.c */
void plat_delay_timer_init(void);
uint32_t plat_get_spsr_for_bl32_entry(void);
uint32_t plat_get_spsr_for_bl33_entry(void);
void set_kernel_k32_64(uint64_t k32_64);
void bl31_prepare_kernel_entry(uint64_t k32_64);
void enable_ns_access_to_cpuectlr(void);

/* Declarations for platform_commnon.c */
extern uint64_t wdt_kernel_cb_addr;

extern atf_arg_t gteearg;

#endif /* __PLAT_PRIVATE_H__ */
