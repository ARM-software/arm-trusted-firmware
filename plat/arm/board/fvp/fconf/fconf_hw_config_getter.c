/*
 * Copyright (c) 2020-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <fconf_hw_config_getter.h>
#include <libfdt.h>
#include <plat/common/platform.h>

struct hw_topology_t soc_topology;
struct uart_serial_config_t uart_serial_config;
struct cpu_timer_t cpu_timer;
struct dram_layout_t dram_layout;
struct pci_props_t pci_props;

/*
 * Each NS DRAM bank entry is 'reg' node property which is
 * a sequence of (address, length) pairs of 32-bit values.
 */
#define DRAM_ENTRY_SIZE		(4UL * sizeof(uint32_t))

CASSERT(ARM_DRAM_NUM_BANKS == 2UL, ARM_DRAM_NUM_BANKS_mismatch);
CASSERT(ARM_PCI_NUM_REGIONS == 2UL, ARM_PCI_NUM_REGIONS_mismatch);

#define ILLEGAL_ADDR	ULL(~0)


int fconf_populate_topology(uintptr_t config)
{
	int err, node, cluster_node, core_node, thread_node;
	uint32_t cluster_count = 0, max_cpu_per_cluster = 0, total_cpu_count = 0;
	uint32_t max_pwr_lvl = 0;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Find the offset of the node containing "arm,psci-1.0" compatible property */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,psci-1.0");
	if (node < 0) {
		/* Fall back to 0.2 */
		node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,psci-0.2");
		if (node < 0) {
			ERROR("FCONF: Unable to locate node with arm,psci compatible property\n");
			return node;
		}
	}

	err = fdt_read_uint32(hw_config_dtb, node, "max-pwr-lvl", &max_pwr_lvl);
	if (err < 0) {
		/*
		 * Some legacy FVP dts may not have this property. Assign the default
		 * value.
		 */
		WARN("FCONF: Could not locate max-pwr-lvl property\n");
		max_pwr_lvl = 2;
	}

	assert(max_pwr_lvl <= MPIDR_AFFLVL2);

	/* Find the offset of the "cpus" node */
	node = fdt_path_offset(hw_config_dtb, "/cpus");
	if (node < 0) {
		ERROR("FCONF: Node '%s' not found in hardware configuration dtb\n", "cpus");
		return node;
	}

	/* A typical cpu-map node in a device tree is shown here for reference
	cpu-map {
		cluster0 {
			core0 {
				cpu = <&CPU0>;
			};
			core1 {
				cpu = <&CPU1>;
			};
		};

		cluster1 {
			core0 {
				cpu = <&CPU2>;
			};
			core1 {
				cpu = <&CPU3>;
			};
		};
	};
	*/

	/* Locate the cpu-map child node */
	node = fdt_subnode_offset(hw_config_dtb, node, "cpu-map");
	if (node < 0) {
		ERROR("FCONF: Node '%s' not found in hardware configuration dtb\n", "cpu-map");
		return node;
	}

	uint32_t cpus_per_cluster[PLAT_ARM_CLUSTER_COUNT] = {0};

	/* Iterate through cluster nodes */
	fdt_for_each_subnode(cluster_node, hw_config_dtb, node) {
		assert(cluster_count < PLAT_ARM_CLUSTER_COUNT);

		/* Iterate through core nodes */
		fdt_for_each_subnode(core_node, hw_config_dtb, cluster_node) {
			/* core nodes may have child nodes i.e., "thread" nodes */
			if (fdt_first_subnode(hw_config_dtb, core_node) < 0) {
				cpus_per_cluster[cluster_count]++;
			} else {
				/* Multi-threaded CPU description is found in dtb */
				fdt_for_each_subnode(thread_node, hw_config_dtb, core_node) {
					cpus_per_cluster[cluster_count]++;
				}

				/* Since in some dtbs, core nodes may not have thread node,
				 * no need to error if even one child node is not found.
				 */
			}
		}

		/* Ensure every cluster node has at least 1 child node */
		if (cpus_per_cluster[cluster_count] < 1U) {
			ERROR("FCONF: Unable to locate the core node in cluster %d\n", cluster_count);
			return -1;
		}

		VERBOSE("CLUSTER ID: %d cpu-count: %d\n", cluster_count,
					cpus_per_cluster[cluster_count]);

		/* Find the maximum number of cpus in any cluster */
		max_cpu_per_cluster = MAX(max_cpu_per_cluster, cpus_per_cluster[cluster_count]);
		total_cpu_count += cpus_per_cluster[cluster_count];
		cluster_count++;
	}


	/* At least one cluster node is expected in hardware configuration dtb */
	if (cluster_count < 1U) {
		ERROR("FCONF: Unable to locate the cluster node in cpu-map node\n");
		return -1;
	}

	soc_topology.plat_max_pwr_level = max_pwr_lvl;
	soc_topology.plat_cluster_count = cluster_count;
	soc_topology.cluster_cpu_count = max_cpu_per_cluster;
	soc_topology.plat_cpu_count = total_cpu_count;

	return 0;
}

