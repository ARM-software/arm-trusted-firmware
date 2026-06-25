/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Standalone SHA-256 (FIPS 180-4). It is a transcription of the
 * spec with any platform dependencies beyond stdint.h and string.h.
 * It is used by the manifest runner to hash the freshly-written NOR
 * It can catch any silent JTAG bit-flips end-to-end, transfert, flash
 * issues.
 *
 * I do not import any external libraries since sha256 c code is
 * quite standalone, let's avoid adding makefile complexities.
 */

#include <stdint.h>
#include <string.h>

#include "sha256.h"

static const uint32_t SHA256_K[64] = {
	0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
	0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
	0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
	0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
	0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
	0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
	0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
	0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
	0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
	0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
	0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
	0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
	0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
	0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
	0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
	0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
};

static inline uint32_t rotr(uint32_t x, unsigned int n)
{
	return (x >> n) | (x << (32U - n));
}

static void sha256_block(struct nbxv3_sha256 *ctx, const uint8_t *blk)
{
	uint32_t W[64];
	uint32_t a, b, c, d, e, f, g, h;
	unsigned int i;

	for (i = 0U; i < 16U; i++) {
		W[i] = ((uint32_t)blk[i * 4U] << 24) |
		       ((uint32_t)blk[i * 4U + 1U] << 16) |
		       ((uint32_t)blk[i * 4U + 2U] << 8) |
		       ((uint32_t)blk[i * 4U + 3U]);
	}
	for (i = 16U; i < 64U; i++) {
		uint32_t s0 = rotr(W[i - 15U], 7) ^ rotr(W[i - 15U], 18) ^
			      (W[i - 15U] >> 3);
		uint32_t s1 = rotr(W[i - 2U], 17) ^ rotr(W[i - 2U], 19) ^
			      (W[i - 2U] >> 10);

		W[i] = W[i - 16U] + s0 + W[i - 7U] + s1;
	}

	a = ctx->state[0]; b = ctx->state[1];
	c = ctx->state[2]; d = ctx->state[3];
	e = ctx->state[4]; f = ctx->state[5];
	g = ctx->state[6]; h = ctx->state[7];

	for (i = 0U; i < 64U; i++) {
		uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
		uint32_t ch = (e & f) ^ (~e & g);
		uint32_t t1 = h + S1 + ch + SHA256_K[i] + W[i];
		uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
		uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
		uint32_t t2 = S0 + mj;

		h = g; g = f; f = e;
		e = d + t1;
		d = c; c = b; b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a; ctx->state[1] += b;
	ctx->state[2] += c; ctx->state[3] += d;
	ctx->state[4] += e; ctx->state[5] += f;
	ctx->state[6] += g; ctx->state[7] += h;
}

void nbxv3_sha256_init(struct nbxv3_sha256 *ctx)
{
	ctx->state[0] = 0x6a09e667U;
	ctx->state[1] = 0xbb67ae85U;
	ctx->state[2] = 0x3c6ef372U;
	ctx->state[3] = 0xa54ff53aU;
	ctx->state[4] = 0x510e527fU;
	ctx->state[5] = 0x9b05688cU;
	ctx->state[6] = 0x1f83d9abU;
	ctx->state[7] = 0x5be0cd19U;
	ctx->bit_count = 0U;
	ctx->buffer_len = 0U;
}

void nbxv3_sha256_update(struct nbxv3_sha256 *ctx,
			 const uint8_t *data, size_t len)
{
	ctx->bit_count += (uint64_t)len * 8U;

	if (ctx->buffer_len > 0U) {
		size_t want = 64U - ctx->buffer_len;
		size_t take = (len < want) ? len : want;

		memcpy(ctx->buffer + ctx->buffer_len, data, take);
		ctx->buffer_len += take;
		data += take;
		len -= take;
		if (ctx->buffer_len == 64U) {
			sha256_block(ctx, ctx->buffer);
			ctx->buffer_len = 0U;
		}
	}
	while (len >= 64U) {
		sha256_block(ctx, data);
		data += 64U;
		len -= 64U;
	}
	if (len > 0U) {
		memcpy(ctx->buffer, data, len);
		ctx->buffer_len = len;
	}
}

void nbxv3_sha256_final(struct nbxv3_sha256 *ctx,
			uint8_t out[NBXV3_SHA256_BYTES])
{
	uint64_t bits = ctx->bit_count;
	uint8_t pad = 0x80U;
	unsigned int i;

	nbxv3_sha256_update(ctx, &pad, 1U);

	pad = 0x00U;
	while (ctx->buffer_len != 56U) {
		nbxv3_sha256_update(ctx, &pad, 1U);
	}

	{
		uint8_t lenbe[8];

		for (i = 0U; i < 8U; i++) {
			lenbe[i] = (uint8_t)(bits >> ((7U - i) * 8U));
		}
		nbxv3_sha256_update(ctx, lenbe, 8U);
	}

	for (i = 0U; i < 8U; i++) {
		out[i * 4U]      = (uint8_t)(ctx->state[i] >> 24);
		out[i * 4U + 1U] = (uint8_t)(ctx->state[i] >> 16);
		out[i * 4U + 2U] = (uint8_t)(ctx->state[i] >> 8);
		out[i * 4U + 3U] = (uint8_t)(ctx->state[i]);
	}
}
