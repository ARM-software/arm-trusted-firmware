/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERRUPT_MGMT_H
#define INTERRUPT_MGMT_H

#include <arch.h>
#include <lib/utils_def.h>

/*******************************************************************************
 * Constants for the types of interrupts recognised by the IM framework
 ******************************************************************************/
#define INTR_TYPE_S_EL1			U(0)
#define INTR_TYPE_EL3			U(1)
#define INTR_TYPE_NS			U(2)
#define MAX_INTR_TYPES			U(3)
#define INTR_TYPE_INVAL			MAX_INTR_TYPES

/* Interrupt routing modes */
#define INTR_ROUTING_MODE_PE		0
#define INTR_ROUTING_MODE_ANY		1

/*
 * Constant passed to the interrupt handler in the 'id' field when the
 * framework does not read the gic registers to determine the interrupt id.
 */
#define INTR_ID_UNAVAILABLE		U(0xFFFFFFFF)


/*******************************************************************************
 * Mask for _both_ the routing model bits in the 'flags' parameter and
 * constants to define the valid routing models for each supported interrupt
 * type
 ******************************************************************************/
#define INTR_RM_FLAGS_SHIFT		U(0x0)
#define INTR_RM_FLAGS_MASK		U(0x3)
/* Routed to EL3 from NS. Taken to S-EL1 from Secure */
#define INTR_SEL1_VALID_RM0		U(0x2)
/* Routed to EL3 from NS and Secure */
#define INTR_SEL1_VALID_RM1		U(0x3)
/* Routed to EL1/EL2 from NS and to S-EL1 from Secure */
#define INTR_NS_VALID_RM0		U(0x0)
/* Routed to EL1/EL2 from NS and to EL3 from Secure */
#define INTR_NS_VALID_RM1		U(0x1)
/* Routed to EL3 from NS. Taken to S-EL1 from Secure and handed over to EL3 */
#define INTR_EL3_VALID_RM0		U(0x2)
/* Routed to EL3 from NS and Secure */
#define INTR_EL3_VALID_RM1		U(0x3)
/* This is the default routing model */
#define INTR_DEFAULT_RM			U(0x0)

/*******************************************************************************
 * Constants for the _individual_ routing model bits in the 'flags' field for
 * each interrupt type and mask to validate the 'flags' parameter while
 * registering an interrupt handler
 ******************************************************************************/
#define INTR_TYPE_FLAGS_MASK		U(0xFFFFFFFC)

#define INTR_RM_FROM_SEC_SHIFT		SECURE		/* BIT[0] */
#define INTR_RM_FROM_NS_SHIFT		NON_SECURE	/* BIT[1] */
#define INTR_RM_FROM_FLAG_MASK		U(1)
#define get_interrupt_rm_flag(flag, ss) \
	((((flag) >> INTR_RM_FLAGS_SHIFT) >> (ss)) & INTR_RM_FROM_FLAG_MASK)
#define set_interrupt_rm_flag(flag, ss)	((flag) |= U(1) << (ss))
#define clr_interrupt_rm_flag(flag, ss)	((flag) &= ~(U(1) << (ss)))

/*******************************************************************************
 * Macros to set the 'flags' parameter passed to an interrupt type handler. Only
 * the flag to indicate the security state when the exception was generated is
 * supported.
 ******************************************************************************/
#define INTR_SRC_SS_FLAG_SHIFT		U(0)		/* BIT[0] */
#define INTR_SRC_SS_FLAG_MASK		U(1)
#define set_interrupt_src_ss(flag, val)	((flag) |= (val) << INTR_SRC_SS_FLAG_SHIFT)
#define clr_interrupt_src_ss(flag)	((flag) &= ~(U(1) << INTR_SRC_SS_FLAG_SHIFT))
#define get_interrupt_src_ss(flag)	(((flag) >> INTR_SRC_SS_FLAG_SHIFT) & \
					 INTR_SRC_SS_FLAG_MASK)

#ifndef __ASSEMBLER__

#include <errno.h>
#include <stdint.h>

/*******************************************************************************
 * Helpers to validate the routing model bits in the 'flags' for a type
 * of interrupt. If the model does not match one of the valid masks
 * -EINVAL is returned.
 ******************************************************************************/
static inline int32_t validate_sel1_interrupt_rm(uint32_t x)
{
	if ((x == INTR_SEL1_VALID_RM0) || (x == INTR_SEL1_VALID_RM1))
		return 0;

	return -EINVAL;
}

static inline int32_t validate_ns_interrupt_rm(uint32_t x)
{
	if ((x == INTR_NS_VALID_RM0) || (x == INTR_NS_VALID_RM1))
		return 0;

	return -EINVAL;
}

static inline int32_t validate_el3_interrupt_rm(uint32_t x)
{
#if EL3_EXCEPTION_HANDLING
	/*
	 * With EL3 exception handling, EL3 interrupts are always routed to EL3
	 * from both Secure and Non-secure, and therefore INTR_EL3_VALID_RM1 is
	 * the only valid routing model.
	 */
	if (x == INTR_EL3_VALID_RM1)
		return 0;
#else
	if ((x == INTR_EL3_VALID_RM0) || (x == INTR_EL3_VALID_RM1))
		return 0;
#endif

	return -EINVAL;
}

/*******************************************************************************
 * Prototype for defining a handler for an interrupt type
 ******************************************************************************/
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
interrupt_type_handler_t get_interrupt_type_handler(uint32_t type);
int disable_intr_rm_local(uint32_t type, uint32_t security_state);
int enable_intr_rm_local(uint32_t type, uint32_t security_state);

#endif /*__ASSEMBLER__*/
#endif /* INTERRUPT_MGMT_H */
