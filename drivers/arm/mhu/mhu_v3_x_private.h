/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MHU_V3_X_PRIVATE_H
#define MHU_V3_X_PRIVATE_H

#include <stdint.h>

/* Flag for PDBCW Interrupt Transfer Acknowledgment  */
#define MHU_V3_X_PDBCW_INT_X_TFR_ACK 0x1

/* Flag for PDBCW CTRL Postbox combined interrupts enable */
#define MHU_V3_X_PDBCW_CTRL_PBX_COMB_EN 0x1

/* Flag for MDBCW CTRL Mailbox combined interrupts enable */
#define MHU_V3_X_MDBCW_CTRL_MBX_COMB_EN 0x1

/**
 * Postbox control page structure
 */
struct _mhu_v3_x_pbx_ctrl_reg_t {
	/* Offset: 0x000 (R/ ) Postbox Block Identifier */
	const volatile uint32_t pbx_blk_id;
	/* Offset: 0x004 (R/ ) Reserved */
	const volatile uint8_t reserved_0[0x10 - 0x04];
	/* Offset: 0x010 (R/ ) Postbox Feature Support 0 */
	const volatile uint32_t pbx_feat_spt0;
	/* Offset: 0x014 (R/ ) Postbox Feature Support 1 */
	const volatile uint32_t pbx_feat_spt1;
	/* Offset: 0x018 (R/ ) Reserved */
	const volatile uint8_t reserved_1[0x20 - 0x18];
	/* Offset: 0x020 (R/ ) Postbox Doorbell Channel Configuration 0 */
	const volatile uint32_t pbx_dbch_cfg0;
	/* Offset: 0x024 (R/ ) Reserved */
	const volatile uint8_t reserved_2[0x30 - 0x24];
	/* Offset: 0x030 (R/ ) Postbox FIFO Channel Configuration 0 */
	const volatile uint32_t pbx_ffch_cfg0;
	/* Offset: 0x034 (R/ ) Reserved */
	const volatile uint8_t reserved_3[0x40 - 0x34];
	/* Offset: 0x040 (R/ ) Postbox Fast Channel Configuration 0 */
	const volatile uint32_t pbx_fch_cfg0;
	/* Offset: 0x044 (R/ ) Reserved */
	const volatile uint8_t reserved_4[0x100 - 0x44];
	/* Offset: 0x100 (R/W) Postbox control */
	volatile uint32_t pbx_ctrl;
	/* Offset: 0x164 (R/ ) Reserved */
	const volatile uint8_t reserved_5[0x400 - 0x104];
	/*
	 * Offset: 0x400 (R/ ) Postbox Doorbell Channel Interrupt Status n,
	 * where n is 0 - 3.
	 */
	const volatile uint32_t pbx_dbch_int_st[4];
	/*
	 * Offset: 0x410 (R/ ) Postbox FIFO Channel <n> Interrupt Status n,
	 * where n is 0 - 1.
	 */
	const volatile uint32_t pbx_ffch_int_st[2];
	/* Offset: 0x418 (R/ ) Reserved */
	const uint8_t reserved_6[0xFC8 - 0x418];
	/* Offset: 0xFC8 (R/ ) Postbox Implementer Identification Register */
	const volatile uint32_t pbx_iidr;
	/* Offset: 0xFCC (R/ ) Postbox Architecture Identification Register */
	const volatile uint32_t pbx_aidr;
	/*
	 * Offset: 0xFD0 (R/ ) Postbox Implementation Defined Identification
	 * Register n, where n is 0 - 11.
	 */
	const volatile uint32_t impl_def_id[12];
};

/**
 * Postbox doorbell channel window page structure
 */
struct _mhu_v3_x_pdbcw_reg_t {
	/* Offset: 0x000 (R/ ) Postbox Doorbell Channel Window Status */
	const volatile uint32_t pdbcw_st;
	/* Offset: 0x004 (R/ ) Reserved */
	const uint8_t reserved_0[0xC - 0x4];
	/* Offset: 0x00C ( /W) Postbox Doorbell Channel Window Set */
	volatile uint32_t pdbcw_set;
	/*
	 * Offset: 0x010 (R/ ) Postbox Doorbell Channel Window Interrupt Status
	 */
	const volatile uint32_t pdbcw_int_st;
	/*
	 * Offset: 0x014 ( /W) Postbox Doorbell Channel Window Interrupt Clear
	 */
	volatile uint32_t pdbcw_int_clr;
	/*
	 * Offset: 0x018 (R/W) Postbox Doorbell Channel Window Interrupt Enable
	 */
	volatile uint32_t pdbcw_int_en;
	/* Offset: 0x01C (R/W) Postbox Doorbell Channel Window Control */
	volatile uint32_t pdbcw_ctrl;
};

/**
 * Postbox structure
 */
struct _mhu_v3_x_pbx {
	/* Postbox Control */
	struct _mhu_v3_x_pbx_ctrl_reg_t pbx_ctrl_page;
	/* Postbox Doorbell Channel Window */
	struct _mhu_v3_x_pdbcw_reg_t pdbcw_page;
};

/**
 * Mailbox control page structure
 */
