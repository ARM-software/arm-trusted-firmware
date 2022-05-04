/*
 * Copyright (c) 2016-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Contains generic routines to fix up the device tree blob passed on to
 * payloads like BL32 and BL33 (and further down the boot chain).
 * This allows to easily add PSCI nodes, when the original DT does not have
 * it or advertises another method.
 * Also it supports to add reserved memory nodes to describe memory that
 * is used by the secure world, so that non-secure software avoids using
 * that.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <libfdt.h>

#include <arch.h>
#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <drivers/console.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>


static int append_psci_compatible(void *fdt, int offs, const char *str)
{
	return fdt_appendprop(fdt, offs, "compatible", str, strlen(str) + 1);
}

/*
 * Those defines are for PSCI v0.1 legacy clients, which we expect to use
 * the same execution state (AArch32/AArch64) as TF-A.
 * Kernels running in AArch32 on an AArch64 TF-A should use PSCI v0.2.
 */
#ifdef __aarch64__
#define PSCI_CPU_SUSPEND_FNID	PSCI_CPU_SUSPEND_AARCH64
#define PSCI_CPU_ON_FNID	PSCI_CPU_ON_AARCH64
#else
#define PSCI_CPU_SUSPEND_FNID	PSCI_CPU_SUSPEND_AARCH32
#define PSCI_CPU_ON_FNID	PSCI_CPU_ON_AARCH32
#endif

/*******************************************************************************
 * dt_add_psci_node() - Add a PSCI node into an existing device tree
 * @fdt:	pointer to the device tree blob in memory
 *
 * Add a device tree node describing PSCI into the root level of an existing
 * device tree blob in memory.
 * This will add v0.1, v0.2 and v1.0 compatible strings and the standard
 * function IDs for v0.1 compatibility.
 * An existing PSCI node will not be touched, the function will return success
 * in this case. This function will not touch the /cpus enable methods, use
 * dt_add_psci_cpu_enable_methods() for that.
 *
 * Return: 0 on success, -1 otherwise.
 ******************************************************************************/
int dt_add_psci_node(void *fdt)
{
	int offs;

	if (fdt_path_offset(fdt, "/psci") >= 0) {
		WARN("PSCI Device Tree node already exists!\n");
		return 0;
	}

	offs = fdt_path_offset(fdt, "/");
	if (offs < 0)
		return -1;
	offs = fdt_add_subnode(fdt, offs, "psci");
	if (offs < 0)
		return -1;
	if (append_psci_compatible(fdt, offs, "arm,psci-1.0"))
		return -1;
	if (append_psci_compatible(fdt, offs, "arm,psci-0.2"))
		return -1;
	if (append_psci_compatible(fdt, offs, "arm,psci"))
		return -1;
	if (fdt_setprop_string(fdt, offs, "method", "smc"))
		return -1;
	if (fdt_setprop_u32(fdt, offs, "cpu_suspend", PSCI_CPU_SUSPEND_FNID))
		return -1;
	if (fdt_setprop_u32(fdt, offs, "cpu_off", PSCI_CPU_OFF))
		return -1;
	if (fdt_setprop_u32(fdt, offs, "cpu_on", PSCI_CPU_ON_FNID))
		return -1;
	return 0;
}

/*
 * Find the first subnode that has a "device_type" property with the value
 * "cpu" and which's enable-method is not "psci" (yet).
 * Returns 0 if no such subnode is found, so all have already been patched
 * or none have to be patched in the first place.
 * Returns 1 if *one* such subnode has been found and successfully changed
 * to "psci".
 * Returns negative values on error.
 *
 * Call in a loop until it returns 0. Recalculate the node offset after
 * it has returned 1.
 */
