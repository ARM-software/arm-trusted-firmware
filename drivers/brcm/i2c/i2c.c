/*
 * Copyright (c) 2016 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <i2c.h>
#include <i2c_regs.h>
#include <lib/mmio.h>

#include <platform_def.h>

/* Max instances */
#define MAX_I2C					2U

/* Transaction error codes defined in Master command register (0x30) */
#define MSTR_STS_XACT_SUCCESS			0U
#define MSTR_STS_LOST_ARB			1U
#define MSTR_STS_NACK_FIRST_BYTE		2U
 /* NACK on a byte other than the first byte */
#define MSTR_STS_NACK_NON_FIRST_BYTE		3U

#define MSTR_STS_TTIMEOUT_EXCEEDED		4U
#define MSTR_STS_TX_TLOW_MEXT_EXCEEDED		5U
#define MSTR_STS_RX_TLOW_MEXT_EXCEEDED		6U

/* SMBUS protocol values defined in register 0x30 */
#define SMBUS_PROT_QUICK_CMD			0U
#define SMBUS_PROT_SEND_BYTE			1U
#define SMBUS_PROT_RECV_BYTE			2U
#define SMBUS_PROT_WR_BYTE			3U
#define SMBUS_PROT_RD_BYTE			4U
#define SMBUS_PROT_WR_WORD			5U
#define SMBUS_PROT_RD_WORD			6U
#define SMBUS_PROT_BLK_WR			7U
#define SMBUS_PROT_BLK_RD			8U
#define SMBUS_PROT_PROC_CALL			9U
#define SMBUS_PROT_BLK_WR_BLK_RD_PROC_CALL	10U

/* Number can be changed later */
#define BUS_BUSY_COUNT				100000U

#define IPROC_I2C_INVALID_ADDR			0xFFU

#define I2C_SMBUS_BLOCK_MAX			32U

/*
 * Enum to specify clock speed. The user will provide it during initialization.
 * If needed, it can be changed dynamically
 */
typedef enum iproc_smb_clk_freq {
	IPROC_SMB_SPEED_100KHz = 0,
	IPROC_SMB_SPEED_400KHz = 1,
	IPROC_SMB_SPEED_INVALID = 255
} smb_clk_freq_t;

/* Structure used to pass information to read/write functions. */
struct iproc_xact_info {
	/* Bus Identifier */
	uint32_t bus_id;
	/* Device Address */
	uint8_t devaddr;
	/* Passed by caller to send SMBus command cod e*/
	uint8_t command;
	/* actual data passed by the caller */
	uint8_t *data;
	/* Size of data buffer passed */
	uint32_t size;
	/* Sent by caller specifying PEC, 10-bit addresses */
	uint16_t flags;
	/* SMBus protocol to use to perform transaction */
	uint8_t smb_proto;
	/* true if command field below is valid. Otherwise, false */
	uint32_t cmd_valid;
};

static const uintptr_t smbus_base_reg_addr[MAX_I2C] = {
	SMBUS0_REGS_BASE,
	SMBUS1_REGS_BASE
};

/* Function to read a value from specified register. */
static uint32_t iproc_i2c_reg_read(uint32_t bus_id, unsigned long reg_addr)
{
	uint32_t val;
	uintptr_t smbus;

	smbus = smbus_base_reg_addr[bus_id];

	val = mmio_read_32(smbus + reg_addr);
	VERBOSE("i2c %u: reg %p read 0x%x\n", bus_id,
		(void *)(smbus + reg_addr), val);
	return val;
}

/* Function to write a value ('val') in to a specified register. */
static void iproc_i2c_reg_write(uint32_t bus_id,
				unsigned long reg_addr,
				uint32_t val)
{
	uintptr_t smbus;

	smbus = smbus_base_reg_addr[bus_id];

	mmio_write_32((smbus + reg_addr), val);
	VERBOSE("i2c %u: reg %p wrote 0x%x\n", bus_id,
		(void *)(smbus + reg_addr), val);
}

