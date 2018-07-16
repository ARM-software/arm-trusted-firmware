/*
 * Copyright (c) 2016-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STM32MP1_I2C_H
#define __STM32MP1_I2C_H

#include <stdint.h>
#include <utils_def.h>

/* Bit definition for I2C_CR1 register */
#define I2C_CR1_PE			BIT(0)
#define I2C_CR1_TXIE			BIT(1)
#define I2C_CR1_RXIE			BIT(2)
#define I2C_CR1_ADDRIE			BIT(3)
#define I2C_CR1_NACKIE			BIT(4)
#define I2C_CR1_STOPIE			BIT(5)
#define I2C_CR1_TCIE			BIT(6)
#define I2C_CR1_ERRIE			BIT(7)
#define I2C_CR1_DNF			GENMASK(11, 8)
#define I2C_CR1_ANFOFF			BIT(12)
#define I2C_CR1_SWRST			BIT(13)
#define I2C_CR1_TXDMAEN			BIT(14)
#define I2C_CR1_RXDMAEN			BIT(15)
#define I2C_CR1_SBC			BIT(16)
#define I2C_CR1_NOSTRETCH		BIT(17)
#define I2C_CR1_WUPEN			BIT(18)
#define I2C_CR1_GCEN			BIT(19)
#define I2C_CR1_SMBHEN			BIT(22)
#define I2C_CR1_SMBDEN			BIT(21)
#define I2C_CR1_ALERTEN			BIT(22)
#define I2C_CR1_PECEN			BIT(23)

/* Bit definition for I2C_CR2 register */
#define I2C_CR2_SADD			GENMASK(9, 0)
#define I2C_CR2_RD_WRN			BIT(10)
#define I2C_CR2_RD_WRN_OFFSET		10U
#define I2C_CR2_ADD10			BIT(11)
#define I2C_CR2_HEAD10R			BIT(12)
#define I2C_CR2_START			BIT(13)
#define I2C_CR2_STOP			BIT(14)
#define I2C_CR2_NACK			BIT(15)
#define I2C_CR2_NBYTES			GENMASK(23, 16)
#define I2C_CR2_NBYTES_OFFSET		16U
#define I2C_CR2_RELOAD			BIT(24)
#define I2C_CR2_AUTOEND			BIT(25)
#define I2C_CR2_PECBYTE			BIT(26)

/* Bit definition for I2C_OAR1 register */
#define I2C_OAR1_OA1			GENMASK(9, 0)
#define I2C_OAR1_OA1MODE		BIT(10)
#define I2C_OAR1_OA1EN			BIT(15)

/* Bit definition for I2C_OAR2 register */
#define I2C_OAR2_OA2			GENMASK(7, 1)
#define I2C_OAR2_OA2MSK			GENMASK(10, 8)
#define I2C_OAR2_OA2NOMASK		0
#define I2C_OAR2_OA2MASK01		BIT(8)
#define I2C_OAR2_OA2MASK02		BIT(9)
#define I2C_OAR2_OA2MASK03		GENMASK(9, 8)
#define I2C_OAR2_OA2MASK04		BIT(10)
#define I2C_OAR2_OA2MASK05		(BIT(8) | BIT(10))
#define I2C_OAR2_OA2MASK06		(BIT(9) | BIT(10))
#define I2C_OAR2_OA2MASK07		GENMASK(10, 8)
#define I2C_OAR2_OA2EN			BIT(15)

/* Bit definition for I2C_TIMINGR register */
#define I2C_TIMINGR_SCLL		GENMASK(7, 0)
#define I2C_TIMINGR_SCLH		GENMASK(15, 8)
#define I2C_TIMINGR_SDADEL		GENMASK(19, 16)
#define I2C_TIMINGR_SCLDEL		GENMASK(23, 20)
#define I2C_TIMINGR_PRESC		GENMASK(31, 28)

/* Bit definition for I2C_TIMEOUTR register */
#define I2C_TIMEOUTR_TIMEOUTA		GENMASK(11, 0)
#define I2C_TIMEOUTR_TIDLE		BIT(12)
#define I2C_TIMEOUTR_TIMOUTEN		BIT(15)
#define I2C_TIMEOUTR_TIMEOUTB		GENMASK(27, 16)
#define I2C_TIMEOUTR_TEXTEN		BIT(31)

