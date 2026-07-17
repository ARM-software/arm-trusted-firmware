/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * JSON flash-manifest parser.
 *
 * Recursive-descent over the strictly-shaped schema:
 *   { "spi_mhz": N, "regions": [ region, region, ... ] }
 *
 *   region: {
 *     "file": str,
 *     "offset": hex-str,
 *     "size_max": hex-str,
 *     "erase": bool,
 *     "verify_erase": bool,
 *     "sha256": hex-str,
 *     "label": str
 *   }
 *
 * Unknown fields are tolerated and skipped (forward compat).
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/semihosting.h>
#include <lib/utils.h>

#include "json_tokens.h"
#include "manifest.h"
#include "manifest_parse.h"

/*
 * Static manifest store. One per BL2 instance. Bumping the raw
 * buffer cap costs OCRAM; the current manifest fits in a few hundred
 * bytes.
 *
 * Avoid malloc().
 */
#define NBXV3_MANIFEST_RAW_MAX	4096U
static char			s_manifest_raw[NBXV3_MANIFEST_RAW_MAX];
static struct nbxv3_manifest	s_manifest;
static bool			s_manifest_loaded;

const struct nbxv3_manifest *nbxv3_manifest_get(void)
{
	return s_manifest_loaded ? &s_manifest : NULL;
}

static int hex_digit(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;

	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;

	return -1;
}

/*
 * Parse a 0x-prefixed hex string (or plain decimal) into a
 * 64-bit unsigned. Returns 0 on success, -EINVAL on malformed
 * input.
 */
