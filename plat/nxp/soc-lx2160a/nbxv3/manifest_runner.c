/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * JSON-manifest runner. See manifest_runner.h for the API
 * contract and MANIFEST.md for the schema.
 *
 * Per region:
 *   - open a semihost-backed source (panic on missing)
 *   - open an xspi_nor-backed destination over the slot
 *   - if requested, erase the slot and optionally verify-blank
 *   - copy chunk by chunk
 *   - if SHA-256 was supplied. It catches both source
 *      transmission errors and chip retention failures in one pass.
 *   - close both endpoints
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_semihosting.h>
#include <drivers/io/io_storage.h>
#include <drivers/nxp/io/io_xspi_nor.h>
#include <flash_info.h>
#include <lib/semihosting.h>

#include "manifest.h"
#include "manifest_runner.h"
#include <platform_def.h>
#include "progress_bar.h"
#include "sha256.h"

/*
 * Chunk buffer sized for the semihost SYS_READ amortisation
 * sweet spot. Cache-line aligned so the AHB-readback used by the
 * post-write SHA-256 pass does not straddle lines.
 */
#define NBXV3_CHUNK_SZ		(64U * 1024U)

static uint8_t nbxv3_chunk_buf[NBXV3_CHUNK_SZ] __aligned(64);

static uintptr_t s_sh_dev;
static uintptr_t s_nor_dev;
static bool      s_io_initialised;

/*
 * Register and open the two io_storage backends used by the
 * runner. Idempotent: a second call is a no-op.
 */
static int nbxv3_runner_io_setup(void)
{
	const io_dev_connector_t *sh_con = NULL;
	const io_dev_connector_t *nor_con = NULL;
	int ret;

	if (s_io_initialised)
		return 0;

	ret = register_io_dev_sh(&sh_con);
	if (ret != 0) {
		ERROR("manifest: register_io_dev_sh failed (%d)\n", ret);
		return ret;
	}
	ret = io_dev_open(sh_con, (uintptr_t)NULL, &s_sh_dev);
	if (ret != 0) {
		ERROR("manifest: io_dev_open(sh) failed (%d)\n", ret);
		return ret;
	}

	ret = register_io_dev_xspi_nor(&nor_con);
	if (ret != 0) {
		ERROR("manifest: register_io_dev_xspi_nor failed (%d)\n", ret);
		return ret;
	}
	ret = io_dev_open(nor_con, (uintptr_t)NULL, &s_nor_dev);
	if (ret != 0) {
		ERROR("manifest: io_dev_open(xspi_nor) failed (%d)\n", ret);
		return ret;
	}

	s_io_initialised = true;

	return 0;
}

/*
 * Round @len up to the driver's sector erase granularity. The
 * NOR controller can only erase whole sectors.
 */
static size_t round_up_to_sector(size_t len)
{
	size_t mask = (size_t)F_SECTOR_ERASE_SZ - 1U;

	return (len + mask) & ~mask;
}

/*
 * AHB-window view of the destination. Used by the post-write
 * SHA-256 pass to read freshly-flashed NOR contents through the
 * controller's prefetch path (faster than IP-path reads). The
 * xspi_nor backend issues an FSPI_MCR0.SWRST after every write,
 * which evicts the controller's AHB read buffer, so the view
 * here is guaranteed to reflect what was just written.
 */
static const uint8_t *flash_ahb_view(uint32_t nor_offset)
{
	return (const uint8_t *)(uintptr_t)
		(NXP_FLEXSPI_FLASH_ADDR + (uintptr_t)nor_offset);
}