/* Bit definition for I2C_ISR register */
#define I2C_ISR_TXE			BIT(0)
#define I2C_ISR_TXIS			BIT(1)
#define I2C_ISR_RXNE			BIT(2)
#define I2C_ISR_ADDR			BIT(3)
#define I2C_ISR_NACKF			BIT(4)
#define I2C_ISR_STOPF			BIT(5)
#define I2C_ISR_TC			BIT(6)
#define I2C_ISR_TCR			BIT(7)
#define I2C_ISR_BERR			BIT(8)
#define I2C_ISR_ARLO			BIT(9)
#define I2C_ISR_OVR			BIT(10)
#define I2C_ISR_PECERR			BIT(11)
#define I2C_ISR_TIMEOUT			BIT(12)
#define I2C_ISR_ALERT			BIT(13)
#define I2C_ISR_BUSY			BIT(15)
#define I2C_ISR_DIR			BIT(16)
#define I2C_ISR_ADDCODE			GENMASK(23, 17)

/* Bit definition for I2C_ICR register */
#define I2C_ICR_ADDRCF			BIT(3)
#define I2C_ICR_NACKCF			BIT(4)
#define I2C_ICR_STOPCF			BIT(5)
#define I2C_ICR_BERRCF			BIT(8)
#define I2C_ICR_ARLOCF			BIT(9)
#define I2C_ICR_OVRCF			BIT(10)
#define I2C_ICR_PECCF			BIT(11)
#define I2C_ICR_TIMOUTCF		BIT(12)
#define I2C_ICR_ALERTCF			BIT(13)

struct stm32_i2c_init_s {
	uint32_t timing;           /* Specifies the I2C_TIMINGR_register value
				    * This parameter is calculated by referring
				    * to I2C initialization section in Reference
				    * manual.
				    */

	uint32_t own_address1;     /* Specifies the first device own address.
				    * This parameter can be a 7-bit or 10-bit
				    * address.
				    */

	uint32_t addressing_mode;  /* Specifies if 7-bit or 10-bit addressing
				    * mode is selected.
				    * This parameter can be a value of @ref
				    * I2C_ADDRESSING_MODE.
				    */

	uint32_t dual_address_mode; /* Specifies if dual addressing mode is
				     * selected.
				     * This parameter can be a value of @ref
				     * I2C_DUAL_ADDRESSING_MODE.
				     */

	uint32_t own_address2;     /* Specifies the second device own address
				    * if dual addressing mode is selected.
				    * This parameter can be a 7-bit address.
				    */

	uint32_t own_address2_masks; /* Specifies the acknowledge mask address
				      * second device own address if dual
				      * addressing mode is selected.
				      * This parameter can be a value of @ref
				      * I2C_OWN_ADDRESS2_MASKS.
				      */

	uint32_t general_call_mode; /* Specifies if general call mode is
				     * selected.
				     * This parameter can be a value of @ref
				     * I2C_GENERAL_CALL_ADDRESSING_MODE.
				     */

	uint32_t no_stretch_mode;  /* Specifies if nostretch mode is
				    * selected.
				    * This parameter can be a value of @ref
				    * I2C_NOSTRETCH_MODE.
				    */

};

enum i2c_state_e {
	I2C_STATE_RESET          = 0x00U,   /* Peripheral is not yet
					     * initialized.
					     */
	I2C_STATE_READY          = 0x20U,   /* Peripheral Initialized
					     * and ready for use.
					     */
	I2C_STATE_BUSY           = 0x24U,   /* An internal process is
					     * ongoing.
					     */
	I2C_STATE_BUSY_TX        = 0x21U,   /* Data Transmission process
					     * is ongoing.
					     */
	I2C_STATE_BUSY_RX        = 0x22U,   /* Data Reception process
					     * is ongoing.
					     */
	I2C_STATE_LISTEN         = 0x28U,   /* Address Listen Mode is
					     * ongoing.
					     */
	I2C_STATE_BUSY_TX_LISTEN = 0x29U,   /* Address Listen Mode
					     * and Data Transmission
					     * process is ongoing.
					     */
	I2C_STATE_BUSY_RX_LISTEN = 0x2AU,   /* Address Listen Mode
					     * and Data Reception
					     * process is ongoing.
					     */
	I2C_STATE_ABORT          = 0x60U,   /* Abort user request ongoing. */
	I2C_STATE_TIMEOUT        = 0xA0U,   /* Timeout state. */
	I2C_STATE_ERROR          = 0xE0U    /* Error. */

};

enum i2c_mode_e {
	I2C_MODE_NONE   = 0x00U,   /* No I2C communication on going.       */
	I2C_MODE_MASTER = 0x10U,   /* I2C communication is in Master Mode. */
	I2C_MODE_SLAVE  = 0x20U,   /* I2C communication is in Slave Mode.  */
	I2C_MODE_MEM    = 0x40U    /* I2C communication is in Memory Mode. */

};

