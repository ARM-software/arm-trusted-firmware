/*
 * Copyright (c) 2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/fdt_wrappers.h>
#include <common/runtime_svc.h>
#include <libfdt.h>
#include <smccc_helpers.h>

/* default platform version is 0.0 */
static int platform_version_major;
static int platform_version_minor;

#define SMC_FASTCALL       0x80000000
#define SMC64_FUNCTION     (SMC_FASTCALL   | 0x40000000)
#define SIP_FUNCTION       (SMC64_FUNCTION | 0x02000000)
#define SIP_FUNCTION_ID(n) (SIP_FUNCTION   | (n))

/*
 * We do not use SMCCC_ARCH_SOC_ID here because qemu_sbsa is virtual platform
 * which uses SoC present in QEMU. And they can change on their own while we
 * need version of whole 'virtual hardware platform'.
 */
#define SIP_SVC_VERSION  SIP_FUNCTION_ID(1)
#define SIP_SVC_GET_GIC  SIP_FUNCTION_ID(100)
#define SIP_SVC_GET_GIC_ITS SIP_FUNCTION_ID(101)
#define SIP_SVC_GET_CPU_COUNT SIP_FUNCTION_ID(200)
#define SIP_SVC_GET_CPU_NODE SIP_FUNCTION_ID(201)
#define SIP_SVC_GET_CPU_TOPOLOGY SIP_FUNCTION_ID(202)
#define SIP_SVC_GET_MEMORY_NODE_COUNT SIP_FUNCTION_ID(300)
#define SIP_SVC_GET_MEMORY_NODE SIP_FUNCTION_ID(301)

static uint64_t gic_its_addr;

typedef struct {
	uint32_t nodeid;
	uint32_t mpidr;
} cpu_data;

typedef struct{
	uint32_t nodeid;
	uint64_t addr_base;
	uint64_t addr_size;
} memory_data;

/*
 * sockets: the number of sockets on sbsa-ref platform.
 * clusters: the number of clusters in one socket.
 * cores: the number of cores in one cluster.
 * threads: the number of threads in one core.
 */
typedef struct {
	uint32_t sockets;
	uint32_t clusters;
	uint32_t cores;
	uint32_t threads;
} cpu_topology;

static struct {
	uint32_t num_cpus;
	uint32_t num_memnodes;
	cpu_data cpu[PLATFORM_CORE_COUNT];
	cpu_topology cpu_topo;
	memory_data memory[PLAT_MAX_MEM_NODES];
} dynamic_platform_info;

void sbsa_set_gic_bases(const uintptr_t gicd_base, const uintptr_t gicr_base);
uintptr_t sbsa_get_gicd(void);
uintptr_t sbsa_get_gicr(void);

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

void read_cpuinfo_from_dt(void *dtb)
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
	 *  	#size-cells = <0x00>;
	 *  	#address-cells = <0x02>;
	 *
	 *  	cpu@0 {
	 *  	        numa-node-id = <0x00>;
	 *  		reg = <0x00 0x00>;
	 *  	};
	 *
	 *  	cpu@1 {
	 *  	        numa-node-id = <0x03>;
	 *  		reg = <0x00 0x01>;
	 *  	};
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

void read_meminfo_from_dt(void *dtb)
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

void read_platform_config_from_dt(void *dtb)
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

void read_platform_version(void *dtb)
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

void sip_svc_init(void)
{
	/* Read DeviceTree data before MMU is enabled */

	void *dtb = (void *)(uintptr_t)ARM_PRELOADED_DTB_BASE;
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

	read_platform_config_from_dt(dtb);
	read_cpuinfo_from_dt(dtb);
	read_meminfo_from_dt(dtb);
}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uintptr_t sbsa_sip_smc_handler(uint32_t smc_fid,
			       u_register_t x1,
			       u_register_t x2,
			       u_register_t x3,
			       u_register_t x4,
			       void *cookie,
			       void *handle,
			       u_register_t flags)
{
	uint32_t ns;
	uint64_t index;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		ERROR("%s: wrong world SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	switch (smc_fid) {
	case SIP_SVC_VERSION:
		INFO("Platform version requested\n");
		SMC_RET3(handle, NULL, platform_version_major, platform_version_minor);

	case SIP_SVC_GET_GIC:
		SMC_RET3(handle, NULL, sbsa_get_gicd(), sbsa_get_gicr());

	case SIP_SVC_GET_GIC_ITS:
		SMC_RET2(handle, NULL, gic_its_addr);

	case SIP_SVC_GET_CPU_COUNT:
		SMC_RET2(handle, NULL, dynamic_platform_info.num_cpus);

	case SIP_SVC_GET_CPU_NODE:
		index = x1;
		if (index < PLATFORM_CORE_COUNT) {
			SMC_RET3(handle, NULL,
				dynamic_platform_info.cpu[index].nodeid,
				dynamic_platform_info.cpu[index].mpidr);
		} else {
			SMC_RET1(handle, SMC_ARCH_CALL_INVAL_PARAM);
		}

	case SIP_SVC_GET_CPU_TOPOLOGY:
		if (dynamic_platform_info.cpu_topo.cores > 0) {
			SMC_RET5(handle, NULL,
			dynamic_platform_info.cpu_topo.sockets,
			dynamic_platform_info.cpu_topo.clusters,
			dynamic_platform_info.cpu_topo.cores,
			dynamic_platform_info.cpu_topo.threads);
		} else {
			/* we do not know topology so we report SMC as unknown */
			SMC_RET1(handle, SMC_UNK);
		}

	case SIP_SVC_GET_MEMORY_NODE_COUNT:
		SMC_RET2(handle, NULL, dynamic_platform_info.num_memnodes);

	case SIP_SVC_GET_MEMORY_NODE:
		index = x1;
		if (index < PLAT_MAX_MEM_NODES) {
			SMC_RET4(handle, NULL,
				dynamic_platform_info.memory[index].nodeid,
				dynamic_platform_info.memory[index].addr_base,
				dynamic_platform_info.memory[index].addr_size);
		} else {
			SMC_RET1(handle, SMC_ARCH_CALL_INVAL_PARAM);
		}

	default:
		ERROR("%s: unhandled SMC (0x%x) (function id: %d)\n", __func__, smc_fid,
		      smc_fid - SIP_FUNCTION);
		SMC_RET1(handle, SMC_UNK);
	}
}

int sbsa_sip_smc_setup(void)
{
	return 0;
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	sbsa_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	sbsa_sip_smc_setup,
	sbsa_sip_smc_handler
);
