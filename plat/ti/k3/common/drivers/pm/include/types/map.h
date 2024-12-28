/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MAP_H
#define MAP_H

#include <tcon.h>
#include <typesafe_cb.h>
#include <types.h>
#include <types/short_types.h>
#include <string.h>

/**
 * struct map - representation of a map
 *
 * It's exposed here to allow you to embed it and so we can inline the
 * trivial functions.
 */
struct map {
	union {
		struct map_node *n;
		const void	*s;
	} u;
	struct map_node *v;
};

struct map_node {
	/* These point to values or nodes. */
	struct map	child[2];
	/* The byte number where first bit differs. */
	size_t		byte_num;
	/* The bit where these children differ. */
	uint8_t		bit_num;
};

/**
 * strmap_get - get a value from a string map
 * \param map_ptr the typed map to search.
 * \param member the string to search for.
 *
 * Returns the node, or NULL if it isn't in the map (and sets errno = ENOENT).
 *
 * Example:
 *	struct map_node *node = strmap_get(&map, "hello");
 */
struct map_node *strmap_get(const struct map *map_ptr, const char *member);

/**
 * uint32_tmap_get - get a value from a uint32_t map
 * \param map_ptr the typed map to search.
 * \param member the uint32_t to search for.
 *
 * Returns the node, or NULL if it isn't in the map (and sets errno = ENOENT).
 *
 * Example:
 *	struct map_node *node = uint32_tmap_get(&map, 32425);
 */
struct map_node *uint32_tmap_get(const struct map *map_ptr, uint32_t member);

struct map *map_add(struct map *map_ptr, const void *member, const uint8_t *bytes, size_t len, struct map_node *newn, const uint8_t *(*get_bytes)(struct map *m));

/**
 * strmap_add - place a member in the string map.
 * \param map_ptr the typed map to add to.
 *
 * This returns false if that string already appears in the map (EEXIST).
 *
 * Note that the pointer is placed in the map, the string is not copied.  If
 * you want a copy in the map, use strdup().  Similarly for the value.
 */
const uint8_t *str_get_bytes(struct map *map_ptr);

static inline struct map *strmap_add(struct map *map_ptr, const char *member,
				     struct map_node *newn)
{
	return map_add(map_ptr, member, (const uint8_t *) member, strlen(member), newn,
		       str_get_bytes);
}

/**
 * uint32_tmap_add - place a member in the e21 map.
 * \param map_ptr the typed map to add to.
 *
 * This returns false if that uint32_t already appears in the map (EEXIST).
 */
const uint8_t *uint32_t_get_bytes(struct map *map_ptr);

static inline struct map *uint32_tmap_add(struct map *map_ptr, uint32_t member,
					  struct map_node *newn)
{
	return map_add(map_ptr, (const void *) member, (const uint8_t *) &member,
		       sizeof(uint32_t), newn, uint32_t_get_bytes);
}

/**
 * strmap_iterate - ordered iteration over a map
 * \param map_ptr the typed map to iterate through.
 * \param handle the function to call.
 * \param arg the argument for the function (types should match).
 *
 * \p handle's prototype should be:
 *	bool \p handle(const char *member, struct map_node *n, typeof(arg) arg)
 *
 * If \p handle returns false, the iteration will stop.
 * You should not alter the map within the \p handle function!
 *
 * Example:
 *	static bool dump_some(const char *member, struct map_node *n, int *num)
 *	{
 *		// Only dump out num nodes.
 *		if (*(num--) == 0)
 *			return false;
 *		printf("%s=>%p\n", member, n);
 *		return true;
 *	}
 *
 *	static void dump_map(const struct strmap_intp *map)
 *	{
 *		int32_t max = 100;
 *		strmap_iterate(map, dump_some, &max);
 *		if (max < 0)
 *			printf("... (truncated to 100 entries)\n");
 *	}
 */
#define strmap_iterate(map_ptr, handle, arg)				\
	strmap_iterate_((map_ptr),					\
			typesafe_cb_cast(bool (*)(const char *, struct map_node *, void *), \
					 bool (*)(const char *, struct	      map_node *, typeof(arg)), (handle)), (arg))
void strmap_iterate_(const struct map *map_ptr, bool (*handle)(const char *const_ptr, struct map_node *node_ptr, void *v_ptr), const void *data);

/**
 * uint32_tmap_iterate - ordered iteration over a map
 * \param map_ptr the typed map to iterate through.
 * \param handle the function to call.
 * \param arg the argument for the function (types should match).
 *
 * \p handle's prototype should be:
 *	bool \p handle(uint32_t member, struct map_node *n, typeof(arg) arg)
 *
 * If \p handle returns false, the iteration will stop.
 * You should not alter the map within the \p handle function!
 */
#define uint32_tmap_iterate(map_ptr, handle, arg)			\
	uint32_tmap_iterate_((map_ptr),					\
			     typesafe_cb_cast(bool (*)(uint32_t, struct map_node *, void *), \
					      bool (*)(uint32_t, struct map_node *, typeof(arg)), (handle)), (arg))
void uint32_tmap_iterate_(const struct map *map_ptr, bool (*handle)(uint32_t hdata, struct map_node *node_ptr, void *v_ptr), const void *data);
/**
 * strmap_prefix - return a submap matching a prefix
 * \param map_ptr the map.
 * \param prefix the prefix.
 *
 * This returns a pointer into \p map_ptr, so don't alter \p map_ptr while using
 * the return value.  You can use strmap_iterate() or strmap_get()
 * on the returned pointer.
 *
 * Example:
 *	static void dump_prefix(const struct map *map,
 *				const char *prefix)
 *	{
 *		int32_t max = 100;
 *		printf("Nodes with prefix %s:\n", prefix);
 *		strmap_iterate(strmap_prefix(map, prefix), dump_some, &max);
 *		if (max < 0)
 *			printf("... (truncated to 100 entries)\n");
 *	}
 */
const struct map *strmap_prefix(const struct map *map_ptr, const char *prefix);

#endif /* _STRMAP_H_ */
