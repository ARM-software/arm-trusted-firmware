/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
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
sdei_entry_t *get_event_entry(sdei_ev_map_t *map)
{
	const sdei_mapping_t *mapping;
	sdei_entry_t *cpu_priv_base;
	unsigned int base_idx;
	long int idx;

	if (is_event_private(map)) {
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
