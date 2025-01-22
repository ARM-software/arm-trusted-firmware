/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TISCI_PM_TISCI_CLOCK_H
#define TISCI_PM_TISCI_CLOCK_H

#include <cdefs.h>
#include <stddef.h>
#include <tisci/tisci_protocol.h>

/**
 * The IP does not require this clock, it can be disabled, regar`dless of the
 * state of the device
 */
#define TISCI_MSG_VALUE_CLOCK_SW_STATE_UNREQ	    0

/**
 * Allow the system controller to automatically manage the state of this clock.
 * If the device is enabled, then the clock is enabled. If the device is set to
 * off or retention, then the clock is internally set as not being required
 * by the device. This is the default state.
 */
#define TISCI_MSG_VALUE_CLOCK_SW_STATE_AUTO	    1

/** Configure the clock to be enabled, regardless of the state of the device. */
#define TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ	    2

/** Indicate hardware state of the clock is that it is not running. */
#define TISCI_MSG_VALUE_CLOCK_HW_STATE_NOT_READY    0

/** Indicate hardware state of the clock is that it is running. */

#define TISCI_MSG_VALUE_CLOCK_HW_STATE_READY	    1

/** Allow this clock to be modified via spread spectrum clocking.
 *  \note: The SSC feature is currently not supported in System Firmware.
 */
#define TISCI_MSG_FLAG_CLOCK_ALLOW_SSC		      BIT(8)

/**
 * Allow this clock's frequency to be changed while it is running
 * so long as it is within the min/max limits.
 */
#define TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE	      BIT(9)

/**
 * Enable input termination, this is only applicable to clock inputs
 * on the SoC pseudo-device, BOARD0.
 */
#define TISCI_MSG_FLAG_CLOCK_INPUT_TERM		      BIT(10)

/** Indicate that SSC is active for this clock.
 *  \note: The SSC feature is currently not supported in System Firmware.
 */
#define TISCI_MSG_FLAG_CLOCK_SSC_ACTIVE		      BIT(11)

/**
 * \brief Mark a clock as required/not required.
 *
 * Indicate that the selected clock is currently required/not required by
 * the IP.
 *
 * Certain flags can be set in the message header for device clocks:
 * TISCI_MSG_FLAG_CLOCK_ALLOW_SSC, TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE,
 * TISCI_MSG_FLAG_CLOCK_INPUT_TERM.
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to modify.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 *
 * \param state
 * The desired state of the clock, TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ if the clock is
 * currently required by the IP and TISCI_MSG_VALUE_CLOCK_SW_STATE_UNREQ if it is
 * not. TISCI_MSG_VALUE_CLOCK_SW_STATE_AUTO enables the clock when the IP is set
 * to enabled and disables it when the IP is set to disabled. This is the
 * default state.
 */
struct tisci_msg_set_clock_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint8_t			clk;
	uint8_t			state;
	uint32_t			clk32;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_SET_CLOCK
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_set_clock_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Get the current state of a clock
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to get.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 */
struct tisci_msg_get_clock_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint8_t			clk;
	uint32_t			clk32;
} __packed;

/**
 * \brief Clock state response.
 *
 * \param hdr TISCI header
 *
 * \param programmed_state
 * The programmed state as set by the set message.
 *
 * \param current_state
 * The actual state of the clock. If it is desired that a clock be on, it
 * is usually better to send a set message with a flag indicating that
 * an ack be sent when the message is processed rather than attempting
 * to poll this state.
 */
struct tisci_msg_get_clock_resp {
	struct tisci_header	hdr;
	uint8_t			programmed_state;
	uint8_t			current_state;
} __packed;

/**
 * \brief Set the clock parent
 *
 * Many IPs have a mux external to the IP that can select among different
 * clock sources. The clock must be disabled (TISCI_MSG_VALUE_CLOCK_SW_STATE_UNREQ)
 * for this message to succeed. If a set frequency command is not issued
 * before the clock is enabled again, then the execution of the enable
 * command will attempt to set the new parent to the old parent's
 * frequency. If this fails, then the enable will fail.
 *
 * Muxes that provide clocks to multiple devices are not currently
 * configurable via this API.
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to modify.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 *
 * \param parent
 * The new clock parent is selectable by an index via this parameter.
 *
 * If the parent clock index is 255 or greater, this field should be set to
 * 255 and the full value placed in the parent32 field. This is kept for
 * backwards compatibility with older firmwares.
 *
 * \param parent32
 * Stores the actual parent clock index if parent field is set to 255. This
 * field is ignored otherwise. This field can hold the full range of possible
 * parent clock indexes, but for compatibility with older firmwares should
 * only be used when the index is 255 or greater.
 */
struct tisci_msg_set_clock_parent_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint8_t			clk;
	uint8_t			parent;
	uint32_t			clk32;
	uint32_t			parent32;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_SET_CLOCK_PARENT
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_set_clock_parent_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Return the current clock parent
 *
 * If the hardware value indicating the current clock parent contains a
 * reserved value, a NAK is returned for this message. Once a valid parent
 * is programmed via a successful SET_CLOCK_PARENT call, calls to
 * GET_CLOCK_PARENT will succeed.
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to get.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 */
struct tisci_msg_get_clock_parent_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint8_t			clk;
	uint32_t			clk32;
} __packed;

