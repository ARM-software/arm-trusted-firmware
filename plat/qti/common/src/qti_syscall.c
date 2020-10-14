/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <lib/coreboot.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

#include <qti_plat.h>
#include <qti_secure_io_cfg.h>
#include <qtiseclib_interface.h>
/*
 * SIP service - SMC function IDs for SiP Service queries
 *
 */
#define	QTI_SIP_SVC_CALL_COUNT_ID			U(0x0200ff00)
#define	QTI_SIP_SVC_UID_ID				U(0x0200ff01)
/*							0x8200ff02 is reserved*/
#define	QTI_SIP_SVC_VERSION_ID				U(0x0200ff03)

/*
 * Syscall's to allow Non Secure world accessing peripheral/IO memory
 * those are secure/proteced BUT not required to be secure.
 */
#define	QTI_SIP_SVC_SECURE_IO_READ_ID		U(0x02000501)
#define	QTI_SIP_SVC_SECURE_IO_WRITE_ID		U(0x02000502)

/*
 * Syscall's to assigns a list of intermediate PAs from a
 * source Virtual Machine (VM) to a destination VM.
 */
#define	QTI_SIP_SVC_MEM_ASSIGN_ID		U(0x02000C16)

#define	QTI_SIP_SVC_SECURE_IO_READ_PARAM_ID	U(0x1)
#define	QTI_SIP_SVC_SECURE_IO_WRITE_PARAM_ID	U(0x2)
#define	QTI_SIP_SVC_MEM_ASSIGN_PARAM_ID		U(0x1117)

#define	QTI_SIP_SVC_CALL_COUNT			U(0x3)
#define QTI_SIP_SVC_VERSION_MAJOR		U(0x0)
#define	QTI_SIP_SVC_VERSION_MINOR		U(0x0)

#define QTI_VM_LAST				U(44)
#define SIZE4K					U(0x1000)
#define QTI_VM_MAX_LIST_SIZE			U(0x20)

#define	FUNCID_OEN_NUM_MASK	((FUNCID_OEN_MASK << FUNCID_OEN_SHIFT)\
				|(FUNCID_NUM_MASK << FUNCID_NUM_SHIFT))

enum {
	QTI_SIP_SUCCESS = 0,
	QTI_SIP_NOT_SUPPORTED = -1,
	QTI_SIP_PREEMPTED = -2,
	QTI_SIP_INVALID_PARAM = -3,
};

/* QTI SiP Service UUID */
DEFINE_SVC_UUID2(qti_sip_svc_uid,
		 0x43864748, 0x217f, 0x41ad, 0xaa, 0x5a,
		 0xba, 0xe7, 0x0f, 0xa5, 0x52, 0xaf);

static bool qti_is_secure_io_access_allowed(u_register_t addr)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(qti_secure_io_allowed_regs); i++) {
		if ((uintptr_t) addr == qti_secure_io_allowed_regs[i]) {
			return true;
		}
	}

	return false;
}

bool qti_mem_assign_validate_param(memprot_info_t *mem_info,
				   u_register_t u_num_mappings,
				   uint32_t *source_vm_list,
				   u_register_t src_vm_list_cnt,
				   memprot_dst_vm_perm_info_t *dest_vm_list,
				   u_register_t dst_vm_list_cnt)
{
	int i;

	if (!source_vm_list || !dest_vm_list || !mem_info
	    || (src_vm_list_cnt == 0)
	    || (src_vm_list_cnt >= QTI_VM_LAST) || (dst_vm_list_cnt == 0)
	    || (dst_vm_list_cnt >= QTI_VM_LAST) || (u_num_mappings == 0)
	    || u_num_mappings > QTI_VM_MAX_LIST_SIZE) {
		ERROR("vm count is 0 or more then QTI_VM_LAST or empty list\n");
		ERROR("source_vm_list %p dest_vm_list %p mem_info %p src_vm_list_cnt %u dst_vm_list_cnt %u u_num_mappings %u\n",
		     source_vm_list, dest_vm_list, mem_info,
		     (unsigned int)src_vm_list_cnt,
		     (unsigned int)dst_vm_list_cnt,
		     (unsigned int)u_num_mappings);
		return false;
	}
	for (i = 0; i < u_num_mappings; i++) {
		if ((mem_info[i].mem_addr & (SIZE4K - 1))
		    || (mem_info[i].mem_size == 0)
		    || (mem_info[i].mem_size & (SIZE4K - 1))) {
			ERROR("mem_info passed buffer 0x%x or size 0x%x is not 4k aligned\n",
			     (unsigned int)mem_info[i].mem_addr,
			     (unsigned int)mem_info[i].mem_size);
			return false;
		}

		if ((mem_info[i].mem_addr + mem_info[i].mem_size) <
		    mem_info[i].mem_addr) {
			ERROR("overflow in mem_addr 0x%x add mem_size 0x%x\n",
			      (unsigned int)mem_info[i].mem_addr,
			      (unsigned int)mem_info[i].mem_size);
			return false;
		}
		coreboot_memory_t mem_type = coreboot_get_memory_type(
						mem_info[i].mem_addr,
						mem_info[i].mem_size);
		if (mem_type != CB_MEM_RAM && mem_type != CB_MEM_RESERVED) {
			ERROR("memory region not in CB MEM RAM or RESERVED area: region start 0x%x size 0x%x\n",
			     (unsigned int)mem_info[i].mem_addr,
			     (unsigned int)mem_info[i].mem_size);
			return false;
		}
	}
	for (i = 0; i < src_vm_list_cnt; i++) {
		if (source_vm_list[i] >= QTI_VM_LAST) {
			ERROR("source_vm_list[%d] 0x%x is more then QTI_VM_LAST\n",
			      i, (unsigned int)source_vm_list[i]);
			return false;
		}
	}
	for (i = 0; i < dst_vm_list_cnt; i++) {
		if (dest_vm_list[i].dst_vm >= QTI_VM_LAST) {
			ERROR("dest_vm_list[%d] 0x%x is more then QTI_VM_LAST\n",
			      i, (unsigned int)dest_vm_list[i].dst_vm);
			return false;
		}
	}
	return true;
}

