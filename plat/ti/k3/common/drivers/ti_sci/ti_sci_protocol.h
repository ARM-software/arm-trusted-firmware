/*
 * Texas Instruments System Control Interface (TISCI) Protocol
 *
 * Communication protocol with TI SCI hardware
 * The system works in a message response protocol
 * See: http://processors.wiki.ti.com/index.php/TISCI for details
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TI_SCI_PROTOCOL_H
#define TI_SCI_PROTOCOL_H

#include <stdint.h>

/* Generic Messages */
#define TI_SCI_MSG_ENABLE_WDT		0x0000
#define TI_SCI_MSG_WAKE_RESET		0x0001
#define TI_SCI_MSG_VERSION		0x0002
#define TI_SCI_MSG_WAKE_REASON		0x0003
#define TI_SCI_MSG_GOODBYE		0x0004
#define TI_SCI_MSG_SYS_RESET		0x0005

/* Device requests */
#define TI_SCI_MSG_SET_DEVICE_STATE	0x0200
#define TI_SCI_MSG_GET_DEVICE_STATE	0x0201
#define TI_SCI_MSG_SET_DEVICE_RESETS	0x0202

/* Clock requests */
#define TI_SCI_MSG_SET_CLOCK_STATE	0x0100
#define TI_SCI_MSG_GET_CLOCK_STATE	0x0101
#define TI_SCI_MSG_SET_CLOCK_PARENT	0x0102
#define TI_SCI_MSG_GET_CLOCK_PARENT	0x0103
#define TI_SCI_MSG_GET_NUM_CLOCK_PARENTS 0x0104
#define TI_SCI_MSG_SET_CLOCK_FREQ	0x010c
#define TI_SCI_MSG_QUERY_CLOCK_FREQ	0x010d
#define TI_SCI_MSG_GET_CLOCK_FREQ	0x010e

/* Processor Control Messages */
#define TISCI_MSG_PROC_REQUEST		0xc000
#define TISCI_MSG_PROC_RELEASE		0xc001
#define TISCI_MSG_PROC_HANDOVER		0xc005
#define TISCI_MSG_SET_PROC_BOOT_CONFIG	0xc100
#define TISCI_MSG_SET_PROC_BOOT_CTRL	0xc101
#define TISCI_MSG_PROC_AUTH_BOOT_IMIAGE	0xc120
#define TISCI_MSG_GET_PROC_BOOT_STATUS	0xc400

/**
 * struct ti_sci_msg_hdr - Generic Message Header for All messages and responses
 * @type:	Type of messages: One of TI_SCI_MSG* values
 * @host:	Host of the message
 * @seq:	Message identifier indicating a transfer sequence
 * @flags:	Flag for the message
 */
struct ti_sci_msg_hdr {
	uint16_t type;
	uint8_t host;
	uint8_t seq;
#define TI_SCI_MSG_FLAG(val)			(1 << (val))
#define TI_SCI_FLAG_REQ_GENERIC_NORESPONSE	0x0
#define TI_SCI_FLAG_REQ_ACK_ON_RECEIVED		TI_SCI_MSG_FLAG(0)
#define TI_SCI_FLAG_REQ_ACK_ON_PROCESSED	TI_SCI_MSG_FLAG(1)
#define TI_SCI_FLAG_RESP_GENERIC_NACK		0x0
#define TI_SCI_FLAG_RESP_GENERIC_ACK		TI_SCI_MSG_FLAG(1)
	/* Additional Flags */
	uint32_t flags;
} __packed;

/**
 * struct ti_sci_msg_resp_version - Response for a message
 * @hdr:		Generic header
 * @firmware_description: String describing the firmware
 * @firmware_revision:	Firmware revision
 * @abi_major:		Major version of the ABI that firmware supports
 * @abi_minor:		Minor version of the ABI that firmware supports
 *
 * In general, ABI version changes follow the rule that minor version increments
 * are backward compatible. Major revision changes in ABI may not be
 * backward compatible.
 *
 * Response to a generic message with message type TI_SCI_MSG_VERSION
 */