static int region_chunked_copy(uintptr_t src, uintptr_t dst,
			       const struct nbxv3_region *r,
			       size_t total)
{
	struct nbxv3_pb pb;
	size_t remaining = total;
	uint32_t nor_pos = 0U;
	size_t n_chunks;

	n_chunks = (total + NBXV3_CHUNK_SZ - 1U) / NBXV3_CHUNK_SZ;
	nbxv3_pb_begin(&pb,
		       (r->label[0] != '\0') ? r->label : r->file,
		       n_chunks);

	while (remaining != 0U) {
		size_t chunk = (remaining > NBXV3_CHUNK_SZ)
			? NBXV3_CHUNK_SZ : remaining;
		size_t got = 0U;
		size_t put = 0U;

		if (io_read(src, (uintptr_t)nbxv3_chunk_buf,
			    chunk, &got) != 0 || got != chunk) {
			nbxv3_pb_step(&pb, NBXV3_PB_GIVE_UP);
			nbxv3_pb_end(&pb);
			ERROR("manifest: %s: read short / failed at NOR 0x%x\n",
			      r->file, r->offset + nor_pos);
			return -EIO;
		}
		nbxv3_pb_step(&pb, NBXV3_PB_READ);

		if (io_seek(dst, IO_SEEK_SET,
			    (signed long long)nor_pos) != 0) {
			nbxv3_pb_step(&pb, NBXV3_PB_GIVE_UP);
			nbxv3_pb_end(&pb);
			return -EIO;
		}
		if (io_write(dst, (uintptr_t)nbxv3_chunk_buf,
			     chunk, &put) != 0 || put != chunk) {
			nbxv3_pb_step(&pb, NBXV3_PB_GIVE_UP);
			nbxv3_pb_end(&pb);
			ERROR("manifest: %s: write failed at NOR 0x%x\n",
			      r->file, r->offset + nor_pos);
			return -EIO;
		}
		nbxv3_pb_step(&pb, NBXV3_PB_WRITE);

		nor_pos += (uint32_t)chunk;
		remaining -= chunk;
	}

	nbxv3_pb_end(&pb);

	/*
	 * Post-write SHA-256 pass.
	 *
	 * Hash the freshly-flashed NOR contents in one sweep and
	 * compare to the manifest's expected digest. This catches
	 * both source transmission errors and chip retention failures
	 * in one shot.
	 *
	 * The AHB-read path is used (not IP) because the controller's
	 * prefetcher gives us bursts at the chip's read rate without
	 * per-byte CPU overhead. The xspi_nor backend already issued
	 * MCR0.SWRST inside io_write (which evicts the controller's
	 * AHB read buffer), so the AHB view is coherent with the
	 * just-written contents.
	 */
	if (r->has_sha256) {
		struct nbxv3_sha256 sha;
		uint8_t computed[NBXV3_SHA256_BYTES];
		size_t off = 0U;

		nbxv3_sha256_init(&sha);
		while (off < total) {
			size_t chunk = (total - off > NBXV3_CHUNK_SZ)
				? NBXV3_CHUNK_SZ : (total - off);
			nbxv3_sha256_update(&sha,
					    flash_ahb_view(r->offset +
							   (uint32_t)off),
					    chunk);
			off += chunk;
		}
		nbxv3_sha256_final(&sha, computed);

		if (memcmp(computed, r->sha256, NBXV3_SHA256_BYTES) != 0) {
			ERROR("manifest: %s SHA-256 mismatch after write\n",
			      r->file);
			return -EIO;
		}
		NOTICE("manifest: %s SHA-256 OK (post-write NOR)\n",
		       r->file);
	}

	return 0;
}

