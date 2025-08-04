/*
 * Copyright (c) 2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_mce.h>
#include <lib/mmio.h>
#include <lib/mmio_poll.h>
#include <libfdt.h>

#include <platform_def.h>
#include <stm32mp_fconf_getter.h>

/* MCE registers (base relative) */
#define MCE_CR		U(0x0000) /* MCE configuration register */
#define MCE_SR		U(0x0004) /* MCE status register */
#define MCE_IASR	U(0x0008) /* MCE illegal access status register */
#define MCE_IACR	U(0x000C) /* MCE illegal access clear register */
#define MCE_IAIER	U(0x0010) /* MCE illegal access interrupt enable reg */
#define MCE_PRIVCFGR	U(0x001C) /* MCE privileged configuration register */
#define MCE_REGCR	U(0x0040) /* MCE region configuration register */
#define MCE_SADDR	U(0x0044) /* MCE start address for region register */
#define MCE_EADDR	U(0x0048) /* MCE end address for region register */
#define MCE_MKEYR	U(0x0200) /* MCE master key register offset */
#define MCE_HWCFGR3	U(0x03E8) /* MCE hardware configuration register 3 */
#define MCE_HWCFGR2	U(0x03EC) /* MCE hardware configuration register 2 */
#define MCE_HWCFGR1	U(0x03F0) /* MCE hardware configuration register 1 */
#define MCE_VERR	U(0x03F4) /* MCE version register */
#define MCE_IPIDR	U(0x03F8) /* MCE identification register */
#define MCE_SIDR	U(0x03FC) /* MCE size ID register */

/* MCE configuration register */
#define MCE_CR_GLOCK			BIT(0)
#define MCE_CR_MKLOCK			BIT(1)

/* MCE status register */
#define MCE_SR_MKVALID			BIT(0)
#define MCE_SR_ENCDIS			BIT(4)

/* MCE privileged configuration register */
#define MCE_PRIVCFGR_PRIV		BIT(0)

/* MCE region configuration register */
#define MCE_REGCR_BREN			BIT(0)
#define MCE_REGCR_ENC			BIT(15)

/* MCE start address for region register */
#define MCE_SADDR_BADDSTART_MASK	GENMASK(31, 16)

/* MCE end address for region register */
#define MCE_EADDR_BADDEND_MASK		GENMASK(31, 16)

/* MCE version register */
#define MCE_VERR_MINREV_MASK		GENMASK(3, 0)
#define MCE_VERR_MINREV_SHIFT		0
#define MCE_VERR_MAJREV_MASK		GENMASK(7, 4)
#define MCE_VERR_MAJREV_SHIFT		4

/* IP configuration */
#define MCE_MAJREV			1U
#define MCE_MINREV			0U
#define MCE_IP_ID			0x00170081U
#define MCE_SIZE_ID			0xA3C5DD01U
#define MCE_ADDR_GRANULARITY		0x10000U

/* Other definitions */
#define MCE_TIMEOUT_1MS_IN_US		1000U
#define DDR_BASE_EXTRA_MASK		GENMASK_32(31, 30)
#define MCE_REGION_PARAMS		3U

struct mce_version_s {
	uint32_t major;
	uint32_t minor;
	uint32_t ip_id;
	uint32_t size_id;
};

struct mce_dt_id_attr {
	fdt32_t id_attr[MCE_IP_MAX_REGION_NB];
};

/*
 * @brief  Configure privileged access to the MCE registers.
 * @param  privilege: Only privileged (true) or all (false) access are granted.
 * @retval None.
 */
static void configure_privilege(bool privilege)
{
	if (privilege) {
		mmio_setbits_32(MCE_BASE + MCE_PRIVCFGR, MCE_PRIVCFGR_PRIV);
	} else {
		mmio_clrbits_32(MCE_BASE + MCE_PRIVCFGR, MCE_PRIVCFGR_PRIV);
	}
}

/*
 * @brief  Check consistency of region settings.
 * @param  config: Ref to the region configuration structure.
 * @retval 0 if OK, negative value else.
 */
static int check_region_settings(struct stm32_mce_region_s *config)
{
	uint32_t end;

	if (config->encrypt_mode > MCE_ENCRYPTION_MODE_MAX) {
		ERROR("MCE: encryption mode out of range error\n");
		return -EINVAL;
	}

	if ((config->start_address < STM32MP_DDR_BASE) ||
	    (config->end_address < STM32MP_DDR_BASE)) {
		ERROR("MCE: start/end address lower than DDR base\n");
		return -EINVAL;
	}

	end = STM32MP_DDR_BASE + dt_get_ddr_size() - 1U;
	if ((config->start_address > end) || (config->end_address > end)) {
		ERROR("MCE: start/end address higher than physical end\n");
		return -EINVAL;
	}

	if (config->start_address >= config->end_address) {
		ERROR("MCE: start address higher than or equal to end one\n");
		return -EINVAL;
	}

	if (((config->start_address % MCE_ADDR_GRANULARITY) != 0U) ||
	    (((config->end_address + 1U) % MCE_ADDR_GRANULARITY) != 0U)) {
		ERROR("MCE: start/end address granularity not respected\n");
		return -EINVAL;
	}

	return 0;
}

