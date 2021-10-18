/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/ufs.h>
#include <lib/mmio.h>

#define CDB_ADDR_MASK			127
#define ALIGN_CDB(x)			(((x) + CDB_ADDR_MASK) & ~CDB_ADDR_MASK)
#define ALIGN_8(x)			(((x) + 7) & ~7)

#define UFS_DESC_SIZE			0x400
#define MAX_UFS_DESC_SIZE		0x8000		/* 32 descriptors */

#define MAX_PRDT_SIZE			0x40000		/* 256KB */

static ufs_params_t ufs_params;
static int nutrs;	/* Number of UTP Transfer Request Slots */

int ufshc_send_uic_cmd(uintptr_t base, uic_cmd_t *cmd)
{
	unsigned int data;

	if (base == 0 || cmd == NULL)
		return -EINVAL;

	data = mmio_read_32(base + HCS);
	if ((data & HCS_UCRDY) == 0)
		return -EBUSY;
	mmio_write_32(base + IS, ~0);
	mmio_write_32(base + UCMDARG1, cmd->arg1);
	mmio_write_32(base + UCMDARG2, cmd->arg2);
	mmio_write_32(base + UCMDARG3, cmd->arg3);
	mmio_write_32(base + UICCMD, cmd->op);

	do {
		data = mmio_read_32(base + IS);
	} while ((data & UFS_INT_UCCS) == 0);
	mmio_write_32(base + IS, UFS_INT_UCCS);
	return mmio_read_32(base + UCMDARG2) & CONFIG_RESULT_CODE_MASK;
}

int ufshc_dme_get(unsigned int attr, unsigned int idx, unsigned int *val)
{
	uintptr_t base;
	unsigned int data;
	int result, retries;
	uic_cmd_t cmd;

	assert(ufs_params.reg_base != 0);

	if (val == NULL)
		return -EINVAL;

	base = ufs_params.reg_base;
	for (retries = 0; retries < 100; retries++) {
		data = mmio_read_32(base + HCS);
		if ((data & HCS_UCRDY) != 0)
			break;
		mdelay(1);
	}
	if (retries >= 100)
		return -EBUSY;

	cmd.arg1 = (attr << 16) | GEN_SELECTOR_IDX(idx);
	cmd.arg2 = 0;
	cmd.arg3 = 0;
	cmd.op = DME_GET;
	for (retries = 0; retries < UFS_UIC_COMMAND_RETRIES; ++retries) {
		result = ufshc_send_uic_cmd(base, &cmd);
		if (result == 0)
			break;
		data = mmio_read_32(base + IS);
		if (data & UFS_INT_UE)
			return -EINVAL;
	}
	if (retries >= UFS_UIC_COMMAND_RETRIES)
		return -EIO;

	*val = mmio_read_32(base + UCMDARG3);
	return 0;
}

int ufshc_dme_set(unsigned int attr, unsigned int idx, unsigned int val)
{
	uintptr_t base;
	unsigned int data;
	int result, retries;
	uic_cmd_t cmd;

	assert((ufs_params.reg_base != 0));

	base = ufs_params.reg_base;
	cmd.arg1 = (attr << 16) | GEN_SELECTOR_IDX(idx);
	cmd.arg2 = 0;
	cmd.arg3 = val;
	cmd.op = DME_SET;

	for (retries = 0; retries < UFS_UIC_COMMAND_RETRIES; ++retries) {
		result = ufshc_send_uic_cmd(base, &cmd);
		if (result == 0)
			break;
		data = mmio_read_32(base + IS);
		if (data & UFS_INT_UE)
			return -EINVAL;
	}
	if (retries >= UFS_UIC_COMMAND_RETRIES)
		return -EIO;

	return 0;
}

static int ufshc_hce_enable(uintptr_t base)
{
	unsigned int data;
	int retries;

	/* Enable Host Controller */
	mmio_write_32(base + HCE, HCE_ENABLE);

	/* Wait until basic initialization sequence completed */
	for (retries = 0; retries < HCE_ENABLE_INNER_RETRIES; ++retries) {
		data = mmio_read_32(base + HCE);
		if (data & HCE_ENABLE) {
			break;
		}
		udelay(HCE_ENABLE_TIMEOUT_US);
	}
	if (retries >= HCE_ENABLE_INNER_RETRIES) {
		return -ETIMEDOUT;
	}

	return 0;
}

