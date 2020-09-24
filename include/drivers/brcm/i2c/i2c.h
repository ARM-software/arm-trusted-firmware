/*
 * Copyright (c) 2016 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define I2C_SPEED_100KHz	100000
#define I2C_SPEED_400KHz	400000
#define I2C_SPEED_DEFAULT	I2C_SPEED_100KHz

/*
 * Function Name:    i2c_probe
 *
 * Description:
 *	This function probes the I2C bus for the existence of the specified
 *	device.
 *
 * Parameters:
 *	bus_id  - I2C bus ID
 *	devaddr - Device Address
 *
 * Return:
 *	0 on success, or -1 on failure.
 */
int i2c_probe(uint32_t bus_id, uint8_t devaddr);

/*
 * Function Name:    i2c_init
 *
 * Description:
 *	This function initializes the SMBUS.
 *
 * Parameters:
 *	bus_id - I2C bus ID
 *	speed  - I2C bus speed in Hz
 *
 * Return:
 *	0 on success, or -1 on failure.
 */
int i2c_init(uint32_t bus_id, int speed);

/*
 * Function Name:    i2c_set_bus_speed
 *
 * Description:
 *	This function configures the SMBUS speed
 *
 * Parameters:
 *	bus_id - I2C bus ID
 *	speed  - I2C bus speed in Hz
 *
 * Return:
 *	0 on success, or -1 on failure.
 */
int i2c_set_bus_speed(uint32_t bus_id, uint32_t speed);

/*
 * Function Name:    i2c_get_bus_speed
 *
 * Description:
 *	This function returns the SMBUS speed.
 *
 * Parameters:
 *	bus_id - I2C bus ID
 *
 * Return:
 *	Bus speed in Hz, 0 on failure
 */
uint32_t i2c_get_bus_speed(uint32_t bus_id);

/*
 * Function Name:    i2c_recv_byte
 *
 * Description:
 *	This function reads I2C data from a device without specifying
 *	a command regsiter.
 *
 * Parameters:
 *	bus_id  - I2C bus ID
 *	devaddr - Device Address
 *	value   - Data Read
 *
 * Return:
 *	0 on success, or -1 on failure.
 */
int i2c_recv_byte(uint32_t bus_id, uint8_t devaddr, uint8_t *value);

/*
 * Function Name:    i2c_send_byte
 *
 * Description:
 *	This function send I2C data to a device without specifying
 *	a command regsiter.
 *
 * Parameters:
 *	bus_id  - I2C bus ID
 *	devaddr - Device Address
 *	value   - Data Send
 *
 * Return:
 *	0 on success, or -1 on failure.
 */
int i2c_send_byte(uint32_t bus_id, uint8_t devaddr, uint8_t value);

/*
 * Function Name:    i2c_read
 *
 * Description:
 *	This function reads I2C data from a device with a designated
 *	command register
 *
 * Parameters:
 *	bus_id  - I2C bus ID
 *	devaddr - Device Address
 *	addr    - Register Offset
 *	alen    - Address Length, 1 for byte, 2 for word (not supported)
 *	buffer  - Data Buffer
 *	len     - Data Length in bytes
 *
 * Return:
 *	0 on success, or -1 on failure.
 */
int i2c_read(uint32_t bus_id,
	     uint8_t devaddr,
	     uint32_t addr,
	     int alen,
	     uint8_t *buffer,
	     int len);

/*
 * Function Name:    i2c_write
 *
 * Description:
 *	This function write I2C data to a device with a designated
 *	command register
 *
 * Parameters:
 *	bus_id  - I2C bus ID
 *	devaddr - Device Address
 *	addr    - Register Offset
 *	alen    - Address Length, 1 for byte, 2 for word (not supported)
 *	buffer  - Data Buffer
 *	len     - Data Length in bytes
 *
 * Return:
 *	0 on success, or -1 on failure.
 */
int i2c_write(uint32_t bus_id,
	      uint8_t devaddr,
	      uint32_t addr,
	      int alen,
	      uint8_t *buffer,
	      int len);


#endif /* I2C_H */
