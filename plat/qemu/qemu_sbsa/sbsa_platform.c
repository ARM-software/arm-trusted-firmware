/*
 * Copyright (c) 2024-2025, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/fdt_wrappers.h>
#include <libfdt.h>

#include <sbsa_platform.h>

#include "qemu_private.h"

/* default platform version is 0.0 */
static int platform_version_major;
static int platform_version_minor;

static uint64_t gic_its_addr;
static struct qemu_platform_info dynamic_platform_info;

void sbsa_set_gic_bases(const uintptr_t gicd_base, const uintptr_t gicr_base);

/*
 * QEMU provides us with minimal information about hardware platform using
 * minimalistic DeviceTree. This is not a Linux DeviceTree. It is not even
 * a firmware DeviceTree.
 *
 * It is information passed from QEMU to describe the information a hardware
 * platform would have other mechanisms to discover at runtime, that are
 * affected by the QEMU command line.
 *
 * Ultimately this device tree will be replaced by IPC calls to an emulated SCP.
 * And when we do that, we won't then have to rewrite Normal world firmware to
 * cope.
 */

static void read_cpu_topology_from_dt(void *dtb)
{
	int node;

	/*
	 * QEMU gives us this DeviceTree node when we config:
	 * -smp 16,sockets=2,clusters=2,cores=2,threads=2
	 *
	 * topology {
	 *	threads = <0x02>;
	 *	cores = <0x02>;
	 *	clusters = <0x02>;
	 *	sockets = <0x02>;
	 * };
	 */

	node = fdt_path_offset(dtb, "/cpus/topology");
	if (node > 0) {
		dynamic_platform_info.cpu_topo.sockets =
			fdt_read_uint32_default(dtb, node, "sockets", 0);
		dynamic_platform_info.cpu_topo.clusters =
			fdt_read_uint32_default(dtb, node, "clusters", 0);
		dynamic_platform_info.cpu_topo.cores =
			fdt_read_uint32_default(dtb, node, "cores", 0);
		dynamic_platform_info.cpu_topo.threads =
			fdt_read_uint32_default(dtb, node, "threads", 0);
	}

	INFO("Cpu topology: sockets: %d, clusters: %d, cores: %d, threads: %d\n",
		dynamic_platform_info.cpu_topo.sockets,
		dynamic_platform_info.cpu_topo.clusters,
		dynamic_platform_info.cpu_topo.cores,
		dynamic_platform_info.cpu_topo.threads);
}

static void read_cpuinfo_from_dt(void *dtb)
{
	int node;
	int prev;
	int cpu = 0;
	uintptr_t mpidr;

	/*
	 * QEMU gives us this DeviceTree node:
	 * numa-node-id entries are only when NUMA config is used
	 *
	 *  cpus {
	 *	#size-cells = <0x00>;
	 *	#address-cells = <0x02>;
	 *
	 *	cpu@0 {
	 *		numa-node-id = <0x00>;
	 *		reg = <0x00 0x00>;
	 *	};
	 *
	 *	cpu@1 {
	 *		numa-node-id = <0x03>;
	 *		reg = <0x00 0x01>;
	 *	};
	 *  };
	 */
	node = fdt_path_offset(dtb, "/cpus");
	if (node < 0) {
		ERROR("No information about cpus in DeviceTree.\n");
		panic();
	}

	/*
	 * QEMU numbers cpus from 0 and there can be /cpus/cpu-map present so we
	 * cannot use fdt_first_subnode() here
	 */
	node = fdt_path_offset(dtb, "/cpus/cpu@0");

	while (node > 0) {
		if (fdt_getprop(dtb, node, "reg", NULL)) {
			fdt_get_reg_props_by_index(dtb, node, 0, &mpidr, NULL);
		} else {
			ERROR("Incomplete information for cpu %d in DeviceTree.\n", cpu);
			panic();
		}

		dynamic_platform_info.cpu[cpu].mpidr = mpidr;
		dynamic_platform_info.cpu[cpu].nodeid =
			fdt_read_uint32_default(dtb, node, "numa-node-id", 0);

		INFO("CPU %d: node-id: %d, mpidr: %ld\n", cpu,
				dynamic_platform_info.cpu[cpu].nodeid, mpidr);

		cpu++;

		prev = node;
		node = fdt_next_subnode(dtb, prev);
	}

	dynamic_platform_info.num_cpus = cpu;
	INFO("Found %d cpus\n", dynamic_platform_info.num_cpus);

	read_cpu_topology_from_dt(dtb);
}

