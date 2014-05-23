/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __CCI_400_H__
#define __CCI_400_H__

/* Slave interface offsets from PERIPHBASE */
#define SLAVE_IFACE4_OFFSET		0x5000
#define SLAVE_IFACE3_OFFSET		0x4000
#define SLAVE_IFACE2_OFFSET		0x3000
#define SLAVE_IFACE1_OFFSET		0x2000
#define SLAVE_IFACE0_OFFSET		0x1000
#define SLAVE_IFACE_OFFSET(index)	SLAVE_IFACE0_OFFSET + (0x1000 * index)

/* Control and ID register offsets */
#define CTRL_OVERRIDE_REG		0x0
#define SPEC_CTRL_REG			0x4
#define SECURE_ACCESS_REG		0x8
#define STATUS_REG			0xc
#define IMPRECISE_ERR_REG		0x10
#define PERFMON_CTRL_REG		0x100

/* Slave interface register offsets */
#define SNOOP_CTRL_REG			0x0
#define SH_OVERRIDE_REG			0x4
#define READ_CHNL_QOS_VAL_OVERRIDE_REG	0x100
#define WRITE_CHNL_QOS_VAL_OVERRIDE_REG	0x104
#define QOS_CTRL_REG			0x10c
#define MAX_OT_REG			0x110
#define TARGET_LATENCY_REG		0x130
#define LATENCY_REGULATION_REG		0x134
#define QOS_RANGE_REG			0x138

/* Snoop Control register bit definitions */
#define DVM_EN_BIT			(1 << 1)
#define SNOOP_EN_BIT			(1 << 0)

/* Status register bit definitions */
#define CHANGE_PENDING_BIT		(1 << 0)

/* Function declarations */
void cci_enable_coherency(unsigned long mpidr);
void cci_disable_coherency(unsigned long mpidr);

#endif /* __CCI_400_H__ */
