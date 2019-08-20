/*
 * Copyright (c) 2016, Linaro Limited
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <arch_helpers.h>
#include <context.h>
#include <common/debug.h>
#include <plat/common/platform.h>

struct source_location {
	const char *file_name;
	uint32_t line;
	uint32_t column;
};

struct type_descriptor {
	uint16_t type_kind;
	uint16_t type_info;
	char type_name[1];
};

struct type_mismatch_data {
	struct source_location loc;
	struct type_descriptor *type;
	unsigned long alignment;
	unsigned char type_check_kind;
};

struct overflow_data {
	struct source_location loc;
	struct type_descriptor *type;
};

struct shift_out_of_bounds_data {
	struct source_location loc;
	struct type_descriptor *lhs_type;
	struct type_descriptor *rhs_type;
};

struct out_of_bounds_data {
	struct source_location loc;
	struct type_descriptor *array_type;
	struct type_descriptor *index_type;
};

struct unreachable_data {
	struct source_location loc;
};

struct vla_bound_data {
	struct source_location loc;
	struct type_descriptor *type;
};

struct invalid_value_data {
	struct source_location loc;
	struct type_descriptor *type;
};

struct nonnull_arg_data {
	struct source_location loc;
};

/*
 * When compiling with -fsanitize=undefined the compiler expects functions
 * with the following signatures. The functions are never called directly,
 * only when undefined behavior is detected in instrumented code.
 */
void __ubsan_handle_type_mismatch_abort(struct type_mismatch_data *data,
					unsigned long ptr);
void __ubsan_handle_type_mismatch_v1_abort(struct type_mismatch_data *data,
					   unsigned long ptr);
void __ubsan_handle_add_overflow_abort(struct overflow_data *data,
					unsigned long lhs, unsigned long rhs);
void __ubsan_handle_sub_overflow_abort(struct overflow_data *data,
				       unsigned long lhs, unsigned long rhs);
void __ubsan_handle_mul_overflow_abort(struct overflow_data *data,
				       unsigned long lhs, unsigned long rhs);
void __ubsan_handle_negate_overflow_abort(struct overflow_data *data,
					  unsigned long old_val);
void __ubsan_handle_pointer_overflow_abort(struct overflow_data *data,
					   unsigned long old_val);
void __ubsan_handle_divrem_overflow_abort(struct overflow_data *data,
					  unsigned long lhs, unsigned long rhs);
void __ubsan_handle_shift_out_of_bounds_abort(struct shift_out_of_bounds_data *data,
					      unsigned long lhs, unsigned long rhs);
void __ubsan_handle_out_of_bounds_abort(struct out_of_bounds_data *data,
					unsigned long idx);
void __ubsan_handle_unreachable_abort(struct unreachable_data *data);
void __ubsan_handle_missing_return_abort(struct unreachable_data *data);
void __ubsan_handle_vla_bound_not_positive_abort(struct vla_bound_data *data,
						 unsigned long bound);
void __ubsan_handle_load_invalid_value_abort(struct invalid_value_data *data,
					     unsigned long val);
void __ubsan_handle_nonnull_arg_abort(struct nonnull_arg_data *data
#if __GCC_VERSION < 60000
				    , size_t arg_no
#endif
				      );

static void print_loc(const char *func, struct source_location *loc)
{
	ERROR("Undefined behavior at %s:%d col %d (%s)",
		loc->file_name, loc->line, loc->column, func);
}


void __ubsan_handle_type_mismatch_abort(struct type_mismatch_data *data,
					unsigned long ptr __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_type_mismatch_v1_abort(struct type_mismatch_data *data,
					unsigned long ptr __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_add_overflow_abort(struct overflow_data *data,
				       unsigned long lhs __unused,
				       unsigned long rhs __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_sub_overflow_abort(struct overflow_data *data,
				       unsigned long lhs __unused,
				       unsigned long rhs __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_mul_overflow_abort(struct overflow_data *data,
				       unsigned long lhs __unused,
				       unsigned long rhs __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_negate_overflow_abort(struct overflow_data *data,
					  unsigned long old_val __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_pointer_overflow_abort(struct overflow_data *data,
					  unsigned long old_val __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_divrem_overflow_abort(struct overflow_data *data,
					  unsigned long lhs __unused,
					  unsigned long rhs __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_shift_out_of_bounds_abort(struct shift_out_of_bounds_data *data,
					      unsigned long lhs __unused,
					      unsigned long rhs __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_out_of_bounds_abort(struct out_of_bounds_data *data,
					unsigned long idx __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_unreachable_abort(struct unreachable_data *data)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_missing_return_abort(struct unreachable_data *data)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_vla_bound_not_positive_abort(struct vla_bound_data *data,
						 unsigned long bound __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_load_invalid_value_abort(struct invalid_value_data *data,
					     unsigned long val __unused)
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}

void __ubsan_handle_nonnull_arg_abort(struct nonnull_arg_data *data
#if __GCC_VERSION < 60000
				   , size_t arg_no __unused
#endif
				     )
{
	print_loc(__func__, &data->loc);
	plat_panic_handler();
}
