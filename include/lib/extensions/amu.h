/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_H
#define AMU_H

#include <stdbool.h>
#include <stdint.h>

#include <context.h>

#include <platform_def.h>

#if ENABLE_FEAT_AMU
#if __aarch64__
void amu_enable(cpu_context_t *ctx);
void amu_init_el3(unsigned int core_pos);
void amu_init_el2_unused(void);
void amu_enable_per_world(per_world_context_t *per_world_ctx);
#else
void amu_enable(bool el2_unused);
#endif /* __aarch64__ */

#else
#if __aarch64__
void amu_enable(cpu_context_t *ctx)
{
}
void amu_init_el3(unsigned int core_pos)
{
}
void amu_init_el2_unused(void)
{
}
void amu_enable_per_world(per_world_context_t *per_world_ctx)
{
}
#else
static inline void amu_enable(bool el2_unused)
{
}
#endif /*__aarch64__ */
#endif /* ENABLE_FEAT_AMU */

/*
 * Per-core list of the counters to be enabled. Value will be written into
 * AMCNTENSET1_EL0 verbatim.
 */
#if ENABLE_AMU_AUXILIARY_COUNTERS
extern uint16_t plat_amu_aux_enables[PLATFORM_CORE_COUNT];
#endif

#if __aarch64__
/* lower 2 handled by context management */
#define CTX_AMU_GRP0_ALL		U(2)
#else
#define CTX_AMU_GRP0_ALL		U(4)
#endif
#define CTX_AMU_GRP1_ALL		U(16)

typedef struct amu_regs {
	u_register_t grp0[CTX_AMU_GRP0_ALL];
#if ENABLE_AMU_AUXILIARY_COUNTERS
	u_register_t grp1[CTX_AMU_GRP1_ALL];
#endif
} amu_regs_t;

static inline u_register_t read_amu_grp0_ctx_reg(amu_regs_t *ctx, size_t index)
{
	return ctx->grp0[index];
}

static inline void write_amu_grp0_ctx_reg(amu_regs_t *ctx, size_t index, u_register_t val)
{
	ctx->grp0[index] = val;
}

static inline uint16_t get_amu_aux_enables(size_t index)
{
#if ENABLE_AMU_AUXILIARY_COUNTERS
	return plat_amu_aux_enables[index];
#else
	return 0;
#endif
}

static inline u_register_t read_amu_grp1_ctx_reg(amu_regs_t *ctx, size_t index)
{
#if ENABLE_AMU_AUXILIARY_COUNTERS
	return ctx->grp1[index];
#else
	return 0;
#endif
}

static inline void write_amu_grp1_ctx_reg(amu_regs_t *ctx, size_t index, u_register_t val)
{
#if ENABLE_AMU_AUXILIARY_COUNTERS
	ctx->grp1[index] = val;
#endif
}

#endif /* AMU_H */