static int dt_update_one_cpu_node(void *fdt, int offset)
{
	int offs;

	/* Iterate over all subnodes to find those with device_type = "cpu". */
	for (offs = fdt_first_subnode(fdt, offset); offs >= 0;
	     offs = fdt_next_subnode(fdt, offs)) {
		const char *prop;
		int len;
		int ret;

		prop = fdt_getprop(fdt, offs, "device_type", &len);
		if (prop == NULL)
			continue;
		if ((strcmp(prop, "cpu") != 0) || (len != 4))
			continue;

		/* Ignore any nodes which already use "psci". */
		prop = fdt_getprop(fdt, offs, "enable-method", &len);
		if ((prop != NULL) &&
		    (strcmp(prop, "psci") == 0) && (len == 5))
			continue;

		ret = fdt_setprop_string(fdt, offs, "enable-method", "psci");
		if (ret < 0)
			return ret;
		/*
		 * Subnode found and patched.
		 * Restart to accommodate potentially changed offsets.
		 */
		return 1;
	}

	if (offs == -FDT_ERR_NOTFOUND)
		return 0;

	return offs;
}

/*******************************************************************************
 * dt_add_psci_cpu_enable_methods() - switch CPU nodes in DT to use PSCI
 * @fdt:	pointer to the device tree blob in memory
 *
 * Iterate over all CPU device tree nodes (/cpus/cpu@x) in memory to change
 * the enable-method to PSCI. This will add the enable-method properties, if
 * required, or will change existing properties to read "psci".
 *
 * Return: 0 on success, or a negative error value otherwise.
 ******************************************************************************/

int dt_add_psci_cpu_enable_methods(void *fdt)
{
	int offs, ret;

	do {
		offs = fdt_path_offset(fdt, "/cpus");
		if (offs < 0)
			return offs;

		ret = dt_update_one_cpu_node(fdt, offs);
	} while (ret > 0);

	return ret;
}

#define HIGH_BITS(x) ((sizeof(x) > 4) ? ((x) >> 32) : (typeof(x))0)

/*******************************************************************************
 * fdt_add_reserved_memory() - reserve (secure) memory regions in DT
 * @dtb:	pointer to the device tree blob in memory
 * @node_name:	name of the subnode to be used
 * @base:	physical base address of the reserved region
 * @size:	size of the reserved region
 *
 * Add a region of memory to the /reserved-memory node in a device tree in
 * memory, creating that node if required. Each region goes into a subnode
 * of that node and has a @node_name, a @base address and a @size.
 * This will prevent any device tree consumer from using that memory. It
 * can be used to announce secure memory regions, as it adds the "no-map"
 * property to prevent mapping and speculative operations on that region.
 *
 * See reserved-memory/reserved-memory.txt in the (Linux kernel) DT binding
 * documentation for details.
 * According to this binding, the address-cells and size-cells must match
 * those of the root node.
 *
 * Return: 0 on success, a negative error value otherwise.
 ******************************************************************************/
int fdt_add_reserved_memory(void *dtb, const char *node_name,
			    uintptr_t base, size_t size)
{
	int offs = fdt_path_offset(dtb, "/reserved-memory");
	uint32_t addresses[4];
	int ac, sc;
	unsigned int idx = 0;

	ac = fdt_address_cells(dtb, 0);
	sc = fdt_size_cells(dtb, 0);
	if (offs < 0) {			/* create if not existing yet */
		offs = fdt_add_subnode(dtb, 0, "reserved-memory");
		if (offs < 0) {
			return offs;
		}
		fdt_setprop_u32(dtb, offs, "#address-cells", ac);
		fdt_setprop_u32(dtb, offs, "#size-cells", sc);
		fdt_setprop(dtb, offs, "ranges", NULL, 0);
	}

	if (ac > 1) {
		addresses[idx] = cpu_to_fdt32(HIGH_BITS(base));
		idx++;
	}
	addresses[idx] = cpu_to_fdt32(base & 0xffffffff);
	idx++;
	if (sc > 1) {
		addresses[idx] = cpu_to_fdt32(HIGH_BITS(size));
		idx++;
	}
	addresses[idx] = cpu_to_fdt32(size & 0xffffffff);
	idx++;
	offs = fdt_add_subnode(dtb, offs, node_name);
	fdt_setprop(dtb, offs, "no-map", NULL, 0);
	fdt_setprop(dtb, offs, "reg", addresses, idx * sizeof(uint32_t));

	return 0;
}

