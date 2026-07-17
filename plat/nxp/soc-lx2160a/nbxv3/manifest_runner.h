/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Drives the JSON-manifest-described provisioning of the on-board
 * NOR. The manifest itself is parsed by manifest_parse.c
 * It walks the resulting region array and performs the actual
 * flash transactions through the io_storage abstraction.
 *
 * - Source endpoint: ARM semihosting (the JTAG probe serves files
 *   from `arm semihosting_basedir`).
 * - Destination endpoint: io_xspi_nor (the on-board FlexSPI NOR).
 *
 * The runner does not know about hard-coded file lists, nor about
 * environment-variable knobs. Everything information is driven by
 * the manifest fields.
 */

#ifndef NBXV3_MANIFEST_RUNNER_H
#define NBXV3_MANIFEST_RUNNER_H

#include "manifest.h"

/*
 * Walk @m and flash every region. Returns the total number of
 * payload bytes written across all regions on success, or a
 * negative errno on a setup failure (IO device registration,
 * etc.).
 * Per-region failures (write or post-write SHA-256
 * mismatch) panic unconditionally.
 */
long nbxv3_manifest_run(const struct nbxv3_manifest *m);

#endif /* NBXV3_MANIFEST_RUNNER_H */
