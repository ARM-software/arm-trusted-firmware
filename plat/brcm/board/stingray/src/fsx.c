/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/common_def.h>

#include <fsx.h>
#include <platform_def.h>
#include <sr_utils.h>

#define FS4_IDM_IO_CONTROL_DIRECT__SRAM_CLK_EN		0

#define FS4_IDM_IO_CONTROL_DIRECT__MEM_POWERON		11
#define FS4_IDM_IO_CONTROL_DIRECT__MEM_POWEROK		12
#define FS4_IDM_IO_CONTROL_DIRECT__MEM_ARRPOWERON	13
#define FS4_IDM_IO_CONTROL_DIRECT__MEM_ARRPOWEROK	14
#define FS4_IDM_IO_CONTROL_DIRECT__MEM_ISO		15
#define FS4_IDM_IO_CONTROL_DIRECT__CLK_EN		31

#define FS4_IDM_IO_STATUS__MEM_POWERON			0
#define FS4_IDM_IO_STATUS__MEM_POWEROK			1
#define FS4_IDM_IO_STATUS__MEM_ARRPOWERON		2
#define FS4_IDM_IO_STATUS__MEM_ARRPOWEROK		3
#define FS4_IDM_IO_STATUS__MEM_ALLOK			0xf

#define FS4_IDM_RESET_CONTROL__RESET			0

#define FSX_RINGx_BASE(__b, __i)			\
		((__b) + (__i) * 0x10000)

#define FSX_RINGx_VERSION_NUMBER(__b, __i)		\
		(FSX_RINGx_BASE(__b, __i) + 0x0)

#define FSX_RINGx_MSI_DEV_ID(__b, __i)			\
		(FSX_RINGx_BASE(__b, __i) + 0x44)

#define FSX_COMM_RINGx_BASE(__b, __i)			\
		((__b) + 0x200000 + (__i) * 0x100)

#define FSX_COMM_RINGx_CONTROL(__b, __i)		\
		(FSX_COMM_RINGx_BASE(__b, __i) + 0x0)
#define FSX_COMM_RINGx_CONTROL__AXI_ID			8
#define FSX_COMM_RINGx_CONTROL__AXI_ID_MASK		0x1f
#define FSX_COMM_RINGx_CONTROL__PRIORITY		4
#define FSX_COMM_RINGx_CONTROL__PRIORITY_MASK		0x7
#define FSX_COMM_RINGx_CONTROL__AE_GROUP		0
#define FSX_COMM_RINGx_CONTROL__AE_GROUP_MASK		0x7

#define FSX_COMM_RINGx_MSI_DEV_ID(__b, __i)		\
		(FSX_COMM_RINGx_BASE(__b, __i) + 0x4)

#define FSX_AEx_BASE(__b, __i)				\
		((__b) + 0x202000 + (__i) * 0x100)

#define FSX_AEx_CONTROL_REGISTER(__b, __i)		\
		(FSX_AEx_BASE(__b, __i) + 0x0)
#define FSX_AEx_CONTROL_REGISTER__ACTIVE		4
#define FSX_AEx_CONTROL_REGISTER__GROUP_ID		0
#define FSX_AEx_CONTROL_REGISTER__GROUP_ID_MASK		0x7

#define FSX_COMM_RM_RING_SECURITY_SETTING		0x0

#define FSX_COMM_RM_SSID_CONTROL			0x4
#define FSX_COMM_RM_SSID_CONTROL__RING_BITS		5
#define FSX_COMM_RM_SSID_CONTROL__MASK			0x3ff

#define FSX_COMM_RM_CONTROL_REGISTER			0x8
#define FSX_COMM_RM_CONTROL_REGISTER__CONFIG_DONE	2
#define FSX_COMM_RM_CONTROL_REGISTER__AE_TIMEOUT	5
#define FSX_COMM_RM_CONTROL_REGISTER__AE_LOCKING	7

#define FSX_COMM_RM_TIMER_CONTROL_0			0xc
#define FSX_COMM_RM_TIMER_CONTROL_0__FAST		16
#define FSX_COMM_RM_TIMER_CONTROL_0__MEDIUM		0

