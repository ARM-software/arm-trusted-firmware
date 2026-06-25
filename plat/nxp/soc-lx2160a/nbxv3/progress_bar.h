/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Bounded-width per-region progress bar for the manifest-driven
 * NOR provisioning path.
 *
 * Markers:
 *
 *   R  read source
 *   W  write to NOR
 *   E  erase sector
 *   e  verify-erase (post-erase 0xFF readback)
 *   X  give-up: failure on this chunk
 *
 * Aggregation rule: when N chunks collapse into one mark, the
 * mark represents the worst event seen in the group, with
 * priority X > R > W > E > e. This keeps failures visible
 * after downscaling.
 */

#ifndef NBXV3_PROGRESS_BAR_H
#define NBXV3_PROGRESS_BAR_H

#include <stddef.h>
#include <stdint.h>

/*
 * Marker value
 *
 * one of these per chunk event. The numeric values
 * encode the X > R > W > E > e priority used by the downscale
 * aggregator: a larger value beats a smaller one when several
 * chunks collapse into a single mark.
 */
enum nbxv3_pb_mark {
	NBXV3_PB_NONE = 0,
	NBXV3_PB_VERIFY_ERASE = 1,	/* 'e' */
	NBXV3_PB_ERASE = 2,		/* 'E' */
	NBXV3_PB_WRITE = 3,		/* 'W' */
	NBXV3_PB_READ = 4,		/* 'R' */
	NBXV3_PB_GIVE_UP = 5,		/* 'X' */
};

/*
 * Opaque bar state. The runner owns one of these per region; it
 * lives on the stack of the driving function. No globals.
 */
struct nbxv3_pb {
	const char	*label;
	size_t		 total_chunks;
	size_t		 chunks_per_mark;
	size_t		 chunk_index;
	size_t		 group_index;
	enum nbxv3_pb_mark group_mark;
	uint8_t		 emitted;	/* number of marks already emitted */
	uint8_t		 finalised;
};

/*
 * Print the header line, open the bar, and prepare aggregation
 * state. Reserves at most 116 marker positions between the
 * brackets so the whole line stays within 120 columns including
 * "  [" and "]".
 *
 * @label         short user-visible region name (e.g. "PBL", "FIP")
 * @total_chunks  number of chunks the runner will emit marks for
 *                (one nbxv3_pb_step() per chunk)
 */
void nbxv3_pb_begin(struct nbxv3_pb *pb, const char *label,
		    size_t total_chunks);

/*
 * Record one chunk event. Call once per chunk. When several
 * chunks aggregate into one mark, only the worst event in the
 * group is emitted (priority X > R > W > V > E > e).
 */
void nbxv3_pb_step(struct nbxv3_pb *pb, enum nbxv3_pb_mark m);

/*
 * Close the bar. Flushes the in-flight aggregation group if any
 * and prints the closing bracket plus a newline. Safe to call
 * twice (the second call is a no-op).
 */
void nbxv3_pb_end(struct nbxv3_pb *pb);

#endif /* NBXV3_PROGRESS_BAR_H */
