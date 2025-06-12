/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <lib/utils.h>

#include "sdei_private.h"

#define MAP_OFF(_map, _mapping) ((_map) - (_mapping)->map)

/*
 * Get SDEI entry with the given mapping: on success, returns pointer to SDEI
 * entry. On error, returns NULL.
 *
 * Both shared and private maps are stored in single-dimensional array. Private
 * event entries are kept for each PE forming a 2D array.
 */
sdei_entry_t *get_event_entry(const sdei_ev_map_t *map)
{
	const sdei_mapping_t *mapping;
	sdei_entry_t *cpu_priv_base;
	unsigned int base_idx;
	long int idx;

	if ((map->map_flags & BIT_32(SDEI_MAPF_PRIVATE_SHIFT_)) != 0U) {
		/*
		 * For a private map, find the index of the mapping in the
		 * array.
		 */
		mapping = SDEI_PRIVATE_MAPPING();
		idx = MAP_OFF(map, mapping);

		/* Base of private mappings for this CPU */
		base_idx = plat_my_core_pos() * ((unsigned int) mapping->num_maps);
		cpu_priv_base = &sdei_private_event_table[base_idx];

		/*
		 * Return the address of the entry at the same index in the
		 * per-CPU event entry.
		 */
		return &cpu_priv_base[idx];
	} else {
		mapping = SDEI_SHARED_MAPPING();
		idx = MAP_OFF(map, mapping);

		return &sdei_shared_event_table[idx];
	}
}

/*
 * Retrieve the SDEI entry for the given mapping and target PE.
 *
 * on success : Returns a pointer to the SDEI entry
 *
 * On error, returns NULL
 *
 * Both shared and private maps are stored in single-dimensional array. Private
 * event entries are kept for each PE forming a 2D array.
 */
sdei_entry_t *get_event_entry_target_pe(long int mapsub, unsigned int nm,
uint64_t target_pe)
{
	sdei_entry_t *cpu_priv_base;
	unsigned int base_idx = 0U;
	long int idx;
	int target_pos;

	/*
	 * For a private map, find the index of the mapping in the
	 * array.
	 */
	idx = mapsub;

	/* Base of private mappings for this CPU */
	target_pos = plat_core_pos_by_mpidr(target_pe);

	if ((target_pos < 0) || ((unsigned int)target_pos >= PLATFORM_CORE_COUNT)) {
		return NULL;
	}

	base_idx = (unsigned int) target_pos;
	base_idx *= nm;

	cpu_priv_base = &sdei_private_event_table[base_idx];

	/*
	 * Return the address of the entry at the same index in the
	 * per-CPU event entry.
	 */
	return &cpu_priv_base[idx];
}

/*
 * Find event mapping for a given interrupt number: On success, returns pointer
 * to the event mapping. On error, returns NULL.
 */
sdei_ev_map_t *find_event_map_by_intr(unsigned int intr_num, bool shared)
{
	const sdei_mapping_t *mapping;
	sdei_ev_map_t *map;
	unsigned int i;

	/*
	 * Look for a match in private and shared mappings, as requested. This
	 * is a linear search. However, if the mappings are required to be
	 * sorted, for large maps, we could consider binary search.
	 */
	mapping = shared ? SDEI_SHARED_MAPPING() : SDEI_PRIVATE_MAPPING();
	iterate_mapping(mapping, i, map) {
		if (map->intr == intr_num)
			return map;
	}

	return NULL;
}

/*
 * Find event mapping for a given event number: On success returns pointer to
 * the event mapping. On error, returns NULL.
 */
sdei_ev_map_t *find_event_map(int ev_num)
{
	const sdei_mapping_t *mapping;
	sdei_ev_map_t *map;
	unsigned int i, j;

	/*
	 * Iterate through mappings to find a match. This is a linear search.
	 * However, if the mappings are required to be sorted, for large maps,
	 * we could consider binary search.
	 */
	for_each_mapping_type(i, mapping) {
		iterate_mapping(mapping, j, map) {
			if (map->ev_num == ev_num)
				return map;
		}
	}

	return NULL;
}

/*
 * Return the total number of currently registered SDEI events.
 */
int sdei_get_registered_event_count(void)
{
	const sdei_mapping_t *mapping;
	sdei_ev_map_t *map;
	unsigned int i;
	unsigned int j;
	int count = 0;

	/* Add up reg counts for each mapping. */
	for_each_mapping_type(i, mapping) {
		iterate_mapping(mapping, j, map) {
			count += map->reg_count;
		}
	}

	return count;
}