static uintptr_t qti_sip_mem_assign(void *handle, uint32_t smc_cc,
				    u_register_t x1,
				    u_register_t x2,
				    u_register_t x3, u_register_t x4)
{
	uintptr_t dyn_map_start = 0, dyn_map_end = 0;
	size_t dyn_map_size = 0;
	u_register_t x6, x7;
	int ret = QTI_SIP_NOT_SUPPORTED;
	u_register_t x5 = read_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X5);

	if (smc_cc == SMC_32) {
		x5 = (uint32_t) x5;
	}
	/* Validate input arg count & retrieve arg3-6 from NS Buffer. */
	if ((x1 != QTI_SIP_SVC_MEM_ASSIGN_PARAM_ID) || (x5 == 0x0)) {
		ERROR("invalid mem_assign param id or no mapping info\n");
		goto unmap_return;
	}

	/* Map NS Buffer. */
	dyn_map_start = x5;
	dyn_map_size =
		(smc_cc ==
		 SMC_32) ? (sizeof(uint32_t) * 4) : (sizeof(uint64_t) * 4);
	if (qti_mmap_add_dynamic_region(dyn_map_start, dyn_map_size,
				(MT_NS | MT_RO_DATA)) != 0) {
		ERROR("map failed for params NS Buffer %x %x\n",
		      (unsigned int)dyn_map_start, (unsigned int)dyn_map_size);
		goto unmap_return;
	}
	/* Retrieve indirect args. */
	if (smc_cc == SMC_32) {
		x6 = *((uint32_t *) x5 + 1);
		x7 = *((uint32_t *) x5 + 2);
		x5 = *(uint32_t *) x5;
	} else {
		x6 = *((uint64_t *) x5 + 1);
		x7 = *((uint64_t *) x5 + 2);
		x5 = *(uint64_t *) x5;
	}
	/* Un-Map NS Buffer. */
	if (qti_mmap_remove_dynamic_region(dyn_map_start, dyn_map_size) != 0) {
		ERROR("unmap failed for params NS Buffer %x %x\n",
		      (unsigned int)dyn_map_start, (unsigned int)dyn_map_size);
		goto unmap_return;
	}

	/*
	 * Map NS Buffers.
	 * arg0,2,4 points to buffers & arg1,3,5 hold sizes.
	 * MAP api's fail to map if it's already mapped. Let's
	 * find lowest start & highest end address, then map once.
	 */
	dyn_map_start = MIN(x2, x4);
	dyn_map_start = MIN(dyn_map_start, x6);
	dyn_map_end = MAX((x2 + x3), (x4 + x5));
	dyn_map_end = MAX(dyn_map_end, (x6 + x7));
	dyn_map_size = dyn_map_end - dyn_map_start;

	if (qti_mmap_add_dynamic_region(dyn_map_start, dyn_map_size,
					(MT_NS | MT_RO_DATA)) != 0) {
		ERROR("map failed for params NS Buffer2 %x %x\n",
		      (unsigned int)dyn_map_start, (unsigned int)dyn_map_size);
		goto unmap_return;
	}
	memprot_info_t *mem_info_p = (memprot_info_t *) x2;
	uint32_t u_num_mappings = x3 / sizeof(memprot_info_t);
	uint32_t *source_vm_list_p = (uint32_t *) x4;
	uint32_t src_vm_list_cnt = x5 / sizeof(uint32_t);
	memprot_dst_vm_perm_info_t *dest_vm_list_p =
		(memprot_dst_vm_perm_info_t *) x6;
	uint32_t dst_vm_list_cnt =
		x7 / sizeof(memprot_dst_vm_perm_info_t);
	if (qti_mem_assign_validate_param(mem_info_p, u_num_mappings,
				source_vm_list_p, src_vm_list_cnt,
				dest_vm_list_p,
				dst_vm_list_cnt) != true) {
		ERROR("Param validation failed\n");
		goto unmap_return;
	}

	memprot_info_t mem_info[QTI_VM_MAX_LIST_SIZE];
	/* Populating the arguments */
	for (int i = 0; i < u_num_mappings; i++) {
		mem_info[i].mem_addr = mem_info_p[i].mem_addr;
		mem_info[i].mem_size = mem_info_p[i].mem_size;
	}

	memprot_dst_vm_perm_info_t dest_vm_list[QTI_VM_LAST];

	for (int i = 0; i < dst_vm_list_cnt; i++) {
		dest_vm_list[i].dst_vm = dest_vm_list_p[i].dst_vm;
		dest_vm_list[i].dst_vm_perm = dest_vm_list_p[i].dst_vm_perm;
		dest_vm_list[i].ctx = dest_vm_list_p[i].ctx;
		dest_vm_list[i].ctx_size = dest_vm_list_p[i].ctx_size;
	}

	uint32_t source_vm_list[QTI_VM_LAST];

	for (int i = 0; i < src_vm_list_cnt; i++) {
		source_vm_list[i] = source_vm_list_p[i];
	}
	/* Un-Map NS Buffers. */
	if (qti_mmap_remove_dynamic_region(dyn_map_start,
				dyn_map_size) != 0) {
		ERROR("unmap failed for params NS Buffer %x %x\n",
		      (unsigned int)dyn_map_start, (unsigned int)dyn_map_size);
		goto unmap_return;
	}
	/* Invoke API lib api. */
	ret = qtiseclib_mem_assign(mem_info, u_num_mappings,
			source_vm_list, src_vm_list_cnt,
			dest_vm_list, dst_vm_list_cnt);

	if (ret == 0) {
		SMC_RET2(handle, QTI_SIP_SUCCESS, ret);
	}
