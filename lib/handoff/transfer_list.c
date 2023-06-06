// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <assert.h>
#include <common/debug.h>
#include <inttypes.h>
#include <lib/transfer_list.h>
#include <lib/utils_def.h>
#include <string.h>

#define ADD_OVERFLOW(a, b, res) __builtin_add_overflow((a), (b), (res))

#define SUB_OVERFLOW(a, b, res) __builtin_sub_overflow((a), (b), (res))

#define MUL_OVERFLOW(a, b, res) __builtin_mul_overflow((a), (b), (res))

#define ROUNDUP_OVERFLOW(v, size, res) (__extension__({ \
	typeof(*(res)) __roundup_tmp = 0; \
	typeof(v) __roundup_mask = (typeof(v))(size) - 1; \
	\
	ADD_OVERFLOW((v), __roundup_mask, &__roundup_tmp) ? 1 : \
		(void)(*(res) = __roundup_tmp & ~__roundup_mask), 0; \
}))

// 2.4.5 Creating a TL
struct transfer_list_header *transfer_list_init(void *p, size_t max_size)
{
	uintptr_t mask = (1 << TRANSFER_LIST_INIT_MAX_ALIGN) - 1;
	struct transfer_list_header *tl = p;

	if (((uintptr_t)p & mask) || (max_size & mask) ||
	    max_size < sizeof(*tl))
		return NULL;

	memset(tl, 0, max_size);
	tl->signature = TRANSFER_LIST_SIGNATURE;
	tl->version = TRANSFER_LIST_VERSION;
	tl->hdr_size = sizeof(*tl);
	tl->alignment = TRANSFER_LIST_INIT_MAX_ALIGN; // initial max align
	tl->size = sizeof(*tl); // initial size is the size of header
	tl->max_size = max_size;
	transfer_list_update_checksum(tl);
	return tl;
}

// 2.4.6 Relocating a TL
bool transfer_list_relocate(struct transfer_list_header *tl, void **p, size_t max_size)
{
	if (!tl || !p || !(*p) || max_size == 0)
		return false;

	uintptr_t mask = (1 << tl->alignment) - 1;
	uintptr_t new_tl_base = ((uintptr_t)(*p) + mask) & ~mask;

	if (new_tl_base < (uintptr_t)(*p))
		new_tl_base += mask + 1;

	if (new_tl_base - (uintptr_t)(*p) + tl->size > max_size)
		return false;

	memmove((void *)new_tl_base, (void *)tl, tl->size);
	((struct transfer_list_header *)new_tl_base)->max_size = max_size; //to-check
	*p = (void *)new_tl_base;
	transfer_list_update_checksum((struct transfer_list_header *)(*p));
	return true;
}

static bool check_header(struct transfer_list_header *tl)
{
	if (!tl)
		return false;

	if (tl->signature != TRANSFER_LIST_SIGNATURE) {
		ERROR("Bad transfer list signature %#"PRIx32"\n",
		      tl->signature);
		return false;
	}
	if (tl->version != TRANSFER_LIST_VERSION) {
		ERROR("Unsupported transfer list version %#"PRIx32"\n",
		      tl->version);
		return false;
	}
	if (!tl->max_size) {
		ERROR("Bad transfer list max size %#"PRIx32"\n",
		      tl->max_size);
		return false;;
	}
	if (tl->size > tl->max_size) {
		ERROR("Bad transfer list size %#"PRIx32"\n", tl->size);
		return false;;
	}
	return true;
}

// 2.4.1 Validating a TL header
struct transfer_list_header *transfer_list_check_header(struct transfer_list_header *tl)
{
	if (!tl || !check_header(tl))
		return NULL;
	if (!transfer_list_verify_checksum(tl)) {
		ERROR("Bad transfer list checksum %#x\n", tl->checksum);
		return NULL;
	}
	return tl;
}

static struct transfer_list_entry *transfer_list_next(struct transfer_list_header *tl,
					  struct transfer_list_entry *last)
{
	if (!tl)
		return NULL;

	struct transfer_list_entry *te = NULL;
	uintptr_t tl_ev = (uintptr_t)tl + tl->size;
	uintptr_t va = 0;
	uintptr_t ev = 0;
	size_t sz = 0;

	if (last) {
		va = (uintptr_t)last;
		if (ADD_OVERFLOW(last->hdr_size, last->data_size, &sz) ||
		    ADD_OVERFLOW(va, sz, &va) ||
		    ROUNDUP_OVERFLOW(va, (1 << tl->alignment), &va))
			return NULL;
	} else {
		va = (uintptr_t)(tl + 1);
	}
	te = (struct transfer_list_entry *)va;

	if (va + sizeof(*te) > tl_ev || te->hdr_size < sizeof(*te) ||
	    ADD_OVERFLOW(va, te->hdr_size, &ev) ||
	    ADD_OVERFLOW(ev, te->data_size, &ev) ||
	    ev > tl_ev)
		return NULL;

	return te;
}

static uint8_t calc_checksum(struct transfer_list_header *tl)
{
	if (!tl)
		return 0;

	uint8_t *b = (uint8_t *)tl;
	uint8_t cs = 0;
	size_t n = 0;

	for (n = 0; n < tl->size; n++)
		cs += b[n];

	return cs;
}

void transfer_list_update_checksum(struct transfer_list_header *tl)
{
	if (!tl)
		return;

	uint8_t cs = calc_checksum(tl);

	cs -= tl->checksum;
	cs = 256 - cs;
	tl->checksum = cs;
	assert((cs = calc_checksum(tl)) == 0 &&
		transfer_list_verify_checksum(tl));
}