/*
 * @brief  Configure (and enable) the MCE region.
 * @param  index: Region index (first region is 0).
 * @param  config: Ref to the region configuration structure.
 * @retval 0 if OK, negative value else.
 */
static int stm32_mce_configure_region(uint32_t index, struct stm32_mce_region_s *config)
{
	int ret;

	if ((index >= MCE_IP_MAX_REGION_NB) || (config == NULL)) {
		return -EINVAL;
	}

	ret = check_region_settings(config);
	if (ret != 0) {
		return ret;
	}

	mmio_clrbits_32(MCE_BASE + MCE_REGCR, MCE_REGCR_BREN);

	mmio_clrsetbits_32(MCE_BASE + MCE_SADDR, MCE_SADDR_BADDSTART_MASK,
			   config->start_address & MCE_SADDR_BADDSTART_MASK);
	mmio_clrsetbits_32(MCE_BASE + MCE_EADDR, MCE_EADDR_BADDEND_MASK,
			   config->end_address & MCE_EADDR_BADDEND_MASK);

	if (config->encrypt_mode == MCE_ENCRYPT_MODE) {
		mmio_setbits_32(MCE_BASE + MCE_REGCR, MCE_REGCR_BREN | MCE_REGCR_ENC);

		if (!stm32_mce_is_hw_encryption_functional()) {
			ERROR("MCE: encryption feature error\n");
			return -EIO;
		}
	} else {
		mmio_clrbits_32(MCE_BASE + MCE_REGCR, MCE_REGCR_ENC);
		mmio_setbits_32(MCE_BASE + MCE_REGCR, MCE_REGCR_BREN);
	}

	return 0;
}

/*
 * @brief  Initialize the MCE driver.
 * @param  None.
 * @retval None.
 */
void stm32_mce_init(void)
{
	const struct mce_version_s exp_version = {
		.major = MCE_MAJREV,
		.minor = MCE_MINREV,
		.ip_id = MCE_IP_ID,
		.size_id = MCE_SIZE_ID
	};
	struct mce_version_s version;

	if (clk_enable(MCE) != 0) {
		panic();
	}

	version = (struct mce_version_s) {
		.major = (mmio_read_32(MCE_BASE + MCE_VERR) &
			  MCE_VERR_MAJREV_MASK) >> MCE_VERR_MAJREV_SHIFT,
		.minor = (mmio_read_32(MCE_BASE + MCE_VERR) &
			  MCE_VERR_MINREV_MASK) >> MCE_VERR_MINREV_SHIFT,
		.ip_id = mmio_read_32(MCE_BASE + MCE_IPIDR),
		.size_id = mmio_read_32(MCE_BASE + MCE_SIDR)
	};

	if ((version.major != exp_version.major) ||
	    (version.minor != exp_version.minor) ||
	    (version.ip_id != exp_version.ip_id) ||
	    (version.size_id != exp_version.size_id)) {
		ERROR("MCE: unexpected IP version { 0x%x, 0x%x, 0x%x, 0x%x }\n",
		      version.major, version.minor, version.ip_id, version.size_id);
		panic();
	}

	configure_privilege(true);
}

/*
 * @brief  Write the MCE master key.
 * @param  mkey: Pointer to the master key buffer.
 * @retval 0 if OK, negative value else.
 */
int stm32_mce_write_master_key(uint8_t *mkey)
{
	uint8_t i;
	uint32_t mce_sr;

	if (mkey == NULL) {
		return -EINVAL;
	}

	if ((mmio_read_32(MCE_BASE + MCE_CR) & MCE_CR_MKLOCK) == MCE_CR_MKLOCK) {
		return -EPERM;
	}

	for (i = 0U; i < MCE_KEY_SIZE_IN_BYTES; i += sizeof(uint32_t)) {
		uint32_t key_val = 0U;

		(void)memcpy(&key_val, mkey + i, sizeof(uint32_t));

		mmio_write_32(MCE_BASE + MCE_MKEYR + i, key_val);
	}

	return mmio_read_32_poll_timeout((MCE_BASE + MCE_SR), mce_sr,
					 ((mce_sr & MCE_SR_MKVALID) == MCE_SR_MKVALID),
					 MCE_TIMEOUT_1MS_IN_US);
}

/*
 * @brief  Lock the MCE master key.
 * @param  None.
 * @retval None.
 */