int fconf_populate_uart_config(uintptr_t config)
{
	int uart_node, node, err;
	uintptr_t addr;
	const char *path;
	uint32_t phandle;
	uint64_t translated_addr;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/*
	 * uart child node is indirectly referenced through its path which is
	 * specified in the `serial1` property of the "aliases" node.
	 * Note that TF-A boot console is mapped to serial0 while runtime
	 * console is mapped to serial1.
	 */

	path = fdt_get_alias(hw_config_dtb, "serial1");
	if (path == NULL) {
		ERROR("FCONF: Could not read serial1 property in aliases node\n");
		return -1;
	}

	/* Find the offset of the uart serial node */
	uart_node = fdt_path_offset(hw_config_dtb, path);
	if (uart_node < 0) {
		ERROR("FCONF: Failed to locate uart serial node using its path\n");
		return -1;
	}

	/* uart serial node has its offset and size of address in reg property */
	err = fdt_get_reg_props_by_index(hw_config_dtb, uart_node, 0, &addr,
						NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property of '%s' node\n",
			"uart serial");
		return err;
	}
	VERBOSE("FCONF: UART node address: %lx\n", addr);

	/*
	 * Perform address translation of local device address to CPU address
	 * domain.
	 */
	translated_addr = fdtw_translate_address(hw_config_dtb,
						 uart_node, (uint64_t)addr);
	if (translated_addr == ILLEGAL_ADDR) {
		ERROR("FCONF: failed to translate UART node base address");
		return -1;
	}

	uart_serial_config.uart_base = translated_addr;

	VERBOSE("FCONF: UART serial device base address: %" PRIx64 "\n",
		uart_serial_config.uart_base);

	/*
	 * The phandle of the DT node which captures the clock info of uart
	 * serial node is specified in the "clocks" property.
	 */
	err = fdt_read_uint32(hw_config_dtb, uart_node, "clocks", &phandle);
	if (err < 0) {
		ERROR("FCONF: Could not read clocks property in uart serial node\n");
		return err;
	}

	node = fdt_node_offset_by_phandle(hw_config_dtb, phandle);
	if (node < 0) {
		ERROR("FCONF: Failed to locate clk node using its path\n");
		return node;
	}

	/*
	 * Retrieve clock frequency. We assume clock provider generates a fixed
	 * clock.
	 */
	err = fdt_read_uint32(hw_config_dtb, node, "clock-frequency",
				&uart_serial_config.uart_clk);
	if (err < 0) {
		ERROR("FCONF: Could not read clock-frequency property in clk node\n");
		return err;
	}

	VERBOSE("FCONF: UART serial device clk frequency: %x\n",
		uart_serial_config.uart_clk);

	return 0;
}

int fconf_populate_cpu_timer(uintptr_t config)
{
	int err, node;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Find the node offset point to "arm,armv8-timer" compatible property,
	 * a per-core architected timer attached to a GIC to deliver its per-processor
	 * interrupts via PPIs */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,armv8-timer");
	if (node < 0) {
		ERROR("FCONF: Unrecognized hardware configuration dtb (%d)\n", node);
		return node;
	}

	/* Locate the cell holding the clock-frequency, an optional field */
	err = fdt_read_uint32(hw_config_dtb, node, "clock-frequency", &cpu_timer.clock_freq);
	if (err < 0) {
		WARN("FCONF failed to read clock-frequency property\n");
	}

	return 0;
}