static int ufshc_reset(uintptr_t base)
{
	unsigned int data;
	int retries, result;

	for (retries = 0; retries < HCE_ENABLE_OUTER_RETRIES; ++retries) {
		result = ufshc_hce_enable(base);
		if (result == 0) {
			break;
		}
	}
	if (retries >= HCE_ENABLE_OUTER_RETRIES) {
		return -EIO;
	}

	/* Enable Interrupts */
	data = UFS_INT_UCCS | UFS_INT_ULSS | UFS_INT_UE | UFS_INT_UTPES |
	       UFS_INT_DFES | UFS_INT_HCFES | UFS_INT_SBFES;
	mmio_write_32(base + IE, data);

	return 0;
}

static int ufshc_dme_link_startup(uintptr_t base)
{
	uic_cmd_t cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.op = DME_LINKSTARTUP;
	return ufshc_send_uic_cmd(base, &cmd);
}

static int ufshc_link_startup(uintptr_t base)
{
	int data, result;
	int retries;

	for (retries = DME_LINKSTARTUP_RETRIES; retries > 0; retries--) {
		result = ufshc_dme_link_startup(base);
		if (result != 0) {
			/* Reset controller before trying again */
			result = ufshc_reset(base);
			if (result != 0) {
				return result;
			}
			continue;
		}
		while ((mmio_read_32(base + HCS) & HCS_DP) == 0)
			;
		data = mmio_read_32(base + IS);
		if (data & UFS_INT_ULSS)
			mmio_write_32(base + IS, UFS_INT_ULSS);
		return 0;
	}
	return -EIO;
}

/* Check Door Bell register to get an empty slot */
static int get_empty_slot(int *slot)
{
	unsigned int data;
	int i;

	data = mmio_read_32(ufs_params.reg_base + UTRLDBR);
	for (i = 0; i < nutrs; i++) {
		if ((data & 1) == 0)
			break;
		data = data >> 1;
	}
	if (i >= nutrs)
		return -EBUSY;
	*slot = i;
	return 0;
}

static void get_utrd(utp_utrd_t *utrd)
{
	uintptr_t base;
	int slot = 0, result;
	utrd_header_t *hd;

	assert(utrd != NULL);
	result = get_empty_slot(&slot);
	assert(result == 0);

	/* clear utrd */
	memset((void *)utrd, 0, sizeof(utp_utrd_t));
	base = ufs_params.desc_base + (slot * UFS_DESC_SIZE);
	/* clear the descriptor */
	memset((void *)base, 0, UFS_DESC_SIZE);

	utrd->header = base;
	utrd->task_tag = slot + 1;
	/* CDB address should be aligned with 128 bytes */
	utrd->upiu = ALIGN_CDB(utrd->header + sizeof(utrd_header_t));
	utrd->resp_upiu = ALIGN_8(utrd->upiu + sizeof(cmd_upiu_t));
	utrd->size_upiu = utrd->resp_upiu - utrd->upiu;
	utrd->size_resp_upiu = ALIGN_8(sizeof(resp_upiu_t));
	utrd->prdt = utrd->resp_upiu + utrd->size_resp_upiu;

	hd = (utrd_header_t *)utrd->header;
	hd->ucdba = utrd->upiu & UINT32_MAX;
	hd->ucdbau = (utrd->upiu >> 32) & UINT32_MAX;
	/* Both RUL and RUO is based on DWORD */
	hd->rul = utrd->size_resp_upiu >> 2;
	hd->ruo = utrd->size_upiu >> 2;
	(void)result;
}

/*
 * Prepare UTRD, Command UPIU, Response UPIU.
 */
