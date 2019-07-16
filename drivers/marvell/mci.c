/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* MCI bus driver for Marvell ARMADA 8K and 8K+ SoCs */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/marvell/mci.h>
#include <lib/mmio.h>

#include <mvebu.h>
#include <mvebu_def.h>
#include <plat_marvell.h>

/* /HB /Units /Direct_regs /Direct regs
 * /Configuration Register Write/Read Data Register
 */
#define MCI_WRITE_READ_DATA_REG(mci_index)	\
					MVEBU_MCI_REG_BASE_REMAP(mci_index)
/* /HB /Units /Direct_regs /Direct regs
 * /Configuration Register Access Command Register
 */
#define MCI_ACCESS_CMD_REG(mci_index)		\
				(MVEBU_MCI_REG_BASE_REMAP(mci_index) + 0x4)

/* Access Command fields :
 * bit[3:0]   - Sub command: 1 => Peripheral Config Register Read,
 *			     0 => Peripheral Config Register Write,
 *			     2 => Peripheral Assign ID request,
 *			     3 => Circular Config Write
 * bit[5]     - 1 => Local (same chip access) 0 => Remote
 * bit[15:8]  - Destination hop ID. Put Global ID (GID) here (see scheme below).
 * bit[23:22] - 0x3 IHB PHY REG address space, 0x0 IHB Controller space
 * bit[21:16] - Low 6 bits of offset. Hight 2 bits are taken from bit[28:27]
 *		of IHB_PHY_CTRL
 *		(must be set before any PHY register access occurs):
 *		/IHB_REG /IHB_REGInterchip Hopping Bus Registers
 *		/IHB Version Control Register
 *
 *		ixi_ihb_top		IHB PHY
 *  AXI -----------------------------   -------------
 *   <--| axi_hb_top | ihb_pipe_top |-->|           |
 *   -->|   GID=1    |     GID=0    |<--|           |
 *      -----------------------------   -------------
 */
#define MCI_INDIRECT_CTRL_READ_CMD		0x1
#define MCI_INDIRECT_CTRL_ASSIGN_CMD		0x2
#define MCI_INDIRECT_CTRL_CIRCULAR_CMD		0x3
#define MCI_INDIRECT_CTRL_LOCAL_PKT		(1 << 5)
#define MCI_INDIRECT_CTRL_CMD_DONE_OFFSET	6
#define MCI_INDIRECT_CTRL_CMD_DONE		\
				(1 << MCI_INDIRECT_CTRL_CMD_DONE_OFFSET)
#define MCI_INDIRECT_CTRL_DATA_READY_OFFSET	7
#define MCI_INDIRECT_CTRL_DATA_READY		\
				(1 << MCI_INDIRECT_CTRL_DATA_READY_OFFSET)
#define MCI_INDIRECT_CTRL_HOPID_OFFSET		8
#define MCI_INDIRECT_CTRL_HOPID(id)		\
			(((id) & 0xFF) << MCI_INDIRECT_CTRL_HOPID_OFFSET)
#define MCI_INDIRECT_CTRL_REG_CHIPID_OFFSET	16
#define MCI_INDIRECT_REG_CTRL_ADDR(reg_num)	\
			(reg_num << MCI_INDIRECT_CTRL_REG_CHIPID_OFFSET)

/* Hop ID values */
#define GID_IHB_PIPE					0
#define GID_AXI_HB					1
#define GID_IHB_EXT					2

#define MCI_DID_GLOBAL_ASSIGNMENT_REQUEST_REG		0x2
/* Target MCi Local ID (LID, which is = self DID) */
#define MCI_DID_GLOBAL_ASSIGN_REQ_MCI_LOCAL_ID(val)	(((val) & 0xFF) << 16)
/* Bits [15:8]: Number of MCis on chip of target MCi */
#define MCI_DID_GLOBAL_ASSIGN_REQ_MCI_COUNT(val)	(((val) & 0xFF) << 8)
/* Bits [7:0]: Number of hops on chip of target MCi */
#define MCI_DID_GLOBAL_ASSIGN_REQ_HOPS_NUM(val)		(((val) & 0xFF) << 0)

/* IHB_REG domain registers */
/* /HB /Units /IHB_REG /IHB_REGInterchip Hopping Bus Registers/
 * Rx Memory Configuration Register (RX_MEM_CFG)
 */