void stm32_mce_lock_master_key(void)
{
	mmio_setbits_32(MCE_BASE + MCE_CR, MCE_CR_MKLOCK);
}

/*
 * @brief  Get the MCE master key lock state.
 * @param  None.
 * @retval True if locked, false else.
 */
bool stm32_mce_is_master_key_locked(void)
{
	return (mmio_read_32(MCE_BASE + MCE_CR) & MCE_CR_MKLOCK) == MCE_CR_MKLOCK;
}

/*
 * @brief  Lock the MCE IP registers.
 * @param  None.
 * @retval None.
 */
void stm32_mce_lock_global(void)
{
	mmio_setbits_32(MCE_BASE + MCE_CR, MCE_CR_GLOCK);
}

/*
 * @brief  Get the MCE global lock state.
 * @param  None.
 * @retval True if locked, false else.
 */
bool stm32_mce_is_globally_locked(void)
{
	return (mmio_read_32(MCE_BASE + MCE_CR) & MCE_CR_GLOCK) == MCE_CR_GLOCK;
}

/*
 * @brief  Get the MCE encryption HW feature state.
 * @param  None.
 * @retval True if functional, false else.
 */
bool stm32_mce_is_hw_encryption_functional(void)
{
	return (mmio_read_32(MCE_BASE + MCE_SR) & MCE_SR_ENCDIS) != MCE_SR_ENCDIS;
}

/*
 * @brief  Get the encryption state of an address.
 * @param  index: Memory address.
 * @param  state: Ref to the encryption state.
 * @retval 0 if OK, negative value else.
 */
int stm32_mce_get_address_encryption_state(uint32_t address, uint32_t *state)
{
	struct stm32_mce_region_s config;

	if ((address < STM32MP_DDR_BASE) ||
	    (address > (STM32MP_DDR_BASE + dt_get_ddr_size() - 1U)) ||
	    (state == NULL)) {
		return -EINVAL;
	}

	if ((mmio_read_32(MCE_BASE + MCE_REGCR) & MCE_REGCR_ENC) != MCE_REGCR_ENC) {
		/* No encrypted region, all DDR area is in plain text */
		*state = MCE_BYPASS_MODE;
		return 0;
	}

	if (!stm32_mce_is_hw_encryption_functional()) {
		ERROR("MCE: encryption feature error\n");
		return -EIO;
	}

	/*
	 * When MCE_SADDR and MCE_EADDR are accessed in read, the 2 MSB bits
	 * return zeros. So DDR base address mask has to be ORed.
	 */
	config.start_address = mmio_read_32(MCE_BASE + MCE_SADDR) |
			       (STM32MP_DDR_BASE & DDR_BASE_EXTRA_MASK);

	config.end_address = mmio_read_32(MCE_BASE + MCE_EADDR) |
			     (STM32MP_DDR_BASE & DDR_BASE_EXTRA_MASK);

	if ((address >= config.start_address) && (address <= config.end_address)) {
		*state = MCE_ENCRYPT_MODE;
	} else {
		*state = MCE_BYPASS_MODE;
	}

	return 0;
}

static int fconf_populate_mce(uintptr_t config)
{
	int node, len;
	unsigned int i;
	const struct mce_dt_id_attr *conf_list;
	const void *dtb = (const void *)config;

	/* Check the node offset point to "st,mem-encrypt" compatible property */
	const char *compatible_str = "st,mem-encrypt";

	if ((stm32mp_check_closed_device() == STM32MP_CHIP_SEC_OPEN) &&
	    !stm32mp_is_auth_supported()) {
		return 0;
	}

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	conf_list = (const struct mce_dt_id_attr *)fdt_getprop(dtb, node, "memory-ranges", &len);
	if (conf_list == NULL) {
		WARN("FCONF: Read cell failed for %s\n", "memory-ranges");
		return -1;
	}

	/* Consider only complete set of values */
	len -= len % MCE_REGION_PARAMS;

	/* Locate the memory cells and read all values */
	for (i = 0U; i < ((unsigned int)len / (sizeof(uint32_t) * MCE_REGION_PARAMS)); i++) {
		uint32_t size;
		struct stm32_mce_region_s region;

		region.start_address = fdt32_to_cpu(conf_list->id_attr[i * MCE_REGION_PARAMS]);
		size = fdt32_to_cpu(conf_list->id_attr[i * MCE_REGION_PARAMS + 1U]);
		region.end_address = region.start_address + size - 1U;
		region.encrypt_mode = fdt32_to_cpu(conf_list->id_attr[i * MCE_REGION_PARAMS + 2U]);

		VERBOSE("FCONF: mce cell found with value = 0x%x 0x%x 0x%x\n",
			region.start_address, size, region.encrypt_mode);

		if (stm32_mce_configure_region(i, &region) != 0) {
			panic();
		}
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(FW_CONFIG, mce_config, fconf_populate_mce);