#define FSX_COMM_RM_TIMER_CONTROL_1			0x10
#define FSX_COMM_RM_TIMER_CONTROL_1__SLOW		16
#define FSX_COMM_RM_TIMER_CONTROL_1__IDLE		0

#define FSX_COMM_RM_BURST_BD_THRESHOLD			0x14
#define FSX_COMM_RM_BURST_BD_THRESHOLD_LOW		0
#define FSX_COMM_RM_BURST_BD_THRESHOLD_HIGH		16

#define FSX_COMM_RM_BURST_LENGTH			0x18
#define FSX_COMM_RM_BURST_LENGTH__FOR_DDR_ADDR_GEN	16
#define FSX_COMM_RM_BURST_LENGTH__FOR_DDR_ADDR_GEN_MASK	0x1ff
#define FSX_COMM_RM_BURST_LENGTH__FOR_TOGGLE		0
#define FSX_COMM_RM_BURST_LENGTH__FOR_TOGGLE_MASK	0x1ff

#define FSX_COMM_RM_FIFO_THRESHOLD			0x1c
#define FSX_COMM_RM_FIFO_THRESHOLD__BD_FIFO_FULL	16
#define FSX_COMM_RM_FIFO_THRESHOLD__BD_FIFO_FULL_MASK	0x1ff
#define FSX_COMM_RM_FIFO_THRESHOLD__AE_FIFO_FULL	0
#define FSX_COMM_RM_FIFO_THRESHOLD__AE_FIFO_FULL_MASK	0x1f

#define FSX_COMM_RM_AE_TIMEOUT				0x24

#define FSX_COMM_RM_RING_FLUSH_TIMEOUT			0x2c

#define FSX_COMM_RM_MEMORY_CONFIGURATION		0x30
#define FSX_COMM_RM_MEMORY_CONFIGURATION__ARRPOWERONIN	12
#define FSX_COMM_RM_MEMORY_CONFIGURATION__ARRPOWEROKIN	13
#define FSX_COMM_RM_MEMORY_CONFIGURATION__POWERONIN	14
#define FSX_COMM_RM_MEMORY_CONFIGURATION__POWEROKIN	15

#define FSX_COMM_RM_AXI_CONTROL				0x34
#define FSX_COMM_RM_AXI_CONTROL__WRITE_CHANNEL_EN	28
#define FSX_COMM_RM_AXI_CONTROL__READ_CHANNEL_EN	24
#define FSX_COMM_RM_AXI_CONTROL__AWQOS			20
#define FSX_COMM_RM_AXI_CONTROL__ARQOS			16
#define FSX_COMM_RM_AXI_CONTROL__AWPROT			12
#define FSX_COMM_RM_AXI_CONTROL__ARPROT			8
#define FSX_COMM_RM_AXI_CONTROL__AWCACHE		4
#define FSX_COMM_RM_AXI_CONTROL__ARCACHE		0

#define FSX_COMM_RM_CONFIG_INTERRUPT_STATUS_CLEAR	0x48

#define FSX_COMM_RM_GROUP_PKT_EXTENSION_SUPPORT		0xc0

#define FSX_COMM_RM_AXI_READ_BURST_THRESHOLD		0xc8
#define FSX_COMM_RM_AXI_READ_BURST_THRESHOLD__MASK	0x1ff
#define FSX_COMM_RM_AXI_READ_BURST_THRESHOLD__MAX	16
#define FSX_COMM_RM_AXI_READ_BURST_THRESHOLD__MIN	0

#define FSX_COMM_RM_GROUP_RING_COUNT			0xcc

#define FSX_COMM_RM_MAIN_HW_INIT_DONE			0x12c
#define FSX_COMM_RM_MAIN_HW_INIT_DONE__MASK		0x1

#define FSX_DMEx_BASE(__b, __i)				\
		((__b) + (__i) * 0x1000)

#define FSX_DMEx_AXI_CONTROL(__b, __i)			\
		(FSX_DMEx_BASE(__b, __i) + 0x4)
#define FSX_DMEx_AXI_CONTROL__WRITE_CHANNEL_EN		28
#define FSX_DMEx_AXI_CONTROL__READ_CHANNEL_EN		24
#define FSX_DMEx_AXI_CONTROL__AWQOS			20
#define FSX_DMEx_AXI_CONTROL__ARQOS			16
#define FSX_DMEx_AXI_CONTROL__AWCACHE			4
#define FSX_DMEx_AXI_CONTROL__ARCACHE			0

