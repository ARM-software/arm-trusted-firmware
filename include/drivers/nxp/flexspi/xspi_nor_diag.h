/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Diagnostic helpers for FlexSPI NOR bring-up. These wrap the
 * single-purpose primitives in fspi.c (fspi_read_sr1, xspi_read_id,
 * xspi_read_sfdp) with human-readable decode + NOTICE-level prints,
 * so board-bring-up code doesn't have to repeat the same hex dumps
 * every iteration.
 *
 * All helpers panic on driver-layer errors (that's a sign the
 * FlexSPI bus is mis-configured, which is a bring-up-time fault --
 * failing loud is the right behaviour). Callers that want softer
 * error handling should use the lower-level fspi_* / xspi_*
 * primitives directly.
 */

#ifndef XSPI_NOR_DIAG_H
#define XSPI_NOR_DIAG_H

#include <stddef.h>
#include <stdint.h>

/*
 * Pretty-print an SR1 byte with a label. Decodes SRP0, BP4..BP0,
 * WEL, WIP and warns if BP bits are non-zero (those silently block
 * SE/PP).
 */
void xspi_print_sr1(const char *label, uint8_t sr1);

/*
 * JEDEC JEP106 manufacturer ID -> human-readable name (bank 0).
 * Returns a static string; does not allocate.
 */
const char *xspi_jedec_mfg_name(uint8_t mfg);

/*
 * Best-effort (mfg, memtype, capacity) -> human-readable part name.
 * Covers the xSPI NOR chips typically populated on NXP LS/LX boards;
 * unknown triplets return "unknown part".
 */
const char *xspi_jedec_part_name(uint8_t mfg, uint8_t type, uint8_t cap);

/*
 * Status probe: reads SR1 before and after a WREN cycle, prints the
 * decoded bits, and warns if WEL did not latch (which means the
 * chip is silently dropping state-changing commands).
 *
 * Returns 0 on success, negative error code on IP failure.
 */
int xspi_probe_status(void);

/*
 * Identity probe: reads JEDEC RDID (opcode 0x9F), prints 5 ID bytes
 * + decoded mfg + part name. Returns 0 on success or a non-zero
 * code if the chip is visibly not responding (all-0x00 / all-0xFF).
 */
int xspi_probe_rdid(void);

/*
 * Geometry probe: reads the SFDP header + first parameter table
 * (BFPT) and prints density, address-byte capability, and the four
 * erase types (size + opcode). Silent no-op if SFDP is absent.
 */
void xspi_probe_sfdp(void);

#endif /* XSPI_NOR_DIAG_H */