/*******************************************************************************
 * fdt_add_cpu()	Add a new CPU node to the DT
 * @dtb:		Pointer to the device tree blob in memory
 * @parent:		Offset of the parent node
 * @mpidr:		MPIDR for the current CPU
 *
 * Create and add a new cpu node to a DTB.
 *
 * Return the offset of the new node or a negative value in case of error
 ******************************************************************************/

static int fdt_add_cpu(void *dtb, int parent, u_register_t mpidr)
{
	int cpu_offs;
	int err;
	char snode_name[15];
	uint64_t reg_prop;

	reg_prop = mpidr & MPID_MASK & ~MPIDR_MT_MASK;

	snprintf(snode_name, sizeof(snode_name), "cpu@%x",
					(unsigned int)reg_prop);

	cpu_offs = fdt_add_subnode(dtb, parent, snode_name);
	if (cpu_offs < 0) {
		ERROR ("FDT: add subnode \"%s\" failed: %i\n",
							snode_name, cpu_offs);
		return cpu_offs;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "compatible", "arm,armv8");
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"compatible", cpu_offs);
		return err;
	}

	err = fdt_setprop_u64(dtb, cpu_offs, "reg", reg_prop);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"reg", cpu_offs);
		return err;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "device_type", "cpu");
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"device_type", cpu_offs);
		return err;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "enable-method", "psci");
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"enable-method", cpu_offs);
		return err;
	}

	return cpu_offs;
}

/******************************************************************************
 * fdt_add_cpus_node() - Add the cpus node to the DTB
 * @dtb:		pointer to the device tree blob in memory
 * @afflv0:		Maximum number of threads per core (affinity level 0).
 * @afflv1:		Maximum number of CPUs per cluster (affinity level 1).
 * @afflv2:		Maximum number of clusters (affinity level 2).
 *
 * Iterate over all the possible MPIDs given the maximum affinity levels and
 * add a cpus node to the DTB with all the valid CPUs on the system.
 * If there is already a /cpus node, exit gracefully
 *
 * A system with two CPUs would generate a node equivalent or similar to:
 *
 *	cpus {
 *		#address-cells = <2>;
 *		#size-cells = <0>;
 *
 *		cpu0: cpu@0 {
 *			compatible = "arm,armv8";
 *			reg = <0x0 0x0>;
 *			device_type = "cpu";
 *			enable-method = "psci";
 *		};
 *		cpu1: cpu@10000 {
 *			compatible = "arm,armv8";
 *			reg = <0x0 0x100>;
 *			device_type = "cpu";
 *			enable-method = "psci";
 *		};
 *	};
 *
 * Full documentation about the CPU bindings can be found at:
 * https://www.kernel.org/doc/Documentation/devicetree/bindings/arm/cpus.txt
 *
 * Return the offset of the node or a negative value on error.
 ******************************************************************************/

