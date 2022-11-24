/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ETHOSN_FIP_H
#define ETHOSN_FIP_H

#define UUID_ETHOSN_FW_KEY_CERTIFICATE					\
	{ { 0x56, 0x66, 0xd0, 0x04 }, { 0xab, 0x98 }, { 0x40, 0xaa },	\
	0x89, 0x88, { 0xb7, 0x2a, 0x3, 0xa2, 0x56, 0xe2 } }

#define UUID_ETHOSN_FW_CONTENT_CERTIFICATE				\
	{ { 0xa5, 0xc4, 0x18, 0xda }, { 0x43, 0x0f }, { 0x48, 0xb1 },	\
	0x88, 0xcd, { 0x93, 0xf6, 0x78, 0x89, 0xd9, 0xed } }

#define UUID_ETHOSN_FW							\
	{ { 0xcf, 0xd4, 0x99, 0xb5 }, { 0xa3, 0xbc }, { 0x4a, 0x7e },	\
	0x98, 0xcb, { 0x48, 0xa4, 0x1c, 0xb8, 0xda, 0xe1 } }

#define ETHOSN_FW_KEY_CERTIFICATE_DEF				\
	{ "Arm(R) Ethos(TM)-N NPU Firmware Key Certificate",	\
	  UUID_ETHOSN_FW_KEY_CERTIFICATE,			\
	  "npu-fw-key-cert" }

#define ETHOSN_FW_CONTENT_CERTIFICATE_DEF			\
	{ "Arm(R) Ethos(TM)-N NPU Firmware Content Certificate",\
	  UUID_ETHOSN_FW_CONTENT_CERTIFICATE,			\
	  "npu-fw-cert" }

#define ETHOSN_FW_DEF						\
	{ "Arm(R) Ethos(TM)-N NPU Firmware",			\
	  UUID_ETHOSN_FW,					\
	  "npu-fw" }

#endif /* ETHOSN_FIP_H */