static int ufs_prepare_cmd(utp_utrd_t *utrd, uint8_t op, uint8_t lun,
			   int lba, uintptr_t buf, size_t length)
{
	utrd_header_t *hd;
	cmd_upiu_t *upiu;
	prdt_t *prdt;
	unsigned int ulba;
	unsigned int lba_cnt;
	int prdt_size;


	mmio_write_32(ufs_params.reg_base + UTRLBA,
		      utrd->header & UINT32_MAX);
	mmio_write_32(ufs_params.reg_base + UTRLBAU,
		      (utrd->upiu >> 32) & UINT32_MAX);

	hd = (utrd_header_t *)utrd->header;
	upiu = (cmd_upiu_t *)utrd->upiu;

	hd->i = 1;
	hd->ct = CT_UFS_STORAGE;
	hd->ocs = OCS_MASK;

	upiu->trans_type = CMD_UPIU;
	upiu->task_tag = utrd->task_tag;
	upiu->cdb[0] = op;
	ulba = (unsigned int)lba;
	lba_cnt = (unsigned int)(length >> UFS_BLOCK_SHIFT);
	switch (op) {
	case CDBCMD_TEST_UNIT_READY:
		break;
	case CDBCMD_READ_CAPACITY_10:
		hd->dd = DD_OUT;
		upiu->flags = UPIU_FLAGS_R | UPIU_FLAGS_ATTR_S;
		upiu->lun = lun;
		break;
	case CDBCMD_READ_10:
		hd->dd = DD_OUT;
		upiu->flags = UPIU_FLAGS_R | UPIU_FLAGS_ATTR_S;
		upiu->lun = lun;
		upiu->cdb[1] = RW_WITHOUT_CACHE;
		/* set logical block address */
		upiu->cdb[2] = (ulba >> 24) & 0xff;
		upiu->cdb[3] = (ulba >> 16) & 0xff;
		upiu->cdb[4] = (ulba >> 8) & 0xff;
		upiu->cdb[5] = ulba & 0xff;
		/* set transfer length */
		upiu->cdb[7] = (lba_cnt >> 8) & 0xff;
		upiu->cdb[8] = lba_cnt & 0xff;
		break;
	case CDBCMD_WRITE_10:
		hd->dd = DD_IN;
		upiu->flags = UPIU_FLAGS_W | UPIU_FLAGS_ATTR_S;
		upiu->lun = lun;
		upiu->cdb[1] = RW_WITHOUT_CACHE;
		/* set logical block address */
		upiu->cdb[2] = (ulba >> 24) & 0xff;
		upiu->cdb[3] = (ulba >> 16) & 0xff;
		upiu->cdb[4] = (ulba >> 8) & 0xff;
		upiu->cdb[5] = ulba & 0xff;
		/* set transfer length */
		upiu->cdb[7] = (lba_cnt >> 8) & 0xff;
		upiu->cdb[8] = lba_cnt & 0xff;
		break;
	default:
		assert(0);
		break;
	}
	if (hd->dd == DD_IN)
		flush_dcache_range(buf, length);
	else if (hd->dd == DD_OUT)
		inv_dcache_range(buf, length);
	if (length) {
		upiu->exp_data_trans_len = htobe32(length);
		assert(lba_cnt <= UINT16_MAX);
		prdt = (prdt_t *)utrd->prdt;

		prdt_size = 0;
		while (length > 0) {
			prdt->dba = (unsigned int)(buf & UINT32_MAX);
			prdt->dbau = (unsigned int)((buf >> 32) & UINT32_MAX);
			/* prdt->dbc counts from 0 */
			if (length > MAX_PRDT_SIZE) {
				prdt->dbc = MAX_PRDT_SIZE - 1;
				length = length - MAX_PRDT_SIZE;
			} else {
				prdt->dbc = length - 1;
				length = 0;
			}
			buf += MAX_PRDT_SIZE;
			prdt++;
			prdt_size += sizeof(prdt_t);
		}
		utrd->size_prdt = ALIGN_8(prdt_size);
		hd->prdtl = utrd->size_prdt >> 2;
		hd->prdto = (utrd->size_upiu + utrd->size_resp_upiu) >> 2;
	}

	flush_dcache_range((uintptr_t)utrd, sizeof(utp_utrd_t));
	flush_dcache_range((uintptr_t)utrd->header, UFS_DESC_SIZE);
	return 0;
}