bool transfer_list_verify_checksum(struct transfer_list_header *tl)
{
	return !calc_checksum(tl);
}

// Set data size of a TE entry
bool transfer_list_set_data_size(struct transfer_list_header *tl,
				 struct transfer_list_entry *te,
				 uint32_t new_data_size)
{
	if (!tl || !te)
		return false;

	uintptr_t tl_max_ev = (uintptr_t)tl + tl->max_size;
	uintptr_t tl_old_ev = (uintptr_t)tl + tl->size;
	uintptr_t new_ev = (uintptr_t)te;
	uintptr_t old_ev = (uintptr_t)te;

	if (ADD_OVERFLOW(old_ev, te->hdr_size, &old_ev) ||
	    ADD_OVERFLOW(old_ev, te->data_size, &old_ev) ||
	    ROUNDUP_OVERFLOW(old_ev, (1 << tl->alignment), &old_ev) ||
	    ADD_OVERFLOW(new_ev, te->hdr_size, &new_ev) ||
	    ADD_OVERFLOW(new_ev, new_data_size, &new_ev) ||
	    ROUNDUP_OVERFLOW(new_ev, (1 << tl->alignment), &new_ev))
		return false;

	if (new_ev > tl_max_ev)
		return false;

	te->data_size = new_data_size;

	if (new_ev > old_ev)
		tl->size += new_ev - old_ev;
	else
		tl->size -= old_ev - new_ev;

	if (new_ev != old_ev)
		memmove((void *)new_ev, (void *)old_ev, tl_old_ev - old_ev);

	transfer_list_update_checksum(tl);
	return true;
}

bool transfer_list_grow_to_max_data_size(struct transfer_list_header *tl,
					 struct transfer_list_entry *te)
{
	if (!tl || !te)
		return false;

	uint32_t sz = tl->max_size - tl->size +
		      round_up(te->data_size, (1 << tl->alignment));

	return transfer_list_set_data_size(tl, te, sz);
}

// Removing a TE
bool transfer_list_rem(struct transfer_list_header *tl, struct transfer_list_entry *te)
{
	if (!tl || !te)
		return false;

	uintptr_t tl_ev = (uintptr_t)tl + tl->size;
	uintptr_t ev = (uintptr_t)te;

	if (ADD_OVERFLOW(ev, te->hdr_size, &ev) ||
	    ADD_OVERFLOW(ev, te->data_size, &ev) ||
	    ROUNDUP_OVERFLOW(ev, (1 << tl->alignment), &ev) || ev > tl_ev)
		panic();

	memmove(te, (void *)ev, tl_ev - ev);
	tl->size -= ev - (uintptr_t)te;
	transfer_list_update_checksum(tl);
	return true;
}

// 2.4.3 Adding a new TE entry
struct transfer_list_entry *transfer_list_add(struct transfer_list_header *tl,
					 uint8_t tag_id, uint32_t data_size,
					 const void *data)
{
	if (!tl)
		return NULL;

	uintptr_t max_tl_ev = (uintptr_t)tl + tl->max_size;
	uintptr_t tl_ev = (uintptr_t)tl + tl->size;
	struct transfer_list_entry *te = NULL;
	uintptr_t ev = tl_ev;
	uint8_t *te_data = NULL;

	// check if the data size is less than the max data size
	if (data_size > tl->max_size - tl->size + 
		round_up(data_size, (1 << tl->alignment)))
		return NULL;

	// skip the step 1 (optional step)
	// new TE will be added into the tail

	if (ADD_OVERFLOW(ev, sizeof(*te), &ev) ||
	    ADD_OVERFLOW(ev, data_size, &ev) ||
	    ROUNDUP_OVERFLOW(ev, (1 << tl->alignment), &ev) || ev > max_tl_ev)
		return NULL;

	te = (struct transfer_list_entry *)tl_ev;
	memset(te, 0, ev - tl_ev);
	te->tag_id = tag_id;
	te->hdr_size = sizeof(*te);
	te->data_size = data_size;
	tl->size += ev - tl_ev;

	// get the data pointer of the TE
	te_data = transfer_list_data(te);

	// fill 0 to the TE data if input data is NULL
	if (!data)
		memset((void *)te_data, 0, data_size);
	else
		memmove((void *)te_data, data, data_size);
	transfer_list_update_checksum(tl);

	return te;
}

// 2.4.4 Adding a new TE with special data alignment requirement
struct transfer_list_entry *transfer_list_add_with_align(struct transfer_list_header *tl,
					 uint8_t tag_id, uint32_t data_size, const void *data,
					 uint8_t alignment)
{
	if (!tl)
		return NULL;
	uintptr_t mask = (1 << alignment) - 1;
	struct transfer_list_entry *te = NULL;

	// TE start address is not aligned to the new alignment
	// fill the gap with an empty TE as a placeholder before
	// adding the desire TE
	if (((uintptr_t)tl + tl->size) & mask)
		if (!transfer_list_add(tl, TL_TAG_EMPTY,
				(1 << alignment) - (((uintptr_t)tl + tl->size) & mask) - 0x8,
				NULL))
			return NULL;

	te = transfer_list_add(tl, tag_id, data_size, data);
	if (alignment > tl->alignment) {
		tl->alignment = alignment;
		transfer_list_update_checksum(tl);
	}

	return te;
}

// Finding a TE
struct transfer_list_entry *transfer_list_find(struct transfer_list_header *tl,
					  uint8_t tag_id)
{
	struct transfer_list_entry *te = NULL;

	if (!tl)
		return NULL;

	do
		te = transfer_list_next(tl, te);
	while (te && te->tag_id != tag_id);

	return te;
}