#define MCI_CTRL_RX_MEM_CFG_REG_NUM			0x0
#define MCI_CTRL_RX_TX_MEM_CFG_RQ_THRESH(val)		(((val) & 0xFF) << 24)
#define MCI_CTRL_RX_TX_MEM_CFG_PQ_THRESH(val)		(((val) & 0xFF) << 16)
#define MCI_CTRL_RX_TX_MEM_CFG_NQ_THRESH(val)		(((val) & 0xFF) << 8)
#define MCI_CTRL_RX_TX_MEM_CFG_DELTA_THRESH(val)	(((val) & 0xF) << 4)
#define MCI_CTRL_RX_TX_MEM_CFG_RTC(val)			(((val) & 0x3) << 2)
#define MCI_CTRL_RX_TX_MEM_CFG_WTC(val)			(((val) & 0x3) << 0)
#define MCI_CTRL_RX_MEM_CFG_REG_DEF_CP_VAL		\
				(MCI_CTRL_RX_TX_MEM_CFG_RQ_THRESH(0x07) | \
				MCI_CTRL_RX_TX_MEM_CFG_PQ_THRESH(0x3f) | \
				MCI_CTRL_RX_TX_MEM_CFG_NQ_THRESH(0x3f) | \
				MCI_CTRL_RX_TX_MEM_CFG_DELTA_THRESH(0xf) | \
				MCI_CTRL_RX_TX_MEM_CFG_RTC(1) | \
				MCI_CTRL_RX_TX_MEM_CFG_WTC(1))

#define MCI_CTRL_RX_MEM_CFG_REG_DEF_AP_VAL		\
				(MCI_CTRL_RX_TX_MEM_CFG_RQ_THRESH(0x3f) | \
				MCI_CTRL_RX_TX_MEM_CFG_PQ_THRESH(0x03) | \
				MCI_CTRL_RX_TX_MEM_CFG_NQ_THRESH(0x3f) | \
				MCI_CTRL_RX_TX_MEM_CFG_DELTA_THRESH(0xf) | \
				MCI_CTRL_RX_TX_MEM_CFG_RTC(1) | \
				MCI_CTRL_RX_TX_MEM_CFG_WTC(1))


/* /HB /Units /IHB_REG /IHB_REGInterchip Hopping Bus Registers/
 * Tx Memory Configuration Register (TX_MEM_CFG)
 */
#define MCI_CTRL_TX_MEM_CFG_REG_NUM			0x1
/* field mapping for TX mem config register
 * are the same as for RX register - see register above
 */
#define MCI_CTRL_TX_MEM_CFG_REG_DEF_VAL			\
				(MCI_CTRL_RX_TX_MEM_CFG_RQ_THRESH(0x20) | \
				MCI_CTRL_RX_TX_MEM_CFG_PQ_THRESH(0x20) | \
				MCI_CTRL_RX_TX_MEM_CFG_NQ_THRESH(0x20) | \
				MCI_CTRL_RX_TX_MEM_CFG_DELTA_THRESH(2) | \
				MCI_CTRL_RX_TX_MEM_CFG_RTC(1) | \
				MCI_CTRL_RX_TX_MEM_CFG_WTC(1))

/* /HB /Units /IHB_REG /IHB_REGInterchip Hopping Bus Registers
 * /IHB Link CRC Control
 */
/* MCi Link CRC Control Register (MCi_CRC_CTRL) */
#define MCI_LINK_CRC_CTRL_REG_NUM			0x4

/* /HB /Units /IHB_REG /IHB_REGInterchip Hopping Bus Registers
 * /IHB Status Register
 */
/* MCi Status Register (MCi_STS) */
#define MCI_CTRL_STATUS_REG_NUM				0x5
#define MCI_CTRL_STATUS_REG_PHY_READY			(1 << 12)
#define MCI_CTRL_STATUS_REG_LINK_PRESENT		(1 << 15)
#define MCI_CTRL_STATUS_REG_PHY_CID_VIO_OFFSET		24
#define MCI_CTRL_STATUS_REG_PHY_CID_VIO_MASK		\
				(0xF << MCI_CTRL_STATUS_REG_PHY_CID_VIO_OFFSET)
/* Expected successful Link result, including reserved bit */
#define MCI_CTRL_PHY_READY		(MCI_CTRL_STATUS_REG_PHY_READY | \
					MCI_CTRL_STATUS_REG_LINK_PRESENT | \
					MCI_CTRL_STATUS_REG_PHY_CID_VIO_MASK)

/* /HB /Units /IHB_REG /IHB_REGInterchip Hopping Bus Registers/
 * MCi PHY Speed Settings Register (MCi_PHY_SETTING)
 */