static int ufs_prepare_query(utp_utrd_t *utrd, uint8_t op, uint8_t idn,
			     uint8_t index, uint8_t sel,
			     uintptr_t buf, size_t length)
{
	utrd_header_t *hd;
	query_upiu_t *query_upiu;


	hd = (utrd_header_t *)utrd->header;
	query_upiu = (query_upiu_t *)utrd->upiu;

	mmio_write_32(ufs_params.reg_base + UTRLBA,
		      utrd->header & UINT32_MAX);
	mmio_write_32(ufs_params.reg_base + UTRLBAU,
		      (utrd->header >> 32) & UINT32_MAX);


	hd->i = 1;
	hd->ct = CT_UFS_STORAGE;
	hd->ocs = OCS_MASK;

	query_upiu->trans_type = QUERY_REQUEST_UPIU;
	query_upiu->task_tag = utrd->task_tag;
	query_upiu->ts.desc.opcode = op;
	query_upiu->ts.desc.idn = idn;
	query_upiu->ts.desc.index = index;
	query_upiu->ts.desc.selector = sel;
	switch (op) {
	case QUERY_READ_DESC:
		query_upiu->query_func = QUERY_FUNC_STD_READ;
		query_upiu->ts.desc.length = htobe16(length);
		break;
	case QUERY_WRITE_DESC:
		query_upiu->query_func = QUERY_FUNC_STD_WRITE;
		query_upiu->ts.desc.length = htobe16(length);
		memcpy((void *)(utrd->upiu + sizeof(query_upiu_t)),
		       (void *)buf, length);
		break;
	case QUERY_READ_ATTR:
	case QUERY_READ_FLAG:
		query_upiu->query_func = QUERY_FUNC_STD_READ;
		break;
	case QUERY_CLEAR_FLAG:
	case QUERY_SET_FLAG:
		query_upiu->query_func = QUERY_FUNC_STD_WRITE;
		break;
	case QUERY_WRITE_ATTR:
		query_upiu->query_func = QUERY_FUNC_STD_WRITE;
		memcpy((void *)&query_upiu->ts.attr.value, (void *)buf, length);
		break;
	default:
		assert(0);
		break;
	}
	flush_dcache_range((uintptr_t)utrd, sizeof(utp_utrd_t));
	flush_dcache_range((uintptr_t)utrd->header, UFS_DESC_SIZE);
	return 0;
}

static void ufs_prepare_nop_out(utp_utrd_t *utrd)
{
	utrd_header_t *hd;
	nop_out_upiu_t *nop_out;

	mmio_write_32(ufs_params.reg_base + UTRLBA,
		      utrd->header & UINT32_MAX);
	mmio_write_32(ufs_params.reg_base + UTRLBAU,
		      (utrd->header >> 32) & UINT32_MAX);

	hd = (utrd_header_t *)utrd->header;
	nop_out = (nop_out_upiu_t *)utrd->upiu;

	hd->i = 1;
	hd->ct = CT_UFS_STORAGE;
	hd->ocs = OCS_MASK;

	nop_out->trans_type = 0;
	nop_out->task_tag = utrd->task_tag;
	flush_dcache_range((uintptr_t)utrd, sizeof(utp_utrd_t));
	flush_dcache_range((uintptr_t)utrd->header, UFS_DESC_SIZE);
}

static void ufs_send_request(int task_tag)
{
	unsigned int data;
	int slot;

	slot = task_tag - 1;
	/* clear all interrupts */
	mmio_write_32(ufs_params.reg_base + IS, ~0);

	mmio_write_32(ufs_params.reg_base + UTRLRSR, 1);
	do {
		data = mmio_read_32(ufs_params.reg_base + UTRLRSR);
	} while (data == 0);

	data = UTRIACR_IAEN | UTRIACR_CTR | UTRIACR_IACTH(0x1F) |
	       UTRIACR_IATOVAL(0xFF);
	mmio_write_32(ufs_params.reg_base + UTRIACR, data);
	/* send request */
	mmio_setbits_32(ufs_params.reg_base + UTRLDBR, 1 << slot);
}

static int ufs_check_resp(utp_utrd_t *utrd, int trans_type)
{
	utrd_header_t *hd;
	resp_upiu_t *resp;
	unsigned int data;
	int slot;

	hd = (utrd_header_t *)utrd->header;
	resp = (resp_upiu_t *)utrd->resp_upiu;
	inv_dcache_range((uintptr_t)hd, UFS_DESC_SIZE);
	inv_dcache_range((uintptr_t)utrd, sizeof(utp_utrd_t));
	do {
		data = mmio_read_32(ufs_params.reg_base + IS);
		if ((data & ~(UFS_INT_UCCS | UFS_INT_UTRCS)) != 0)
			return -EIO;
	} while ((data & UFS_INT_UTRCS) == 0);
	slot = utrd->task_tag - 1;

	data = mmio_read_32(ufs_params.reg_base + UTRLDBR);
	assert((data & (1 << slot)) == 0);
	assert(hd->ocs == OCS_SUCCESS);
	assert((resp->trans_type & TRANS_TYPE_CODE_MASK) == trans_type);
	(void)resp;
	(void)slot;
	return 0;
}

