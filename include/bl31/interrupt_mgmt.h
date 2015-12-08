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

#ifndef __INTERRUPT_MGMT_H__
#define __INTERRUPT_MGMT_H__

#include <arch.h>

/*******************************************************************************
 * Constants for the types of interrupts recognised by the IM framework
 ******************************************************************************/
#define INTR_TYPE_S_EL1			0
#define INTR_TYPE_EL3			1
#define INTR_TYPE_NS			2
#define MAX_INTR_TYPES			3
#define INTR_TYPE_INVAL			MAX_INTR_TYPES
/*
 * Constant passed to the interrupt handler in the 'id' field when the
 * framework does not read the gic registers to determine the interrupt id.
 */
#define INTR_ID_UNAVAILABLE		0xFFFFFFFF


/*******************************************************************************
 * Mask for _both_ the routing model bits in the 'flags' parameter and
 * constants to define the valid routing models for each supported interrupt
 * type
 ******************************************************************************/
#define INTR_RM_FLAGS_SHIFT		0x0
#define INTR_RM_FLAGS_MASK		0x3
/* Routed to EL3 from NS. Taken to S-EL1 from Secure */
#define INTR_SEL1_VALID_RM0		0x2
/* Routed to EL3 from NS and Secure */
#define INTR_SEL1_VALID_RM1		0x3
/* Routed to EL1/EL2 from NS and to S-EL1 from Secure */
#define INTR_NS_VALID_RM0		0x0
/* Routed to EL1/EL2 from NS and to EL3 from Secure */
#define INTR_NS_VALID_RM1		0x1
/* Routed to EL3 from NS. Taken to S-EL1 from Secure and handed over to EL3 */
#define INTR_EL3_VALID_RM0		0x2
/* Routed to EL3 from NS and Secure */
#define INTR_EL3_VALID_RM1		0x3
/* This is the default routing model */
#define INTR_DEFAULT_RM		0x0

/*******************************************************************************
 * Constants for the _individual_ routing model bits in the 'flags' field for
 * each interrupt type and mask to validate the 'flags' parameter while
 * registering an interrupt handler
 ******************************************************************************/
#define INTR_TYPE_FLAGS_MASK		0xFFFFFFFC

#define INTR_RM_FROM_SEC_SHIFT		SECURE		/* BIT[0] */
#define INTR_RM_FROM_NS_SHIFT		NON_SECURE	/* BIT[1] */
#define INTR_RM_FROM_FLAG_MASK		1
#define get_interrupt_rm_flag(flag, ss)	(((flag >> INTR_RM_FLAGS_SHIFT) >> ss) \
					 & INTR_RM_FROM_FLAG_MASK)
#define set_interrupt_rm_flag(flag, ss)	(flag |= 1 << ss)
#define clr_interrupt_rm_flag(flag, ss)	(flag &= ~(1 << ss))


/*******************************************************************************
 * Macros to validate the routing model bits in the 'flags' for a type
 * of interrupt. If the model does not match one of the valid masks
 * -EINVAL is returned.
 ******************************************************************************/
#define validate_sel1_interrupt_rm(x)	((x) == INTR_SEL1_VALID_RM0 ? 0 : \
					 ((x) == INTR_SEL1_VALID_RM1 ? 0 :\
					  -EINVAL))

#define validate_ns_interrupt_rm(x)	((x) == INTR_NS_VALID_RM0 ? 0 : \
					 ((x) == INTR_NS_VALID_RM1 ? 0 :\
					  -EINVAL))

#define validate_el3_interrupt_rm(x)	((x) == INTR_EL3_VALID_RM0 ? 0 : \
					 ((x) == INTR_EL3_VALID_RM1 ? 0 :\
					  -EINVAL))

/*******************************************************************************
 * Macros to set the 'flags' parameter passed to an interrupt type handler. Only
 * the flag to indicate the security state when the exception was generated is
 * supported.
 ******************************************************************************/
#define INTR_SRC_SS_FLAG_SHIFT		0		/* BIT[0] */
#define INTR_SRC_SS_FLAG_MASK		1
#define set_interrupt_src_ss(flag, val)	(flag |= val << INTR_SRC_SS_FLAG_SHIFT)
#define clr_interrupt_src_ss(flag)	(flag &= ~(1 << INTR_SRC_SS_FLAG_SHIFT))
#define get_interrupt_src_ss(flag)	((flag >> INTR_SRC_SS_FLAG_SHIFT) & \
					 INTR_SRC_SS_FLAG_MASK)

#ifndef __ASSEMBLY__

/* Prototype for defining a handler for an interrupt type */
typedef uint64_t (*interrupt_type_handler_t)(uint32_t id,
					     uint32_t flags,
					     void *handle,
					     void *cookie);

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
uint32_t get_scr_el3_from_routing_model(uint32_t security_state);
int32_t set_routing_model(uint32_t type, uint32_t flags);
int32_t register_interrupt_type_handler(uint32_t type,
					interrupt_type_handler_t handler,
					uint32_t flags);
interrupt_type_handler_t get_interrupt_type_handler(uint32_t interrupt_type);
int disable_intr_rm_local(uint32_t type, uint32_t security_state);
int enable_intr_rm_local(uint32_t type, uint32_t security_state);

#endif /*__ASSEMBLY__*/
#endif /* __INTERRUPT_MGMT_H__ */
