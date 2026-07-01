/*
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libfdt.h>

#include <common/debug.h>
#include <plat/common/platform.h>

#include <plat_fdt.h>
#include <platform_def.h>

#include <plat_private.h>

/* Global definition of primary core variable */
uint32_t plat_primary_cpu_core = PLAT_INVALID_CPU_CORE;

/* Global definition of core and cluster variables defaulting to fixed configuration */
uint32_t plat_cluster_count = PLATFORM_CLUSTER_COUNT;
uint32_t plat_cores_per_cluster = PLATFORM_CORE_COUNT_PER_CLUSTER;

#if VERSAL2_VARIANT == 14
static const uint8_t plat_power_domain_tree_desc[] = {
	/* Number of root nodes */
	1,
	/* Number of clusters */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the only cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
};
#elif VERSAL2_VARIANT == 42
static const uint8_t plat_power_domain_tree_desc[] = {
	/* Number of root nodes */
	1,
	/* Number of clusters */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the first cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
	/* Number of children for the second cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
	/* Number of children for the third cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
	/* Number of children for the fourth cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
};
#endif

#if !((VERSAL2_VARIANT == 42) || (VERSAL2_VARIANT == 14))
/* cluster and core updated at runtime */
static uint8_t plat_power_domain_tree_dyn[PLATFORM_CLUSTER_COUNT + PLAT_PWR_DOMAIN_PREFIX_SIZE] = {0};

/*
 * Parse DT /cpus and cpu-map to determine cluster/core topology.
 * Returns 0 on success, <0 on error.
 */
static int32_t parse_dt_topology(uint8_t *out_clusters, uint8_t *out_cores_per_cluster,
		uint8_t *out_children_per_cluster, uint32_t max_clusters)
{
	void *dtb = (void *)plat_retrieve_dt_addr();
	int32_t cpus_off, cmap_off, cluster_off;
	int32_t ret = 0;
	int32_t core_off;
	uint8_t clusters = 0, cores = 0;
	const char *node_name;
	uint8_t i;

	if (dtb == NULL) {
		ret = -FDT_ERR_NOTFOUND;
		goto exit_on_failure;
	}

	ret = add_mmap_dynamic_region((unsigned long long)dtb,
			(uintptr_t)dtb,
			XILINX_OF_BOARD_DTB_MAX_SIZE,
			MT_MEMORY | MT_RO | MT_NS);
	if (ret != 0) {
		goto exit_on_failure;
	}

	ret = fdt_check_header(dtb);
	if (ret < 0) {
		goto unmap_dtb;
	}

	/* cpus node in DT */
	cpus_off = fdt_path_offset(dtb, "/cpus");
	if (cpus_off < 0) {
		ret = cpus_off;
		goto unmap_dtb;
	}

	/* cpu-map dt path /cpus/cpu-map */
	cmap_off = fdt_subnode_offset(dtb, cpus_off, "cpu-map");
	if (cmap_off < 0) {
		ret = cmap_off;
		goto unmap_dtb;
	}

	/* Count clusters under cpu-map */
	fdt_for_each_subnode(cluster_off, dtb, cmap_off) {
		/* Validate cluster node name */
		node_name = fdt_get_name(dtb, cluster_off, NULL);
		if ((node_name == NULL) || (strncmp(node_name, "cluster", 7) != 0)) {
			continue;
		}

		if (clusters >= max_clusters) {
			WARN("cluster count exceeds max clusters: %u\n", max_clusters);
			break;
		}

		/* Count cores in this cluster */
		fdt_for_each_subnode(core_off, dtb, cluster_off) {
			/* Validate core node name */
			node_name = fdt_get_name(dtb, core_off, NULL);
			if ((node_name == NULL) || (strncmp(node_name, "core", 4) != 0)) {
				continue;
			}
			cores++;
		}

		/* If this cluster has no cores return error */
		if (cores == 0) {
			ret = E_INVALID_CORE_COUNT;
			goto unmap_dtb;
		}

		out_children_per_cluster[clusters] = cores;
		clusters++;
		/* reset the core count for next cluster */
		cores = 0;
	}

	if (clusters == 0) {
		ret = -FDT_ERR_NOTFOUND;
		goto unmap_dtb;
	}

	/* Check if all clusters have the same number of cores */
	for (i = 1; i < clusters; i++) {
		if (out_children_per_cluster[i] != out_children_per_cluster[0]) {
			ret = E_INVALID_CORE_COUNT;
			WARN("Asymmetric cluster: cluster %u has %u cores, cluster0 has %u cores\n",
					i, out_children_per_cluster[i],
					out_children_per_cluster[0]);
			goto unmap_dtb;
		}
	}

	*out_clusters = clusters;
	*out_cores_per_cluster = out_children_per_cluster[0];

unmap_dtb:
	(void)remove_mmap_dynamic_region((uintptr_t)dtb,
			XILINX_OF_BOARD_DTB_MAX_SIZE);

exit_on_failure:
	return ret;
}

static int32_t init_topology_from_dt(void)
{
	static bool topology_initialized;
	static int32_t cached_result = E_INVALID_CORE_COUNT;
	uint8_t clusters = PLATFORM_CLUSTER_COUNT;
	uint8_t cores_per_cluster = PLATFORM_CORE_COUNT_PER_CLUSTER;
	uint8_t children[PLATFORM_CLUSTER_COUNT] = {0};
	int32_t ret = 0;
	uint8_t i;

	if (topology_initialized) {
		ret = cached_result;
		goto done;
	}

	ret = parse_dt_topology(&clusters, &cores_per_cluster, children, PLATFORM_CLUSTER_COUNT);
	if (ret != 0) {
		goto exit_on_failure;
	}

	if ((clusters * cores_per_cluster) > PLATFORM_CORE_COUNT) {
		ERROR("cluster * core count exceeds max cores %u\n", PLATFORM_CORE_COUNT);
		ret = E_INVALID_CORE_COUNT;
		goto exit_on_failure;
	}

	plat_cluster_count = clusters;
	plat_cores_per_cluster = cores_per_cluster;

	plat_power_domain_tree_dyn[0] = 1;
	plat_power_domain_tree_dyn[1] = clusters;

	for (i = 0; i < clusters; i++) {
		plat_power_domain_tree_dyn[PLAT_PWR_DOMAIN_PREFIX_SIZE + i] = children[i];
	}

exit_on_failure:
	topology_initialized = true;
	cached_result = ret;
done:
	return ret;
}
#endif

const uint8_t *plat_get_power_domain_tree_desc(void)
{
	const uint8_t *ret = NULL;

#if (VERSAL2_VARIANT == 14) || (VERSAL2_VARIANT == 42)
	ret = plat_power_domain_tree_desc;
#else
	if (init_topology_from_dt() == 0) {
		ret = plat_power_domain_tree_dyn;
	} else {
		ret = plat_power_domain_tree_desc;
	}
#endif

	return ret;
}
