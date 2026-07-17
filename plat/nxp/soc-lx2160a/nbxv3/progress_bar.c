/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 */

#include <stdio.h>

#include <common/debug.h>

#include "progress_bar.h"

/*
 * Layout budget: 120 terminal columns total.
 *   "  [" prefix   = 3
 *   "]" suffix     = 1
 *   marker area    = 116
 * The header line above the bar carries the NOTICE prefix and
 * the label/scale text; that line is printed via NOTICE() and
 * uses the platform's normal log path.
 */
#define NBXV3_PB_MAX_MARKS	116U

static char nbxv3_pb_glyph(enum nbxv3_pb_mark m)
{
	switch (m) {
	case NBXV3_PB_VERIFY_ERASE:	return 'e';
	case NBXV3_PB_ERASE:		return 'E';
	case NBXV3_PB_WRITE:		return 'W';
	case NBXV3_PB_READ:		return 'R';
	case NBXV3_PB_GIVE_UP:		return 'X';
	case NBXV3_PB_NONE:
	default:			return ' ';
	}
}

static void nbxv3_pb_flush_group(struct nbxv3_pb *pb)
{
	if (pb->group_mark == NBXV3_PB_NONE) {
		return;
	}

	(void)putchar(nbxv3_pb_glyph(pb->group_mark));
	pb->emitted++;
	pb->group_mark = NBXV3_PB_NONE;
	pb->group_index = 0U;
}

void nbxv3_pb_begin(struct nbxv3_pb *pb, const char *label,
		    size_t total_chunks)
{
	size_t cpm;

	pb->label = (label != NULL) ? label : "";
	pb->total_chunks = total_chunks;
	pb->chunk_index = 0U;
	pb->group_index = 0U;
	pb->group_mark = NBXV3_PB_NONE;
	pb->emitted = 0U;
	pb->finalised = 0U;

	if (total_chunks == 0U) {
		pb->chunks_per_mark = 1U;
	} else {
		cpm = (total_chunks + NBXV3_PB_MAX_MARKS - 1U) /
		      NBXV3_PB_MAX_MARKS;
		pb->chunks_per_mark = (cpm == 0U) ? 1U : cpm;
	}

	NOTICE("nbxv3 flash: %s, %zu chunk(s), 1 mark = %zu chunk(s)\n",
	       pb->label, pb->total_chunks, pb->chunks_per_mark);
	NOTICE("  [");
}

void nbxv3_pb_step(struct nbxv3_pb *pb, enum nbxv3_pb_mark m)
{
	if (pb->finalised) {
		return;
	}

	/*
	 * Worst-event-wins aggregation: the enum's numeric ordering
	 * encodes X > R > W > V > E > e
	 */
	if ((unsigned int)m > (unsigned int)pb->group_mark) {
		pb->group_mark = m;
	}

	pb->chunk_index++;
	pb->group_index++;
	if (pb->group_index >= pb->chunks_per_mark) {
		nbxv3_pb_flush_group(pb);
	}
}

void nbxv3_pb_end(struct nbxv3_pb *pb)
{
	if (pb->finalised) {
		return;
	}

	nbxv3_pb_flush_group(pb);
	(void)putchar(']');
	(void)putchar('\n');
	pb->finalised = 1U;
}
