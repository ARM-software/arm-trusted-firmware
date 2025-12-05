/*
 * Copyright (c) 2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/runtime_svc.h>
#include <smccc_helpers.h>

#include <sbsa_platform.h>

#define SMC_FASTCALL       0x80000000
#define SMC64_FUNCTION     (SMC_FASTCALL   | 0x40000000)
#define SIP_FUNCTION       (SMC64_FUNCTION | 0x02000000)
#define SIP_FUNCTION_ID(n) (SIP_FUNCTION   | (n))

/*
 * We do not use SMCCC_ARCH_SOC_ID here because qemu_sbsa is virtual platform
 * which uses SoC present in QEMU. And they can change on their own while we
 * need version of whole 'virtual hardware platform'.
 */
#define SIP_SVC_VERSION  SIP_FUNCTION_ID(1)
#define SIP_SVC_GET_GIC  SIP_FUNCTION_ID(100)
#define SIP_SVC_GET_GIC_ITS SIP_FUNCTION_ID(101)
#define SIP_SVC_GET_CPU_COUNT SIP_FUNCTION_ID(200)
#define SIP_SVC_GET_CPU_NODE SIP_FUNCTION_ID(201)
#define SIP_SVC_GET_CPU_TOPOLOGY SIP_FUNCTION_ID(202)
#define SIP_SVC_GET_MEMORY_NODE_COUNT SIP_FUNCTION_ID(300)
#define SIP_SVC_GET_MEMORY_NODE SIP_FUNCTION_ID(301)

uintptr_t sbsa_get_gicd(void);
uintptr_t sbsa_get_gicr(void);

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uintptr_t sbsa_sip_smc_handler(uint32_t smc_fid,
			       u_register_t x1,
			       u_register_t x2,
			       u_register_t x3,
			       u_register_t x4,
			       void *cookie,
			       void *handle,
			       u_register_t flags)
{
	uint32_t ns;
	uint64_t index;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		ERROR("%s: wrong world SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	switch (smc_fid) {
	case SIP_SVC_VERSION:
		INFO("Platform version requested\n");
		SMC_RET3(handle, NULL, sbsa_platform_version_major(),
			 sbsa_platform_version_minor());

	case SIP_SVC_GET_GIC:
		SMC_RET3(handle, NULL, sbsa_get_gicd(), sbsa_get_gicr());

	case SIP_SVC_GET_GIC_ITS:
		SMC_RET2(handle, NULL, sbsa_platform_gic_its_addr());

	case SIP_SVC_GET_CPU_COUNT:
		SMC_RET2(handle, NULL, sbsa_platform_num_cpus());

	case SIP_SVC_GET_CPU_NODE:
		index = x1;
		if (index < PLATFORM_CORE_COUNT) {
			struct platform_cpu_data data;

			data = sbsa_platform_cpu_node(index);

			SMC_RET3(handle, NULL, data.nodeid, data.mpidr);
		} else {
			SMC_RET1(handle, SMC_ARCH_CALL_INVAL_PARAM);
		}

	case SIP_SVC_GET_CPU_TOPOLOGY: {
		struct platform_cpu_topology topology;

		topology = sbsa_platform_cpu_topology();

		if (topology.cores > 0) {
			SMC_RET5(handle, NULL, topology.sockets,
				 topology.clusters, topology.cores,
				 topology.threads);
		} else {
			/* we do not know topology so we report SMC as unknown */
			SMC_RET1(handle, SMC_UNK);
		}
	}

	case SIP_SVC_GET_MEMORY_NODE_COUNT:
		SMC_RET2(handle, NULL, sbsa_platform_num_memnodes());

	case SIP_SVC_GET_MEMORY_NODE:
		index = x1;
		if (index < PLAT_MAX_MEM_NODES) {
			struct platform_memory_data data;

			data = sbsa_platform_memory_node(index);

			SMC_RET4(handle, NULL, data.nodeid,
				 data.addr_base, data.addr_size);
		} else {
			SMC_RET1(handle, SMC_ARCH_CALL_INVAL_PARAM);
		}

	default:
		ERROR("%s: unhandled SMC (0x%x) (function id: %d)\n", __func__, smc_fid,
		      smc_fid - SIP_FUNCTION);
		SMC_RET1(handle, SMC_UNK);
	}
}

int sbsa_sip_smc_setup(void)
{
	return 0;
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	sbsa_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	sbsa_sip_smc_setup,
	sbsa_sip_smc_handler
);
