/*
 * Copyright (c) 2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp2_risaf.h>
#include <dt-bindings/soc/rif.h>
#include <lib/mmio.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <stm32mp_fconf_getter.h>

static struct stm32mp2_risaf_platdata stm32mp2_risaf;
static int region_per_instance[RISAF_MAX_INSTANCE];

#if ENABLE_ASSERTIONS
static bool valid_protreg_id(int instance, uint32_t id)
{
	uint32_t max_id;

	max_id = mmio_read_32(stm32mp2_risaf.base[instance] + _RISAF_HWCFGR);
	max_id = (max_id & _RISAF_HWCFGR_CFG1_MASK) >> _RISAF_HWCFGR_CFG1_SHIFT;

	return id < max_id;
}

static bool valid_instance(int instance)
{
	return (instance < RISAF_MAX_INSTANCE) && (stm32mp2_risaf.base[instance] != 0U);
}
#endif

static bool risaf_is_hw_encryption_functional(int instance)
{
	return (mmio_read_32(stm32mp2_risaf.base[instance] + _RISAF_SR) & _RISAF_SR_ENCDIS) !=
	       _RISAF_SR_ENCDIS;
}

static int check_region_boundaries(int instance, uintptr_t addr, size_t len)
{
	uintptr_t end_address;
	uintptr_t mem_base = stm32_risaf_get_memory_base(instance);

	if ((addr < mem_base) || (len == 0U)) {
		return -EINVAL;
	}

	/* Get physical end address */
	end_address = mem_base + stm32_risaf_get_memory_size(instance) - 1U;
	if ((addr > end_address) || ((addr - 1U + len) > end_address)) {
		return -EINVAL;
	}

	if ((stm32mp2_risaf.granularity[instance] == 0U) ||
	    ((addr % stm32mp2_risaf.granularity[instance]) != 0U) ||
	    ((len % stm32mp2_risaf.granularity[instance]) != 0U)) {
		return -EINVAL;
	}

	return 0;
}

static bool do_regions_overlap(uintptr_t addr1, size_t len1, uintptr_t addr2, size_t len2)
{
	return !((addr1 >= (addr2 + len2)) || (addr2 >= (addr1 + len1)));
}

static int check_region_overlap(void)
{
	struct stm32mp2_risaf_platdata *pdata = &stm32mp2_risaf;
	int i;
	uintptr_t addr;
	size_t length;
	int instance;
	int region_id;

	if (pdata->nregions <= 1) {
		/*
		 * No region found, or first region found.
		 * No need to check overlap with previous ones.
		 */
		return 0;
	}

	region_id = pdata->nregions - 1;
	addr = pdata->region[region_id].addr;
	length = pdata->region[region_id].len;
	instance = pdata->region[region_id].instance;

	for (i = 0; i < region_id; i++) {
		if (pdata->region[i].instance != instance) {
			continue;
		}

		if (do_regions_overlap(addr, length,
				       pdata->region[i].addr, pdata->region[i].len)) {
			ERROR("RISAF%d: Regions %d and %d overlap\n", instance + 1, region_id, i);
			return -EINVAL;
		}
	}

	return 0;
}

static int risaf_configure_region(int instance, uint32_t region_id, uint32_t cfg,
				  uint32_t cid_cfg, uintptr_t saddr, uintptr_t eaddr)
{
	uintptr_t base = stm32mp2_risaf.base[instance];
	uint32_t hwcfgr;
	uint32_t mask_lsb;
	uint32_t mask_msb;
	uint32_t mask;

	assert(valid_instance(instance));
	assert(valid_protreg_id(instance, region_id));

	mmio_clrbits_32(base + _RISAF_REG_CFGR(region_id), _RISAF_REG_CFGR_BREN);

	/* Get address mask depending on RISAF instance HW configuration */
	hwcfgr =  mmio_read_32(base + _RISAF_HWCFGR);
	mask_lsb = (hwcfgr & _RISAF_HWCFGR_CFG3_MASK) >> _RISAF_HWCFGR_CFG3_SHIFT;
	mask_msb = mask_lsb + ((hwcfgr & _RISAF_HWCFGR_CFG4_MASK) >> _RISAF_HWCFGR_CFG4_SHIFT) - 1U;
	mask = GENMASK_32(mask_msb, mask_lsb);

	mmio_clrsetbits_32(base + _RISAF_REG_STARTR(region_id), mask,
			   (saddr - stm32_risaf_get_memory_base(instance)) & mask);
	mmio_clrsetbits_32(base + _RISAF_REG_ENDR(region_id), mask,
			   (eaddr - stm32_risaf_get_memory_base(instance)) & mask);

	mmio_clrsetbits_32(base + _RISAF_REG_CIDCFGR(region_id), _RISAF_REG_CIDCFGR_ALL_MASK,
			   cid_cfg & _RISAF_REG_CIDCFGR_ALL_MASK);

	mmio_clrsetbits_32(base + _RISAF_REG_CFGR(region_id),
			   _RISAF_REG_CFGR_ALL_MASK, cfg & _RISAF_REG_CFGR_ALL_MASK);

	if ((cfg & _RISAF_REG_CFGR_ENC) == _RISAF_REG_CFGR_ENC) {
		if (!risaf_is_hw_encryption_functional(instance)) {
			ERROR("RISAF%d: encryption disabled\n", instance + 1);
			return -EIO;
		}

		if ((cfg & _RISAF_REG_CFGR_SEC) != _RISAF_REG_CFGR_SEC) {
			ERROR("RISAF%d: encryption on non secure area\n", instance + 1);
			return -EIO;
		}
	}

	return 0;
}

