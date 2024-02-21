/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * Packing scheme of the control parameter
 *
 *  31           30-28   27    26-24  23-20   19     18-16   15-0
 * +------------+-----+------+-------+-----+-------+-------+------+
 * |            |     |      | invec |     |       | outvec| type |
 * | Res        | Res | Res  | number| Res | Res   | number|      |
 * +------------+-----+------+-------+-----+-------+-------+------+
 *
 * Res: Reserved.
 */

#ifndef RSE_COMMS_PROTOCOL_COMMON
#define RSE_COMMS_PROTOCOL_COMMON

#define TYPE_OFFSET	(0U)
#define TYPE_MASK	(0xFFFFUL << TYPE_OFFSET)
#define IN_LEN_OFFSET	(24U)
#define IN_LEN_MASK	(0x7UL << IN_LEN_OFFSET)
#define OUT_LEN_OFFSET	(16U)
#define OUT_LEN_MASK	(0x7UL << OUT_LEN_OFFSET)

#define PARAM_PACK(type, in_len, out_len)			   \
	(((((uint32_t)(type)) << TYPE_OFFSET) & TYPE_MASK)	 | \
	 ((((uint32_t)(in_len)) << IN_LEN_OFFSET) & IN_LEN_MASK) | \
	 ((((uint32_t)(out_len)) << OUT_LEN_OFFSET) & OUT_LEN_MASK))

#endif /* RSE_COMMS_PROTOCOL_COMMON */