int fdt_add_cpus_node(void *dtb, unsigned int afflv0,
		      unsigned int afflv1, unsigned int afflv2)
{
	int offs;
	int err;
	unsigned int i, j, k;
	u_register_t mpidr;
	int cpuid;

	if (fdt_path_offset(dtb, "/cpus") >= 0) {
		return -EEXIST;
	}

	offs = fdt_add_subnode(dtb, 0, "cpus");
	if (offs < 0) {
		ERROR ("FDT: add subnode \"cpus\" node to parent node failed");
		return offs;
	}

	err = fdt_setprop_u32(dtb, offs, "#address-cells", 2);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"#address-cells", offs);
		return err;
	}

	err = fdt_setprop_u32(dtb, offs, "#size-cells", 0);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"#size-cells", offs);
		return err;
	}

	/*
	 * Populate the node with the CPUs.
	 * As libfdt prepends subnodes within a node, reverse the index count
	 * so the CPU nodes would be better ordered.
	 */
	for (i = afflv2; i > 0U; i--) {
		for (j = afflv1; j > 0U; j--) {
			for (k = afflv0; k > 0U; k--) {
				mpidr = ((i - 1) << MPIDR_AFF2_SHIFT) |
					((j - 1) << MPIDR_AFF1_SHIFT) |
					((k - 1) << MPIDR_AFF0_SHIFT) |
					(read_mpidr_el1() & MPIDR_MT_MASK);

				cpuid = plat_core_pos_by_mpidr(mpidr);
				if (cpuid >= 0) {
					/* Valid MPID found */
					err = fdt_add_cpu(dtb, offs, mpidr);
					if (err < 0) {
						ERROR ("FDT: %s 0x%08x\n",
							"error adding CPU",
							(uint32_t)mpidr);
						return err;
					}
				}
			}
		}
	}

	return offs;
}

/*******************************************************************************
 * fdt_add_cpu_idle_states() - add PSCI CPU idle states to cpu nodes in the DT
 * @dtb:	pointer to the device tree blob in memory
 * @states:	array of idle state descriptions, ending with empty element
 *
 * Add information about CPU idle states to the devicetree. This function
 * assumes that CPU idle states are not already present in the devicetree, and
 * that all CPU states are equally applicable to all CPUs.
 *
 * See arm/idle-states.yaml and arm/psci.yaml in the (Linux kernel) DT binding
 * documentation for more details.
 *
 * Return: 0 on success, a negative error value otherwise.
 ******************************************************************************/
int fdt_add_cpu_idle_states(void *dtb, const struct psci_cpu_idle_state *state)
{
	int cpu_node, cpus_node, idle_states_node, ret;
	uint32_t count, phandle;

	ret = fdt_find_max_phandle(dtb, &phandle);
	phandle++;
	if (ret < 0) {
		return ret;
	}

	cpus_node = fdt_path_offset(dtb, "/cpus");
	if (cpus_node < 0) {
		return cpus_node;
	}

	/* Create the idle-states node and its child nodes. */
	idle_states_node = fdt_add_subnode(dtb, cpus_node, "idle-states");
	if (idle_states_node < 0) {
		return idle_states_node;
	}

	ret = fdt_setprop_string(dtb, idle_states_node, "entry-method", "psci");
	if (ret < 0) {
		return ret;
	}

	for (count = 0U; state->name != NULL; count++, phandle++, state++) {
		int idle_state_node;

		idle_state_node = fdt_add_subnode(dtb, idle_states_node,
						  state->name);
		if (idle_state_node < 0) {
			return idle_state_node;
		}

		fdt_setprop_string(dtb, idle_state_node, "compatible",
				   "arm,idle-state");
		fdt_setprop_u32(dtb, idle_state_node, "arm,psci-suspend-param",
				state->power_state);
		if (state->local_timer_stop) {
			fdt_setprop_empty(dtb, idle_state_node,
					  "local-timer-stop");
		}
		fdt_setprop_u32(dtb, idle_state_node, "entry-latency-us",
				state->entry_latency_us);
		fdt_setprop_u32(dtb, idle_state_node, "exit-latency-us",
				state->exit_latency_us);
		fdt_setprop_u32(dtb, idle_state_node, "min-residency-us",
				state->min_residency_us);
		if (state->wakeup_latency_us) {
			fdt_setprop_u32(dtb, idle_state_node,
					"wakeup-latency-us",
					state->wakeup_latency_us);
		}
		fdt_setprop_u32(dtb, idle_state_node, "phandle", phandle);
	}

	if (count == 0U) {
		return 0;
	}

	/* Link each cpu node to the idle state nodes. */
	fdt_for_each_subnode(cpu_node, dtb, cpus_node) {
		const char *device_type;
		fdt32_t *value;

		/* Only process child nodes with device_type = "cpu". */
		device_type = fdt_getprop(dtb, cpu_node, "device_type", NULL);
		if (device_type == NULL || strcmp(device_type, "cpu") != 0) {
			continue;
		}

		/* Allocate space for the list of phandles. */
		ret = fdt_setprop_placeholder(dtb, cpu_node, "cpu-idle-states",
					      count * sizeof(phandle),
					      (void **)&value);
		if (ret < 0) {
			return ret;
		}

		/* Fill in the phandles of the idle state nodes. */
		for (uint32_t i = 0U; i < count; ++i) {
			value[i] = cpu_to_fdt32(phandle - count + i);
		}
	}

	return 0;
}