struct ti_sci_msg_resp_version {
	struct ti_sci_msg_hdr hdr;
#define FIRMWARE_DESCRIPTION_LENGTH 32
	char firmware_description[FIRMWARE_DESCRIPTION_LENGTH];
	uint16_t firmware_revision;
	uint8_t abi_major;
	uint8_t abi_minor;
} __packed;

/**
 * struct ti_sci_msg_req_reboot - Reboot the SoC
 * @hdr:	Generic Header
 *
 * Request type is TI_SCI_MSG_SYS_RESET, responded with a generic
 * ACK/NACK message.
 */
struct ti_sci_msg_req_reboot {
	struct ti_sci_msg_hdr hdr;
} __packed;

/**
 * struct ti_sci_msg_req_set_device_state - Set the desired state of the device
 * @hdr:		Generic header
 * @id:	Indicates which device to modify
 * @reserved: Reserved space in message, must be 0 for backward compatibility
 * @state: The desired state of the device.
 *
 * Certain flags can also be set to alter the device state:
 * + MSG_FLAG_DEVICE_WAKE_ENABLED - Configure the device to be a wake source.
 * The meaning of this flag will vary slightly from device to device and from
 * SoC to SoC but it generally allows the device to wake the SoC out of deep
 * suspend states.
 * + MSG_FLAG_DEVICE_RESET_ISO - Enable reset isolation for this device.
 * + MSG_FLAG_DEVICE_EXCLUSIVE - Claim this device exclusively. When passed
 * with STATE_RETENTION or STATE_ON, it will claim the device exclusively.
 * If another host already has this device set to STATE_RETENTION or STATE_ON,
 * the message will fail. Once successful, other hosts attempting to set
 * STATE_RETENTION or STATE_ON will fail.
 *
 * Request type is TI_SCI_MSG_SET_DEVICE_STATE, responded with a generic
 * ACK/NACK message.
 */
struct ti_sci_msg_req_set_device_state {
	/* Additional hdr->flags options */
#define MSG_FLAG_DEVICE_WAKE_ENABLED	TI_SCI_MSG_FLAG(8)
#define MSG_FLAG_DEVICE_RESET_ISO	TI_SCI_MSG_FLAG(9)
#define MSG_FLAG_DEVICE_EXCLUSIVE	TI_SCI_MSG_FLAG(10)
	struct ti_sci_msg_hdr hdr;
	uint32_t id;
	uint32_t reserved;

#define MSG_DEVICE_SW_STATE_AUTO_OFF	0
#define MSG_DEVICE_SW_STATE_RETENTION	1
#define MSG_DEVICE_SW_STATE_ON		2
	uint8_t state;
} __packed;

/**
 * struct ti_sci_msg_req_get_device_state - Request to get device.
 * @hdr:		Generic header
 * @id:		Device Identifier
 *
 * Request type is TI_SCI_MSG_GET_DEVICE_STATE, responded device state
 * information
 */
struct ti_sci_msg_req_get_device_state {
	struct ti_sci_msg_hdr hdr;
	uint32_t id;
} __packed;

/**
 * struct ti_sci_msg_resp_get_device_state - Response to get device request.
 * @hdr:		Generic header
 * @context_loss_count: Indicates how many times the device has lost context. A
 *	driver can use this monotonic counter to determine if the device has
 *	lost context since the last time this message was exchanged.
 * @resets: Programmed state of the reset lines.
 * @programmed_state:	The state as programmed by set_device.
 *			- Uses the MSG_DEVICE_SW_* macros
 * @current_state:	The actual state of the hardware.
 *
 * Response to request TI_SCI_MSG_GET_DEVICE_STATE.
 */
struct ti_sci_msg_resp_get_device_state {
	struct ti_sci_msg_hdr hdr;
	uint32_t context_loss_count;
	uint32_t resets;
	uint8_t programmed_state;
#define MSG_DEVICE_HW_STATE_OFF		0
#define MSG_DEVICE_HW_STATE_ON		1
#define MSG_DEVICE_HW_STATE_TRANS	2
	uint8_t current_state;
} __packed;

