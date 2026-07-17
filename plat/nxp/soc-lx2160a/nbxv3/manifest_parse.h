/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Manifest parser entry points. Reads the JSON manifest from
 * the host (via the io_storage semihost backend) and populates
 * the static manifest store. The schema is documented in
 * MANIFEST.md.
 *
 * Errors are reported via NOTICE / ERROR with enough context
 * (token kind, byte offset) for the operator to fix the JSON
 * source, then a non-zero return; on a clean parse the manifest
 * is available via nbxv3_manifest_get().
 */

#ifndef NBXV3_MANIFEST_PARSE_H
#define NBXV3_MANIFEST_PARSE_H

#include "manifest.h"

/*
 * Read @path from the semihost basedir, parse it, and stash the
 * result in the static manifest store. Returns 0 on success, a
 * negative errno otherwise (-ENOENT if the file is missing,
 * -EINVAL on malformed JSON, -ENOSPC if the manifest exceeds the
 * inline caps).
 *
 * The path is interpreted by the host's semihosting layer, so
 * exact resolution rules are host-side: typically the path is
 * looked up under the OpenOCD `arm semihosting_basedir`.
 */
int nbxv3_manifest_load(const char *path);

/*
 * Accessor. Returns NULL if no manifest has been parsed yet
 * (caller forgot to call nbxv3_manifest_load() or it failed).
 */
const struct nbxv3_manifest *nbxv3_manifest_get(void);

#endif /* NBXV3_MANIFEST_PARSE_H */