#define MCI_CTRL_MCI_PHY_SETTINGS_REG_NUM		0x8
#define MCI_CTRL_MCI_PHY_SET_DLO_FIFO_FULL_TRESH(val)	(((val) & 0xF) << 28)
#define MCI_CTRL_MCI_PHY_SET_PHY_MAX_SPEED(val)		(((val) & 0xF) << 12)
#define MCI_CTRL_MCI_PHY_SET_PHYCLK_SEL(val)		(((val) & 0xF) << 8)
#define MCI_CTRL_MCI_PHY_SET_REFCLK_FREQ_SEL(val)	(((val) & 0xF) << 4)
#define MCI_CTRL_MCI_PHY_SET_AUTO_LINK_EN(val)		(((val) & 0x1) << 1)
#define MCI_CTRL_MCI_PHY_SET_REG_DEF_VAL		\
			(MCI_CTRL_MCI_PHY_SET_DLO_FIFO_FULL_TRESH(0x3) | \
			MCI_CTRL_MCI_PHY_SET_PHY_MAX_SPEED(0x3) | \
			MCI_CTRL_MCI_PHY_SET_PHYCLK_SEL(0x2) | \
			MCI_CTRL_MCI_PHY_SET_REFCLK_FREQ_SEL(0x1))
#define MCI_CTRL_MCI_PHY_SET_REG_DEF_VAL2		\
			(MCI_CTRL_MCI_PHY_SET_DLO_FIFO_FULL_TRESH(0x3) | \
			MCI_CTRL_MCI_PHY_SET_PHY_MAX_SPEED(0x3) | \
			MCI_CTRL_MCI_PHY_SET_PHYCLK_SEL(0x5) | \
			MCI_CTRL_MCI_PHY_SET_REFCLK_FREQ_SEL(0x1))

/* /HB /Units /IHB_REG /IHB_REGInterchip Hopping Bus Registers
 * /IHB Mode Config
 */
#define MCI_CTRL_IHB_MODE_CFG_REG_NUM			0x25
#define MCI_CTRL_IHB_MODE_HBCLK_DIV(val)		((val) & 0xFF)
#define MCI_CTRL_IHB_MODE_CHUNK_MOD_OFFSET		8
#define MCI_CTRL_IHB_MODE_CHUNK_MOD			\
				(1 << MCI_CTRL_IHB_MODE_CHUNK_MOD_OFFSET)
#define MCI_CTRL_IHB_MODE_FWD_MOD_OFFSET		9
#define MCI_CTRL_IHB_MODE_FWD_MOD			\
				(1 << MCI_CTRL_IHB_MODE_FWD_MOD_OFFSET)
#define MCI_CTRL_IHB_MODE_SEQFF_FINE_MOD(val)		(((val) & 0xF) << 12)
#define MCI_CTRL_IHB_MODE_RX_COMB_THRESH(val)		(((val) & 0xFF) << 16)
#define MCI_CTRL_IHB_MODE_TX_COMB_THRESH(val)		(((val) & 0xFF) << 24)

#define MCI_CTRL_IHB_MODE_CFG_REG_DEF_VAL		\
				(MCI_CTRL_IHB_MODE_HBCLK_DIV(6) | \
				MCI_CTRL_IHB_MODE_FWD_MOD | \
				MCI_CTRL_IHB_MODE_SEQFF_FINE_MOD(0xF) | \
				MCI_CTRL_IHB_MODE_RX_COMB_THRESH(0x3f) | \
				MCI_CTRL_IHB_MODE_TX_COMB_THRESH(0x40))
/* AXI_HB registers */
#define MCI_AXI_ACCESS_DATA_REG_NUM			0x0
#define MCI_AXI_ACCESS_PCIE_MODE			1
#define MCI_AXI_ACCESS_CACHE_CHECK_OFFSET		5
#define MCI_AXI_ACCESS_CACHE_CHECK			\
				(1 << MCI_AXI_ACCESS_CACHE_CHECK_OFFSET)
#define MCI_AXI_ACCESS_FORCE_POST_WR_OFFSET		6
#define MCI_AXI_ACCESS_FORCE_POST_WR			\
				(1 << MCI_AXI_ACCESS_FORCE_POST_WR_OFFSET)
#define MCI_AXI_ACCESS_DISABLE_CLK_GATING_OFFSET	9
#define MCI_AXI_ACCESS_DISABLE_CLK_GATING		\
				(1 << MCI_AXI_ACCESS_DISABLE_CLK_GATING_OFFSET)

/* /HB /Units /HB_REG /HB_REGHopping Bus Registers
 * /Window 0 Address Mask Register
 */
#define MCI_HB_CTRL_WIN0_ADDRESS_MASK_REG_NUM		0x2

/* /HB /Units /HB_REG /HB_REGHopping Bus Registers
 * /Window 0 Destination Register
 */