/**
 * struct ti_sci_msg_req_set_device_resets - Set the desired resets
 *				configuration of the device
 * @hdr:		Generic header
 * @id:	Indicates which device to modify
 * @resets: A bit field of resets for the device. The meaning, behavior,
 *	and usage of the reset flags are device specific. 0 for a bit
 *	indicates releasing the reset represented by that bit while 1
 *	indicates keeping it held.
 *
 * Request type is TI_SCI_MSG_SET_DEVICE_RESETS, responded with a generic
 * ACK/NACK message.
 */
struct ti_sci_msg_req_set_device_resets {
	struct ti_sci_msg_hdr hdr;
	uint32_t id;
	uint32_t resets;
} __packed;

/**
 * struct ti_sci_msg_req_set_clock_state - Request to setup a Clock state
 * @hdr:	Generic Header, Certain flags can be set specific to the clocks:
 *		MSG_FLAG_CLOCK_ALLOW_SSC: Allow this clock to be modified
 *		via spread spectrum clocking.
 *		MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE: Allow this clock's
 *		frequency to be changed while it is running so long as it
 *		is within the min/max limits.
 *		MSG_FLAG_CLOCK_INPUT_TERM: Enable input termination, this
 *		is only applicable to clock inputs on the SoC pseudo-device.
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @request_state: Request the state for the clock to be set to.
 *		MSG_CLOCK_SW_STATE_UNREQ: The IP does not require this clock,
 *		it can be disabled, regardless of the state of the device
 *		MSG_CLOCK_SW_STATE_AUTO: Allow the System Controller to
 *		automatically manage the state of this clock. If the device
 *		is enabled, then the clock is enabled. If the device is set
 *		to off or retention, then the clock is internally set as not
 *		being required by the device.(default)
 *		MSG_CLOCK_SW_STATE_REQ:  Configure the clock to be enabled,
 *		regardless of the state of the device.
 *
 * Normally, all required clocks are managed by TISCI entity, this is used
 * only for specific control *IF* required. Auto managed state is
 * MSG_CLOCK_SW_STATE_AUTO, in other states, TISCI entity assume remote
 * will explicitly control.
 *
 * Request type is TI_SCI_MSG_SET_CLOCK_STATE, response is a generic
 * ACK or NACK message.
 */
struct ti_sci_msg_req_set_clock_state {
	/* Additional hdr->flags options */
#define MSG_FLAG_CLOCK_ALLOW_SSC		TI_SCI_MSG_FLAG(8)
#define MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE	TI_SCI_MSG_FLAG(9)
#define MSG_FLAG_CLOCK_INPUT_TERM		TI_SCI_MSG_FLAG(10)
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint8_t clk_id;
#define MSG_CLOCK_SW_STATE_UNREQ	0
#define MSG_CLOCK_SW_STATE_AUTO		1
#define MSG_CLOCK_SW_STATE_REQ		2
	uint8_t request_state;
} __packed;

/**
 * struct ti_sci_msg_req_get_clock_state - Request for clock state
 * @hdr:	Generic Header
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to get state of.
 *
 * Request type is TI_SCI_MSG_GET_CLOCK_STATE, response is state
 * of the clock
 */
struct ti_sci_msg_req_get_clock_state {
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint8_t clk_id;
} __packed;

/**
 * struct ti_sci_msg_resp_get_clock_state - Response to get clock state
 * @hdr:	Generic Header
 * @programmed_state: Any programmed state of the clock. This is one of
 *		MSG_CLOCK_SW_STATE* values.
 * @current_state: Current state of the clock. This is one of:
 *		MSG_CLOCK_HW_STATE_NOT_READY: Clock is not ready
 *		MSG_CLOCK_HW_STATE_READY: Clock is ready
 *
 * Response to TI_SCI_MSG_GET_CLOCK_STATE.
 */
struct ti_sci_msg_resp_get_clock_state {
	struct ti_sci_msg_hdr hdr;
	uint8_t programmed_state;
#define MSG_CLOCK_HW_STATE_NOT_READY	0
#define MSG_CLOCK_HW_STATE_READY	1
	uint8_t current_state;
} __packed;

/**
 * struct ti_sci_msg_req_set_clock_parent - Set the clock parent
 * @hdr:	Generic Header
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @parent_id:	The new clock parent is selectable by an index via this
 *		parameter.
 *
 * Request type is TI_SCI_MSG_SET_CLOCK_PARENT, response is generic
 * ACK / NACK message.
 */
