/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Container Offset Macro
 *
 * This header provides the container_of macro for obtaining a pointer to
 * the enclosing structure given a pointer to a member within that structure.
 */

#ifndef CONTAINER_OF_H
#define CONTAINER_OF_H

#include <stddef.h>

#include <ti_build_assert.h>

#define check_types_match(expr1, expr2) \
	BUILD_ASSERT_OR_ZERO(sizeof(expr1) == sizeof(expr2))

/**
 * container_of - get pointer to enclosing structure
 * @member_ptr: pointer to the structure member
 * @containing_type: the type this member is within
 * @member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 */
#define container_of(member_ptr, containing_type, member) \
	 ((containing_type *) ((char *)(member_ptr) - offsetof(containing_type, member)) \
	  + check_types_match(*(member_ptr), ((containing_type *)0)->member))

#endif /* CONTAINER_OF_H */