static void read_meminfo_from_dt(void *dtb)
{
	const fdt32_t *prop;
	const char *type;
	int prev, node;
	int len;
	uint32_t memnode = 0;
	uint32_t higher_value, lower_value;
	uint64_t cur_base, cur_size;

	/*
	 * QEMU gives us this DeviceTree node:
	 *
	 *	memory@100c0000000 {
	 *		numa-node-id = <0x01>;
	 *		reg = <0x100 0xc0000000 0x00 0x40000000>;
	 *		device_type = "memory";
	 *	};
	 *
	 *	memory@10000000000 {
	 *		numa-node-id = <0x00>;
	 *		reg = <0x100 0x00 0x00 0xc0000000>;
	 *		device_type = "memory";
	 *	}
	 */

	for (prev = 0;; prev = node) {
		node = fdt_next_node(dtb, prev, NULL);
		if (node < 0) {
			break;
		}

		type = fdt_getprop(dtb, node, "device_type", &len);
		if (type && strncmp(type, "memory", len) == 0) {
			dynamic_platform_info.memory[memnode].nodeid =
				fdt_read_uint32_default(dtb, node, "numa-node-id", 0);

			/*
			 * Get the 'reg' property of this node and
			 * assume two 8 bytes for base and size.
			 */
			prop = fdt_getprop(dtb, node, "reg", &len);
			if (prop != 0 && len == (2 * sizeof(int64_t))) {
				higher_value = fdt32_to_cpu(*prop);
				lower_value = fdt32_to_cpu(*(prop + 1));
				cur_base = (uint64_t)(lower_value | ((uint64_t)higher_value) << 32);

				higher_value = fdt32_to_cpu(*(prop + 2));
				lower_value = fdt32_to_cpu(*(prop + 3));
				cur_size = (uint64_t)(lower_value | ((uint64_t)higher_value) << 32);

				dynamic_platform_info.memory[memnode].addr_base = cur_base;
				dynamic_platform_info.memory[memnode].addr_size = cur_size;

				INFO("RAM %d: node-id: %d, address: 0x%lx - 0x%lx\n",
					memnode,
					dynamic_platform_info.memory[memnode].nodeid,
					dynamic_platform_info.memory[memnode].addr_base,
					dynamic_platform_info.memory[memnode].addr_base +
					dynamic_platform_info.memory[memnode].addr_size - 1);
			}

			memnode++;
		}
	}

	dynamic_platform_info.num_memnodes = memnode;
}

static void read_platform_config_from_dt(void *dtb)
{
	int node;
	const fdt64_t *data;
	int err;
	uintptr_t gicd_base;
	uintptr_t gicr_base;

	/*
	 * QEMU gives us this DeviceTree node:
	 *
	 * intc {
	 *	 reg = < 0x00 0x40060000 0x00 0x10000
	 *		 0x00 0x40080000 0x00 0x4000000>;
	 *       its {
	 *               reg = <0x00 0x44081000 0x00 0x20000>;
	 *       };
	 * };
	 */
	node = fdt_path_offset(dtb, "/intc");
	if (node < 0) {
		return;
	}

	data = fdt_getprop(dtb, node, "reg", NULL);
	if (data == NULL) {
		return;
	}

	err = fdt_get_reg_props_by_index(dtb, node, 0, &gicd_base, NULL);
	if (err < 0) {
		ERROR("Failed to read GICD reg property of GIC node\n");
		return;
	}
	INFO("GICD base = 0x%lx\n", gicd_base);

	err = fdt_get_reg_props_by_index(dtb, node, 1, &gicr_base, NULL);
	if (err < 0) {
		ERROR("Failed to read GICR reg property of GIC node\n");
		return;
	}
	INFO("GICR base = 0x%lx\n", gicr_base);

	sbsa_set_gic_bases(gicd_base, gicr_base);

	node = fdt_path_offset(dtb, "/intc/its");
	if (node < 0) {
		return;
	}

	err = fdt_get_reg_props_by_index(dtb, node, 0, &gic_its_addr, NULL);
	if (err < 0) {
		ERROR("Failed to read GICI reg property of GIC node\n");
		return;
	}
	INFO("GICI base = 0x%lx\n", gic_its_addr);
}

static void read_platform_version(void *dtb)
{
	int node;

	node = fdt_path_offset(dtb, "/");
	if (node >= 0) {
		platform_version_major =
			fdt_read_uint32_default(dtb, node, "machine-version-major", 0);
		platform_version_minor =
			fdt_read_uint32_default(dtb, node, "machine-version-minor", 0);
	}
}