#define MCI_HB_CTRL_WIN0_DESTINATION_REG_NUM		0x3
#define MCI_HB_CTRL_WIN0_DEST_VALID_FLAG(val)		(((val) & 0x1) << 16)
#define MCI_HB_CTRL_WIN0_DEST_ID(val)			(((val) & 0xFF) << 0)

/* /HB /Units /HB_REG /HB_REGHopping Bus Registers /Tx Control Register */
#define MCI_HB_CTRL_TX_CTRL_REG_NUM			0xD
#define MCI_HB_CTRL_TX_CTRL_PCIE_MODE_OFFSET		24
#define MCI_HB_CTRL_TX_CTRL_PCIE_MODE			\
				(1 << MCI_HB_CTRL_TX_CTRL_PCIE_MODE_OFFSET)
#define MCI_HB_CTRL_TX_CTRL_PRI_TH_QOS(val)		(((val) & 0xF) << 12)
#define MCI_HB_CTRL_TX_CTRL_MAX_RD_CNT(val)		(((val) & 0x1F) << 6)
#define MCI_HB_CTRL_TX_CTRL_MAX_WR_CNT(val)		(((val) & 0x1F) << 0)

/* /HB /Units /IHB_REG /IHB_REGInterchip Hopping Bus Registers
 * /IHB Version Control Register
 */
#define MCI_PHY_CTRL_REG_NUM				0x7
#define MCI_PHY_CTRL_MCI_MINOR				0x8 /* BITS [3:0] */
#define MCI_PHY_CTRL_MCI_MAJOR_OFFSET			4
#define MCI_PHY_CTRL_MCI_MAJOR				\
				(1 << MCI_PHY_CTRL_MCI_MAJOR_OFFSET)
#define MCI_PHY_CTRL_MCI_SLEEP_REQ_OFFSET		11
#define MCI_PHY_CTRL_MCI_SLEEP_REQ			\
				(1 << MCI_PHY_CTRL_MCI_SLEEP_REQ_OFFSET)
/* Host=1 / Device=0 PHY mode */
#define MCI_PHY_CTRL_MCI_PHY_MODE_OFFSET		24
#define MCI_PHY_CTRL_MCI_PHY_MODE_HOST			\
				(1 << MCI_PHY_CTRL_MCI_PHY_MODE_OFFSET)
/* Register=1 / PWM=0 interface */
#define MCI_PHY_CTRL_MCI_PHY_REG_IF_MODE_OFFSET		25
#define MCI_PHY_CTRL_MCI_PHY_REG_IF_MODE		\
				(1 << MCI_PHY_CTRL_MCI_PHY_REG_IF_MODE_OFFSET)
 /* PHY code InReset=1 */
#define MCI_PHY_CTRL_MCI_PHY_RESET_CORE_OFFSET		26
#define MCI_PHY_CTRL_MCI_PHY_RESET_CORE			\
				(1 << MCI_PHY_CTRL_MCI_PHY_RESET_CORE_OFFSET)
#define MCI_PHY_CTRL_PHY_ADDR_MSB_OFFSET		27
#define MCI_PHY_CTRL_PHY_ADDR_MSB(addr)			\
				(((addr) & 0x3) << \
				MCI_PHY_CTRL_PHY_ADDR_MSB_OFFSET)
#define MCI_PHY_CTRL_PIDI_MODE_OFFSET			31
#define MCI_PHY_CTRL_PIDI_MODE				\
				(1U << MCI_PHY_CTRL_PIDI_MODE_OFFSET)

/* Number of times to wait for the MCI link ready after MCI configurations
 * Normally takes 34-35 successive reads
 */
#define LINK_READY_TIMEOUT				100

enum mci_register_type {
	MCI_REG_TYPE_PHY = 0,
	MCI_REG_TYPE_CTRL,
};

enum {
	MCI_CMD_WRITE,
	MCI_CMD_READ
};

/* Write wrapper callback for debug:
 * will print written data in case LOG_LEVEL >= 40
 */
static void mci_mmio_write_32(uintptr_t addr, uint32_t value)
{
	VERBOSE("Write:\t0x%x = 0x%x\n", (uint32_t)addr, value);
	mmio_write_32(addr, value);
}
/* Read wrapper callback for debug:
 * will print read data in case LOG_LEVEL >= 40
 */
static uint32_t mci_mmio_read_32(uintptr_t addr)
{
	uint32_t value;

	value = mmio_read_32(addr);
	VERBOSE("Read:\t0x%x = 0x%x\n", (uint32_t)addr, value);
	return value;
}

/* MCI indirect access command completion polling:
 * Each write/read command done via MCI indirect registers must be polled
 * for command completions status.
 *
 * Returns 1 in case of error
 * Returns 0 in case of command completed successfully.
 */
