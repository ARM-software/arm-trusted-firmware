/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Minimal and standalone JSON token scanner for manifests.
 * See json_tokens.h for the contract and manifest for the
 * schema it is designed to handle.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "json_tokens.h"

void json_scan_init(struct json_scan *s, const char *src, size_t src_len)
{
	s->src = src;
	s->src_len = src_len;
	s->cursor = 0U;
	s->strbuf[0] = '\0';
}

static int json_peek(const struct json_scan *s)
{
	if (s->cursor >= s->src_len) {
		return -1;
	}
	return (int)(unsigned char)s->src[s->cursor];
}

static void json_skip_ws(struct json_scan *s)
{
	while (s->cursor < s->src_len) {
		char c = s->src[s->cursor];

		if ((c == ' ') || (c == '\t') ||
		    (c == '\n') || (c == '\r')) {
			s->cursor++;
			continue;
		}
		break;
	}
}

static bool json_match_keyword(struct json_scan *s, const char *kw)
{
	size_t n = strlen(kw);

	if ((s->cursor + n) > s->src_len) {
		return false;
	}
	if (memcmp(s->src + s->cursor, kw, n) != 0) {
		return false;
	}
	s->cursor += n;
	return true;
}

/*
 * Parse a JSON string literal starting at the opening quote.
 * Decoded payload is placed in s->strbuf and the cursor is
 * advanced past the closing quote. Returns true on success.
 */
static bool json_scan_string(struct json_scan *s, struct json_tok *out)
{
	size_t w = 0U;
	bool   esc = false;

	if (json_peek(s) != '"') {
		return false;
	}
	s->cursor++;

	while (s->cursor < s->src_len) {
		char c = s->src[s->cursor++];

		if (esc) {
			char emit;

			switch (c) {
			case '"':
				emit = '"';
				break;
			case '\\':
				emit = '\\';
				break;
			case '/':
				emit = '/';
				break;
			case 'n':
				emit = '\n';
				break;
			case 'r':
				emit = '\r';
				break;
			case 't':
				emit = '\t';
				break;
			default:
				return false;
			}
			if (w >= (sizeof(s->strbuf) - 1U)) {
				return false;
			}
			s->strbuf[w++] = emit;
			esc = false;
			continue;
		}
		if (c == '\\') {
			esc = true;
			continue;
		}
		if (c == '"') {
			s->strbuf[w] = '\0';
			out->kind = JSON_TOK_STRING;
			out->text = s->strbuf;
			out->len  = w;
			return true;
		}
		if (w >= (sizeof(s->strbuf) - 1U)) {
			return false;
		}
		s->strbuf[w++] = c;
	}
	return false;
}

/*
 * Parse a non-negative integer in either decimal or 0x-prefixed
 * hex form. The schema in manifest uses hex-as-string for
 * offsets and sizes, spi_mhz is a plain decimal number.
 */
static bool json_scan_number(struct json_scan *s, struct json_tok *out)
{
	uint64_t v = 0U;
	bool   any = false;

	if ((s->cursor + 2U) <= s->src_len &&
	    (s->src[s->cursor] == '0') &&
	    ((s->src[s->cursor + 1U] == 'x') ||
	     (s->src[s->cursor + 1U] == 'X'))) {
		s->cursor += 2U;
		while (s->cursor < s->src_len) {
			char c = s->src[s->cursor];
			uint8_t d;

			if ((c >= '0') && (c <= '9')) {
				d = (uint8_t)(c - '0');
			} else if ((c >= 'a') && (c <= 'f')) {
				d = (uint8_t)(c - 'a' + 10);
			} else if ((c >= 'A') && (c <= 'F')) {
				d = (uint8_t)(c - 'A' + 10);
			} else {
				break;
			}
			v = (v << 4) | d;
			any = true;
			s->cursor++;
		}
	} else {
		while (s->cursor < s->src_len) {
			char c = s->src[s->cursor];

			if ((c < '0') || (c > '9')) {
				break;
			}
			v = (v * 10U) + (uint64_t)(c - '0');
			any = true;
			s->cursor++;
		}
	}

	if (!any) {
		return false;
	}
	out->kind = JSON_TOK_NUMBER;
	out->num  = v;
	return true;
}

enum json_tok_kind json_tok_next(struct json_scan *s, struct json_tok *out)
{
	int c;

	out->kind = JSON_TOK_END;
	out->text = NULL;
	out->len  = 0U;
	out->num  = 0U;

	json_skip_ws(s);
	c = json_peek(s);
	if (c < 0) {
		return JSON_TOK_END;
	}

	switch (c) {
	case '{':
		s->cursor++;
		out->kind = JSON_TOK_LBRACE;
		break;
	case '}':
		s->cursor++;
		out->kind = JSON_TOK_RBRACE;
		break;
	case '[':
		s->cursor++;
		out->kind = JSON_TOK_LBRACK;
		break;
	case ']':
		s->cursor++;
		out->kind = JSON_TOK_RBRACK;
		break;
	case ':':
		s->cursor++;
		out->kind = JSON_TOK_COLON;
		break;
	case ',':
		s->cursor++;
		out->kind = JSON_TOK_COMMA;
		break;
	case '"':
		if (!json_scan_string(s, out)) {
			out->kind = JSON_TOK_ERROR;
		}
		break;
	case 't':
		out->kind = json_match_keyword(s, "true")
			? JSON_TOK_TRUE : JSON_TOK_ERROR;
		break;
	case 'f':
		out->kind = json_match_keyword(s, "false")
			? JSON_TOK_FALSE : JSON_TOK_ERROR;
		break;
	case 'n':
		out->kind = json_match_keyword(s, "null")
			? JSON_TOK_NULL : JSON_TOK_ERROR;
		break;
	default:
		if ((c >= '0') && (c <= '9')) {
			if (!json_scan_number(s, out)) {
				out->kind = JSON_TOK_ERROR;
			}
		} else {
			out->kind = JSON_TOK_ERROR;
		}
		break;
	}
	return out->kind;
}