#if !ENABLE_RME
static int set_system_memory_base(void *dtb, uintptr_t new_base)
{
	(void)dtb;
	(void)new_base;

	return 0;
}
#else /* !ENABLE_RME */
static int set_system_memory_base(void *dtb, uintptr_t new_base)
{
	uint64_t cur_base, cur_size, new_size, delta;
	int len, prev, node, ret;
	const fdt32_t *prop;
	uint32_t node_id;
	const char *type;
	fdt64_t new[2];

	/*
	 * QEMU gives us this DeviceTree node:
	 *
	 *	memory@100c0000000 {
	 *		numa-node-id = <0x01>;
	 *		reg = <0x100 0xc0000000 0x00 0x40000000>;
	 *		device_type = "memory";
	 *	};
	 *
	 *	memory@10000000000 {
	 *		numa-node-id = <0x00>;
	 *		reg = <0x100 0x00 0x00 0xc0000000>;
	 *		device_type = "memory";
	 *	}
	 */

	for (prev = 0;; prev = node) {
		node = fdt_next_node(dtb, prev, NULL);
		if (node < 0) {
			return node;
		}

		type = fdt_getprop(dtb, node, "device_type", &len);
		if (type && strncmp(type, "memory", len) == 0) {

			/*
			 * We are looking for numa node 0, i.e the start of the
			 * system memory.  If a "numa-node-id" doesn't exists we
			 * take the first one.
			 */
			node_id = fdt_read_uint32_default(dtb, node,
							  "numa-node-id", 0);

			if (node_id == 0) {
				break;
			}
		}
	}

	/*
	 * Get the 'reg' property of this node and
	 * assume two 8 bytes for base and size.
	 */
	prop = fdt_getprop(dtb, node, "reg", &len);
	if (!prop || len < 0) {
		return len;
	}

	if (len != (2 * sizeof(uint64_t))) {
		return -FDT_ERR_BADVALUE;
	}

	ret = fdt_get_reg_props_by_index(dtb, node, 0, &cur_base, &cur_size);
	if (ret < 0)
		return ret;

	/*
	 * @cur_base is the base of the NS RAM given to us by QEMU, we can't
	 * go lower than that.
	 */
	if (new_base < cur_base) {
		return -FDT_ERR_BADVALUE;
	}

	if (new_base == cur_base) {
		return 0;
	}

	/*
	 * The new base is higher than the base set by QEMU, i.e we are moving
	 * the base memory up and shrinking the size.
	 */
	delta = (size_t)(new_base - cur_base);

	/*
	 * Make sure the new base is still within the base memory node, i.e
	 * the base memory node is big enough for the RMM.
	 */
	if (delta >= cur_size) {
		ERROR("Not enough space in base memory node for RMM\n");
		return -FDT_ERR_BADVALUE;
	}

	new_size = cur_size - delta;

	new[0] = cpu_to_fdt64(new_base);
	new[1] = cpu_to_fdt64(new_size);

	ret = fdt_setprop(dtb, node, "reg", new, len);
	if (ret < 0) {
		return ret;
	}

	return fdt_pack(dtb);
}
#endif /* !ENABLE_RME */

void sbsa_platform_init(void)
{
	/* Read DeviceTree data before MMU is enabled */

	void *dtb = plat_qemu_dt_runtime_address();
	int err;

	err = fdt_open_into(dtb, dtb, PLAT_QEMU_DT_MAX_SIZE);
	if (err < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", dtb, err);
		return;
	}

	err = fdt_check_header(dtb);
	if (err < 0) {
		ERROR("Invalid DTB file passed\n");
		return;
	}

	read_platform_version(dtb);
	INFO("Platform version: %d.%d\n", platform_version_major, platform_version_minor);

	if (set_system_memory_base(dtb, NS_DRAM0_BASE)) {
		ERROR("Failed to set system memory in Device Tree\n");
		return;
	}

	read_platform_config_from_dt(dtb);
	read_cpuinfo_from_dt(dtb);
	read_meminfo_from_dt(dtb);
}

int sbsa_platform_version_major(void)
{
	return platform_version_major;
}

int sbsa_platform_version_minor(void)
{
	return platform_version_minor;
}

uint32_t sbsa_platform_num_cpus(void)
{
	return dynamic_platform_info.num_cpus;
}

uint32_t sbsa_platform_num_memnodes(void)
{
	return dynamic_platform_info.num_memnodes;
}

uint64_t sbsa_platform_gic_its_addr(void)
{
	return gic_its_addr;
}

struct platform_cpu_data sbsa_platform_cpu_node(uint64_t index)
{
	return dynamic_platform_info.cpu[index];
}

struct platform_memory_data sbsa_platform_memory_node(uint64_t index)
{
	return dynamic_platform_info.memory[index];
}

struct platform_cpu_topology sbsa_platform_cpu_topology(void)
{
	return dynamic_platform_info.cpu_topo;
}
