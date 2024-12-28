/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CONTAINER_OF_H
#define CONTAINER_OF_H

#include <stddef.h>
#include <check_type.h>

/**
 * container_of - get pointer to enclosing structure
 * \param member_ptr: pointer to the structure member
 * \param containing_type: the type this member is within
 * \param member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 *
 * Example:
 *	struct foo {
 *		int32_t fielda, fieldb;
 *		// ...
 *	};
 *	struct info {
 *		int32_t some_other_field;
 *		struct foo my_foo;
 *	};
 *
 *	static struct info *foo_to_info(struct foo *foo)
 *	{
 *		return container_of(foo, struct info, my_foo);
 *	}
 */
#define container_of(member_ptr, containing_type, member)		\
	((containing_type *)						\
	 ((char *) (member_ptr)						\
	  - container_off(containing_type, member))			\
	 + (check_types_match(*(member_ptr), ((containing_type *) 0)->member) \
	    ? 1 : 0))

/**
 * container_off - get offset to enclosing structure
 * \param containing_type: the type this member is within
 * \param member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does
 * typechecking and figures out the offset to the enclosing type.
 *
 * Example:
 *	struct foo {
 *		int32_t fielda, fieldb;
 *		// ...
 *	};
 *	struct info {
 *		int32_t some_other_field;
 *		struct foo my_foo;
 *	};
 *
 *	static struct info *foo_to_info(struct foo *foo)
 *	{
 *		size_t off = container_off(struct info, my_foo);
 *		return (void *)((char *)foo - off);
 *	}
 */
#define container_off(containing_type, member)	\
	offsetof(containing_type, member)

/**
 * container_of_var - get pointer to enclosing structure using a variable
 * \param member_ptr: pointer to the structure member
 * \param container_var: a pointer of same type as this member's container
 * \param member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 *
 * Example:
 *	static struct info *foo_to_i(struct foo *foo)
 *	{
 *		struct info *i = container_of_var(foo, i, my_foo);
 *		return i;
 *	}
 */
#define container_of_var(member_ptr, container_var, member)		\
	container_of(member_ptr, typeof(*container_var), member)

/**
 * container_off_var - get offset of a field in enclosing structure
 * \param var: a pointer to a container structure
 * \param member: the name of a member within the structure.
 *
 * Given (any) pointer to a structure and a its member name, this
 * macro does pointer subtraction to return offset of member in a
 * structure memory layout.
 *
 */
#define container_off_var(var, member)		\
	container_off(typeof(*var), member)

#endif /* _CONTAINER_OF_ */
