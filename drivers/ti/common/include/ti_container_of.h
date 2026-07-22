/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Container Offset Macro
 *
 * This header provides the ti_container_of macro for obtaining a pointer to
 * the enclosing structure given a pointer to a member within that structure.
 */

#ifndef TI_CONTAINER_OF_H
#define TI_CONTAINER_OF_H

#include <stddef.h>

#include <ti_build_assert.h>

#define ti_check_types_match(expr1, expr2) \
	TI_BUILD_ASSERT_OR_ZERO(sizeof(expr1) == sizeof(expr2))

/*
 * ti_container_of - get pointer to enclosing structure
 * @member_ptr: pointer to the structure member
 * @containing_type: the type this member is within
 * @member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 */
#define ti_container_of(member_ptr, containing_type, member) \
	 ((containing_type *) ((char *)(member_ptr) - offsetof(containing_type, member)) \
	  + ti_check_types_match(*(member_ptr), ((containing_type *)0)->member))

#endif /* TI_CONTAINER_OF_H */