static int parse_hex_or_dec(const char *s, size_t len, uint64_t *out)
{
	uint64_t v = 0U;
	size_t i = 0U;
	bool any = false;
	bool hex = false;

	if ((len >= 2U) && (s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) {
		hex = true;
		i = 2U;
	}

	for (; i < len; i++) {
		int d;

		if (hex) {
			d = hex_digit(s[i]);
			if (d < 0) {
				return -EINVAL;
			}
			v = (v << 4) | (uint64_t)d;
		} else {
			if ((s[i] < '0') || (s[i] > '9')) {
				return -EINVAL;
			}
			v = (v * 10U) + (uint64_t)(s[i] - '0');
		}
		any = true;
	}
	if (!any) {
		return -EINVAL;
	}
	*out = v;

	return 0;
}

static int parse_sha256_hex(const char *s, size_t len, uint8_t *out)
{
	size_t i;

	if (len != (NBXV3_MANIFEST_SHA256_BYTES * 2U)) {
		return -EINVAL;
	}
	for (i = 0U; i < NBXV3_MANIFEST_SHA256_BYTES; i++) {
		int hi = hex_digit(s[i * 2U]);
		int lo = hex_digit(s[i * 2U + 1U]);

		if ((hi < 0) || (lo < 0)) {
			return -EINVAL;
		}
		out[i] = (uint8_t)((hi << 4) | lo);
	}

	return 0;
}

/*
 * Skip the JSON value at the current cursor. Used when the
 * parser hits an unrecognised field name (forward compat).
 */
static int skip_value(struct json_scan *s)
{
	struct json_tok t;
	enum json_tok_kind k = json_tok_next(s, &t);
	int depth = 0;

	switch (k) {
	case JSON_TOK_STRING:
	case JSON_TOK_NUMBER:
	case JSON_TOK_TRUE:
	case JSON_TOK_FALSE:
	case JSON_TOK_NULL:
		return 0;
	case JSON_TOK_LBRACE:
	case JSON_TOK_LBRACK:
		depth = 1;
		break;
	default:
		return -EINVAL;
	}

	while (depth > 0) {
		k = json_tok_next(s, &t);
		switch (k) {
		case JSON_TOK_LBRACE:
		case JSON_TOK_LBRACK:
			depth++;
			break;
		case JSON_TOK_RBRACE:
		case JSON_TOK_RBRACK:
			depth--;
			break;
		case JSON_TOK_END:
		case JSON_TOK_ERROR:
			return -EINVAL;
		default:
			break;
		}
	}

	return 0;
}

static int expect_kind(struct json_scan *s, struct json_tok *t,
		       enum json_tok_kind want, const char *ctx)
{
	enum json_tok_kind got = json_tok_next(s, t);

	if (got != want) {
		ERROR("manifest: expected %s, got token kind %d (cursor %zu)\n",
		      ctx, (int)got, s->cursor);
		return -EINVAL;
	}

	return 0;
}

static int parse_bool(struct json_scan *s, bool *out)
{
	struct json_tok t;
	enum json_tok_kind k = json_tok_next(s, &t);

	if (k == JSON_TOK_TRUE) {
		*out = true;
		return 0;
	}
	if (k == JSON_TOK_FALSE) {
		*out = false;
		return 0;
	}
	ERROR("manifest: expected boolean, got kind %d\n", (int)k);

	return -EINVAL;
}

static int parse_string_into(struct json_scan *s, char *dst, size_t dst_sz)
{
	struct json_tok t;

	if (expect_kind(s, &t, JSON_TOK_STRING, "string") != 0) {
		return -EINVAL;
	}
	if ((t.len + 1U) > dst_sz) {
		ERROR("manifest: string too long (%zu, max %zu)\n",
		      t.len, dst_sz - 1U);
		return -ENOSPC;
	}
	memcpy(dst, t.text, t.len);
	dst[t.len] = '\0';

	return 0;
}

static int parse_hex_string_u64(struct json_scan *s, uint64_t *out)
{
	struct json_tok t;

	if (expect_kind(s, &t, JSON_TOK_STRING, "hex string") != 0) {
		return -EINVAL;
	}
	if (parse_hex_or_dec(t.text, t.len, out) != 0) {
		ERROR("manifest: malformed hex value '%s'\n", t.text);
		return -EINVAL;
	}

	return 0;
}

static int parse_uint(struct json_scan *s, uint64_t *out)
{
	struct json_tok t;
	enum json_tok_kind k = json_tok_next(s, &t);

	if (k == JSON_TOK_NUMBER) {
		*out = t.num;
		return 0;
	}
	if (k == JSON_TOK_STRING) {
		if (parse_hex_or_dec(t.text, t.len, out) == 0) {
			return 0;
		}
	}
	ERROR("manifest: expected unsigned number, got kind %d\n", (int)k);

	return -EINVAL;
}

static int parse_region(struct json_scan *s, struct nbxv3_region *r)
{
	struct json_tok t;
	uint64_t v;
	int ret;

	if (expect_kind(s, &t, JSON_TOK_LBRACE, "{") != 0) {
		return -EINVAL;
	}

	for (;;) {
		enum json_tok_kind k = json_tok_next(s, &t);

		if (k == JSON_TOK_RBRACE) {
			break;
		}
		if (k == JSON_TOK_COMMA) {
			continue;
		}
		if (k != JSON_TOK_STRING) {
			ERROR("manifest: expected field name, got kind %d\n",
			      (int)k);
			return -EINVAL;
		}

		/*
		 * The field name lives in the scanner's strbuf and
		 * will be overwritten by the next json_tok_next()
		 * call, so capture it before consuming the value.
		 */
		char name[24];
		size_t name_len = (t.len < (sizeof(name) - 1U))
			? t.len : (sizeof(name) - 1U);

		memcpy(name, t.text, name_len);
		name[name_len] = '\0';

		if (expect_kind(s, &t, JSON_TOK_COLON, ":") != 0) {
			return -EINVAL;
		}

		if (strcmp(name, "file") == 0) {
			ret = parse_string_into(s, r->file, sizeof(r->file));
		} else if (strcmp(name, "label") == 0) {
			ret = parse_string_into(s, r->label, sizeof(r->label));
		} else if (strcmp(name, "offset") == 0) {
			ret = parse_hex_string_u64(s, &v);
			if (ret == 0) {
				r->offset = (uint32_t)v;
			}
		} else if (strcmp(name, "size_max") == 0) {
			ret = parse_hex_string_u64(s, &v);
			if (ret == 0) {
				r->size_max = (uint32_t)v;
			}
		} else if (strcmp(name, "erase") == 0) {
			ret = parse_bool(s, &r->erase);
		} else if (strcmp(name, "verify_erase") == 0) {
			ret = parse_bool(s, &r->verify_erase);
		} else if (strcmp(name, "sha256") == 0) {
			struct json_tok st;

			if (expect_kind(s, &st, JSON_TOK_STRING,
					"sha256 hex") != 0) {
				return -EINVAL;
			}
			ret = parse_sha256_hex(st.text, st.len, r->sha256);
			if (ret == 0) {
				r->has_sha256 = true;
			}
		} else {
			NOTICE("manifest: ignoring unknown region field '%s'\n",
			       name);
			ret = skip_value(s);
		}

		if (ret != 0) {
			ERROR("manifest: bad value for field '%s'\n", name);
			return ret;
		}
	}

	if (r->file[0] == '\0') {
		ERROR("manifest: region missing required 'file'\n");
		return -EINVAL;
	}
	if (r->size_max == 0U) {
		ERROR("manifest: region '%s' missing required 'size_max'\n",
		      r->file);
		return -EINVAL;
	}

	return 0;
}

static int parse_regions_array(struct json_scan *s, struct nbxv3_manifest *m)
{
	struct json_tok t;
	enum json_tok_kind k;

	if (expect_kind(s, &t, JSON_TOK_LBRACK, "[") != 0) {
		return -EINVAL;
	}

	for (;;) {
		k = json_tok_next(s, &t);

		if (k == JSON_TOK_RBRACK)
			break;

		if (k == JSON_TOK_COMMA)
			continue;

		if (k != JSON_TOK_LBRACE) {
			ERROR("manifest: expected region object, got kind %d\n",
			      (int)k);
			return -EINVAL;
		}
		/*
		 * We just consumed the opening brace; rewind so
		 * parse_region() can re-consume it via expect_kind().
		 */
		s->cursor--;

		if (m->n_regions >= NBXV3_MANIFEST_MAX_REGIONS) {
			ERROR("manifest: too many regions (max %u)\n",
			      NBXV3_MANIFEST_MAX_REGIONS);
			return -ENOSPC;
		}

		if (parse_region(s, &m->regions[m->n_regions]) != 0)
			return -EINVAL;

		m->n_regions++;
	}

	if (m->n_regions == 0U) {
		ERROR("manifest: regions array is empty\n");
		return -EINVAL;
	}

	return 0;
}

static int parse_top(struct json_scan *s, struct nbxv3_manifest *m)
{
	struct json_tok t;
	uint64_t v;

	if (expect_kind(s, &t, JSON_TOK_LBRACE, "{") != 0)
		return -EINVAL;

	for (;;) {
		enum json_tok_kind k = json_tok_next(s, &t);
		char name[16];
		size_t name_len;
		int ret;

		if (k == JSON_TOK_RBRACE)
			break;

		if (k == JSON_TOK_COMMA)
			continue;

		if (k != JSON_TOK_STRING) {
			ERROR("manifest: expected top-level field, got kind %d\n",
			      (int)k);
			return -EINVAL;
		}

		name_len = (t.len < (sizeof(name) - 1U))
			? t.len : (sizeof(name) - 1U);
		memcpy(name, t.text, name_len);
		name[name_len] = '\0';

		if (expect_kind(s, &t, JSON_TOK_COLON, ":") != 0)
			return -EINVAL;

		if (strcmp(name, "spi_mhz") == 0) {
			ret = parse_uint(s, &v);
			if (ret == 0) {
				m->spi_mhz = (uint32_t)v;
			}
		} else if (strcmp(name, "regions") == 0) {
			ret = parse_regions_array(s, m);
		} else {
			NOTICE("manifest: ignoring unknown top-level field '%s'\n",
			       name);
			ret = skip_value(s);
		}

		if (ret != 0)
			return ret;
	}

	return 0;
}

int nbxv3_manifest_load(const char *path)
{
	long fh;
	long flen;
	size_t got = 0U;
	struct json_scan scan;

	if (path == NULL)
		return -EINVAL;

	zeromem(&s_manifest, sizeof(s_manifest));
	s_manifest_loaded = false;

	fh = semihosting_file_open(path, FOPEN_MODE_RB);
	if (fh == -1L) {
		NOTICE("manifest: %s not found in semihost basedir\n", path);
		return -ENOENT;
	}

	flen = semihosting_file_length(fh);
	if (flen <= 0) {
		ERROR("manifest: %s has invalid length %ld\n", path, flen);
		(void)semihosting_file_close(fh);
		return -EIO;
	}
	if ((size_t)flen > sizeof(s_manifest_raw)) {
		ERROR("manifest: %s is %ld bytes, max %zu\n",
		      path, flen, sizeof(s_manifest_raw));
		(void)semihosting_file_close(fh);
		return -ENOSPC;
	}

	got = (size_t)flen;
	if (semihosting_file_read(fh, &got, (uintptr_t)s_manifest_raw) != 0) {
		ERROR("manifest: read of %s failed\n", path);
		(void)semihosting_file_close(fh);
		return -EIO;
	}
	(void)semihosting_file_close(fh);

	if (got != (size_t)flen) {
		ERROR("manifest: short read of %s (%zu of %ld)\n",
		      path, got, flen);
		return -EIO;
	}

	json_scan_init(&scan, s_manifest_raw, got);
	if (parse_top(&scan, &s_manifest) != 0) {
		ERROR("manifest: parse failed\n");
		zeromem(&s_manifest, sizeof(s_manifest));
		return -EINVAL;
	}

	s_manifest_loaded = true;
	NOTICE("manifest: %s parsed, %u region(s), spi_mhz=%u\n",
	       path, s_manifest.n_regions, s_manifest.spi_mhz);

	return 0;
}
