/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LIB_SM_SMCALL_H
#define __LIB_SM_SMCALL_H

#define SMC_NUM_ENTITIES	64
#define SMC_NUM_ARGS		4
#define SMC_NUM_PARAMS		(SMC_NUM_ARGS - 1)

#define SMC_IS_FASTCALL(smc_nr)	((smc_nr) & 0x80000000)
#define SMC_IS_SMC64(smc_nr)	((smc_nr) & 0x40000000)
#define SMC_ENTITY(smc_nr)	(((smc_nr) & 0x3F000000) >> 24)
#define SMC_FUNCTION(smc_nr)	((smc_nr) & 0x0000FFFF)

#define SMC_NR(entity, fn, fastcall, smc64) ((((fastcall) & 0x1) << 31) | \
					     (((smc64) & 0x1) << 30) | \
					     (((entity) & 0x3F) << 24) | \
					     ((fn) & 0xFFFF) \
					    )

#define SMC_FASTCALL_NR(entity, fn)	SMC_NR((entity), (fn), 1, 0)
#define SMC_STDCALL_NR(entity, fn)	SMC_NR((entity), (fn), 0, 0)
#define SMC_FASTCALL64_NR(entity, fn)	SMC_NR((entity), (fn), 1, 1)
#define SMC_STDCALL64_NR(entity, fn)	SMC_NR((entity), (fn), 0, 1)

#define	SMC_ENTITY_ARCH			0	/* ARM Architecture calls */
#define	SMC_ENTITY_CPU			1	/* CPU Service calls */
#define	SMC_ENTITY_SIP			2	/* SIP Service calls */
#define	SMC_ENTITY_OEM			3	/* OEM Service calls */
#define	SMC_ENTITY_STD			4	/* Standard Service calls */
#define	SMC_ENTITY_RESERVED		5	/* Reserved for future use */
#define	SMC_ENTITY_TRUSTED_APP		48	/* Trusted Application calls */
#define	SMC_ENTITY_TRUSTED_OS		50	/* Trusted OS calls */
#define SMC_ENTITY_LOGGING              51	/* Used for secure -> nonsecure logging */
#define	SMC_ENTITY_SECURE_MONITOR	60	/* Trusted OS calls internal to secure monitor */

/* FC = Fast call, SC = Standard call */
#define SMC_SC_RESTART_LAST	SMC_STDCALL_NR  (SMC_ENTITY_SECURE_MONITOR, 0)
#define SMC_SC_NOP		SMC_STDCALL_NR  (SMC_ENTITY_SECURE_MONITOR, 1)

/*
 * Return from secure os to non-secure os with return value in r1
 */
#define SMC_SC_NS_RETURN	SMC_STDCALL_NR  (SMC_ENTITY_SECURE_MONITOR, 0)

#define SMC_FC_RESERVED		SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 0)
#define SMC_FC_FIQ_EXIT		SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 1)
#define SMC_FC_REQUEST_FIQ	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 2)
#define SMC_FC_GET_NEXT_IRQ	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 3)
#define SMC_FC_FIQ_ENTER	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 4)

#define SMC_FC64_SET_FIQ_HANDLER SMC_FASTCALL64_NR(SMC_ENTITY_SECURE_MONITOR, 5)
#define SMC_FC64_GET_FIQ_REGS	SMC_FASTCALL64_NR (SMC_ENTITY_SECURE_MONITOR, 6)

#define SMC_FC_CPU_SUSPEND	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 7)
#define SMC_FC_CPU_RESUME	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 8)

#define SMC_FC_AARCH_SWITCH	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 9)
#define SMC_FC_GET_VERSION_STR	SMC_FASTCALL_NR (SMC_ENTITY_SECURE_MONITOR, 10)

/* Trusted OS entity calls */
#define SMC_SC_VIRTIO_GET_DESCR	  SMC_STDCALL_NR(SMC_ENTITY_TRUSTED_OS, 20)
#define SMC_SC_VIRTIO_START	  SMC_STDCALL_NR(SMC_ENTITY_TRUSTED_OS, 21)
#define SMC_SC_VIRTIO_STOP	  SMC_STDCALL_NR(SMC_ENTITY_TRUSTED_OS, 22)

#define SMC_SC_VDEV_RESET	  SMC_STDCALL_NR(SMC_ENTITY_TRUSTED_OS, 23)
#define SMC_SC_VDEV_KICK_VQ	  SMC_STDCALL_NR(SMC_ENTITY_TRUSTED_OS, 24)

#endif /* __LIB_SM_SMCALL_H */
