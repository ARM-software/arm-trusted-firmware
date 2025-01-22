/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TISCI_PM_TISCI_DEVICE_H
#define TISCI_PM_TISCI_DEVICE_H

#include <stddef.h>
#include <tisci/tisci_protocol.h>

/**
 * Configure the device to be a wake source. The meaning of this flag will
 * vary slightly from device to device and from SoC to SoC but it
 * generally allows the device to wake the SoC out of low power states.
 */
#define TISCI_MSG_FLAG_DEVICE_WAKE_ENABLED    BIT(8)

/** Enable reset isolation for this device. */
#define TISCI_MSG_FLAG_DEVICE_RESET_ISO	      BIT(9)

/**
 * Claim a device exclusively. When passed with STATE_RETENTION or STATE_ON,
 * it will claim the device exclusively. If another host already has this
 * device set to STATE_RETENTION or STATE_ON, the message will fail. Once
 * successful, other hosts attempting to set STATE_RETENTION or STATE_ON
 * will fail.
 */
#define TISCI_MSG_FLAG_DEVICE_EXCLUSIVE	      BIT(10)

/**
 * Used by TISCI_MSG_SET_DEVICE to turn device off when possible. This
 * must be used in conjunction with SoC dependencies to identify the
 * overall power domain state being achieved.
 */
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF    0

/** Used by TISCI_MSG_SET_DEVICE to disable device but keep in retention. */
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_RETENTION   1

/** Used by TISCI_MSG_SET_DEVICE to turn device on for usage. */
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_ON	    2

/**
 * TISCI_MSG_GET_DEVICE sets this as current state to indicate device
 * is off.
 */
#define TISCI_MSG_VALUE_DEVICE_HW_STATE_OFF	    0

/**
 * TISCI_MSG_GET_DEVICE sets this as current state to indicate device
 * is on.
 */
#define TISCI_MSG_VALUE_DEVICE_HW_STATE_ON	    1

/**
 * TISCI_MSG_GET_DEVICE sets this as current state to indicate device is
 * transitioning between states. When a device stays in this state it is
 * typically due to the fact that some resource that the device is
 * dependent on (example IRQs) are pending preventing completion of
 * hardware handshake. Please refer to Technical Reference Manual for
 * additional information.
 */
#define TISCI_MSG_VALUE_DEVICE_HW_STATE_TRANS	    2

/**
 * \brief Set the desired state of the device.
 *
 * Certain flags can also be set to alter the device state:
 * TISCI_MSG_FLAG_DEVICE_WAKE_ENABLED, TISCI_MSG_FLAG_DEVICE_RESET_ISO,
 * TISCI_MSG_FLAG_DEVICE_EXCLUSIVE
 *
 * \param hdr TISCI header
 *
 * \param id
 * Indicates which device to modify
 *
 * \param reserved
 * Field is ignored
 *
 * \param state
 * The desired state of the device.
 */
struct tisci_msg_set_device_req {
	struct tisci_header	hdr;
	uint32_t			id;
	uint32_t			reserved;
	uint8_t			state;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_SET_DEVICE
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_set_device_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Request to get device based on id.
 *
 * \param hdr TISCI header.
 *
 * \param id
 * Indicates which device to modify
 */
struct tisci_msg_get_device_req {
	struct tisci_header	hdr;
	uint32_t			id;
} __packed;

/**
 * \brief Response to get device request.
 *
 * \param hdr TISCI header.
 *
 * \param context_loss_count
 * Indicates how many times the device has lost context. A driver can use this
 * monotonic counter to determine if the device has lost context since the
 * last time this message was exchanged.
 *
 * \param resets
 * Programmed state of the reset lines.
 *
 * \param programmed_state
 * The state as programmed by set_device.
 *
 * \param current_state
 * The actual state of the hardware.
 */
struct tisci_msg_get_device_resp {
	struct tisci_header	hdr;
	uint32_t			context_loss_count;
	uint32_t			resets;
	uint8_t			programmed_state;
	uint8_t			current_state;
} __packed;

/**
 * \brief Set the desired state of the resets of a device.
 *
 * \param hdr TISCI header.
 *
 * \param id
 * Indicates which device to modify
 *
 * \param resets
 * A bit field of resets for the device. The meaning, behavior, and usage of
 * the reset flags are device specific. Although the flags can be set
 * independently of device state, the firmware will control when resets are
 * actually set to insure compliance with the SoC specific reset requirements.
 */
struct tisci_msg_set_device_resets_req {
	struct tisci_header	hdr;
	uint32_t			id;
	uint32_t			resets;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_SET_DEVICE_RESETS
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_set_device_resets_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Drop any remaining powerup device references.
 *
 * At boot, any powered up devices receive a powerup reference. This keeps
 * the device on. When a host enables a device, the powerup reference is
 * dropped and replaced with a host reference. Similarly, when a device
 * disables a device the powerup reference is dropped.
 *
 * This messages drops powerup reference from all devices, turning off
 * any devices with a powerup reference.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_device_drop_powerup_ref_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_DEVICE_DROP_POWERUP_REF
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_device_drop_powerup_ref_resp {
	struct tisci_header hdr;
} __packed;

#endif /* TISCI_PM_TISCI_DEVICE_H */
