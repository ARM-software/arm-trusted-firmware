/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MHU_V3_X_H
#define MHU_V3_X_H

#include <stdbool.h>
#include <stdint.h>

/* MHU Architecture Major Revision 3 */
#define MHU_MAJOR_REV_V3 U(0x2)
/* MHU Architecture Minor Revision 0 */
#define MHU_MINOR_REV_3_0 U(0x0)

/* MHU Architecture Major Revision offset */
#define MHU_ARCH_MAJOR_REV_OFF U(0x4)
/* MHU Architecture Major Revision mask */
#define MHU_ARCH_MAJOR_REV_MASK (U(0xf) << MHU_ARCH_MAJOR_REV_OFF)

/* MHU Architecture Minor Revision offset */
#define MHU_ARCH_MINOR_REV_OFF U(0x0)
/* MHU Architecture Minor Revision mask */
#define MHU_ARCH_MINOR_REV_MASK (U(0xf) << MHU_ARCH_MINOR_REV_OFF)

/* MHUv3 PBX/MBX Operational Request offset */
#define MHU_V3_OP_REQ_OFF U(0)
/* MHUv3 PBX/MBX Operational Request */
#define MHU_V3_OP_REQ (U(1) << MHU_V3_OP_REQ_OFF)

/**
 * MHUv3 error enumeration types
 */
enum mhu_v3_x_error_t {
	/* No error */
	MHU_V_3_X_ERR_NONE,
	/* MHU driver not initialized */
	MHU_V_3_X_ERR_NOT_INIT,
	/* MHU driver alreary initialized */
	MHU_V_3_X_ERR_ALREADY_INIT,
	/* MHU Revision not supported error */
	MHU_V_3_X_ERR_UNSUPPORTED_VERSION,
	/* Operation not supported */
	MHU_V_3_X_ERR_UNSUPPORTED,
	/* Invalid parameter */
	MHU_V_3_X_ERR_INVALID_PARAM,
	/* General MHU driver error */
	MHU_V_3_X_ERR_GENERAL,
};

/**
 * MHUv3 channel types
 */
enum mhu_v3_x_channel_type_t {
	/* Doorbell channel */
	MHU_V3_X_CHANNEL_TYPE_DBCH,
	/* Channel type count */
	MHU_V3_X_CHANNEL_TYPE_COUNT,
};

/**
 * MHUv3 frame types
 */
enum mhu_v3_x_frame_t {
	/* MHUv3 postbox frame */
	MHU_V3_X_PBX_FRAME,
	/* MHUv3 mailbox frame */
	MHU_V3_X_MBX_FRAME,
};

/**
 * MHUv3 device structure
 */
struct mhu_v3_x_dev_t {
	/* Base address of the MHUv3 frame */
	uintptr_t base;
	/* Type of the MHUv3 frame */
	enum mhu_v3_x_frame_t frame;
	/* Minor revision of the MHUv3 */
	uint32_t subversion;
	/* Flag to indicate if the MHUv3 is initialized */
	bool is_initialized;
};

/**
 * Initializes the MHUv3
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_driver_init(struct mhu_v3_x_dev_t *dev);

/**
 * Returns the number of channels implemented
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * ch_type	MHU channel type mhu_v3_x_channel_type_t
 * num_ch	Pointer to the variable that will store the value
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_get_num_channel_implemented(
	 const struct mhu_v3_x_dev_t *dev, enum mhu_v3_x_channel_type_t ch_type,
	 uint8_t *num_ch);

/**
 * Clear flags from a doorbell channel
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * flags	Flags to be cleared from the channel
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_doorbell_clear(const struct mhu_v3_x_dev_t *dev,
	 const uint32_t channel, uint32_t flags);

/**
 * Write flags to a doorbell channel
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * flags	Flags to be written to the channel
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_doorbell_write(const struct mhu_v3_x_dev_t *dev,
	 const uint32_t channel, uint32_t flags);

/**
 * Read value from a doorbell channel
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * flags	Pointer to the variable that will store the flags read from the
 *		channel
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_doorbell_read(const struct mhu_v3_x_dev_t *dev,
	 const uint32_t channel, uint32_t *flags);

/**
 * Set bits in a doorbell channel mask which is used to disable interrupts for
 * received flags corresponding to the mask
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * flags	Flags to set mask bits in this doorbell channel
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_doorbell_mask_set(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 uint32_t flags);

/**
 * Clear bits in a doorbell channel mask which is used to disable interrupts
 * for received flags corresponding to the mask
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * flags	Flags to clear mask bits in this doorbell channel
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_doorbell_mask_clear(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel, uint32_t flags);

/**
 * Get the mask of a doorbell channel which is used to disable interrupts for
 * received flags corresponding to the mask
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * flags	Pointer to the variable that will store the flags read from the
 *		mask value
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_doorbell_mask_get(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel, uint32_t *flags);

/**
 * Enable the channel interrupt
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * ch_type	MHU channel type mhu_v3_x_channel_type_t
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_channel_interrupt_enable(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 enum mhu_v3_x_channel_type_t ch_type);

/**
 * Disable the channel interrupt
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * ch_type	MHU channel type mhu_v3_x_channel_type_t
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_channel_interrupt_disable(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 enum mhu_v3_x_channel_type_t ch_type);

/**
 * Clear the channel interrupt
 *
 * dev		MHU device struct mhu_v3_x_dev_t
 * channel	Doorbell channel number
 * ch_type	MHU channel type mhu_v3_x_channel_type_t
 *
 * Returns mhu_v3_x_error_t error code
 */
enum mhu_v3_x_error_t mhu_v3_x_channel_interrupt_clear(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 enum mhu_v3_x_channel_type_t ch_type);

#endif /* MHU_V3_X_H */
