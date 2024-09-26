/*
 * Copyright (c) 2024-2025, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBSA_PLATFORM_H
#define SBSA_PLATFORM_H

#include <stdint.h>

#include <platform_def.h>

struct platform_cpu_data {
	uint32_t nodeid;
	uint32_t mpidr;
};

struct platform_memory_data {
	uint32_t nodeid;
	uint64_t addr_base;
	uint64_t addr_size;
};

/*
 * sockets: the number of sockets on sbsa-ref platform.
 * clusters: the number of clusters in one socket.
 * cores: the number of cores in one cluster.
 * threads: the number of threads in one core.
 */
struct platform_cpu_topology {
	uint32_t sockets;
	uint32_t clusters;
	uint32_t cores;
	uint32_t threads;
};

struct qemu_platform_info {
	uint32_t num_cpus;
	uint32_t num_memnodes;
	struct platform_cpu_data cpu[PLATFORM_CORE_COUNT];
	struct platform_cpu_topology cpu_topo;
	struct platform_memory_data memory[PLAT_MAX_MEM_NODES];
};

#endif /* SBSA_PLATFORM_H */