/**
 * fdt_adjust_gic_redist() - Adjust GICv3 redistributor size
 * @dtb: Pointer to the DT blob in memory
 * @nr_cores: Number of CPU cores on this system.
 * @gicr_base: Base address of the first GICR frame, or ~0 if unchanged
 * @gicr_frame_size: Size of the GICR frame per core
 *
 * On a GICv3 compatible interrupt controller, the redistributor provides
 * a number of 64k pages per each supported core. So with a dynamic topology,
 * this size cannot be known upfront and thus can't be hardcoded into the DTB.
 *
 * Find the DT node describing the GICv3 interrupt controller, and adjust
 * the size of the redistributor to match the number of actual cores on
 * this system.
 * A GICv4 compatible redistributor uses four 64K pages per core, whereas GICs
 * without support for direct injection of virtual interrupts use two 64K pages.
 * The @gicr_frame_size parameter should be 262144 and 131072, respectively.
 * Also optionally allow adjusting the GICR frame base address, when this is
 * different due to ITS frames between distributor and redistributor.
 *
 * Return: 0 on success, negative error value otherwise.
 */
int fdt_adjust_gic_redist(void *dtb, unsigned int nr_cores,
			  uintptr_t gicr_base, unsigned int gicr_frame_size)
{
	int offset = fdt_node_offset_by_compatible(dtb, 0, "arm,gic-v3");
	uint64_t reg_64;
	uint32_t reg_32;
	void *val;
	int parent, ret;
	int ac, sc;

	if (offset < 0) {
		return offset;
	}

	parent = fdt_parent_offset(dtb, offset);
	if (parent < 0) {
		return parent;
	}
	ac = fdt_address_cells(dtb, parent);
	sc = fdt_size_cells(dtb, parent);
	if (ac < 0 || sc < 0) {
		return -EINVAL;
	}

	if (gicr_base != INVALID_BASE_ADDR) {
		if (ac == 1) {
			reg_32 = cpu_to_fdt32(gicr_base);
			val = &reg_32;
		} else {
			reg_64 = cpu_to_fdt64(gicr_base);
			val = &reg_64;
		}
		/*
		 * The redistributor base address is the second address in
		 * the "reg" entry, so we have to skip one address and one
		 * size cell.
		 */
		ret = fdt_setprop_inplace_namelen_partial(dtb, offset,
							  "reg", 3,
							  (ac + sc) * 4,
							  val, ac * 4);
		if (ret < 0) {
			return ret;
		}
	}

	if (sc == 1) {
		reg_32 = cpu_to_fdt32(nr_cores * gicr_frame_size);
		val = &reg_32;
	} else {
		reg_64 = cpu_to_fdt64(nr_cores * (uint64_t)gicr_frame_size);
		val = &reg_64;
	}

	/*
	 * The redistributor is described in the second "reg" entry.
	 * So we have to skip one address and one size cell, then another
	 * address cell to get to the second size cell.
	 */
	return fdt_setprop_inplace_namelen_partial(dtb, offset, "reg", 3,
						   (ac + sc + ac) * 4,
						   val, sc * 4);
}