static int mci_poll_command_completion(int mci_index, int command_type)
{
	uint32_t mci_cmd_value = 0, retry_count = 100, ret = 0;
	uint32_t completion_flags = MCI_INDIRECT_CTRL_CMD_DONE;

	debug_enter();
	/* Read commands require validating that requested data is ready */
	if (command_type == MCI_CMD_READ)
		completion_flags |= MCI_INDIRECT_CTRL_DATA_READY;

	do {
		/* wait 1 ms before each polling */
		mdelay(1);
		mci_cmd_value = mci_mmio_read_32(MCI_ACCESS_CMD_REG(mci_index));
	} while (((mci_cmd_value & completion_flags) != completion_flags) &&
			 (retry_count-- > 0));

	if (retry_count == 0) {
		ERROR("%s: MCI command timeout (command status = 0x%x)\n",
		      __func__, mci_cmd_value);
		ret = 1;
	}

	debug_exit();
	return ret;
}

int mci_read(int mci_idx, uint32_t cmd, uint32_t *value)
{
	int rval;

	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_idx), cmd);

	rval = mci_poll_command_completion(mci_idx, MCI_CMD_READ);

	*value = mci_mmio_read_32(MCI_WRITE_READ_DATA_REG(mci_idx));

	return rval;
}

int  mci_write(int mci_idx, uint32_t cmd, uint32_t data)
{
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_idx), data);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_idx), cmd);

	return mci_poll_command_completion(mci_idx, MCI_CMD_WRITE);
}

/* Perform 3 configurations in one command: PCI mode,
 * queues separation and cache bit
 */
static int mci_axi_set_pcie_mode(int mci_index)
{
	uint32_t reg_data, ret = 1;

	debug_enter();
	/* This configuration makes MCI IP behave consistently with AXI protocol
	 * It should be configured at one side only (for example locally at AP).
	 * The IP takes care of performing the same configurations at MCI on
	 * another side (for example remotely at CP).
	 */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_AXI_ACCESS_PCIE_MODE |
			  MCI_AXI_ACCESS_CACHE_CHECK |
			  MCI_AXI_ACCESS_FORCE_POST_WR |
			  MCI_AXI_ACCESS_DISABLE_CLK_GATING);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_AXI_ACCESS_DATA_REG_NUM)  |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT |
			  MCI_INDIRECT_CTRL_CIRCULAR_CMD);

	/* if Write command was successful, verify PCIe mode */
	if (mci_poll_command_completion(mci_index, MCI_CMD_WRITE) == 0) {
		/* Verify the PCIe mode selected */
		mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
				  MCI_INDIRECT_REG_CTRL_ADDR(
					MCI_HB_CTRL_TX_CTRL_REG_NUM)  |
				  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB) |
				  MCI_INDIRECT_CTRL_LOCAL_PKT |
				  MCI_INDIRECT_CTRL_READ_CMD);
		/* if read was completed, verify PCIe mode */
		if (mci_poll_command_completion(mci_index, MCI_CMD_READ) == 0) {
			reg_data = mci_mmio_read_32(
					MCI_WRITE_READ_DATA_REG(mci_index));
			if (reg_data & MCI_HB_CTRL_TX_CTRL_PCIE_MODE)
				ret = 0;
		}
	}

	debug_exit();
	return ret;
}

/* Reduce sequence FIFO timer expiration threshold */
static int mci_axi_set_fifo_thresh(int mci_index)
{
	uint32_t reg_data, ret = 0;

	debug_enter();
	/* This configuration reduces sequence FIFO timer expiration threshold
	 * (to 0x7 instead of 0xA).
	 * In MCI 1.6 version this configuration prevents possible functional
	 * issues.
	 * In version 1.82 the configuration prevents performance degradation
	 */

	/* Configure local AP side */
	reg_data = MCI_PHY_CTRL_PIDI_MODE |
		   MCI_PHY_CTRL_MCI_PHY_REG_IF_MODE |
		   MCI_PHY_CTRL_MCI_PHY_MODE_HOST |
		   MCI_PHY_CTRL_MCI_MAJOR |
		   MCI_PHY_CTRL_MCI_MINOR;
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index), reg_data);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(MCI_PHY_CTRL_REG_NUM) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* Reduce the threshold */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_IHB_MODE_CFG_REG_DEF_VAL);

	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_IHB_MODE_CFG_REG_NUM) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* Exit PIDI mode */
	reg_data = MCI_PHY_CTRL_MCI_PHY_REG_IF_MODE |
		   MCI_PHY_CTRL_MCI_PHY_MODE_HOST |
		   MCI_PHY_CTRL_MCI_MAJOR |
		   MCI_PHY_CTRL_MCI_MINOR;
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index), reg_data);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(MCI_PHY_CTRL_REG_NUM) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* Configure remote CP side */
	reg_data = MCI_PHY_CTRL_PIDI_MODE |
		   MCI_PHY_CTRL_MCI_MAJOR |
		   MCI_PHY_CTRL_MCI_MINOR |
		   MCI_PHY_CTRL_MCI_PHY_REG_IF_MODE;
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index), reg_data);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(MCI_PHY_CTRL_REG_NUM) |
			  MCI_CTRL_IHB_MODE_FWD_MOD);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* Reduce the threshold */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_IHB_MODE_CFG_REG_DEF_VAL);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_IHB_MODE_CFG_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_IHB_EXT));
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* Exit PIDI mode */
	reg_data = MCI_PHY_CTRL_MCI_MAJOR |
		   MCI_PHY_CTRL_MCI_MINOR |
		   MCI_PHY_CTRL_MCI_PHY_REG_IF_MODE;
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index), reg_data);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(MCI_PHY_CTRL_REG_NUM) |
			  MCI_CTRL_IHB_MODE_FWD_MOD);

	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	debug_exit();
	return ret;
}