#ifdef UFS_RESP_DEBUG
static void dump_upiu(utp_utrd_t *utrd)
{
	utrd_header_t *hd;
	int i;

	hd = (utrd_header_t *)utrd->header;
	INFO("utrd:0x%x, ruo:0x%x, rul:0x%x, ocs:0x%x, UTRLDBR:0x%x\n",
		(unsigned int)(uintptr_t)utrd, hd->ruo, hd->rul, hd->ocs,
		mmio_read_32(ufs_params.reg_base + UTRLDBR));
	for (i = 0; i < sizeof(utrd_header_t); i += 4) {
		INFO("[%lx]:0x%x\n",
			(uintptr_t)utrd->header + i,
			*(unsigned int *)((uintptr_t)utrd->header + i));
	}

	for (i = 0; i < sizeof(cmd_upiu_t); i += 4) {
		INFO("cmd[%lx]:0x%x\n",
			utrd->upiu + i,
			*(unsigned int *)(utrd->upiu + i));
	}
	for (i = 0; i < sizeof(resp_upiu_t); i += 4) {
		INFO("resp[%lx]:0x%x\n",
			utrd->resp_upiu + i,
			*(unsigned int *)(utrd->resp_upiu + i));
	}
	for (i = 0; i < sizeof(prdt_t); i += 4) {
		INFO("prdt[%lx]:0x%x\n",
			utrd->prdt + i,
			*(unsigned int *)(utrd->prdt + i));
	}
}
#endif

static void ufs_verify_init(void)
{
	utp_utrd_t utrd;
	int result;

	get_utrd(&utrd);
	ufs_prepare_nop_out(&utrd);
	ufs_send_request(utrd.task_tag);
	result = ufs_check_resp(&utrd, NOP_IN_UPIU);
	assert(result == 0);
	(void)result;
}

static void ufs_verify_ready(void)
{
	utp_utrd_t utrd;
	int result;

	get_utrd(&utrd);
	ufs_prepare_cmd(&utrd, CDBCMD_TEST_UNIT_READY, 0, 0, 0, 0);
	ufs_send_request(utrd.task_tag);
	result = ufs_check_resp(&utrd, RESPONSE_UPIU);
	assert(result == 0);
	(void)result;
}

static void ufs_query(uint8_t op, uint8_t idn, uint8_t index, uint8_t sel,
		      uintptr_t buf, size_t size)
{
	utp_utrd_t utrd;
	query_resp_upiu_t *resp;
	int result;

	switch (op) {
	case QUERY_READ_FLAG:
	case QUERY_READ_ATTR:
	case QUERY_READ_DESC:
	case QUERY_WRITE_DESC:
	case QUERY_WRITE_ATTR:
		assert(((buf & 3) == 0) && (size != 0));
		break;
	default:
		/* Do nothing in default case */
		break;
	}
	get_utrd(&utrd);
	ufs_prepare_query(&utrd, op, idn, index, sel, buf, size);
	ufs_send_request(utrd.task_tag);
	result = ufs_check_resp(&utrd, QUERY_RESPONSE_UPIU);
	assert(result == 0);
	resp = (query_resp_upiu_t *)utrd.resp_upiu;
#ifdef UFS_RESP_DEBUG
	dump_upiu(&utrd);
#endif
	assert(resp->query_resp == QUERY_RESP_SUCCESS);

	switch (op) {
	case QUERY_READ_FLAG:
		*(uint32_t *)buf = (uint32_t)resp->ts.flag.value;
		break;
	case QUERY_READ_ATTR:
	case QUERY_READ_DESC:
		memcpy((void *)buf,
		       (void *)(utrd.resp_upiu + sizeof(query_resp_upiu_t)),
		       size);
		break;
	default:
		/* Do nothing in default case */
		break;
	}
	(void)result;
}

unsigned int ufs_read_attr(int idn)
{
	unsigned int value;

	ufs_query(QUERY_READ_ATTR, idn, 0, 0,
		  (uintptr_t)&value, sizeof(value));
	return value;
}

void ufs_write_attr(int idn, unsigned int value)
{
	ufs_query(QUERY_WRITE_ATTR, idn, 0, 0,
		  (uintptr_t)&value, sizeof(value));
}