unmap_return:
	/* Un-Map NS Buffers if mapped */
	if (dyn_map_start && dyn_map_size) {
		qti_mmap_remove_dynamic_region(dyn_map_start, dyn_map_size);
	}

	SMC_RET2(handle, QTI_SIP_INVALID_PARAM, ret);
}

/*
 * This function handles QTI specific syscalls. Currently only SiP calls are present.
 * Both FAST & YIELD type call land here.
 */
static uintptr_t qti_sip_handler(uint32_t smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie, void *handle, u_register_t flags)
{
	uint32_t l_smc_fid = smc_fid & FUNCID_OEN_NUM_MASK;

	if (GET_SMC_CC(smc_fid) == SMC_32) {
		x1 = (uint32_t) x1;
		x2 = (uint32_t) x2;
		x3 = (uint32_t) x3;
		x4 = (uint32_t) x4;
	}

	switch (l_smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
		{
			SMC_RET1(handle, QTI_SIP_SVC_CALL_COUNT);
			break;
		}
	case QTI_SIP_SVC_UID_ID:
		{
			/* Return UID to the caller */
			SMC_UUID_RET(handle, qti_sip_svc_uid);
			break;
		}
	case QTI_SIP_SVC_VERSION_ID:
		{
			/* Return the version of current implementation */
			SMC_RET2(handle, QTI_SIP_SVC_VERSION_MAJOR,
				 QTI_SIP_SVC_VERSION_MINOR);
			break;
		}
	case QTI_SIP_SVC_SECURE_IO_READ_ID:
		{
			if ((x1 == QTI_SIP_SVC_SECURE_IO_READ_PARAM_ID) &&
			    qti_is_secure_io_access_allowed(x2)) {
				SMC_RET2(handle, QTI_SIP_SUCCESS,
					 *((volatile uint32_t *)x2));
			}
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
			break;
		}
	case QTI_SIP_SVC_SECURE_IO_WRITE_ID:
		{
			if ((x1 == QTI_SIP_SVC_SECURE_IO_WRITE_PARAM_ID) &&
			    qti_is_secure_io_access_allowed(x2)) {
				*((volatile uint32_t *)x2) = x3;
				SMC_RET1(handle, QTI_SIP_SUCCESS);
			}
			SMC_RET1(handle, QTI_SIP_INVALID_PARAM);
			break;
		}
	case QTI_SIP_SVC_MEM_ASSIGN_ID:
		{
			return qti_sip_mem_assign(handle, GET_SMC_CC(smc_fid),
						  x1, x2, x3, x4);
			break;
		}
	default:
		{
			SMC_RET1(handle, QTI_SIP_NOT_SUPPORTED);
		}
	}
	return (uintptr_t) handle;
}

/* Define a runtime service descriptor for both fast & yield SiP calls */
DECLARE_RT_SVC(qti_sip_fast_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_FAST, NULL, qti_sip_handler);

DECLARE_RT_SVC(qti_sip_yield_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_YIELD, NULL, qti_sip_handler);