/* Configure:
 * 1. AP & CP TX thresholds and delta configurations
 * 2. DLO & DLI FIFO full threshold
 * 3. RX thresholds and delta configurations
 * 4. CP AR and AW outstanding
 * 5. AP AR and AW outstanding
 */
static int mci_axi_set_fifo_rx_tx_thresh(int mci_index)
{
	uint32_t ret = 0;

	debug_enter();
	/* AP TX thresholds and delta configurations (IHB_reg 0x1) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_TX_MEM_CFG_REG_DEF_VAL);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_TX_MEM_CFG_REG_NUM) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* CP TX thresholds and delta configurations (IHB_reg 0x1) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_TX_MEM_CFG_REG_DEF_VAL);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_TX_MEM_CFG_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_IHB_EXT));
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* AP DLO & DLI FIFO full threshold & Auto-Link enable (IHB_reg 0x8) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_MCI_PHY_SET_REG_DEF_VAL |
			  MCI_CTRL_MCI_PHY_SET_AUTO_LINK_EN(1));
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_MCI_PHY_SETTINGS_REG_NUM) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* CP DLO & DLI FIFO full threshold (IHB_reg 0x8) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_MCI_PHY_SET_REG_DEF_VAL);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_MCI_PHY_SETTINGS_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_IHB_EXT));
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* AP RX thresholds and delta configurations (IHB_reg 0x0) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_RX_MEM_CFG_REG_DEF_AP_VAL);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_RX_MEM_CFG_REG_NUM) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* CP RX thresholds and delta configurations (IHB_reg 0x0) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_CTRL_RX_MEM_CFG_REG_DEF_CP_VAL);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_CTRL_RX_MEM_CFG_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_IHB_EXT));
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* AP AR & AW maximum AXI outstanding request cfg (HB_reg 0xd) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_HB_CTRL_TX_CTRL_PRI_TH_QOS(8) |
			  MCI_HB_CTRL_TX_CTRL_MAX_RD_CNT(3) |
			  MCI_HB_CTRL_TX_CTRL_MAX_WR_CNT(3));
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_HB_CTRL_TX_CTRL_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* CP AR & AW maximum AXI outstanding request cfg (HB_reg 0xd) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(mci_index),
			  MCI_HB_CTRL_TX_CTRL_PRI_TH_QOS(8) |
			  MCI_HB_CTRL_TX_CTRL_MAX_RD_CNT(0xB) |
			  MCI_HB_CTRL_TX_CTRL_MAX_WR_CNT(0x11));
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(mci_index),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_HB_CTRL_TX_CTRL_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_IHB_EXT) |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB));
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	debug_exit();
	return ret;
}

/* configure MCI to allow read & write transactions to arrive at the same time.
 * Without the below configuration, MCI won't sent response to CPU for
 * transactions which arrived simultaneously and will lead to CPU hang.
 * The below will configure MCI to be able to pass transactions from/to CP/AP.
 */
static int mci_enable_simultaneous_transactions(int mci_index)
{
	uint32_t ret = 0;

	debug_enter();
	/* ID assignment (assigning global ID offset to CP) */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(0),
			  MCI_DID_GLOBAL_ASSIGN_REQ_MCI_LOCAL_ID(2) |
			  MCI_DID_GLOBAL_ASSIGN_REQ_MCI_COUNT(2) |
			  MCI_DID_GLOBAL_ASSIGN_REQ_HOPS_NUM(2));
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(0),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_DID_GLOBAL_ASSIGNMENT_REQUEST_REG) |
			  MCI_INDIRECT_CTRL_ASSIGN_CMD);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* Assigning dest. ID=3 to all transactions entering from AXI at AP */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(0),
			  MCI_HB_CTRL_WIN0_DEST_VALID_FLAG(1) |
			  MCI_HB_CTRL_WIN0_DEST_ID(3));
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(0),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_HB_CTRL_WIN0_DESTINATION_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* Assigning dest. ID=1 to all transactions entering from AXI at CP */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(0),
			  MCI_HB_CTRL_WIN0_DEST_VALID_FLAG(1) |
			  MCI_HB_CTRL_WIN0_DEST_ID(1));
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(0),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_HB_CTRL_WIN0_DESTINATION_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_IHB_EXT) |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB));
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* End address to all transactions entering from AXI at AP.
	 * This will lead to get match for any AXI address
	 * and receive destination ID=3
	 */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(0), 0xffffffff);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(0),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_HB_CTRL_WIN0_ADDRESS_MASK_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT);
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	/* End address to all transactions entering from AXI at CP.
	 * This will lead to get match for any AXI address
	 * and receive destination ID=1
	 */
	mci_mmio_write_32(MCI_WRITE_READ_DATA_REG(0), 0xffffffff);
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(0),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_HB_CTRL_WIN0_ADDRESS_MASK_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_IHB_EXT) |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB));
	ret |= mci_poll_command_completion(mci_index, MCI_CMD_WRITE);

	debug_exit();
	return ret;
}