unsigned int ufs_read_flag(int idn)
{
	unsigned int value;

	ufs_query(QUERY_READ_FLAG, idn, 0, 0,
		  (uintptr_t)&value, sizeof(value));
	return value;
}

void ufs_set_flag(int idn)
{
	ufs_query(QUERY_SET_FLAG, idn, 0, 0, 0, 0);
}

void ufs_clear_flag(int idn)
{
	ufs_query(QUERY_CLEAR_FLAG, idn, 0, 0, 0, 0);
}

void ufs_read_desc(int idn, int index, uintptr_t buf, size_t size)
{
	ufs_query(QUERY_READ_DESC, idn, index, 0, buf, size);
}

void ufs_write_desc(int idn, int index, uintptr_t buf, size_t size)
{
	ufs_query(QUERY_WRITE_DESC, idn, index, 0, buf, size);
}

static void ufs_read_capacity(int lun, unsigned int *num, unsigned int *size)
{
	utp_utrd_t utrd;
	resp_upiu_t *resp;
	sense_data_t *sense;
	unsigned char data[CACHE_WRITEBACK_GRANULE << 1];
	uintptr_t buf;
	int result;
	int retry;

	assert((ufs_params.reg_base != 0) &&
	       (ufs_params.desc_base != 0) &&
	       (ufs_params.desc_size >= UFS_DESC_SIZE) &&
	       (num != NULL) && (size != NULL));

	/* align buf address */
	buf = (uintptr_t)data;
	buf = (buf + CACHE_WRITEBACK_GRANULE - 1) &
	      ~(CACHE_WRITEBACK_GRANULE - 1);
	memset((void *)buf, 0, CACHE_WRITEBACK_GRANULE);
	flush_dcache_range(buf, CACHE_WRITEBACK_GRANULE);
	do {
		get_utrd(&utrd);
		ufs_prepare_cmd(&utrd, CDBCMD_READ_CAPACITY_10, lun, 0,
				buf, READ_CAPACITY_LENGTH);
		ufs_send_request(utrd.task_tag);
		result = ufs_check_resp(&utrd, RESPONSE_UPIU);
		assert(result == 0);
#ifdef UFS_RESP_DEBUG
		dump_upiu(&utrd);
#endif
		resp = (resp_upiu_t *)utrd.resp_upiu;
		retry = 0;
		sense = &resp->sd.sense;
		if (sense->resp_code == SENSE_DATA_VALID) {
			if ((sense->sense_key == SENSE_KEY_UNIT_ATTENTION) &&
			    (sense->asc == 0x29) && (sense->ascq == 0)) {
				retry = 1;
			}
		}
		inv_dcache_range(buf, CACHE_WRITEBACK_GRANULE);
		/* last logical block address */
		*num = be32toh(*(unsigned int *)buf);
		if (*num)
			*num += 1;
		/* logical block length in bytes */
		*size = be32toh(*(unsigned int *)(buf + 4));
	} while (retry);
	(void)result;
}

size_t ufs_read_blocks(int lun, int lba, uintptr_t buf, size_t size)
{
	utp_utrd_t utrd;
	resp_upiu_t *resp;
	int result;

	assert((ufs_params.reg_base != 0) &&
	       (ufs_params.desc_base != 0) &&
	       (ufs_params.desc_size >= UFS_DESC_SIZE));

	memset((void *)buf, 0, size);
	get_utrd(&utrd);
	ufs_prepare_cmd(&utrd, CDBCMD_READ_10, lun, lba, buf, size);
	ufs_send_request(utrd.task_tag);
	result = ufs_check_resp(&utrd, RESPONSE_UPIU);
	assert(result == 0);
#ifdef UFS_RESP_DEBUG
	dump_upiu(&utrd);
#endif
	resp = (resp_upiu_t *)utrd.resp_upiu;
	(void)result;
	return size - resp->res_trans_cnt;
}

size_t ufs_write_blocks(int lun, int lba, const uintptr_t buf, size_t size)
{
	utp_utrd_t utrd;
	resp_upiu_t *resp;
	int result;

	assert((ufs_params.reg_base != 0) &&
	       (ufs_params.desc_base != 0) &&
	       (ufs_params.desc_size >= UFS_DESC_SIZE));

	memset((void *)buf, 0, size);
	get_utrd(&utrd);
	ufs_prepare_cmd(&utrd, CDBCMD_WRITE_10, lun, lba, buf, size);
	ufs_send_request(utrd.task_tag);
	result = ufs_check_resp(&utrd, RESPONSE_UPIU);
	assert(result == 0);
#ifdef UFS_RESP_DEBUG
	dump_upiu(&utrd);
#endif
	resp = (resp_upiu_t *)utrd.resp_upiu;
	(void)result;
	return size - resp->res_trans_cnt;
}

