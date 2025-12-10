/*
 * Copyright (c) 2023-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_features.h>
#include <common/debug.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>

/********************************************************************************
 * Function that returns the corresponding string constant for a security state
 * index.
 *******************************************************************************/
static const char *get_context_name_by_idx(unsigned int security_state_idx)
{
	assert(security_state_idx < CPU_CONTEXT_NUM);
	static const char * const state_names[] = {
		"Secure",
		"Non Secure"
#if ENABLE_RME
		, "Realm"
#endif /* ENABLE_RME */
	};
	return state_names[security_state_idx];
}

#define PRINT_MEM_USAGE_SEPARATOR()					\
	do {								\
		printf("+-----------+-----------+-----------"		\
		"+-----------+-----------+-----------+\n");		\
	} while (false)

#define NAME_PLACEHOLDER_LEN 14

#define PRINT_DASH(n)							\
	for (; n > 0; n--) {						\
		putchar('-');						\
	}

#define PRINT_SINGLE_MEM_USAGE_SEP_BLOCK()				\
	do {								\
		printf("+-----------");					\
	} while (false)

/********************************************************************************
 * This function prints the allocated memory for a specific security state.
 * Values are grouped by exception level and core. The memory usage for the
 * global context and the total memory for the security state are also computed.
 *******************************************************************************/
static size_t report_allocated_memory(unsigned int security_state_idx)
{
	size_t core_total = 0U;
	size_t gp_total = 0U;
	size_t el3_total = 0U;
	size_t other_total = 0U;
	size_t total = 0U;
	size_t per_world_ctx_size = 0U;

	size_t elx_total = 0U;
	size_t pauth_total = 0U;

	if (is_ctx_pauth_supported()) {
		PRINT_SINGLE_MEM_USAGE_SEP_BLOCK();
	}

	PRINT_MEM_USAGE_SEPARATOR();

	printf("|    Core   |     GP    |    EL3    ");
#if CTX_INCLUDE_EL2_REGS
	printf("|    EL2    ");
#else
	printf("|    EL1    ");
#endif /* CTX_INCLUDE_EL2_REGS */

	if (is_ctx_pauth_supported()) {
		printf("|   PAUTH   ");
	}

	printf("|   Other   |   Total   |\n");

	/* Compute memory usage for each core's context */
	for (unsigned int i = 0U; i < PLATFORM_CORE_COUNT; i++) {
		size_t size_other = 0U;
		size_t el3_size = 0U;
		size_t gp_size = 0U;
		size_t elx_size = 0U;

		if (is_ctx_pauth_supported()) {
			PRINT_SINGLE_MEM_USAGE_SEP_BLOCK();
		}

		PRINT_MEM_USAGE_SEPARATOR();

		cpu_context_t *ctx = (cpu_context_t *)cm_get_context_by_index(i,
			security_state_idx);
		core_total = sizeof(*ctx);
		el3_size = sizeof(ctx->el3state_ctx);
		gp_size = sizeof(ctx->gpregs_ctx);
		size_other = core_total - (el3_size + gp_size);
		printf("| %9u | %8luB | %8luB ", i, gp_size, el3_size);

#if CTX_INCLUDE_EL2_REGS
		elx_size = sizeof(ctx->el2_sysregs_ctx);
#else
		elx_size = sizeof(ctx->el1_sysregs_ctx);
#endif /* CTX_INCLUDE_EL2_REGS */
		size_other -= elx_size;
		elx_total += elx_size;
		printf("| %8luB ", elx_size);

		if (is_ctx_pauth_supported()) {
			size_t pauth_size = sizeof(get_pauth_ctx(ctx));
			size_other -= pauth_size;
			pauth_total += pauth_size;
			printf("| %8luB ", pauth_size);
		}
		printf("| %8luB | %8luB |\n", size_other, core_total);

		gp_total += gp_size;
		el3_total += el3_size;
		other_total += size_other;
		total += core_total;
	}

	if (is_ctx_pauth_supported()) {
		PRINT_SINGLE_MEM_USAGE_SEP_BLOCK();
	}

	PRINT_MEM_USAGE_SEPARATOR();

	if (is_ctx_pauth_supported()) {
		PRINT_SINGLE_MEM_USAGE_SEP_BLOCK();
	}

	PRINT_MEM_USAGE_SEPARATOR();

	printf("|    All    | %8luB | %8luB ", gp_total, el3_total);

	printf("| %8luB ", elx_total);

	if (is_ctx_pauth_supported()) {
		printf("| %8luB ", pauth_total);
	}

	printf("| %8luB | %8luB |\n", other_total, total);

	if (is_ctx_pauth_supported()) {
		PRINT_SINGLE_MEM_USAGE_SEP_BLOCK();
	}
	PRINT_MEM_USAGE_SEPARATOR();
	printf("\n");

	/* Compute memory usage for the global context */
	per_world_ctx_size = sizeof(per_world_context[security_state_idx]);

	total += per_world_ctx_size;

	printf("Per-world context: %luB\n\n", per_world_ctx_size);

	printf("TOTAL: %luB\n", total);

	return total;
}

/********************************************************************************
 * Reports the allocated memory for every security state and then reports the
 * total system-wide allocated memory.
 *******************************************************************************/
void report_ctx_memory_usage(void)
{
	INFO("Context memory allocation:\n");

	size_t total = 0U;

	for (unsigned int i = 0U; i < CPU_CONTEXT_NUM; i++) {
		const char *context_name = get_context_name_by_idx(i);
		size_t len = 0U;

		printf("Memory usage for %s:\n", context_name);
		total += report_allocated_memory(i);
			printf("------------------------");
			len = NAME_PLACEHOLDER_LEN - printf("End %s", context_name);
			PRINT_DASH(len);
			printf("-----------------------\n\n");
	}

	printf("Total context memory allocated: %luB\n\n", total);
}