struct ti_sci_msg_req_set_clock_parent {
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint8_t clk_id;
	uint8_t parent_id;
} __packed;

/**
 * struct ti_sci_msg_req_get_clock_parent - Get the clock parent
 * @hdr:	Generic Header
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to get the parent for.
 *
 * Request type is TI_SCI_MSG_GET_CLOCK_PARENT, response is parent information
 */
struct ti_sci_msg_req_get_clock_parent {
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint8_t clk_id;
} __packed;

/**
 * struct ti_sci_msg_resp_get_clock_parent - Response with clock parent
 * @hdr:	Generic Header
 * @parent_id:	The current clock parent
 *
 * Response to TI_SCI_MSG_GET_CLOCK_PARENT.
 */
struct ti_sci_msg_resp_get_clock_parent {
	struct ti_sci_msg_hdr hdr;
	uint8_t parent_id;
} __packed;

/**
 * struct ti_sci_msg_req_get_clock_num_parents - Request to get clock parents
 * @hdr:	Generic header
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *
 * This request provides information about how many clock parent options
 * are available for a given clock to a device. This is typically used
 * for input clocks.
 *
 * Request type is TI_SCI_MSG_GET_NUM_CLOCK_PARENTS, response is appropriate
 * message, or NACK in case of inability to satisfy request.
 */
struct ti_sci_msg_req_get_clock_num_parents {
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint8_t clk_id;
} __packed;

/**
 * struct ti_sci_msg_resp_get_clock_num_parents - Response for get clk parents
 * @hdr:		Generic header
 * @num_parents:	Number of clock parents
 *
 * Response to TI_SCI_MSG_GET_NUM_CLOCK_PARENTS
 */
struct ti_sci_msg_resp_get_clock_num_parents {
	struct ti_sci_msg_hdr hdr;
	uint8_t num_parents;
} __packed;

/**
 * struct ti_sci_msg_req_query_clock_freq - Request to query a frequency
 * @hdr:	Generic Header
 * @dev_id:	Device identifier this request is for
 * @min_freq_hz: The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq_hz: The target clock frequency. A frequency will be found
 *		as close to this target frequency as possible.
 * @max_freq_hz: The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @clk_id:	Clock identifier for the device for this request.
 *
 * NOTE: Normally clock frequency management is automatically done by TISCI
 * entity. In case of specific requests, TISCI evaluates capability to achieve
 * requested frequency within provided range and responds with
 * result message.
 *
 * Request type is TI_SCI_MSG_QUERY_CLOCK_FREQ, response is appropriate message,
 * or NACK in case of inability to satisfy request.
 */
struct ti_sci_msg_req_query_clock_freq {
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint64_t min_freq_hz;
	uint64_t target_freq_hz;
	uint64_t max_freq_hz;
	uint8_t clk_id;
} __packed;

/**
 * struct ti_sci_msg_resp_query_clock_freq - Response to a clock frequency query
 * @hdr:	Generic Header
 * @freq_hz:	Frequency that is the best match in Hz.
 *
 * Response to request type TI_SCI_MSG_QUERY_CLOCK_FREQ. NOTE: if the request
 * cannot be satisfied, the message will be of type NACK.
 */
struct ti_sci_msg_resp_query_clock_freq {
	struct ti_sci_msg_hdr hdr;
	uint64_t freq_hz;
} __packed;

/**
 * struct ti_sci_msg_req_set_clock_freq - Request to setup a clock frequency
 * @hdr:	Generic Header
 * @dev_id:	Device identifier this request is for
 * @min_freq_hz: The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq_hz: The target clock frequency. The clock will be programmed
 *		at a rate as close to this target frequency as possible.
 * @max_freq_hz: The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @clk_id:	Clock identifier for the device for this request.
 *
 * NOTE: Normally clock frequency management is automatically done by TISCI
 * entity. In case of specific requests, TISCI evaluates capability to achieve
 * requested range and responds with success/failure message.
 *
 * This sets the desired frequency for a clock within an allowable
 * range. This message will fail on an enabled clock unless
 * MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE is set for the clock. Additionally,
 * if other clocks have their frequency modified due to this message,
 * they also must have the MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE or be disabled.
 *
 * Calling set frequency on a clock input to the SoC pseudo-device will
 * inform the PMMC of that clock's frequency. Setting a frequency of
 * zero will indicate the clock is disabled.
 *
 * Calling set frequency on clock outputs from the SoC pseudo-device will
 * function similarly to setting the clock frequency on a device.
 *
 * Request type is TI_SCI_MSG_SET_CLOCK_FREQ, response is a generic ACK/NACK
 * message.
 */
