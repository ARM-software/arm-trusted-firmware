/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UUID_COMMON_H
#define UUID_COMMON_H

#define UUID_BYTES_LENGTH	16
#define UUID_STRING_LENGTH	36

int read_uuid(uint8_t *dest, char *uuid);
bool uuid_match(uint32_t *uuid1, uint32_t *uuid2);
void copy_uuid(uint32_t *to_uuid, uint32_t *from_uuid);
bool is_null_uuid(uint32_t *uuid);

#endif /* UUID_COMMON_H */