#define I2C_ERROR_NONE		0x00000000U	/* No error              */
#define I2C_ERROR_BERR		0x00000001U	/* BERR error            */
#define I2C_ERROR_ARLO		0x00000002U	/* ARLO error            */
#define I2C_ERROR_AF		0x00000004U	/* ACKF error            */
#define I2C_ERROR_OVR		0x00000008U	/* OVR error             */
#define I2C_ERROR_DMA		0x00000010U	/* DMA transfer error    */
#define I2C_ERROR_TIMEOUT	0x00000020U	/* Timeout error         */
#define I2C_ERROR_SIZE		0x00000040U	/* Size Management error */

struct i2c_handle_s {
	uint32_t i2c_base_addr;			/* Registers base address */

	struct stm32_i2c_init_s i2c_init;	/* Communication parameters */

	uint8_t *p_buff;			/* Pointer to transfer buffer */

	uint16_t xfer_size;			/* Transfer size */

	uint16_t xfer_count;			/* Transfer counter */

	uint32_t prev_state;			/* Communication previous
						 * state
						 */

	uint8_t lock;				/* Locking object */

	enum i2c_state_e i2c_state;		/* Communication state */

	enum i2c_mode_e i2c_mode;		/* Communication mode */

	uint32_t i2c_err;			/* Error code */
};

#define I2C_ADDRESSINGMODE_7BIT		0x00000001U
#define I2C_ADDRESSINGMODE_10BIT	0x00000002U

#define I2C_DUALADDRESS_DISABLE		0x00000000U
#define I2C_DUALADDRESS_ENABLE		I2C_OAR2_OA2EN

#define I2C_GENERALCALL_DISABLE		0x00000000U
#define I2C_GENERALCALL_ENABLE		I2C_CR1_GCEN

#define I2C_NOSTRETCH_DISABLE		0x00000000U
#define I2C_NOSTRETCH_ENABLE		I2C_CR1_NOSTRETCH

#define I2C_MEMADD_SIZE_8BIT		0x00000001U
#define I2C_MEMADD_SIZE_16BIT		0x00000002U

#define  I2C_RELOAD_MODE		I2C_CR2_RELOAD
#define  I2C_AUTOEND_MODE		I2C_CR2_AUTOEND
#define  I2C_SOFTEND_MODE		0x00000000U

#define  I2C_NO_STARTSTOP		0x00000000U
#define  I2C_GENERATE_STOP		(BIT(31) | I2C_CR2_STOP)
#define  I2C_GENERATE_START_READ	(BIT(31) | I2C_CR2_START | \
					 I2C_CR2_RD_WRN)
#define  I2C_GENERATE_START_WRITE	(BIT(31) | I2C_CR2_START)

#define I2C_FLAG_TXE			I2C_ISR_TXE
#define I2C_FLAG_TXIS			I2C_ISR_TXIS
#define I2C_FLAG_RXNE			I2C_ISR_RXNE
#define I2C_FLAG_ADDR			I2C_ISR_ADDR
#define I2C_FLAG_AF			I2C_ISR_NACKF
#define I2C_FLAG_STOPF			I2C_ISR_STOPF
#define I2C_FLAG_TC			I2C_ISR_TC
#define I2C_FLAG_TCR			I2C_ISR_TCR
#define I2C_FLAG_BERR			I2C_ISR_BERR
#define I2C_FLAG_ARLO			I2C_ISR_ARLO
#define I2C_FLAG_OVR			I2C_ISR_OVR
#define I2C_FLAG_PECERR			I2C_ISR_PECERR
#define I2C_FLAG_TIMEOUT		I2C_ISR_TIMEOUT
#define I2C_FLAG_ALERT			I2C_ISR_ALERT
#define I2C_FLAG_BUSY			I2C_ISR_BUSY
#define I2C_FLAG_DIR			I2C_ISR_DIR

#define I2C_RESET_CR2			(I2C_CR2_SADD | I2C_CR2_HEAD10R | \
					 I2C_CR2_NBYTES | I2C_CR2_RELOAD  | \
					 I2C_CR2_RD_WRN)

#define I2C_ANALOGFILTER_ENABLE		((uint32_t)0x00000000U)
#define I2C_ANALOGFILTER_DISABLE	I2C_CR1_ANFOFF

int stm32_i2c_init(struct i2c_handle_s *hi2c);

int stm32_i2c_mem_write(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			uint16_t mem_addr, uint16_t mem_add_size,
			uint8_t *p_data, uint16_t size, uint32_t timeout);
int stm32_i2c_mem_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
		       uint16_t mem_addr, uint16_t mem_add_size,
		       uint8_t *p_data, uint16_t size, uint32_t timeout);
int stm32_i2c_is_device_ready(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			      uint32_t trials, uint32_t timeout);

int stm32_i2c_config_analog_filter(struct i2c_handle_s *hi2c,
				   uint32_t analog_filter);

#endif /* __STM32MP1_I2C_H */