#define FSX_DMEx_WR_FIFO_THRESHOLD(__b, __i)		\
		(FSX_DMEx_BASE(__b, __i) + 0xc)
#define FSX_DMEx_WR_FIFO_THRESHOLD__MASK		0x3ff
#define FSX_DMEx_WR_FIFO_THRESHOLD__MAX			10
#define FSX_DMEx_WR_FIFO_THRESHOLD__MIN			0

#define FSX_DMEx_RD_FIFO_THRESHOLD(__b, __i)		\
		(FSX_DMEx_BASE(__b, __i) + 0x14)
#define FSX_DMEx_RD_FIFO_THRESHOLD__MASK		0x3ff
#define FSX_DMEx_RD_FIFO_THRESHOLD__MAX			10
#define FSX_DMEx_RD_FIFO_THRESHOLD__MIN			0

#define FS6_SUB_TOP_BASE				0x66D8F800
#define FS6_PKI_DME_RESET				0x4
#define PKI_DME_RESET					1

char *fsx_type_names[] = {
	"fs4-raid",
	"fs4-crypto",
	"fs6-pki",
};

void fsx_init(eFSX_TYPE fsx_type,
	      unsigned int ring_count,
	      unsigned int dme_count,
	      unsigned int ae_count,
	      unsigned int start_stream_id,
	      unsigned int msi_dev_id,
	      uintptr_t idm_io_control_direct,
	      uintptr_t idm_reset_control,
	      uintptr_t base,
	      uintptr_t dme_base)
{
	int try;
	unsigned int i, v, data;
	uintptr_t fs4_idm_io_control_direct = idm_io_control_direct;
	uintptr_t fs4_idm_reset_control = idm_reset_control;
	uintptr_t fsx_comm_rm = (base + 0x203000);

	VERBOSE("fsx %s init start\n", fsx_type_names[fsx_type]);

	if (fsx_type == eFS4_RAID || fsx_type == eFS4_CRYPTO) {
		/* Enable FSx engine clock */
		VERBOSE(" - enable fsx clock\n");
		mmio_write_32(fs4_idm_io_control_direct,
		      (1U << FS4_IDM_IO_CONTROL_DIRECT__CLK_EN));
		udelay(500);

		/* Reset FSx engine */
		VERBOSE(" - reset fsx\n");
		v = mmio_read_32(fs4_idm_reset_control);
		v |= (1 << FS4_IDM_RESET_CONTROL__RESET);
		mmio_write_32(fs4_idm_reset_control, v);
		udelay(500);
		v = mmio_read_32(fs4_idm_reset_control);
		v &= ~(1 << FS4_IDM_RESET_CONTROL__RESET);
		mmio_write_32(fs4_idm_reset_control, v);
	} else {
		/*
		 * Default RM and AE are out of reset,
		 * So only DME Reset added here
		 */
		v = mmio_read_32(FS6_SUB_TOP_BASE + FS6_PKI_DME_RESET);
		v &= ~(PKI_DME_RESET);
		mmio_write_32(FS6_SUB_TOP_BASE + FS6_PKI_DME_RESET, v);
	}

	/* Wait for HW-init done */
	VERBOSE(" - wait for HW-init done\n");
	try = 10000;
	do {
		udelay(1);
		data = mmio_read_32(fsx_comm_rm +
				    FSX_COMM_RM_MAIN_HW_INIT_DONE);
		try--;
	} while (!(data & FSX_COMM_RM_MAIN_HW_INIT_DONE__MASK) && (try > 0));

	if (try <= 0)
		ERROR("fsx_comm_rm + 0x%x: 0x%x\n",
		      data, FSX_COMM_RM_MAIN_HW_INIT_DONE);

	/* Make all rings non-secured */
	VERBOSE(" - make all rings non-secured\n");
	v = 0xffffffff;
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_RING_SECURITY_SETTING, v);

	/* Set start stream-id for rings to */
	VERBOSE(" - set start stream-id for rings to 0x%x\n",
		start_stream_id);
	v = start_stream_id >> FSX_COMM_RM_SSID_CONTROL__RING_BITS;
	v &= FSX_COMM_RM_SSID_CONTROL__MASK;
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_SSID_CONTROL, v);

	/* Set timer configuration */
	VERBOSE(" - set timer configuration\n");
	v = 0x0271 << FSX_COMM_RM_TIMER_CONTROL_0__MEDIUM;
	v |= (0x0138 << FSX_COMM_RM_TIMER_CONTROL_0__FAST);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_TIMER_CONTROL_0, v);
	v = 0x09c4 << FSX_COMM_RM_TIMER_CONTROL_1__IDLE;
	v |= (0x04e2 << FSX_COMM_RM_TIMER_CONTROL_1__SLOW);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_TIMER_CONTROL_1, v);
	v = 0x0000f424;
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_RING_FLUSH_TIMEOUT, v);

	/* Set burst length and fifo threshold */
	VERBOSE(" - set burst length, fifo and bd threshold\n");
	v = 0x0;
	v |= (0x8 << FSX_COMM_RM_BURST_LENGTH__FOR_DDR_ADDR_GEN);
	v |= (0x8 << FSX_COMM_RM_BURST_LENGTH__FOR_TOGGLE);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_BURST_LENGTH, v);
	v = 0x0;
	v |= (0x67 << FSX_COMM_RM_FIFO_THRESHOLD__BD_FIFO_FULL);
	v |= (0x18 << FSX_COMM_RM_FIFO_THRESHOLD__AE_FIFO_FULL);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_FIFO_THRESHOLD, v);
	v = 0x0;
	v |= (0x8 << FSX_COMM_RM_BURST_BD_THRESHOLD_LOW);
	v |= (0x8 << FSX_COMM_RM_BURST_BD_THRESHOLD_HIGH);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_BURST_BD_THRESHOLD, v);

	/* Set memory configuration */
	VERBOSE(" - set memory configuration\n");
	v = 0x0;
	v |= (1 << FSX_COMM_RM_MEMORY_CONFIGURATION__POWERONIN);
	v |= (1 << FSX_COMM_RM_MEMORY_CONFIGURATION__POWEROKIN);
	v |= (1 << FSX_COMM_RM_MEMORY_CONFIGURATION__ARRPOWERONIN);
	v |= (1 << FSX_COMM_RM_MEMORY_CONFIGURATION__ARRPOWEROKIN);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_MEMORY_CONFIGURATION, v);

	/* AXI configuration for RM */
	v = 0;
	v |= (0x1 << FSX_COMM_RM_AXI_CONTROL__WRITE_CHANNEL_EN);
	v |= (0x1 << FSX_COMM_RM_AXI_CONTROL__READ_CHANNEL_EN);
	v |= (0xe << FSX_COMM_RM_AXI_CONTROL__AWQOS);
	v |= (0xa << FSX_COMM_RM_AXI_CONTROL__ARQOS);
	v |= (0x2 << FSX_COMM_RM_AXI_CONTROL__AWPROT);
	v |= (0x2 << FSX_COMM_RM_AXI_CONTROL__ARPROT);
	v |= (0xf << FSX_COMM_RM_AXI_CONTROL__AWCACHE);
	v |= (0xf << FSX_COMM_RM_AXI_CONTROL__ARCACHE);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_AXI_CONTROL, v);
	VERBOSE(" - set AXI control = 0x%x\n",
		mmio_read_32(fsx_comm_rm + FSX_COMM_RM_AXI_CONTROL));
	v = 0x0;
	v |= (0x10 << FSX_COMM_RM_AXI_READ_BURST_THRESHOLD__MAX);
	v |= (0x10 << FSX_COMM_RM_AXI_READ_BURST_THRESHOLD__MIN);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_AXI_READ_BURST_THRESHOLD, v);
	VERBOSE(" - set AXI read burst threshold = 0x%x\n",
	mmio_read_32(fsx_comm_rm + FSX_COMM_RM_AXI_READ_BURST_THRESHOLD));

	/* Configure group ring count for all groups */
	/* By default we schedule extended packets
	 * on all AEs/DMEs in a group.
	 */
	v = (dme_count & 0xf) << 0;
	v |= (dme_count & 0xf) << 4;
	v |= (dme_count & 0xf) << 8;
	v |= (dme_count & 0xf) << 12;
	v |= (dme_count & 0xf) << 16;
	v |= (dme_count & 0xf) << 20;
	v |= (dme_count & 0xf) << 24;
	v |= (dme_count & 0xf) << 28;
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_GROUP_RING_COUNT, v);

	/*
	 * Due to HW issue spurious interrupts are getting generated.
	 * To fix sw needs to clear the config status interrupts
	 * before setting CONFIG_DONE.
	 */
	mmio_write_32(fsx_comm_rm +
		      FSX_COMM_RM_CONFIG_INTERRUPT_STATUS_CLEAR,
		      0xffffffff);

	/* Configure RM control */
	VERBOSE(" - configure RM control\n");
	v = mmio_read_32(fsx_comm_rm + FSX_COMM_RM_CONTROL_REGISTER);
	v |= (1 << FSX_COMM_RM_CONTROL_REGISTER__AE_LOCKING);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_CONTROL_REGISTER, v);
	v |= (1 << FSX_COMM_RM_CONTROL_REGISTER__CONFIG_DONE);
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_CONTROL_REGISTER, v);

	/* Configure AE timeout */
	VERBOSE(" - configure AE timeout\n");
	v = 0x00003fff;
	mmio_write_32(fsx_comm_rm + FSX_COMM_RM_AE_TIMEOUT, v);

	/* Initialize all AEs */
	for (i = 0; i < ae_count; i++) {
		VERBOSE(" - initialize AE%d\n", i);
		v = (0x1 << FSX_AEx_CONTROL_REGISTER__ACTIVE);
		mmio_write_32(FSX_AEx_CONTROL_REGISTER(base, i), v);
	}

	/* Initialize all DMEs */
	for (i = 0; i < dme_count; i++) {
		VERBOSE(" - initialize DME%d\n", i);
		v = 0;
		v |= (0x1 << FSX_DMEx_AXI_CONTROL__WRITE_CHANNEL_EN);
		v |= (0x1 << FSX_DMEx_AXI_CONTROL__READ_CHANNEL_EN);
		v |= (0xe << FSX_DMEx_AXI_CONTROL__AWQOS);
		v |= (0xa << FSX_DMEx_AXI_CONTROL__ARQOS);
		v |= (0xf << FSX_DMEx_AXI_CONTROL__AWCACHE);
		v |= (0xf << FSX_DMEx_AXI_CONTROL__ARCACHE);
		mmio_write_32(FSX_DMEx_AXI_CONTROL(dme_base, i), v);
		VERBOSE(" -- AXI_CONTROL = 0x%x\n",
		mmio_read_32(FSX_DMEx_AXI_CONTROL(dme_base, i)));
		v = 0;
		v |= (0x4 << FSX_DMEx_WR_FIFO_THRESHOLD__MIN);
		v |= (0x4 << FSX_DMEx_WR_FIFO_THRESHOLD__MAX);
		mmio_write_32(FSX_DMEx_WR_FIFO_THRESHOLD(dme_base, i), v);
		VERBOSE(" -- WR_FIFO_THRESHOLD = 0x%x\n",
		mmio_read_32(FSX_DMEx_WR_FIFO_THRESHOLD(dme_base, i)));
		v = 0;
		v |= (0x4 << FSX_DMEx_RD_FIFO_THRESHOLD__MIN);
		v |= (0x4 << FSX_DMEx_RD_FIFO_THRESHOLD__MAX);
		mmio_write_32(FSX_DMEx_RD_FIFO_THRESHOLD(dme_base, i), v);
		VERBOSE(" -- RD_FIFO_THRESHOLD = 0x%x\n",
		mmio_read_32(FSX_DMEx_RD_FIFO_THRESHOLD(dme_base, i)));
	}

	/* Configure ring axi id and msi device id */
	for (i = 0; i < ring_count; i++) {
		VERBOSE(" - ring%d version=0x%x\n", i,
			mmio_read_32(FSX_RINGx_VERSION_NUMBER(base, i)));
		mmio_write_32(FSX_COMM_RINGx_MSI_DEV_ID(base, i),
			      msi_dev_id);
		v = 0;
		v |= ((i & FSX_COMM_RINGx_CONTROL__AXI_ID_MASK) <<
		      FSX_COMM_RINGx_CONTROL__AXI_ID);
		mmio_write_32(FSX_COMM_RINGx_CONTROL(base, i), v);
	}

	INFO("fsx %s init done\n", fsx_type_names[fsx_type]);
}