struct _mhu_v3_x_mbx_ctrl_reg_t {
	/* Offset: 0x000 (R/ ) Mailbox Block Identifier */
	const volatile uint32_t mbx_blk_id;
	/* Offset: 0x004 (R/ ) Reserved */
	const volatile uint8_t reserved_0[0x10 - 0x04];
	/* Offset: 0x010 (R/ ) Mailbox Feature Support 0 */
	const volatile uint32_t mbx_feat_spt0;
	/* Offset: 0x014 (R/ ) Mailbox Feature Support 1 */
	const volatile uint32_t mbx_feat_spt1;
	/* Offset: 0x018 (R/ ) Reserved */
	const volatile uint8_t reserved_1[0x20 - 0x18];
	/* Offset: 0x020 (R/ ) Mailbox Doorbell Channel Configuration 0 */
	const volatile uint32_t mbx_dbch_cfg0;
	/* Offset: 0x024 (R/ ) Reserved */
	const volatile uint8_t reserved_2[0x30 - 0x24];
	/* Offset: 0x030 (R/ ) Mailbox FIFO Channel Configuration 0 */
	const volatile uint32_t mbx_ffch_cfg0;
	/* Offset: 0x034 (R/ ) Reserved */
	const volatile uint8_t reserved_4[0x40 - 0x34];
	/* Offset: 0x040 (R/ ) Mailbox Fast Channel Configuration 0 */
	const volatile uint32_t mbx_fch_cfg0;
	/* Offset: 0x044 (R/ ) Reserved */
	const volatile uint8_t reserved_5[0x100 - 0x44];
	/* Offset: 0x100 (R/W) Mailbox control */
	volatile uint32_t mbx_ctrl;
	/* Offset: 0x104 (R/ ) Reserved */
	const volatile uint8_t reserved_6[0x140 - 0x104];
	/* Offset: 0x140 (R/W) Mailbox Fast Channel control */
	volatile uint32_t mbx_fch_ctrl;
	/* Offset: 0x144 (R/W) Mailbox Fast Channel Group Interrupt Enable */
	volatile uint32_t mbx_fcg_int_en;
	/* Offset: 0x148 (R/ ) Reserved */
	const volatile uint8_t reserved_7[0x400 - 0x148];
	/*
	 * Offset: 0x400 (R/ ) Mailbox Doorbell Channel Interrupt Status n,
	 * where n = 0 - 3.
	 */
	const volatile uint32_t mbx_dbch_int_st[4];
	/*
	 * Offset: 0x410 (R/ ) Mailbox FIFO Channel Interrupt Status n, where
	 * n = 0 - 1.
	 */
	const volatile uint32_t mbx_ffch_int_st[2];
	/* Offset: 0x418 (R/ ) Reserved */
	const volatile uint8_t reserved_8[0x470 - 0x418];
	/* Offset: 0x470 (R/ ) Mailbox Fast Channel Group Interrupt Status */
	const volatile uint32_t mbx_fcg_int_st;
	/* Offset: 0x474 (R/ ) Reserved */
	const volatile uint8_t reserved_9[0x480 - 0x474];
	/*
	 * Offset: 0x480 (R/ ) Mailbox Fast Channel Group <n> Interrupt Status,
	 * where n = 0 - 31.
	 */
	const volatile uint32_t mbx_fch_grp_int_st[32];
	/* Offset: 0x500 (R/ ) Reserved */
	const volatile uint8_t reserved_10[0xFC8 - 0x500];
	/* Offset: 0xFC8 (R/ ) Mailbox Implementer Identification Register */
	const volatile uint32_t mbx_iidr;
	/* Offset: 0xFCC (R/ ) Mailbox Architecture Identification Register */
	const volatile uint32_t mbx_aidr;
	/*
	 * Offset: 0xFD0 (R/ ) Mailbox Implementation Defined Identification
	 * Register n, where n is 0 - 11.
	 */
	const volatile uint32_t impl_def_id[12];
};

/**
 * Mailbox doorbell channel window page structure
 */
struct _mhu_v3_x_mdbcw_reg_t {
	/* Offset: 0x000 (R/ ) Mailbox Doorbell Channel Window Status */
	const volatile uint32_t mdbcw_st;
	/* Offset: 0x004 (R/ ) Mailbox Doorbell Channel Window Status Masked */
	const volatile uint32_t mdbcw_st_msk;
	/* Offset: 0x008 ( /W) Mailbox Doorbell Channel Window Clear */
	volatile uint32_t mdbcw_clr;
	/* Offset: 0x00C (R/ ) Reserved */
	const volatile uint8_t reserved_0[0x10 - 0x0C];
	/* Offset: 0x010 (R/ ) Mailbox Doorbell Channel Window Mask Status */
	const volatile uint32_t mdbcw_msk_st;
	/* Offset: 0x014 ( /W) Mailbox Doorbell Channel Window Mask Set */
	volatile uint32_t mdbcw_msk_set;
	/* Offset: 0x018 ( /W) Mailbox Doorbell Channel Window Mask Clear */
	volatile uint32_t mdbcw_msk_clr;
	/* Offset: 0x01C (R/W) Mailbox Doorbell Channel Window Control */
	volatile uint32_t mdbcw_ctrl;
};

/**
 * Mailbox structure
 */
struct _mhu_v3_x_mbx {
	/* Mailbox control */
	struct _mhu_v3_x_mbx_ctrl_reg_t mbx_ctrl_page;
	/* Mailbox Doorbell Channel Window */
	struct _mhu_v3_x_mdbcw_reg_t mdbcw_page;
};

/**
 * MHUv3 frame type
 */
union _mhu_v3_x_frame_t {
	/* Postbox Frame */
	struct _mhu_v3_x_pbx pbx_frame;
	/* Mailbox Frame */
	struct _mhu_v3_x_mbx mbx_frame;
};

#endif /* MHU_V3_X_PRIVATE_H */
