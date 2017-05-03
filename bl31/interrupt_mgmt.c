/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <errno.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <stdio.h>

/*******************************************************************************
 * Local structure and corresponding array to keep track of the state of the
 * registered interrupt handlers for each interrupt type.
 * The field descriptions are:
 *
 * 'flags' : Bit[0], Routing model for this interrupt type when execution is
 *                   not in EL3 in the secure state. '1' implies that this
 *                   interrupt will be routed to EL3. '0' implies that this
 *                   interrupt will be routed to the current exception level.
 *
 *           Bit[1], Routing model for this interrupt type when execution is
 *                   not in EL3 in the non-secure state. '1' implies that this
 *                   interrupt will be routed to EL3. '0' implies that this
 *                   interrupt will be routed to the current exception level.
 *
 *           All other bits are reserved and SBZ.
 *
 * 'scr_el3[2]'  : Mapping of the routing model in the 'flags' field to the
 *                 value of the SCR_EL3.IRQ or FIQ bit for each security state.
 *                 There are two instances of this field corresponding to the
 *                 two security states.
 ******************************************************************************/
typedef struct intr_type_desc {
	interrupt_type_handler_t handler;
	uint32_t flags;
	uint32_t scr_el3[2];
} intr_type_desc_t;

static intr_type_desc_t intr_type_descs[MAX_INTR_TYPES];

/*******************************************************************************
 * This function validates the interrupt type.
 ******************************************************************************/
static int32_t validate_interrupt_type(uint32_t type)
{
	if (type == INTR_TYPE_S_EL1 || type == INTR_TYPE_NS ||
			type == INTR_TYPE_EL3)
		return 0;

	return -EINVAL;
}

/*******************************************************************************
* This function validates the routing model for this type of interrupt
 ******************************************************************************/
static int32_t validate_routing_model(uint32_t type, uint32_t flags)
{
	flags >>= INTR_RM_FLAGS_SHIFT;
	flags &= INTR_RM_FLAGS_MASK;

	if (type == INTR_TYPE_S_EL1)
		return validate_sel1_interrupt_rm(flags);

	if (type == INTR_TYPE_NS)
		return validate_ns_interrupt_rm(flags);

	if (type == INTR_TYPE_EL3)
		return validate_el3_interrupt_rm(flags);

	return -EINVAL;
}

/*******************************************************************************
 * This function returns the cached copy of the SCR_EL3 which contains the
 * routing model (expressed through the IRQ and FIQ bits) for a security state
 * which was stored through a call to 'set_routing_model()' earlier.
 ******************************************************************************/
uint32_t get_scr_el3_from_routing_model(uint32_t security_state)
{
	uint32_t scr_el3;

	assert(sec_state_is_valid(security_state));
	scr_el3 = intr_type_descs[INTR_TYPE_NS].scr_el3[security_state];
	scr_el3 |= intr_type_descs[INTR_TYPE_S_EL1].scr_el3[security_state];
	scr_el3 |= intr_type_descs[INTR_TYPE_EL3].scr_el3[security_state];
	return scr_el3;
}

/*******************************************************************************
 * This function uses the 'interrupt_type_flags' parameter to obtain the value
 * of the trap bit (IRQ/FIQ) in the SCR_EL3 for a security state for this
 * interrupt type. It uses it to update the SCR_EL3 in the cpu context and the
 * 'intr_type_desc' for that security state.
 ******************************************************************************/
static void set_scr_el3_from_rm(uint32_t type,
				uint32_t interrupt_type_flags,
				uint32_t security_state)
{
	uint32_t flag, bit_pos;

	flag = get_interrupt_rm_flag(interrupt_type_flags, security_state);
	bit_pos = plat_interrupt_type_to_line(type, security_state);
	intr_type_descs[type].scr_el3[security_state] = flag << bit_pos;

	/* Update scr_el3 only if there is a context available. If not, it
	 * will be updated later during context initialization which will obtain
	 * the scr_el3 value to be used via get_scr_el3_from_routing_model() */
	if (cm_get_context(security_state))
		cm_write_scr_el3_bit(security_state, bit_pos, flag);
}

/*******************************************************************************
 * This function validates the routing model specified in the 'flags' and
 * updates internal data structures to reflect the new routing model. It also
 * updates the copy of SCR_EL3 for each security state with the new routing
 * model in the 'cpu_context' structure for this cpu.
 ******************************************************************************/
int32_t set_routing_model(uint32_t type, uint32_t flags)
{
	int32_t rc;

	rc = validate_interrupt_type(type);
	if (rc)
		return rc;

	rc = validate_routing_model(type, flags);
	if (rc)
		return rc;

	/* Update the routing model in internal data structures */
	intr_type_descs[type].flags = flags;
	set_scr_el3_from_rm(type, flags, SECURE);
	set_scr_el3_from_rm(type, flags, NON_SECURE);

	return 0;
}

/******************************************************************************
 * This function disables the routing model of interrupt 'type' from the
 * specified 'security_state' on the local core. The disable is in effect
 * till the core powers down or till the next enable for that interrupt
 * type.
 *****************************************************************************/
int disable_intr_rm_local(uint32_t type, uint32_t security_state)
{
	uint32_t bit_pos, flag;

	assert(intr_type_descs[type].handler);

	flag = get_interrupt_rm_flag(INTR_DEFAULT_RM, security_state);

	bit_pos = plat_interrupt_type_to_line(type, security_state);
	cm_write_scr_el3_bit(security_state, bit_pos, flag);

	return 0;
}

/******************************************************************************
 * This function enables the routing model of interrupt 'type' from the
 * specified 'security_state' on the local core.
 *****************************************************************************/
int enable_intr_rm_local(uint32_t type, uint32_t security_state)
{
	uint32_t bit_pos, flag;

	assert(intr_type_descs[type].handler);

	flag = get_interrupt_rm_flag(intr_type_descs[type].flags,
				security_state);

	bit_pos = plat_interrupt_type_to_line(type, security_state);
	cm_write_scr_el3_bit(security_state, bit_pos, flag);

	return 0;
}

/*******************************************************************************
 * This function registers a handler for the 'type' of interrupt specified. It
 * also validates the routing model specified in the 'flags' for this type of
 * interrupt.
 ******************************************************************************/
int32_t register_interrupt_type_handler(uint32_t type,
					interrupt_type_handler_t handler,
					uint32_t flags)
{
	int32_t rc;

	/* Validate the 'handler' parameter */
	if (!handler)
		return -EINVAL;

	/* Validate the 'flags' parameter */
	if (flags & INTR_TYPE_FLAGS_MASK)
		return -EINVAL;

	/* Check if a handler has already been registered */
	if (intr_type_descs[type].handler)
		return -EALREADY;

	rc = set_routing_model(type, flags);
	if (rc)
		return rc;

	/* Save the handler */
	intr_type_descs[type].handler = handler;

	return 0;
}

/*******************************************************************************
 * This function is called when an interrupt is generated and returns the
 * handler for the interrupt type (if registered). It returns NULL if the
 * interrupt type is not supported or its handler has not been registered.
 ******************************************************************************/
interrupt_type_handler_t get_interrupt_type_handler(uint32_t type)
{
	if (validate_interrupt_type(type))
		return NULL;

	return intr_type_descs[type].handler;
}

