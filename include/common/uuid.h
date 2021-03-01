/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UUID_H
#define UUID_H

#define UUID_BYTES_LENGTH	16
#define UUID_STRING_LENGTH	36

int read_uuid(uint8_t *dest, char *uuid);

#endif /* UUID_H */