void fsx_meminit(const char *name,
		 uintptr_t idm_io_control_direct,
		 uintptr_t idm_io_status)
{
	int try;
	unsigned int val;

	VERBOSE("fsx %s meminit start\n", name);

	VERBOSE(" - arrpoweron\n");
	mmio_setbits_32(idm_io_control_direct,
			BIT(FS4_IDM_IO_CONTROL_DIRECT__MEM_ARRPOWERON));
	while (!(mmio_read_32(idm_io_status) &
		 BIT(FS4_IDM_IO_STATUS__MEM_ARRPOWERON)))
		;

	VERBOSE(" - arrpowerok\n");
	mmio_setbits_32(idm_io_control_direct,
			(1 << FS4_IDM_IO_CONTROL_DIRECT__MEM_ARRPOWEROK));
	while (!(mmio_read_32(idm_io_status) &
		 BIT(FS4_IDM_IO_STATUS__MEM_ARRPOWEROK)))
		;

	VERBOSE(" - poweron\n");
	mmio_setbits_32(idm_io_control_direct,
			(1 << FS4_IDM_IO_CONTROL_DIRECT__MEM_POWERON));
	while (!(mmio_read_32(idm_io_status) &
		 BIT(FS4_IDM_IO_STATUS__MEM_POWERON)))
		;

	VERBOSE(" - powerok\n");
	mmio_setbits_32(idm_io_control_direct,
			(1 << FS4_IDM_IO_CONTROL_DIRECT__MEM_POWEROK));
	while (!(mmio_read_32(idm_io_status) &
		 BIT(FS4_IDM_IO_STATUS__MEM_POWEROK)))
		;

	/* Final check on all power bits */
	try = 10;
	do {
		val = mmio_read_32(idm_io_status);
		if (val == FS4_IDM_IO_STATUS__MEM_ALLOK)
			break;

		/* Wait sometime */
		mdelay(1);

		try--;
	} while (try > 0);

	/* Remove memory isolation if things are fine. */
	if (try <= 0) {
		INFO(" - powerup failed\n");
	} else {
		VERBOSE(" - remove isolation\n");
		mmio_clrbits_32(idm_io_control_direct,
				(1 << FS4_IDM_IO_CONTROL_DIRECT__MEM_ISO));
		VERBOSE(" - powerup done\n");
	}

	INFO("fsx %s meminit done\n", name);
}