static void risaf_conf_protreg(void)
{
	struct stm32mp2_risaf_platdata *pdata = &stm32mp2_risaf;
	int idx;

	for (idx = 0; idx < RISAF_MAX_INSTANCE; idx++) {
		int n;

		if (pdata->base[idx] == 0) {
			continue;
		}

		if (clk_enable(pdata->clock[idx]) != 0) {
			ERROR("%s: RISAF@%lx clock failed\n", __func__, pdata->base[idx]);
			panic();
		}

		for (n = 0; n < pdata->nregions; n++) {
			uint32_t id;
			uint32_t value;
			uint32_t cfg;
			uint32_t cid_cfg;
			uintptr_t start_addr;
			uintptr_t end_addr;

			if (pdata->region[n].instance != idx) {
				continue;
			}

			value = pdata->region[n].cfg;
			id = (value & DT_RISAF_REG_ID_MASK);
			assert(valid_protreg_id(idx, id));

			cfg = (((value & DT_RISAF_EN_MASK) >> DT_RISAF_EN_SHIFT) <<
			       _RISAF_REG_CFGR_BREN_SHIFT) |
			      (((value & DT_RISAF_SEC_MASK) >> DT_RISAF_SEC_SHIFT) <<
			       _RISAF_REG_CFGR_SEC_SHIFT) |
			      (((value & DT_RISAF_ENC_MASK) >> DT_RISAF_ENC_SHIFT) <<
			       _RISAF_REG_CFGR_ENC_SHIFT) |
			      (((value & DT_RISAF_PRIV_MASK) >> DT_RISAF_PRIV_SHIFT) <<
			       _RISAF_REG_CFGR_PRIVC_SHIFT);

			cid_cfg = (((value & DT_RISAF_WRITE_MASK) >> DT_RISAF_WRITE_SHIFT) <<
				   _RISAF_REG_CIDCFGR_WRENC_SHIFT) |
				  (((value & DT_RISAF_READ_MASK) >> DT_RISAF_READ_SHIFT) <<
				   _RISAF_REG_CIDCFGR_RDENC_SHIFT);

			start_addr = pdata->region[n].addr;
			end_addr = (start_addr - 1U) + pdata->region[n].len;

			if (risaf_configure_region(idx, id, cfg, cid_cfg,
						   start_addr, end_addr) < 0) {
				ERROR("%s: failed to configure region %u of RISAF@%lx\n",
				      __func__, id, pdata->base[idx]);
				panic();
			}
		}

		clk_disable(pdata->clock[idx]);
	}
}

static int risaf_get_dt_node(struct dt_node_info *info, int offset)
{
	return dt_get_node(info, offset, DT_RISAF_COMPAT);
}

static int risaf_get_instance_from_region(uintptr_t address, size_t length)
{
	struct stm32mp2_risaf_platdata *pdata = &stm32mp2_risaf;
	unsigned int idx;
	int instance = -1;

	for (idx = 0U; idx < RISAF_MAX_INSTANCE; idx++) {
		if (pdata->base[idx] == 0U) {
			continue;
		}

		if (check_region_boundaries(idx, address, length) == 0) {
			instance = idx;
		}
	}

	return instance;
}

/*
 * Register region in platfoirm data structure if parameters are valid.
 * If instance is known, related entry parameter is filled, else it is equal to -1.
 */