static int region_erase_and_verify(uintptr_t dst,
				   const struct nbxv3_region *r,
				   size_t payload_size)
{
	size_t erase_len = round_up_to_sector(payload_size);

	if (!r->erase)
		return 0;

	if (io_seek(dst, IO_SEEK_SET, 0LL) != 0)
		return -EIO;

	NOTICE("manifest: %s: erasing %zu bytes at NOR 0x%x...\n",
	       r->file, erase_len, r->offset);

	if (io_erase(dst, erase_len) != 0) {
		ERROR("manifest: %s: erase of %zu bytes at NOR 0x%x failed\n",
		      r->file, erase_len, r->offset);
		return -EIO;
	}

	if (r->verify_erase) {
		const uint8_t *view = flash_ahb_view(r->offset);
		size_t i;
		struct nbxv3_pb pb;
		size_t marks = (erase_len / F_SECTOR_ERASE_SZ);

		nbxv3_pb_begin(&pb,
			       (r->label[0] != '\0') ? r->label : r->file,
			       marks);
		for (i = 0U; i < erase_len; i++) {
			if (view[i] != 0xFFU) {
				nbxv3_pb_step(&pb, NBXV3_PB_GIVE_UP);
				nbxv3_pb_end(&pb);
				ERROR("manifest: %s: erase-verify fail at NOR 0x%x: 0x%02x!=0xFF\n",
				      r->file,
				      (uint32_t)(r->offset + i),
				      view[i]);
				return -EIO;
			}
			if ((i % F_SECTOR_ERASE_SZ) ==
			    (F_SECTOR_ERASE_SZ - 1U)) {
				nbxv3_pb_step(&pb, NBXV3_PB_VERIFY_ERASE);
			}
		}
		nbxv3_pb_end(&pb);
	}

	return 0;
}

/*
 * Process one region. Returns the number of payload bytes
 * written, or a negative errno on hard failure. The caller
 * panics on any negative return.
 */
static long process_region(const struct nbxv3_region *r)
{
	uintptr_t src = 0U;
	uintptr_t dst = 0U;
	size_t flen = 0U;
	io_block_spec_t blk_spec = {
		.offset = r->offset,
		.length = r->size_max,
	};
	int ret;
	io_file_spec_t file_spec = {
		.path = r->file,
		.mode = FOPEN_MODE_RB,
	};

	if (io_open(s_sh_dev, (uintptr_t)&file_spec, &src) != 0) {
		ERROR("manifest: %s missing\n", r->file);
		return -ENOENT;
	}

	if (io_size(src, &flen) != 0 || flen == 0U) {
		ERROR("manifest: %s has invalid size\n", r->file);
		(void)io_close(src);
		return -EIO;
	}
	if (flen > r->size_max) {
		ERROR("manifest: %s is %zu bytes, slot max %u\n",
		      r->file, flen, r->size_max);
		(void)io_close(src);
		return -ENOSPC;
	}

	NOTICE("manifest: %s -> NOR 0x%x, %zu bytes\n",
	       r->file, r->offset, flen);

	if (io_open(s_nor_dev, (uintptr_t)&blk_spec, &dst) != 0) {
		ERROR("manifest: open xspi_nor at NOR 0x%x failed\n",
		      r->offset);
		if (src != 0U) {
			(void)io_close(src);
		}
		return -EIO;
	}

	ret = region_erase_and_verify(dst, r, flen);
	if (ret != 0) {
		(void)io_close(dst);
		if (src != 0U) {
			(void)io_close(src);
		}
		return ret;
	}

	if (io_seek(dst, IO_SEEK_SET, 0LL) != 0) {
		(void)io_close(dst);
		if (src != 0U) {
			(void)io_close(src);
		}
		return -EIO;
	}
	ret = region_chunked_copy(src, dst, r, flen);

	(void)io_close(dst);

	if (src != 0U)
		(void)io_close(src);

	if (ret != 0)
		return ret;

	return (long)flen;
}

long nbxv3_manifest_run(const struct nbxv3_manifest *m)
{
	long total = 0;
	uint32_t i;
	int ret;

	if (m == NULL)
		return -EINVAL;

	ret = nbxv3_runner_io_setup();
	if (ret != 0)
		return ret;

	for (i = 0U; i < m->n_regions; i++) {
		long n = process_region(&m->regions[i]);

		if (n < 0) {
			panic();
		}
		total += n;
	}

	return total;
}
