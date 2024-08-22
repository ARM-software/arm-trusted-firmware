/*
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_SHA_H
#define SOCFPGA_SHA_H

#include <stdlib.h>


#define SHA384_SUM_LEN			48
#define SHA384_DER_LEN			19
#define SHA512_SUM_LEN			64
#define SHA512_DER_LEN			19
#define SHA512_BLOCK_SIZE		128


/* MACRO Function */
#define GET_UINT64_BE(n, b, i) {	\
	(n) = ((unsigned long long) (b)[(i)] << 56) |\
	((unsigned long long) (b)[(i) + 1] << 48) |\
	((unsigned long long) (b)[(i) + 2] << 40) |\
	((unsigned long long) (b)[(i) + 3] << 32) |\
	((unsigned long long) (b)[(i) + 4] << 24) |\
	((unsigned long long) (b)[(i) + 5] << 16) |\
	((unsigned long long) (b)[(i) + 6] <<  8) |\
	((unsigned long long) (b)[(i) + 7]);\
}

#define PUT_UINT64_BE(n, b, i) {	\
	(b)[(i)] = (unsigned char) ((n) >> 56);\
	(b)[(i) + 1] = (unsigned char) ((n) >> 48);\
	(b)[(i) + 2] = (unsigned char) ((n) >> 40);\
	(b)[(i) + 3] = (unsigned char) ((n) >> 32);\
	(b)[(i) + 4] = (unsigned char) ((n) >> 24);\
	(b)[(i) + 5] = (unsigned char) ((n) >> 16);\
	(b)[(i) + 6] = (unsigned char) ((n) >>  8);\
	(b)[(i) + 7] = (unsigned char) ((n));\
}

#define e0(x)		(ror64(x, 28) ^ ror64(x, 34) ^ ror64(x, 39))
#define e1(x)		(ror64(x, 14) ^ ror64(x, 18) ^ ror64(x, 41))
#define s0(x)		(ror64(x, 1) ^ ror64(x, 8) ^ (x >> 7))
#define s1(x)		(ror64(x, 19) ^ ror64(x, 61) ^ (x >> 6))

/* Inline Function Definitions */
/* ror64() to rotate its right in 64 bits. */
static inline uint64_t ror64(uint64_t input, unsigned int shift)
{
	return (input >> (shift & 63)) | (input << ((-shift) & 63));
}

static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z)
{
	return z ^ (x & (y ^ z));
}

static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z)
{
	return (x & y) | (z & (x | y));
}

static inline void LOAD_OP(int I, uint64_t *W, const uint8_t *input)
{
	GET_UINT64_BE(W[I], input, I*8);
}

static inline void BLEND_OP(int I, uint64_t *W)
{
	W[I & 15] += s1(W[(I-2) & 15]) + W[(I-7) & 15] + s0(W[(I-15) & 15]);
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
inline uint32_t le32_to_cpue(const uint32_t *p)
{
	return (uint32_t)*p;
}
#else
inline uint32_t le32_to_cpue(const uint32_t *p)
{
	return swab32(*p);
}
#endif

static const uint64_t sha512_K[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL,
	0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
	0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL,
	0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL,
	0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
	0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 0x2de92c6f592b0275ULL,
	0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL,
	0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
	0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL,
	0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL,
	0x92722c851482353bULL, 0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
	0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL,
	0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL,
	0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
	0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL,
	0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL,
	0xc67178f2e372532bULL, 0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
	0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL,
	0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL,
	0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
	0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL,
};

#define	__cpu_to_le64(x)	((__force __le64)(__u64)(x))

#define _uswap_64(x, sfx)	\
	((((x) & 0xff00000000000000##sfx) >> 56) |\
	(((x) & 0x00ff000000000000##sfx) >> 40) |\
	(((x) & 0x0000ff0000000000##sfx) >> 24) |\
	(((x) & 0x000000ff00000000##sfx) >>  8) |\
	(((x) & 0x00000000ff000000##sfx) <<  8) |\
	(((x) & 0x0000000000ff0000##sfx) << 24) |\
	(((x) & 0x000000000000ff00##sfx) << 40) |\
	(((x) & 0x00000000000000ff##sfx) << 56))

#if defined(__GNUC__)
#define uswap_64(x)		_uswap_64(x, ull)
#else
#define uswap_64(x)		_uswap_64(x)
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be64(x)		uswap_64(x)
#else
#define cpu_to_be64(x)		(x)
#endif

typedef struct {
	uint64_t state[SHA512_SUM_LEN / 8];
	uint64_t count[2];
	uint8_t buf[SHA512_BLOCK_SIZE];
} sha512_context;

/* Function Definitions */
/* SHA384 Start Here */
void sha384_init(sha512_context *ctx);
void sha384_update(sha512_context *ctx, const uint8_t *input, uint32_t length);
void sha384_finish(sha512_context *ctx, uint8_t digest[SHA384_SUM_LEN]);
void sha384_start(const unsigned char *input, unsigned int len,
			unsigned char *output, unsigned int chunk_sz);
/* SHA512 Start Here */
void sha512_init(sha512_context *ctx);
void sha512_update(sha512_context *ctx, const uint8_t *input, uint32_t length);
void sha512_finish(sha512_context *ctx, uint8_t digest[SHA512_SUM_LEN]);
void sha512_start(const unsigned char *input, unsigned int len,
			unsigned char *output);
void sha512_transform(uint64_t *state, const uint8_t *input);
void sha512_block_fn(sha512_context *sst, const uint8_t *src, int blocks);
void sha512_base_do_finalize(sha512_context *sctx);
void sha512_base_do_update(sha512_context *sctx, const uint8_t *data,
				unsigned int len);

#endif