/**
 * \brief Clock parent response
 *
 * \param hdr TISCI header
 *
 * \param parent
 * The current clock parent.
 *
 * If the parent clock index is 255 or greater, this field will be
 * set to 255 and the full value placed in the parent32 field. This is
 * for backwards compatibility with older firmwares.
 *
 * \param parent32
 * Parent Clock index if 255 or greater and parent field is set
 * to 255. This field will contain 0xFFFFFFFF otherwise.
 */
struct tisci_msg_get_clock_parent_resp {
	struct tisci_header	hdr;
	uint8_t			parent;
	uint32_t			parent32;
} __packed;

/**
 * \brief Return the number of possible parents for a clock
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to query.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 */
struct tisci_msg_get_num_clock_parents_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint8_t			clk;
	uint32_t			clk32;
} __packed;

/**
 * \brief Num clock parents response
 *
 * \param hdr TISCI header
 *
 * \param num_parents
 * The number of clock parents.
 *
 * If the number of clock parents is 255 or greater, this field will
 * be set to 255 and the full value placed in the num_parentint32_t field.
 * This is for backwards compatibility with older firmwares.
 *
 * \param num_parentint32_t
 * Number of clock parents if 255 or greater and num_parents field
 * is set to 255. This field will contain 0xFFFFFFFF otherwise.
 */
struct tisci_msg_get_num_clock_parents_resp {
	struct tisci_header	hdr;
	uint8_t			num_parents;
	uint32_t			num_parentint32_t;
} __packed;

/**
 * \brief Set the desired frequency for a clock.
 *
 * This set the desired frequency for a clock within an allowable
 * range. This message will fail on an enabled clock unless
 * TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE is set for the clock. Additionally,
 * if other clocks have their frequency modified due to this message,
 * they also must have the TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE or be
 * disabled.
 *
 * Calling set frequency on a clock input to the SoC psuedo-device will
 * inform the PMMC of that clock's frequency. Setting a frequency of
 * zero will indicate the clock is disabled.
 *
 * Calling set frequency on clock outputs from the SoC pseudo-device will
 * function similarly to setting the clock frequency on a device.
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param min_freq_hz
 * The minimum allowable frequency in Hz. This is the minimum allowable
 * programmed frequency and does not account for clock tolerances and jitter.
 *
 * \param target_freq_hz
 * The target clock frequency. The clock will be programmed at a rate as
 * close to this target frequency as possible.
 *
 * \param max_freq_hz
 * The maximum allowable frequency in Hz. This is the maximum allowable
 * programmed frequency and does not account for clock tolerances and jitter.
 * The firmware will actually accept any frequency up to but not including
 * max + 1.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to modify.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 */
struct tisci_msg_set_freq_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint64_t			min_freq_hz;
	uint64_t			target_freq_hz;
	uint64_t			max_freq_hz;
	uint8_t			clk;
	uint32_t			clk32;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_SET_FREQ
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_set_freq_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Determine the result of a hypothetical set frequency operation.
 *
 * This allows the OS to determine what rate would be set given a set
 * of parameters. A nack will be received if a frequency is not available
 * in the given range.
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param min_freq_hz
 * The minimum allowable frequency in Hz. This is the minimum allowable
 * programmed frequency and does not account for clock tolerances and jitter.
 *
 * \param target_freq_hz
 * The target clock frequency. A frequency will be found as close to the
 * target frequency as possible.
 *
 * \param max_freq_hz
 * The maximum allowable frequency in Hz. This is the maximum allowable
 * programmed frequency and does not account for clock tolerances and jitter.
 * The firmware will actually accept any frequency up to but not including
 * max + 1.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to query.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 */
struct tisci_msg_query_freq_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint64_t			min_freq_hz;
	uint64_t			target_freq_hz;
	uint64_t			max_freq_hz;
	uint8_t			clk;
	uint32_t			clk32;
} __packed;

/**
 * \brief Result of a query operation
 *
 * \param hdr TISCI header
 *
 * \param freq_hz
 * The frequency in Hz that the hardware would set for the given parameters.
 */
struct tisci_msg_query_freq_resp {
	struct tisci_header	hdr;
	uint64_t			freq_hz;
} __packed;

/**
 * \brief Get the current frequency of a device's clock
 *
 * This message will only succeed if the clock is currently enabled, otherwise
 * it returns nack.
 *
 * \param hdr TISCI header
 *
 * \param device
 * The device ID that the clock is connected to.
 *
 * \param clk
 * Each device has its own set of clock inputs. This indexes which clock
 * input to query.
 *
 * If the clock index is 255 or greater, this field should be set to 255
 * and the full value placed in the clk32 field. This is kept for backwards
 * compatibility with older firmwares.
 *
 * \param clk32
 * Stores the actual clock index if clk field is set to 255. This field is
 * ignored otherwise. This field can hold the full range of possible clock
 * indexes, but for compatibility with older firmwares should only be used
 * when the index is 255 or greater.
 */
struct tisci_msg_get_freq_req {
	struct tisci_header	hdr;
	uint32_t			device;
	uint8_t			clk;
	uint32_t			clk32;
} __packed;

/**
 * \brief Result of get frequency request
 *
 * \param hdr TISCI header
 *
 * \param freq_hz
 * The current frequency in Hz.
 */
struct tisci_msg_get_freq_resp {
	struct tisci_header	hdr;
	uint64_t			freq_hz;
} __packed;

#endif /* TISCI_PM_TISCI_CLOCK_H */