/* Function to clear and set bits in a specified register. */
static void iproc_i2c_reg_clearset(uint32_t bus_id,
				   unsigned long reg_addr,
				   uint32_t clear,
				   uint32_t set)
{
	uintptr_t smbus;

	smbus = smbus_base_reg_addr[bus_id];

	mmio_clrsetbits_32((smbus + reg_addr), clear, set);
	VERBOSE("i2c %u: reg %p clear 0x%x, set 0x%x\n", bus_id,
		(void *)(smbus + reg_addr), clear, set);
}

/* Function to dump all SMBUS register */
#ifdef BCM_I2C_DEBUG
static int iproc_dump_i2c_regs(uint32_t bus_id)
{
	uint32_t regval;

	if (bus_id > MAX_I2C) {
		return -1;
	}

	INFO("----------------------------------------------\n");
	INFO("%s: Dumping SMBus %u registers...\n", __func__, bus_id);

	regval = iproc_i2c_reg_read(bus_id, SMB_CFG_REG);
	INFO("SMB_CFG_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_TIMGCFG_REG);
	INFO("SMB_TIMGCFG_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_ADDR_REG);
	INFO("SMB_ADDR_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_MSTRFIFOCTL_REG);
	INFO("SMB_MSTRFIFOCTL_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_SLVFIFOCTL_REG);
	INFO("SMB_SLVFIFOCTL_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_BITBANGCTL_REG);
	INFO("SMB_BITBANGCTL_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_MSTRCMD_REG);
	INFO("SMB_MSTRCMD_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_SLVCMD_REG);
	INFO("SMB_SLVCMD_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_EVTEN_REG);
	INFO("SMB_EVTEN_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_EVTSTS_REG);
	INFO("SMB_EVTSTS_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_MSTRDATAWR_REG);
	INFO("SMB_MSTRDATAWR_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_MSTRDATARD_REG);
	INFO("SMB_MSTRDATARD_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_SLVDATAWR_REG);
	INFO("SMB_SLVDATAWR_REG=0x%x\n", regval);

	regval = iproc_i2c_reg_read(bus_id, SMB_SLVDATARD_REG);
	INFO("SMB_SLVDATARD_REG=0x%x\n", regval);

	INFO("----------------------------------------------\n");
	return 0;
}
#endif

/*
 * Function to ensure that the previous transaction was completed before
 * initiating a new transaction. It can also be used in polling mode to
 * check status of completion of a command
 */
static int iproc_i2c_startbusy_wait(uint32_t bus_id)
{
	uint32_t regval;
	uint32_t retry = 0U;

	/*
	 * Check if an operation is in progress. During probe it won't be.
	 * Want to make sure that the transaction in progress is completed.
	 */
	do {
		udelay(1U);
		regval = iproc_i2c_reg_read(bus_id, SMB_MSTRCMD_REG);
		regval &= SMB_MSTRSTARTBUSYCMD_MASK;
		if (retry++ > BUS_BUSY_COUNT) {
			ERROR("%s: START_BUSY bit didn't clear, exiting\n",
			      __func__);
			return -1;
		}

	} while (regval != 0U);

	return 0;
}

/*
 * This function copies data to SMBus's Tx FIFO. Valid for write transactions
 * info: Data to copy in to Tx FIFO. For read commands, the size should be
 * set to zero by the caller
 */
static void iproc_i2c_write_trans_data(struct iproc_xact_info *info)
{
	uint32_t regval;
	uint8_t devaddr;
	uint32_t i;
	uint32_t num_data_bytes = 0U;

#ifdef BCM_I2C_DEBUG
	INFO("%s:dev_addr=0x%x,cmd_valid=%d, cmd=0x%x, size=%u proto=%d\n",
	     __func__, info->devaddr, info->cmd_valid, info->command,
	     info->size, info->smb_proto);
#endif
	/* Shift devaddr by 1 bit since SMBus uses the low bit[0] for R/W_n */
	devaddr = (info->devaddr << 1);

	/*
	 * Depending on the SMBus protocol, we need to write additional
	 * transaction data in to Tx FIFO. Refer to section 5.5 of SMBus spec
	 * for sequence for a transaction
	 */
	switch (info->smb_proto) {
	case SMBUS_PROT_RECV_BYTE:
		/* No additional data to be written */
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    devaddr | 0x1U | SMB_MSTRWRSTS_MASK);
		break;
	case SMBUS_PROT_SEND_BYTE:
		num_data_bytes = info->size;
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    devaddr);
		break;
	case SMBUS_PROT_RD_BYTE:
	case SMBUS_PROT_RD_WORD:
	case SMBUS_PROT_BLK_RD:
		/* Write slave address with R/W~ set (bit #0) */
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    devaddr | 0x1U);
		break;
	case SMBUS_PROT_BLK_WR_BLK_RD_PROC_CALL:
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    devaddr | 0x1U | SMB_MSTRWRSTS_MASK);
		break;
	case SMBUS_PROT_WR_BYTE:
	case SMBUS_PROT_WR_WORD:
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    devaddr);
		/*
		 * No additional bytes to be written. Data portion is written
		 * in the 'for' loop below
		 */
		num_data_bytes = info->size;
		break;
	case SMBUS_PROT_BLK_WR:
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    devaddr);
		/* 3rd byte is byte count */
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    info->size);
		num_data_bytes = info->size;
		break;
	default:
		return;
	}

	/* If the protocol needs command code, copy it */
	if (info->cmd_valid) {
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    info->command);
	}

	/*
	 * Copy actual data from caller. In general, for reads,
	 * no data is copied.
	 */
	for (i = 0U; num_data_bytes; --num_data_bytes, i++) {
		/* For the last byte, set MASTER_WR_STATUS bit */
		regval = (num_data_bytes == 1U) ?
			 info->data[i] | SMB_MSTRWRSTS_MASK : info->data[i];
		iproc_i2c_reg_write(info->bus_id, SMB_MSTRDATAWR_REG,
				    regval);
	}
}

/*
 * This function writes to the master command register and
 * then polls for completion
 */
static int iproc_i2c_write_master_command(uint32_t mastercmd,
					  struct iproc_xact_info *info)
{
	uint32_t retry = 0U;
	uint32_t regval;

	iproc_i2c_reg_write(info->bus_id, SMB_MSTRCMD_REG, mastercmd);

	/* Check for Master Busy status */
	regval = iproc_i2c_reg_read(info->bus_id, SMB_MSTRCMD_REG);
	while ((regval & SMB_MSTRSTARTBUSYCMD_MASK) != 0U) {
		udelay(1U);
		if (retry++ > BUS_BUSY_COUNT) {
			ERROR("%s: START_BUSY bit didn't clear, exiting\n",
				__func__);
			return -1;
		}
		regval = iproc_i2c_reg_read(info->bus_id, SMB_MSTRCMD_REG);
	}

	/* If start_busy bit cleared, check if there are any errors */
	if (!(regval & SMB_MSTRSTARTBUSYCMD_MASK)) {
		/* start_busy bit cleared, check master_status field now */
		regval &= SMB_MSTRSTS_MASK;
		regval >>= SMB_MSTRSTS_SHIFT;
		if (regval != MSTR_STS_XACT_SUCCESS) {
			/* Error We can flush Tx FIFO here */
			ERROR("%s: ERROR: %u exiting\n", __func__, regval);
			return -1;
		}
	}
	return 0;

}
/* Function to initiate data send and verify completion status */
static int iproc_i2c_data_send(struct iproc_xact_info *info)
{
	int rc;
	uint32_t mastercmd;

	/* Make sure the previous transaction completed */
	rc = iproc_i2c_startbusy_wait(info->bus_id);

	if (rc < 0) {
		WARN("%s: Send: bus is busy, exiting\n", __func__);
		return rc;
	}
	/* Write transaction bytes to Tx FIFO */
	iproc_i2c_write_trans_data(info);

	/*
	 * Program master command register (0x30) with protocol type and set
	 * start_busy_command bit to initiate the write transaction
	 */
	mastercmd = (info->smb_proto << SMB_MSTRSMBUSPROTO_SHIFT) |
	    SMB_MSTRSTARTBUSYCMD_MASK;

	if (iproc_i2c_write_master_command(mastercmd, info)) {
		return -1;
	}

	return 0;
}

/*
 * Function to initiate data receive, verify completion status,
 * and read from SMBUS Read FIFO
 */
static int iproc_i2c_data_recv(struct iproc_xact_info *info,
			       uint32_t *num_bytes_read)
{
	int rc;
	uint32_t mastercmd;
	uint32_t regval;

	/* Make sure the previous transaction completed */
	rc = iproc_i2c_startbusy_wait(info->bus_id);

	if (rc < 0) {
		WARN("%s: Receive: Bus is busy, exiting\n", __func__);
		return rc;
	}

	/* Program all transaction bytes into master Tx FIFO */
	iproc_i2c_write_trans_data(info);

	/*
	 * Program master command register (0x30) with protocol type and set
	 * start_busy_command bit to initiate the write transaction
	 */
	mastercmd = (info->smb_proto << SMB_MSTRSMBUSPROTO_SHIFT) |
		     SMB_MSTRSTARTBUSYCMD_MASK | info->size;

	if (iproc_i2c_write_master_command(mastercmd, info)) {
		return -1;
	}

	/* Read received byte(s), after TX out address etc */
	regval = iproc_i2c_reg_read(info->bus_id, SMB_MSTRDATARD_REG);

	/* For block read, protocol (hw) returns byte count,as the first byte */
	if (info->smb_proto == SMBUS_PROT_BLK_RD) {
		uint32_t i;

		*num_bytes_read = regval & SMB_MSTRRDDATA_MASK;
		/*
		 * Limit to reading a max of 32 bytes only; just a safeguard.
		 * If # bytes read is a number > 32, check transaction set up,
		 * and contact hw engg.
		 * Assumption: PEC is disabled
		 */
		for (i = 0U; (i < *num_bytes_read) &&
		     (i < I2C_SMBUS_BLOCK_MAX); i++) {
			/* Read Rx FIFO for data bytes */
			regval = iproc_i2c_reg_read(info->bus_id,
						    SMB_MSTRDATARD_REG);
			info->data[i] = regval & SMB_MSTRRDDATA_MASK;
		}
	} else {
		/* 1 Byte data */
		*info->data = regval & SMB_MSTRRDDATA_MASK;
		*num_bytes_read = 1U;
	}

	return 0;
}

/*
 * This function set clock frequency for SMBus block. As per hardware
 * engineering, the clock frequency can be changed dynamically.
 */
static int iproc_i2c_set_clk_freq(uint32_t bus_id, smb_clk_freq_t freq)
{
	uint32_t val;

	switch (freq) {
	case IPROC_SMB_SPEED_100KHz:
		val = 0U;
		break;
	case IPROC_SMB_SPEED_400KHz:
		val = 1U;
		break;
	default:
		return -1;
	}

	iproc_i2c_reg_clearset(bus_id, SMB_TIMGCFG_REG,
			       SMB_TIMGCFG_MODE400_MASK,
			       val << SMB_TIMGCFG_MODE400_SHIFT);

	return 0;
}

/* Helper function to fill the iproc_xact_info structure */
static void iproc_i2c_fill_info(struct iproc_xact_info *info, uint32_t bus_id,
				uint8_t devaddr, uint8_t cmd, uint8_t *value,
				uint8_t smb_proto, uint32_t cmd_valid)
{
	info->bus_id = bus_id;
	info->devaddr = devaddr;
	info->command = (uint8_t)cmd;
	info->smb_proto = smb_proto;
	info->data = value;
	info->size = 1U;
	info->flags = 0U;
	info->cmd_valid = cmd_valid;
}

/* This function initializes the SMBUS */
static void iproc_i2c_init(uint32_t bus_id, int speed)
{
	uint32_t regval;

#ifdef BCM_I2C_DEBUG
	INFO("%s: Enter Init\n", __func__);
#endif

	/* Put controller in reset */
	regval = iproc_i2c_reg_read(bus_id, SMB_CFG_REG);
	regval |= BIT(SMB_CFG_RST_SHIFT);
	regval &= ~(BIT(SMB_CFG_SMBEN_SHIFT));
	iproc_i2c_reg_write(bus_id, SMB_CFG_REG, regval);

	/* Wait 100 usec per spec */
	udelay(100U);

	/* Bring controller out of reset */
	regval &= ~(BIT(SMB_CFG_RST_SHIFT));
	iproc_i2c_reg_write(bus_id, SMB_CFG_REG, regval);

	/*
	 * Flush Tx, Rx FIFOs. Note we are setting the Rx FIFO threshold to 0.
	 * May be OK since we are setting RX_EVENT and RX_FIFO_FULL interrupts
	 */
	regval = SMB_MSTRRXFIFOFLSH_MASK | SMB_MSTRTXFIFOFLSH_MASK;
	iproc_i2c_reg_write(bus_id, SMB_MSTRFIFOCTL_REG, regval);

	/*
	 * Enable SMbus block. Note, we are setting MASTER_RETRY_COUNT to zero
	 * since there will be only one master
	 */

	regval = iproc_i2c_reg_read(bus_id, SMB_CFG_REG);
	regval |= SMB_CFG_SMBEN_MASK;
	iproc_i2c_reg_write(bus_id, SMB_CFG_REG, regval);
	/* Wait a minimum of 50 Usec, as per SMB hw doc. But we wait longer */
	mdelay(10U);

	/* If error then set default speed */
	if (i2c_set_bus_speed(bus_id, speed)) {
		i2c_set_bus_speed(bus_id, I2C_SPEED_DEFAULT);
	}

	/* Disable intrs */
	regval = 0x0U;
	iproc_i2c_reg_write(bus_id, SMB_EVTEN_REG, regval);

	/* Clear intrs (W1TC) */
	regval = iproc_i2c_reg_read(bus_id, SMB_EVTSTS_REG);
	iproc_i2c_reg_write(bus_id, SMB_EVTSTS_REG, regval);

#ifdef BCM_I2C_DEBUG
	iproc_dump_i2c_regs(bus_id);

	INFO("%s: Exit Init Successfully\n", __func__);
#endif
}

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
int i2c_init(uint32_t bus_id, int speed)
{
	if (bus_id > MAX_I2C) {
		WARN("%s: Invalid Bus %u\n", __func__, bus_id);
		return -1;
	}

	iproc_i2c_init(bus_id, speed);
	return 0U;
}

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
int i2c_probe(uint32_t bus_id, uint8_t devaddr)
{
	uint32_t regval;
	int rc;

	/*
	 * i2c_init() Initializes internal regs, disable intrs (and then clear intrs),
	 * set fifo thresholds, etc.
	 * Shift devaddr by 1 bit since SMBus uses the low bit[0] for R/W_n
	 */
	regval = (devaddr << 1U);
	iproc_i2c_reg_write(bus_id, SMB_MSTRDATAWR_REG, regval);

	regval = ((SMBUS_PROT_QUICK_CMD << SMB_MSTRSMBUSPROTO_SHIFT) |
		  SMB_MSTRSTARTBUSYCMD_MASK);
	iproc_i2c_reg_write(bus_id, SMB_MSTRCMD_REG, regval);

	rc = iproc_i2c_startbusy_wait(bus_id);

	if (rc < 0) {
		WARN("%s: Probe: bus is busy, exiting\n", __func__);
		return rc;
	}

	regval = iproc_i2c_reg_read(bus_id, SMB_MSTRCMD_REG);
	if (((regval & SMB_MSTRSTS_MASK) >> SMB_MSTRSTS_SHIFT) == 0)
		VERBOSE("i2c device address: 0x%x\n", devaddr);
	else
		return -1;

#ifdef BCM_I2C_DEBUG
	iproc_dump_i2c_regs(bus_id);
#endif
	return 0;
}

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
int i2c_recv_byte(uint32_t bus_id, uint8_t devaddr, uint8_t *value)
{
	int rc;
	struct iproc_xact_info info;
	uint32_t num_bytes_read = 0;

	iproc_i2c_fill_info(&info, bus_id, devaddr, 0U, value,
			    SMBUS_PROT_RECV_BYTE, 0U);

	/* Refer to i2c_smbus_read_byte for params passed. */
	rc = iproc_i2c_data_recv(&info, &num_bytes_read);

	if (rc < 0) {
		printf("%s: %s error accessing device 0x%x\n",
		__func__, "Read", devaddr);
	}

	return rc;
}

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
int i2c_send_byte(uint32_t bus_id, uint8_t devaddr, uint8_t value)
{
	int rc;
	struct iproc_xact_info info;

	iproc_i2c_fill_info(&info, bus_id, devaddr, 0U, &value,
			    SMBUS_PROT_SEND_BYTE, 0U);

	/* Refer to i2c_smbus_write_byte params passed. */
	rc = iproc_i2c_data_send(&info);

	if (rc < 0) {
		ERROR("%s: %s error accessing device 0x%x\n",
		__func__, "Write", devaddr);
	}

	return rc;
}

/* Helper function to read a single byte */
static int i2c_read_byte(uint32_t bus_id,
			 uint8_t devaddr,
			 uint8_t regoffset,
			 uint8_t *value)
{
	int rc;
	struct iproc_xact_info info;
	uint32_t num_bytes_read = 0U;

	iproc_i2c_fill_info(&info, bus_id, devaddr, regoffset, value,
			    SMBUS_PROT_RD_BYTE, 1U);

	/* Refer to i2c_smbus_read_byte for params passed. */
	rc = iproc_i2c_data_recv(&info, &num_bytes_read);

	if (rc < 0) {
		ERROR("%s: %s error accessing device 0x%x\n",
		       __func__, "Read", devaddr);
	}
	return rc;
}

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
	     int len)
{
	uint32_t i;

	if (alen > 1) {
		WARN("I2C read: addr len %d not supported\n", alen);
		return -1;
	}

	if (addr + len > 256) {
		WARN("I2C read: address out of range\n");
		return -1;
	}

	for (i = 0U; i < len; i++) {
		if (i2c_read_byte(bus_id, devaddr, addr + i, &buffer[i])) {
			ERROR("I2C read: I/O error\n");
			iproc_i2c_init(bus_id, i2c_get_bus_speed(bus_id));
			return -1;
		}
	}

	return 0;
}

/* Helper function to write a single byte */
static int i2c_write_byte(uint32_t bus_id,
			  uint8_t devaddr,
			  uint8_t regoffset,
			  uint8_t value)
{
	int rc;
	struct iproc_xact_info info;

	iproc_i2c_fill_info(&info, bus_id, devaddr, regoffset, &value,
			    SMBUS_PROT_WR_BYTE, 1U);

	/* Refer to i2c_smbus_write_byte params passed. */
	rc = iproc_i2c_data_send(&info);

	if (rc < 0) {
		ERROR("%s: %s error accessing device 0x%x\n",
		       __func__, "Write", devaddr);
		return -1;
	}

	return 0;
}

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
	      int len)
{
	uint32_t i;

	if (alen > 1) {
		WARN("I2C write: addr len %d not supported\n", alen);
		return -1;
	}

	if (addr + len > 256U) {
		WARN("I2C write: address out of range\n");
		return -1;
	}

	for (i = 0U; i < len; i++) {
		if (i2c_write_byte(bus_id, devaddr, addr + i, buffer[i])) {
			ERROR("I2C write: I/O error\n");
			iproc_i2c_init(bus_id, i2c_get_bus_speed(bus_id));
			return -1;
		}
	}
	return 0;
}

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
int i2c_set_bus_speed(uint32_t bus_id, uint32_t speed)
{
	switch (speed) {
	case I2C_SPEED_100KHz:
		iproc_i2c_set_clk_freq(bus_id, IPROC_SMB_SPEED_100KHz);
		break;

	case I2C_SPEED_400KHz:
		iproc_i2c_set_clk_freq(bus_id, IPROC_SMB_SPEED_400KHz);
		break;

	default:
		return -1;
	}
	return 0;
}

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
uint32_t i2c_get_bus_speed(uint32_t bus_id)
{
	uint32_t regval;
	uint32_t retval = 0U;

	regval = iproc_i2c_reg_read(bus_id, SMB_TIMGCFG_REG);
	regval &= SMB_TIMGCFG_MODE400_MASK;
	regval >>= SMB_TIMGCFG_MODE400_SHIFT;

	switch (regval) {
	case IPROC_SMB_SPEED_100KHz:
		retval = I2C_SPEED_100KHz;
		break;

	case IPROC_SMB_SPEED_400KHz:
		retval = I2C_SPEED_400KHz;
		break;

	default:
		break;
	}
	return retval;
}