struct ti_sci_msg_req_set_clock_freq {
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint64_t min_freq_hz;
	uint64_t target_freq_hz;
	uint64_t max_freq_hz;
	uint8_t clk_id;
} __packed;

/**
 * struct ti_sci_msg_req_get_clock_freq - Request to get the clock frequency
 * @hdr:	Generic Header
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *
 * NOTE: Normally clock frequency management is automatically done by TISCI
 * entity. In some cases, clock frequencies are configured by host.
 *
 * Request type is TI_SCI_MSG_GET_CLOCK_FREQ, responded with clock frequency
 * that the clock is currently at.
 */
struct ti_sci_msg_req_get_clock_freq {
	struct ti_sci_msg_hdr hdr;
	uint32_t dev_id;
	uint8_t clk_id;
} __packed;

/**
 * struct ti_sci_msg_resp_get_clock_freq - Response of clock frequency request
 * @hdr:	Generic Header
 * @freq_hz:	Frequency that the clock is currently on, in Hz.
 *
 * Response to request type TI_SCI_MSG_GET_CLOCK_FREQ.
 */
struct ti_sci_msg_resp_get_clock_freq {
	struct ti_sci_msg_hdr hdr;
	uint64_t freq_hz;
} __packed;

#define TISCI_ADDR_LOW_MASK		0x00000000ffffffff
#define TISCI_ADDR_HIGH_MASK		0xffffffff00000000
#define TISCI_ADDR_HIGH_SHIFT		32

/**
 * struct ti_sci_msg_req_proc_request - Request a processor
 *
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 *
 * Request type is TISCI_MSG_PROC_REQUEST, response is a generic ACK/NACK
 * message.
 */
struct ti_sci_msg_req_proc_request {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
} __packed;

/**
 * struct ti_sci_msg_req_proc_release - Release a processor
 *
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 *
 * Request type is TISCI_MSG_PROC_RELEASE, response is a generic ACK/NACK
 * message.
 */
struct ti_sci_msg_req_proc_release {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
} __packed;

/**
 * struct ti_sci_msg_req_proc_handover - Handover a processor to a host
 *
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 * @host_id:		New Host we want to give control to
 *
 * Request type is TISCI_MSG_PROC_HANDOVER, response is a generic ACK/NACK
 * message.
 */
struct ti_sci_msg_req_proc_handover {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
	uint8_t host_id;
} __packed;

/* A53 Config Flags */
#define PROC_BOOT_CFG_FLAG_ARMV8_DBG_EN         0x00000001
#define PROC_BOOT_CFG_FLAG_ARMV8_DBG_NIDEN      0x00000002
#define PROC_BOOT_CFG_FLAG_ARMV8_DBG_SPIDEN     0x00000004
#define PROC_BOOT_CFG_FLAG_ARMV8_DBG_SPNIDEN    0x00000008
#define PROC_BOOT_CFG_FLAG_ARMV8_AARCH32        0x00000100

/* R5 Config Flags */
#define PROC_BOOT_CFG_FLAG_R5_DBG_EN            0x00000001
#define PROC_BOOT_CFG_FLAG_R5_DBG_NIDEN         0x00000002
#define PROC_BOOT_CFG_FLAG_R5_LOCKSTEP          0x00000100
#define PROC_BOOT_CFG_FLAG_R5_TEINIT            0x00000200
#define PROC_BOOT_CFG_FLAG_R5_NMFI_EN           0x00000400
#define PROC_BOOT_CFG_FLAG_R5_TCM_RSTBASE       0x00000800
#define PROC_BOOT_CFG_FLAG_R5_BTCM_EN           0x00001000
#define PROC_BOOT_CFG_FLAG_R5_ATCM_EN           0x00002000