int fconf_populate_dram_layout(uintptr_t config)
{
	int node, len;
	const uint32_t *reg;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Find 'memory' node */
	node = fdt_node_offset_by_prop_value(hw_config_dtb, -1, "device_type",
					     "memory", sizeof("memory"));
	if (node < 0) {
		WARN("FCONF: Unable to locate 'memory' node\n");
		return node;
	}

	reg = fdt_getprop(hw_config_dtb, node, "reg", &len);
	if (reg == NULL) {
		ERROR("FCONF failed to read 'reg' property\n");
		return len;
	}

	switch (len) {
	case DRAM_ENTRY_SIZE:
		/* 1 DRAM bank */
		dram_layout.num_banks = 1UL;
		break;
	case 2UL * DRAM_ENTRY_SIZE:
		/* 2 DRAM banks */
		dram_layout.num_banks = 2UL;
		break;
	default:
		ERROR("FCONF: Invalid 'memory' node\n");
		return -FDT_ERR_BADLAYOUT;
	}

	for (unsigned long i = 0UL; i < dram_layout.num_banks; i++) {
		int err = fdt_get_reg_props_by_index(
				hw_config_dtb, node, (int)i,
				(uintptr_t *)&dram_layout.dram_bank[i].base,
				(size_t *)&dram_layout.dram_bank[i].size);
		if (err < 0) {
			ERROR("FCONF: Failed to read 'reg' property #%lu of 'memory' node\n", i);
			return err;
		}
	}

	return 0;
}

/*
 * Each PCIe memory region entry is 'ranges' node property which is
 * an arbitrary number of (child-bus-address, parent-bus-address, length)
 * triplets. E.g. with
 * #address-cells = <3>
 * #size-cells = <2>
 * parent's #address-cells = <2>
 * each entry occupies 7 32-bit words.
 */
int fconf_populate_pci_props(uintptr_t config)
{
	int node, parent, len, err;
	int parent_ac, ac, sc, entry_len;
	const uint32_t *reg, *ranges;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Find 'pci' node */
	node = fdt_node_offset_by_prop_value(hw_config_dtb, -1, "device_type",
					     "pci", sizeof("pci"));
	if (node < 0) {
		WARN("FCONF: Unable to locate 'pci' node\n");
		pci_props.ecam_base = 0UL;
		pci_props.size = 0UL;
		pci_props.num_ncoh_regions = 0UL;
		/* Don't return error code if 'pci' node not found */
		return 0;
	}

	reg = fdt_getprop(hw_config_dtb, node, "reg", &len);
	if (reg == NULL) {
		ERROR("FCONF failed to read 'reg' property\n");
		return len;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0,
					 (uintptr_t *)&pci_props.ecam_base,
					 (size_t *)&pci_props.size);
	if (err < 0) {
		ERROR("FCONF: Failed to read 'reg' property of 'pci' node\n");
		return err;
	}

	parent = fdt_parent_offset(hw_config_dtb, node);
	if (parent < 0) {
		return -FDT_ERR_BADOFFSET;
	}

	parent_ac = fdt_address_cells(hw_config_dtb, parent);
	ac = fdt_address_cells(hw_config_dtb, node);
	sc = fdt_size_cells(hw_config_dtb, node);

	entry_len = parent_ac + ac + sc;

	ranges = fdt_getprop(hw_config_dtb, node, "ranges", &len);
	if (ranges == NULL) {
		ERROR("FCONF failed to read 'ranges' property\n");
		return len;
	}

	/* 'ranges' length in 32-bit words */
	len /= sizeof(uint32_t);
	if ((len % entry_len) != 0) {
		return -FDT_ERR_BADVALUE;
	}

	pci_props.num_ncoh_regions = (uint64_t)(len / entry_len);

	if (pci_props.num_ncoh_regions > ARM_PCI_NUM_REGIONS) {
		WARN("FCONF: 'ranges' reports more memory regions than supported\n");
		pci_props.num_ncoh_regions = ARM_PCI_NUM_REGIONS;
	}

	for (unsigned int i = 0U; i < (unsigned int)pci_props.num_ncoh_regions; i++) {
		unsigned int cell = i * entry_len + ac;

		/* Read CPU address (parent-bus-address) space */
		pci_props.ncoh_regions[i].base =
			fdt_read_prop_cells(&ranges[cell], ac);

		/* Read CPU address size */
		pci_props.ncoh_regions[i].size =
			fdt_read_prop_cells(&ranges[cell + parent_ac], sc);
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, topology, fconf_populate_topology);
FCONF_REGISTER_POPULATOR(HW_CONFIG, uart_config, fconf_populate_uart_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, cpu_timer, fconf_populate_cpu_timer);
FCONF_REGISTER_POPULATOR(HW_CONFIG, dram_layout, fconf_populate_dram_layout);
FCONF_REGISTER_POPULATOR(HW_CONFIG, pci_props, fconf_populate_pci_props);
