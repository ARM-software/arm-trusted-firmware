/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <setjmp.h>

typedef jmp_buf panic_environment_t;

#define SETUP_PANIC_ENVIRONMENT(env) (expect_panic(&env) && (setjmp(env) == 0))

int expect_panic(panic_environment_t *env);
void expect_tf_log(const char *str);