/**
 * struct ti_sci_msg_req_set_proc_boot_config - Set Processor boot configuration
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 * @bootvector_low:	Lower 32bit (Little Endian) of boot vector
 * @bootvector_high:	Higher 32bit (Little Endian) of boot vector
 * @config_flags_set:	Optional Processor specific Config Flags to set.
 *			Setting a bit here implies required bit sets to 1.
 * @config_flags_clear:	Optional Processor specific Config Flags to clear.
 *			Setting a bit here implies required bit gets cleared.
 *
 * Request type is TISCI_MSG_SET_PROC_BOOT_CONFIG, response is a generic
 * ACK/NACK message.
 */
struct ti_sci_msg_req_set_proc_boot_config {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
	uint32_t bootvector_low;
	uint32_t bootvector_high;
	uint32_t config_flags_set;
	uint32_t config_flags_clear;
} __packed;

/* R5 Control Flags */
#define PROC_BOOT_CTRL_FLAG_R5_CORE_HALT                0x00000001

/**
 * struct ti_sci_msg_req_set_proc_boot_ctrl - Set Processor boot control flags
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 * @config_flags_set:	Optional Processor specific Config Flags to set.
 *			Setting a bit here implies required bit sets to 1.
 * @config_flags_clear:	Optional Processor specific Config Flags to clear.
 *			Setting a bit here implies required bit gets cleared.
 *
 * Request type is TISCI_MSG_SET_PROC_BOOT_CTRL, response is a generic ACK/NACK
 * message.
 */
struct ti_sci_msg_req_set_proc_boot_ctrl {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
	uint32_t control_flags_set;
	uint32_t control_flags_clear;
} __packed;

/**
 * struct ti_sci_msg_req_proc_auth_start_image - Authenticate and start image
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 * @cert_addr_low:	Lower 32bit (Little Endian) of certificate
 * @cert_addr_high:	Higher 32bit (Little Endian) of certificate
 *
 * Request type is TISCI_MSG_PROC_AUTH_BOOT_IMAGE, response is a generic
 * ACK/NACK message.
 */
struct ti_sci_msg_req_proc_auth_boot_image {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
	uint32_t cert_addr_low;
	uint32_t cert_addr_high;
} __packed;

/**
 * struct ti_sci_msg_req_get_proc_boot_status - Get processor boot status
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 *
 * Request type is TISCI_MSG_GET_PROC_BOOT_STATUS, response is appropriate
 * message, or NACK in case of inability to satisfy request.
 */
struct ti_sci_msg_req_get_proc_boot_status {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
} __packed;

/* ARMv8 Status Flags */
#define PROC_BOOT_STATUS_FLAG_ARMV8_WFE			0x00000001
#define PROC_BOOT_STATUS_FLAG_ARMV8_WFI			0x00000002

/* R5 Status Flags */
#define PROC_BOOT_STATUS_FLAG_R5_WFE			0x00000001
#define PROC_BOOT_STATUS_FLAG_R5_WFI			0x00000002
#define PROC_BOOT_STATUS_FLAG_R5_CLK_GATED		0x00000004
#define PROC_BOOT_STATUS_FLAG_R5_LOCKSTEP_PERMITTED	0x00000100

/**
 * \brief Processor Status Response
 * struct ti_sci_msg_resp_get_proc_boot_status - Processor boot status response
 * @hdr:		Generic Header
 * @processor_id:	ID of processor
 * @bootvector_low:	Lower 32bit (Little Endian) of boot vector
 * @bootvector_high:	Higher 32bit (Little Endian) of boot vector
 * @config_flags:	Optional Processor specific Config Flags set.
 * @control_flags:	Optional Processor specific Control Flags.
 * @status_flags:	Optional Processor specific Status Flags set.
 *
 * Response to TISCI_MSG_GET_PROC_BOOT_STATUS.
 */
struct ti_sci_msg_resp_get_proc_boot_status {
	struct ti_sci_msg_hdr hdr;
	uint8_t processor_id;
	uint32_t bootvector_low;
	uint32_t bootvector_high;
	uint32_t config_flags;
	uint32_t control_flags;
	uint32_t status_flags;
} __packed;

#endif /* TI_SCI_PROTOCOL_H */