static void ufs_enum(void)
{
	unsigned int blk_num, blk_size;
	int i;

	/* 0 means 1 slot */
	nutrs = (mmio_read_32(ufs_params.reg_base + CAP) & CAP_NUTRS_MASK) + 1;
	if (nutrs > (ufs_params.desc_size / UFS_DESC_SIZE))
		nutrs = ufs_params.desc_size / UFS_DESC_SIZE;

	ufs_verify_init();
	ufs_verify_ready();

	ufs_set_flag(FLAG_DEVICE_INIT);
	mdelay(200);
	/* dump available LUNs */
	for (i = 0; i < UFS_MAX_LUNS; i++) {
		ufs_read_capacity(i, &blk_num, &blk_size);
		if (blk_num && blk_size) {
			INFO("UFS LUN%d contains %d blocks with %d-byte size\n",
			     i, blk_num, blk_size);
		}
	}
}

static void ufs_get_device_info(struct ufs_dev_desc *card_data)
{
	uint8_t desc_buf[DESC_DEVICE_MAX_SIZE];

	ufs_query(QUERY_READ_DESC, DESC_TYPE_DEVICE, 0, 0,
				(uintptr_t)desc_buf, DESC_DEVICE_MAX_SIZE);

	/*
	 * getting vendor (manufacturerID) and Bank Index in big endian
	 * format
	 */
	card_data->wmanufacturerid = (uint16_t)((desc_buf[DEVICE_DESC_PARAM_MANF_ID] << 8) |
				     (desc_buf[DEVICE_DESC_PARAM_MANF_ID + 1]));
}

int ufs_init(const ufs_ops_t *ops, ufs_params_t *params)
{
	int result;
	unsigned int data;
	uic_cmd_t cmd;
	struct ufs_dev_desc card = {0};

	assert((params != NULL) &&
	       (params->reg_base != 0) &&
	       (params->desc_base != 0) &&
	       (params->desc_size >= UFS_DESC_SIZE));

	memcpy(&ufs_params, params, sizeof(ufs_params_t));

	if (ufs_params.flags & UFS_FLAGS_SKIPINIT) {
		result = ufshc_dme_get(0x1571, 0, &data);
		assert(result == 0);
		result = ufshc_dme_get(0x41, 0, &data);
		assert(result == 0);
		if (data == 1) {
			/* prepare to exit hibernate mode */
			memset(&cmd, 0, sizeof(uic_cmd_t));
			cmd.op = DME_HIBERNATE_EXIT;
			result = ufshc_send_uic_cmd(ufs_params.reg_base,
						    &cmd);
			assert(result == 0);
			data = mmio_read_32(ufs_params.reg_base + UCMDARG2);
			assert(data == 0);
			do {
				data = mmio_read_32(ufs_params.reg_base + IS);
			} while ((data & UFS_INT_UHXS) == 0);
			mmio_write_32(ufs_params.reg_base + IS, UFS_INT_UHXS);
			data = mmio_read_32(ufs_params.reg_base + HCS);
			assert((data & HCS_UPMCRS_MASK) == HCS_PWR_LOCAL);
		}
		result = ufshc_dme_get(0x1568, 0, &data);
		assert(result == 0);
		assert((data > 0) && (data <= 3));
	} else {
		assert((ops != NULL) && (ops->phy_init != NULL) &&
		       (ops->phy_set_pwr_mode != NULL));

		result = ufshc_reset(ufs_params.reg_base);
		assert(result == 0);
		ops->phy_init(&ufs_params);
		result = ufshc_link_startup(ufs_params.reg_base);
		assert(result == 0);

		ufs_enum();

		ufs_get_device_info(&card);
		if (card.wmanufacturerid == UFS_VENDOR_SKHYNIX) {
			ufs_params.flags |= UFS_FLAGS_VENDOR_SKHYNIX;
		}

		ops->phy_set_pwr_mode(&ufs_params);
	}

	(void)result;
	return 0;
}
