/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Minimal JSON token scanner. It avoids pulling a
 * third-party library into BL2: the manifest format
 * only needs strict JSON with strings,
 * integers, booleans, objects, and arrays. No floats, no
 * Unicode escapes beyond the standard backslash set
 * (quote, backslash, slash, n, r, t).
 *
 * The scanner does NOT mutate the source buffer: token text
 * pointers reference into the original buffer, lengths report
 * the byte run, and callers copy out only what they keep.
 *
 * This module knows nothing about the manifest schema; it
 * exposes only "what is the next token". manifest_parse.c is
 * responsible for turning a stream of tokens into a region
 * array.
 */

#ifndef NBXV3_JSON_TOKENS_H
#define NBXV3_JSON_TOKENS_H

#include <stddef.h>
#include <stdint.h>

enum json_tok_kind {
	JSON_TOK_END = 0,	/* end of input, no more tokens */
	JSON_TOK_LBRACE,	/* { */
	JSON_TOK_RBRACE,	/* } */
	JSON_TOK_LBRACK,	/* [ */
	JSON_TOK_RBRACK,	/* ] */
	JSON_TOK_COLON,		/* : */
	JSON_TOK_COMMA,		/* , */
	/* "..." (quotes stripped, escapes resolved into a private buffer) */
	JSON_TOK_STRING,
	JSON_TOK_NUMBER,	/* integer literal (negative not supported here) */
	JSON_TOK_TRUE,		/* literal true */
	JSON_TOK_FALSE,		/* literal false */
	JSON_TOK_NULL,		/* literal null */
	JSON_TOK_ERROR,		/* malformed input at the current position */
};

/*
 * Token returned by json_tok_next().
 *
 *  STRING  -  @text points at the unescaped payload (NUL-terminated
 *             inside the scanner's small private buffer; valid until
 *             the next json_tok_next() call), @len is its byte length.
 *  NUMBER  -  @num holds the parsed unsigned 64-bit value.
 *  others  -  @text / @len / @num undefined.
 */
struct json_tok {
	enum json_tok_kind kind;
	const char	*text;
	size_t		 len;
	uint64_t	 num;
};

/*
 * Scanner state. Caller allocates one of these on the stack and
 * passes it to every json_tok_next() call. No heap allocation.
 */
struct json_scan {
	const char	*src;		/* original buffer */
	size_t		 src_len;
	size_t		 cursor;	/* byte index inside @src */
	/*
	 * Decoded-string scratch. Sized for the longest field value
	 * the manifest schema admits (file path, hex offset, label,
	 * sha256). Bumped if a future schema needs longer.
	 */
	char		 strbuf[128];
};

/*
 * Initialise a scanner over a buffer. The buffer is borrowed,
 * not copied, and must outlive the scanner.
 */
void json_scan_init(struct json_scan *s, const char *src, size_t src_len);

/*
 * Pull the next token. Returns the token kind for convenience
 * (also written to @out->kind). On JSON_TOK_ERROR the cursor is
 * left at the offending position; callers should stop parsing.
 */
enum json_tok_kind json_tok_next(struct json_scan *s, struct json_tok *out);

#endif /* NBXV3_JSON_TOKENS_H */
