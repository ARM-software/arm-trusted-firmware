/*
 * Copyright (c) 2013, ARM Limited and Contributors. All rights reserved.
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

#ifndef __RUNTIME_SVC_H__
#define __RUNTIME_SVC_H__
#include <psci.h>
#include <bl_common.h>

/*******************************************************************************
 * Bit definitions inside the function id as per the SMC calling convention
 ******************************************************************************/
#define FUNCID_TYPE_SHIFT		31
#define FUNCID_CC_SHIFT			30
#define FUNCID_OWNER_SHIFT		24
#define FUNCID_NUM_SHIFT		0

#define FUNCID_TYPE_MASK		0x1
#define FUNCID_CC_MASK			0x1
#define FUNCID_OWNER_MASK		0x3f
#define FUNCID_NUM_MASK			0xffff

#define GET_SMC_CC(id)			((id >> FUNCID_CC_SHIFT) & \
					 FUNCID_CC_MASK)

#define SMC_64				1
#define SMC_32				0
#define SMC_UNK				0xffffffff

/*******************************************************************************
 * Constants to indicate type of exception to the common exception handler.
 ******************************************************************************/
#define SYNC_EXCEPTION_SP_EL0		0x0
#define IRQ_SP_EL0			0x1
#define FIQ_SP_EL0			0x2
#define SERROR_SP_EL0			0x3
#define SYNC_EXCEPTION_SP_ELX		0x4
#define IRQ_SP_ELX			0x5
#define FIQ_SP_ELX			0x6
#define SERROR_SP_ELX			0x7
#define SYNC_EXCEPTION_AARCH64		0x8
#define IRQ_AARCH64			0x9
#define FIQ_AARCH64			0xa
#define SERROR_AARCH64			0xb
#define SYNC_EXCEPTION_AARCH32		0xc
#define IRQ_AARCH32			0xd
#define FIQ_AARCH32			0xe
#define SERROR_AARCH32			0xf

/*******************************************************************************
 * Constants that allow assembler code to access members of the 'gp_regs'
 * structure at their correct offsets.
 ******************************************************************************/
#define SIZEOF_GPREGS		0x110
#define GPREGS_X0_OFF		0x0
#define GPREGS_X1_OFF		0x8
#define GPREGS_X2_OFF		0x10
#define GPREGS_X3_OFF		0x18
#define GPREGS_X4_OFF		0x20
#define GPREGS_X5_OFF		0x28
#define GPREGS_X6_OFF		0x30
#define GPREGS_X7_OFF		0x38
#define GPREGS_X8_OFF		0x40
#define GPREGS_X9_OFF		0x48
#define GPREGS_X10_OFF		0x50
#define GPREGS_X11_OFF		0x58
#define GPREGS_X12_OFF		0x60
#define GPREGS_X13_OFF		0x68
#define GPREGS_X14_OFF		0x70
#define GPREGS_X15_OFF		0x78
#define GPREGS_X16_OFF		0x80
#define GPREGS_X17_OFF		0x88
#define GPREGS_X18_OFF		0x90
#define GPREGS_X19_OFF		0x98
#define GPREGS_X20_OFF		0xA0
#define GPREGS_X21_OFF		0xA8
#define GPREGS_X22_OFF		0xB0
#define GPREGS_X23_OFF		0xB8
#define GPREGS_X24_OFF		0xC0
#define GPREGS_X25_OFF		0xC8
#define GPREGS_X26_OFF		0xD0
#define GPREGS_X27_OFF		0xD8
#define GPREGS_X28_OFF		0xE0
#define GPREGS_SP_EL0_OFF	0xE8
#define GPREGS_SPSR_OFF		0xF0
#define GPREGS_FP_OFF		0x100
#define GPREGS_LR_OFF		0x108

#ifndef __ASSEMBLY__

typedef struct {
	unsigned long x0;
	unsigned long x1;
	unsigned long x2;
	unsigned long x3;
	unsigned long x4;
	unsigned long x5;
	unsigned long x6;
	unsigned long x7;
	unsigned long x8;
	unsigned long x9;
	unsigned long x10;
	unsigned long x11;
	unsigned long x12;
	unsigned long x13;
	unsigned long x14;
	unsigned long x15;
	unsigned long x16;
	unsigned long x17;
	unsigned long x18;
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long sp_el0;
	unsigned long spsr;
	/*
	 * Alignment constraint which allows save & restore of fp & lr on the
	 * stack during exception handling
	 */
	unsigned long fp  __attribute__((__aligned__(16)));
	unsigned long lr;
} __attribute__((__aligned__(16))) gp_regs;

/*******************************************************************************
 * Compile time assertions to ensure that:
 * 1)  the assembler code's view of the size of the 'gp_regs' data structure is
 *     the same as the actual size of this data structure.
 * 2)  the assembler code's view of the offset of the frame pointer member of
 *     the 'gp_regs' structure is the same as the actual offset of this member.
 ******************************************************************************/
CASSERT((sizeof(gp_regs) == SIZEOF_GPREGS), assert_sizeof_gpregs_mismatch);
CASSERT(GPREGS_FP_OFF == __builtin_offsetof(gp_regs, fp), \
	assert_gpregs_fp_offset_mismatch);

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
extern void runtime_svc_init(unsigned long mpidr);

#endif /*__ASSEMBLY__*/
#endif /* __RUNTIME_SVC_H__ */
