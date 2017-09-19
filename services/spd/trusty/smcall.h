/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMCALL_H
#define SMCALL_H

#define SMC_NUM_ENTITIES	64U
#define SMC_NUM_ARGS		4U
#define SMC_NUM_PARAMS		(SMC_NUM_ARGS - 1U)

#define SMC_IS_FASTCALL(smc_nr)	((smc_nr) & 0x80000000U)
#define SMC_IS_SMC64(smc_nr)	((smc_nr) & 0x40000000U)
#define SMC_ENTITY(smc_nr)	(((smc_nr) & 0x3F000000U) >> 24U)
#define SMC_FUNCTION(smc_nr)	((smc_nr) & 0x0000FFFFU)

#define SMC_NR(entity, fn, fastcall, smc64)			\
		(((((uint32_t)(fastcall)) & 0x1U) << 31U) |	\
		(((smc64) & 0x1U) << 30U) |			\
		(((entity) & 0x3FU) << 24U) |			\
		((fn) & 0xFFFFU))

#define SMC_FASTCALL_NR(entity, fn)	SMC_NR((entity), (fn), 1U, 0U)
#define SMC_FASTCALL64_NR(entity, fn)	SMC_NR((entity), (fn), 1U, 1U)
#define SMC_YIELDCALL_NR(entity, fn)	SMC_NR((entity), (fn), 0U, 0U)
#define SMC_YIELDCALL64_NR(entity, fn)	SMC_NR((entity), (fn), 0U, 1U)

#define	SMC_ENTITY_ARCH			0U	/* ARM Architecture calls */
#define	SMC_ENTITY_CPU			1U	/* CPU Service calls */
#define	SMC_ENTITY_SIP			2U	/* SIP Service calls */
#define	SMC_ENTITY_OEM			3U	/* OEM Service calls */
#define	SMC_ENTITY_STD			4U	/* Standard Service calls */
#define	SMC_ENTITY_RESERVED		5U	/* Reserved for future use */
#define	SMC_ENTITY_TRUSTED_APP		48U	/* Trusted Application calls */
#define	SMC_ENTITY_TRUSTED_OS		50U	/* Trusted OS calls */
#define SMC_ENTITY_LOGGING              51U	/* Used for secure -> nonsecure logging */
#define	SMC_ENTITY_SECURE_MONITOR	60U	/* Trusted OS calls internal to secure monitor */

/* FC = Fast call, YC = Yielding call */
#define SMC_YC_RESTART_LAST	SMC_YIELDCALL_NR  (SMC_ENTITY_SECURE_MONITOR, 0U)
#define SMC_YC_NOP		SMC_YIELDCALL_NR  (SMC_ENTITY_SECURE_MONITOR, 1U)

/*
 * Return from secure os to non-secure os with return value in r1
 */
#define SMC_YC_NS_RETURN	SMC_YIELDCALL_NR  (SMC_ENTITY_SECURE_MONITOR, 0U)

#define SMC_FC_RESERVED		SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 0U)
#define SMC_FC_FIQ_EXIT		SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 1U)
#define SMC_FC_REQUEST_FIQ	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 2U)
#define SMC_FC_GET_NEXT_IRQ	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 3U)
#define SMC_FC_FIQ_ENTER	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 4U)

#define SMC_FC64_SET_FIQ_HANDLER SMC_FASTCALL64_NR(SMC_ENTITY_SECURE_MONITOR, 5U)
#define SMC_FC64_GET_FIQ_REGS	SMC_FASTCALL64_NR (SMC_ENTITY_SECURE_MONITOR, 6U)

#define SMC_FC_CPU_SUSPEND	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 7U)
#define SMC_FC_CPU_RESUME	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 8U)

#define SMC_FC_AARCH_SWITCH	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 9U)
#define SMC_FC_GET_VERSION_STR	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 10U)

/* Trusted OS entity calls */
#define SMC_YC_VIRTIO_GET_DESCR	  SMC_YIELDCALL_NR(SMC_ENTITY_TRUSTED_OS, 20U)
#define SMC_YC_VIRTIO_START	  SMC_YIELDCALL_NR(SMC_ENTITY_TRUSTED_OS, 21U)
#define SMC_YC_VIRTIO_STOP	  SMC_YIELDCALL_NR(SMC_ENTITY_TRUSTED_OS, 22U)

#define SMC_YC_VDEV_RESET	  SMC_YIELDCALL_NR(SMC_ENTITY_TRUSTED_OS, 23U)
#define SMC_YC_VDEV_KICK_VQ	  SMC_YIELDCALL_NR(SMC_ENTITY_TRUSTED_OS, 24U)
#define SMC_YC_SET_ROT_PARAMS	  SMC_YIELDCALL_NR(SMC_ENTITY_TRUSTED_OS, 65535U)

#endif /* SMCALL_H */