static int risaf_register_region(void *fdt, int node, int instance)
{
	struct stm32mp2_risaf_platdata *pdata = &stm32mp2_risaf;
	const fdt32_t *cuint;
	int len = 0;
	int inst;
	uintptr_t address;
	size_t length;
	uint32_t protreg;

	/*  Get address and length */
	cuint = fdt_getprop(fdt, node, "reg", &len);
	if ((cuint == NULL) || (len != RISAF_REGION_REG_SIZE)) {
		ERROR("RISAF: No or bad reg entry in DT\n");
		return -EINVAL;
	}

	address = (uintptr_t)fdt32_to_cpu(cuint[0]) << 32;
	address |= fdt32_to_cpu(cuint[1]);
	length = (size_t)fdt32_to_cpu(cuint[2]) << 32;
	length |= fdt32_to_cpu(cuint[3]);

	/* Get instance */
	inst = risaf_get_instance_from_region(address, length);
	if (inst < 0) {
		ERROR("RISAF: No instance found in DT\n");
		return -EINVAL;
	}

	if ((instance != -1) && (inst != instance)) {
		ERROR("RISAF%d: Region not located in expected address space\n", instance + 1);
		return -EINVAL;
	}

	/* Get protreg configuration */
	cuint = fdt_getprop(fdt, node, "st,protreg", &len);
	if ((cuint == NULL) || (len != RISAF_REGION_PROTREG_SIZE)) {
		ERROR("RISAF%d: No or bad st,protreg entry in DT\n", inst + 1);
		return -EINVAL;
	}

	protreg = fdt32_to_cpu(*cuint);

	/* Check if region max is reached for the current instance */
	region_per_instance[inst]++;
	if (region_per_instance[inst] > stm32_risaf_get_max_region(inst)) {
		ERROR("RISAF%d: Too many entries in DT\n", inst + 1);
		return -EINVAL;
	}

	if (check_region_boundaries(inst, address, length) != 0) {
		ERROR("RISAF%d: Region %d exceeds limits\n", inst + 1, pdata->nregions);
		return -EINVAL;
	}

	/* Register region configuration */
	pdata->region[pdata->nregions].instance = inst;
	pdata->region[pdata->nregions].cfg = protreg;
	pdata->region[pdata->nregions].addr = address;
	pdata->region[pdata->nregions].len = length;
	pdata->nregions++;

	if (check_region_overlap() != 0) {
		return -EINVAL;
	}

	return 0;
}

/*
 * From DT, retrieve base address, clock ID and all region information for each RISAF instance.
 * Check boundaries for each region and overlap for each instance.
 */
