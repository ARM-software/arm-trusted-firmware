/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef ARMADA_COMMON_H
#define ARMADA_COMMON_H

#include <drivers/marvell/amb_adec.h>
#include <drivers/marvell/ccu.h>
#include <drivers/marvell/io_win.h>
#include <drivers/marvell/iob.h>

/*
 * This struct supports skip image request
 * detection_method: the method used to detect the request "signal".
 * info:
 *	GPIO:
 *		detection_method: HIGH (pressed button), LOW (unpressed button),
 *		num (button mpp number).
 *	i2c:
 *		i2c_addr: the address of the i2c chosen.
 *		i2d_reg: the i2c register chosen.
 *	test:
 *		choose the DIE you picked the button in (AP or CP).
 *		in case of CP(cp_index = 0 if CP0, cp_index = 1 if CP1)
 */
struct skip_image {
	enum {
		GPIO,
		I2C,
		USER_DEFINED
	} detection_method;

	struct {
		struct {
			int num;
			enum {
				HIGH,
				LOW
			} button_state;

		} gpio;

		struct {
			int i2c_addr;
			int i2c_reg;
		} i2c;

		struct {
			enum {
				CP,
				AP
			} cp_ap;
			int cp_index;
		} test;
	} info;
};

/*
 * This struct supports SoC power off method
 * type: the method used to power off the SoC
 * cfg:
 *	PMIC_GPIO:
 *	pin_count: current GPIO pin number used for toggling the signal for
 *		   notifying external PMIC
 *	info:	   holds the GPIOs information, CP GPIO should be used and
 *		   all GPIOs should be within same GPIO config. register
 *	step_count: current step number to toggle the GPIO for PMIC
 *	seq:       GPIO toggling values in sequence, each bit represents a GPIO.
 *		   For example, bit0 represents first GPIO used for toggling
 *		   the GPIO the last step is used to trigger the power off
 *		   signal
 *	delay_ms:  transition interval for the GPIO setting to take effect
 *		   in unit of ms
 */
/* Max GPIO number used to notify PMIC to power off the SoC */
#define PMIC_GPIO_MAX_NUMBER		8
/* Max GPIO toggling steps in sequence to power off the SoC */
#define PMIC_GPIO_MAX_TOGGLE_STEP	8

enum gpio_output_state {
	GPIO_LOW = 0,
	GPIO_HIGH
};

typedef struct gpio_info {
	int cp_index;
	int gpio_index;
} gpio_info_t;

struct power_off_method {
	enum {
		PMIC_GPIO,
	} type;

	struct {
		struct {
			int pin_count;
			struct gpio_info info[PMIC_GPIO_MAX_NUMBER];
			int step_count;
			uint32_t seq[PMIC_GPIO_MAX_TOGGLE_STEP];
			int delay_ms;
		} gpio;
	} cfg;
};

int marvell_gpio_config(void);
uint32_t marvell_get_io_win_gcr_target(int ap_idx);
uint32_t marvell_get_ccu_gcr_target(int ap_idx);


/*
 * The functions below are defined as Weak and may be overridden
 * in specific Marvell standard platform
 */
int marvell_get_amb_memory_map(struct addr_map_win **win,
			       uint32_t *size, uintptr_t base);
int marvell_get_io_win_memory_map(int ap_idx, struct addr_map_win **win,
				  uint32_t *size);
int marvell_get_iob_memory_map(struct addr_map_win **win,
			       uint32_t *size, uintptr_t base);
int marvell_get_ccu_memory_map(int ap_idx, struct addr_map_win **win,
			       uint32_t *size);
int system_power_off(void);

#endif /* ARMADA_COMMON_H */