/* Check if MCI simultaneous transaction was already enabled.
 * Currently bootrom does this mci configuration only when the boot source is
 * SAR_MCIX4, in other cases it should be done at this stage.
 * It is worth noticing that in case of booting from uart, the bootrom
 * flow is different and this mci initialization is skipped even if boot
 * source is SAR_MCIX4. Therefore new verification bases on appropriate mci's
 * register content: if the appropriate reg contains 0x0 it means that the
 * bootrom didn't perform required mci configuration.
 *
 * Returns:
 * 0 - configuration already done
 * 1 - configuration missing
 */
static _Bool mci_simulatenous_trans_missing(int mci_index)
{
	uint32_t reg, ret;

	/* read 'Window 0 Destination ID assignment' from HB register 0x3
	 * (TX_CFG_W0_DST_ID) to check whether ID assignment was already
	 * performed by BootROM.
	 */
	debug_enter();
	mci_mmio_write_32(MCI_ACCESS_CMD_REG(0),
			  MCI_INDIRECT_REG_CTRL_ADDR(
				MCI_HB_CTRL_WIN0_DESTINATION_REG_NUM) |
			  MCI_INDIRECT_CTRL_HOPID(GID_AXI_HB) |
			  MCI_INDIRECT_CTRL_LOCAL_PKT |
			  MCI_INDIRECT_CTRL_READ_CMD);
	ret = mci_poll_command_completion(mci_index, MCI_CMD_READ);

	reg = mci_mmio_read_32(MCI_WRITE_READ_DATA_REG(mci_index));

	if (ret)
		ERROR("Failed to verify MCI simultaneous read/write status\n");

	debug_exit();
	/* default ID assignment is 0, so if register doesn't contain zeros
	 * it means that bootrom already performed required configuration.
	 */
	if (reg != 0)
		return 0;

	return 1;
}

/* For A1 revision, configure the MCI link for performance improvement:
 * - set MCI to support read/write transactions to arrive at the same time
 * - Switch AXI to PCIe mode
 * - Reduce sequence FIFO threshold
 * - Configure RX/TX FIFO thresholds
 *
 *   Note:
 *   We don't exit on error code from any sub routine, to try (best effort) to
 *   complete the MCI configuration.
 *   (If we exit - Bootloader will surely fail to boot)
 */
int mci_configure(int mci_index)
{
	int rval;

	debug_enter();
	/* According to design guidelines the MCI simultaneous transaction
	 * shouldn't be enabled more then once - therefore make sure that it
	 * wasn't already enabled in bootrom.
	 */
	if (mci_simulatenous_trans_missing(mci_index)) {
		VERBOSE("Enabling MCI simultaneous transaction\n");
		/* set MCI to support read/write transactions
		 * to arrive at the same time
		 */
		rval = mci_enable_simultaneous_transactions(mci_index);
		if (rval)
			ERROR("Failed to set MCI simultaneous read/write\n");
	} else
		VERBOSE("Skip MCI ID assignment - already done by bootrom\n");

	/* Configure MCI for more consistent behavior with AXI protocol */
	rval = mci_axi_set_pcie_mode(mci_index);
	if (rval)
		ERROR("Failed to set MCI to AXI PCIe mode\n");

	/* reduce FIFO global threshold */
	rval = mci_axi_set_fifo_thresh(mci_index);
	if (rval)
		ERROR("Failed to set MCI FIFO global threshold\n");

	/* configure RX/TX FIFO thresholds */
	rval = mci_axi_set_fifo_rx_tx_thresh(mci_index);
	if (rval)
		ERROR("Failed to set MCI RX/TX FIFO threshold\n");

	debug_exit();
	return 1;
}

