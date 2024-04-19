/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPINLOCK_H
#define SPINLOCK_H

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct spinlock {
	volatile uint32_t lock;
} spinlock_t;

typedef struct bitlock {
	volatile uint8_t lock;
} bitlock_t;

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

void bit_lock(bitlock_t *lock, uint8_t mask);
void bit_unlock(bitlock_t *lock, uint8_t mask);

#else

/* Spin lock definitions for use in assembly */
#define SPINLOCK_ASM_ALIGN	2
#define SPINLOCK_ASM_SIZE	4

#endif /* __ASSEMBLER__ */
#endif /* SPINLOCK_H */