void fs4_disable_clocks(bool disable_sram,
			bool disable_crypto,
			bool disable_raid)
{
	VERBOSE("fs4 disable clocks start\n");

	if (disable_sram) {
		VERBOSE(" - disable sram clock\n");
		mmio_clrbits_32(FS4_SRAM_IDM_IO_CONTROL_DIRECT,
			(1 << FS4_IDM_IO_CONTROL_DIRECT__SRAM_CLK_EN));
	}

	if (disable_crypto) {
		VERBOSE(" - disable crypto clock\n");
		mmio_setbits_32(CDRU_GENPLL5_CONTROL1,
				CDRU_GENPLL5_CONTROL1__CHNL1_CRYPTO_AE_CLK);
	}

	if (disable_raid) {
		VERBOSE(" - disable raid clock\n");
		mmio_setbits_32(CDRU_GENPLL5_CONTROL1,
				CDRU_GENPLL5_CONTROL1__CHNL2_RAID_AE_CLK);
	}

	if (disable_sram && disable_crypto && disable_raid) {
		VERBOSE(" - disable root clock\n");
		mmio_setbits_32(CDRU_GENPLL5_CONTROL1,
				CDRU_GENPLL5_CONTROL1__CHNL0_DME_CLK);
		mmio_setbits_32(CDRU_GENPLL2_CONTROL1,
				CDRU_GENPLL2_CONTROL1__CHNL6_FS4_CLK);
	}

	INFO("fs4 disable clocks done\n");
}