int mci_get_link_status(void)
{
	uint32_t cmd, data;

	cmd = (MCI_INDIRECT_REG_CTRL_ADDR(MCI_CTRL_STATUS_REG_NUM) |
		MCI_INDIRECT_CTRL_LOCAL_PKT | MCI_INDIRECT_CTRL_READ_CMD);
	if (mci_read(0, cmd, &data)) {
		ERROR("Failed to read status register\n");
		return -1;
	}

	/* Check if the link is ready */
	if (data != MCI_CTRL_PHY_READY) {
		ERROR("Bad link status %x\n", data);
		return -1;
	}

	return 0;
}

void mci_turn_link_down(void)
{
	uint32_t cmd, data;
	int rval = 0;

	debug_enter();

	/* Turn off auto-link */
	cmd = (MCI_INDIRECT_REG_CTRL_ADDR(MCI_CTRL_MCI_PHY_SETTINGS_REG_NUM) |
			MCI_INDIRECT_CTRL_LOCAL_PKT);
	data = (MCI_CTRL_MCI_PHY_SET_REG_DEF_VAL2 |
		MCI_CTRL_MCI_PHY_SET_AUTO_LINK_EN(0));
	rval = mci_write(0, cmd, data);
	if (rval)
		ERROR("Failed to turn off auto-link\n");

	/* Reset AP PHY */
	cmd = (MCI_INDIRECT_REG_CTRL_ADDR(MCI_PHY_CTRL_REG_NUM) |
		MCI_INDIRECT_CTRL_LOCAL_PKT);
	data = (MCI_PHY_CTRL_MCI_MINOR |
		MCI_PHY_CTRL_MCI_MAJOR |
		MCI_PHY_CTRL_MCI_PHY_MODE_HOST |
		MCI_PHY_CTRL_MCI_PHY_RESET_CORE);
	rval = mci_write(0, cmd, data);
	if (rval)
		ERROR("Failed to reset AP PHY\n");

	/* Clear all status & CRC values */
	cmd = (MCI_INDIRECT_REG_CTRL_ADDR(MCI_LINK_CRC_CTRL_REG_NUM) |
	       MCI_INDIRECT_CTRL_LOCAL_PKT);
	data = 0x0;
	mci_write(0, cmd, data);
	cmd = (MCI_INDIRECT_REG_CTRL_ADDR(MCI_CTRL_STATUS_REG_NUM) |
	       MCI_INDIRECT_CTRL_LOCAL_PKT);
	data = 0x0;
	rval = mci_write(0, cmd, data);
	if (rval)
		ERROR("Failed to reset AP PHY\n");

	/* Wait 5ms before un-reset the PHY */
	mdelay(5);

	/* Un-reset AP PHY */
	cmd = (MCI_INDIRECT_REG_CTRL_ADDR(MCI_PHY_CTRL_REG_NUM) |
	       MCI_INDIRECT_CTRL_LOCAL_PKT);
	data = (MCI_PHY_CTRL_MCI_MINOR | MCI_PHY_CTRL_MCI_MAJOR |
		MCI_PHY_CTRL_MCI_PHY_MODE_HOST);
	rval = mci_write(0, cmd, data);
	if (rval)
		ERROR("Failed to un-reset AP PHY\n");

	debug_exit();
}

void mci_turn_link_on(void)
{
	uint32_t cmd, data;
	int rval = 0;

	debug_enter();
	/* Turn on auto-link */
	cmd = (MCI_INDIRECT_REG_CTRL_ADDR(MCI_CTRL_MCI_PHY_SETTINGS_REG_NUM) |
			MCI_INDIRECT_CTRL_LOCAL_PKT);
	data = (MCI_CTRL_MCI_PHY_SET_REG_DEF_VAL2 |
		MCI_CTRL_MCI_PHY_SET_AUTO_LINK_EN(1));
	rval = mci_write(0, cmd, data);
	if (rval)
		ERROR("Failed to turn on auto-link\n");

	debug_exit();
}

/* Initialize MCI for performance improvements */
int mci_initialize(int mci_index)
{
	int ret;

	debug_enter();
	INFO("MCI%d initialization:\n", mci_index);

	ret = mci_configure(mci_index);

	debug_exit();
	return ret;
}