static int risaf_parse_fdt(void)
{
	struct stm32mp2_risaf_platdata *pdata = &stm32mp2_risaf;
	struct dt_node_info risaf_info;
	int node = -1;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	for (node = risaf_get_dt_node(&risaf_info, node); node >= 0;
	     node = risaf_get_dt_node(&risaf_info, node)) {
		int idx;
		int nregions;
		int inst_maxregions;
		int i;
		int len = 0;
		const fdt32_t *conf_list;
		uint32_t granularity;

		idx = stm32_risaf_get_instance(risaf_info.base);
		if ((idx < 0) || (risaf_info.clock < 0)) {
			continue;
		}

		pdata->base[idx] = risaf_info.base;
		pdata->clock[idx] = (unsigned long)risaf_info.clock;

		/* Get IP region granularity */
		if (clk_enable(pdata->clock[idx]) != 0) {
			ERROR("%s: clock enable failed.\n", __func__);
			panic();
		}

		granularity = mmio_read_32(pdata->base[idx] + _RISAF_HWCFGR);
		clk_disable(pdata->clock[idx]);
		granularity = BIT_32((granularity & _RISAF_HWCFGR_CFG3_MASK) >>
				  _RISAF_HWCFGR_CFG3_SHIFT);
		pdata->granularity[idx] = granularity;

		conf_list = fdt_getprop(fdt, node, "memory-region", &len);
		if (conf_list == NULL) {
			len = 0;
		}

		nregions = (unsigned int)len / sizeof(uint32_t);

		inst_maxregions = stm32_risaf_get_max_region(idx);
		if (inst_maxregions <= 0) {
			continue;
		}

		if ((nregions > inst_maxregions) ||
		    ((pdata->nregions + nregions) > RISAF_MAX_REGION)) {
			ERROR("RISAF%d: Too many entries in DT\n", idx + 1);
			return -EINVAL;
		}

		for (i = 0; i < nregions; i++) {
			int pnode = 0;

			pnode = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(conf_list[i]));
			if (pnode < 0) {
				continue;
			}

			if (risaf_register_region(fdt, pnode, idx) != 0) {
				ERROR("RISAF%d: Region %d error\n", idx + 1, pdata->nregions);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static uintptr_t risaf_base[RISAF_MAX_INSTANCE];
static unsigned long risaf_clock[RISAF_MAX_INSTANCE];
static uint32_t risaf_granularity[RISAF_MAX_INSTANCE];
static struct stm32mp2_risaf_region risaf_region[RISAF_MAX_REGION];

/* Construct platform data structure */
static int risaf_get_platdata(struct stm32mp2_risaf_platdata *pdata)
{
	pdata->base = risaf_base;
	pdata->clock = risaf_clock;
	pdata->granularity = risaf_granularity;
	pdata->region = risaf_region;

	return 0;
}

/*
 * @brief  Write the encryption key for a given instance.
 * @param  instance: RISAF instance ID.
 *         key: Pointer to the encryption key buffer.
 * @retval 0 if OK, negative value else.
 */
int stm32mp2_risaf_write_encryption_key(int instance, uint8_t *key)
{
	uint64_t timeout_ref;
	uint32_t i;
	uintptr_t base = stm32mp2_risaf.base[instance];

	if (base == 0U) {
		return -EINVAL;
	}

	if (key == NULL) {
		return -EINVAL;
	}

	for (i = 0U; i < RISAF_ENCRYPTION_KEY_SIZE_IN_BYTES; i += sizeof(uint32_t)) {
		uint32_t key_val = 0U;

		memcpy(&key_val, key + i, sizeof(uint32_t));

		mmio_write_32(base + _RISAF_KEYR + i, key_val);
	}

	timeout_ref = timeout_init_us(RISAF_TIMEOUT_1MS_IN_US);

	while (((mmio_read_32(base + _RISAF_SR) & _RISAF_SR_KEYVALID) != _RISAF_SR_KEYVALID) ||
	       ((mmio_read_32(base + _RISAF_SR) & _RISAF_SR_KEYRDY) != _RISAF_SR_KEYRDY)) {
		if (timeout_elapsed(timeout_ref)) {
			return -EIO;
		}
	}

	return 0;
}

/*
 * @brief  Lock the RISAF IP registers for a given instance.
 * @param  instance: RISAF instance ID.
 * @retval 0 if OK, negative value else.
 */
int stm32mp2_risaf_lock(int instance)
{
	uintptr_t base = stm32mp2_risaf.base[instance];

	if (base == 0U) {
		return -EINVAL;
	}

	mmio_setbits_32(base + _RISAF_CR, _RISAF_CR_GLOCK);

	return 0;
}

/*
 * @brief  Get the RISAF lock state for a given instance.
 * @param  instance: RISAF instance ID.
 *         state: lock state, true if locked, false else.
 * @retval 0 if OK, negative value else.
 */
int stm32mp2_risaf_is_locked(int instance, bool *state)
{
	uintptr_t base = stm32mp2_risaf.base[instance];

	if (base == 0U) {
		return -EINVAL;
	}

	*state = (mmio_read_32(base + _RISAF_CR) & _RISAF_CR_GLOCK) == _RISAF_CR_GLOCK;

	return 0;
}

int stm32mp2_risaf_init(void)
{
	int err;

	err = risaf_get_platdata(&stm32mp2_risaf);
	if (err != 0) {
		return err;
	}

	err = risaf_parse_fdt();
	if (err != 0) {
		return err;
	}

	risaf_conf_protreg();

	return err;
}

static int risaf_parse_fwconfig(uintptr_t config)
{
	struct stm32mp2_risaf_platdata *pdata = &stm32mp2_risaf;
	unsigned int i;
	int node = -1;
	int subnode;
	const void *fdt = (const void *)config;
	const char *compatible_str = "st,stm32mp2-mem-firewall";

	node = fdt_node_offset_by_compatible(fdt, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	fdt_for_each_subnode(subnode, fdt, node) {
		if (risaf_register_region((void *)fdt, subnode, -1) != 0) {
			ERROR("RISAF: Region %d error\n", pdata->nregions);
			return -EINVAL;
		}
	}

	for (i = 0U; i < RISAF_MAX_INSTANCE; i++) {
		if ((region_per_instance[i] == 0) && (stm32_risaf_get_max_region(i) != 0)) {
			INFO("RISAF%u: No configuration in DT, use default\n", i + 1);
		}
	}

	return 0;
}

static int fconf_populate_risaf(uintptr_t config)
{
	int err;

	err = risaf_parse_fwconfig(config);
	if (err != 0) {
		return err;
	}

	risaf_conf_protreg();

	return err;
}

FCONF_REGISTER_POPULATOR(FW_CONFIG, risaf_config, fconf_populate_risaf);
