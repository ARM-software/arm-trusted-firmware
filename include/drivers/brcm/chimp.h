/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SR_CHIMP_H
#define SR_CHIMP_H

#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define CHIMP_WINDOW_SIZE 0x400000
#define CHIMP_ERROR_OFFSET 28
#define CHIMP_ERROR_MASK 0xf0000000

#ifndef EMULATION_SETUP
#define CHIMP_HANDSHAKE_TIMEOUT_MS 10000
#else
/*
 * 1hr timeout for test in emulator
 * By doing this ChiMP is given a chance to boot
 * fully from the QSPI
 * (on Palladium this takes upto 50 min depending on QSPI clk)
 */

#define CHIMP_HANDSHAKE_TIMEOUT_MS 3600000
#endif

#define CHIMP_BPE_MODE_ID_PATTERN				(0x25000000)
#define CHIMP_BPE_MODE_ID_MASK					(0x7f000000)
#define NIC_RESET_RELEASE_TIMEOUT_US		(10)

/* written by M0, used by ChiMP ROM */
#define SR_IN_SMARTNIC_MODE_BIT					0
/* written by M0, used by ChiMP ROM */
#define SR_CHIMP_SECURE_BOOT_BIT				1
/* cleared by AP, set by ChiMP BC2 code */
#define SR_FLASH_ACCESS_DONE_BIT				2

#ifdef USE_CHIMP
void bcm_chimp_write(uintptr_t addr, uint32_t value);
uint32_t bcm_chimp_read(uintptr_t addr);
uint32_t bcm_chimp_read_ctrl(uint32_t offset);
void bcm_chimp_clrbits(uintptr_t addr, uint32_t bits);
void bcm_chimp_setbits(uintptr_t addr, uint32_t bits);
int bcm_chimp_is_nic_mode(void);
void bcm_chimp_fru_prog_done(bool status);
int bcm_chimp_handshake_done(void);
int bcm_chimp_wait_handshake(void);
/* Fastboot-related*/
int bcm_chimp_initiate_fastboot(int fastboot_type);
#else
static inline void bcm_chimp_write(uintptr_t addr, uint32_t value)
{
}
static inline uint32_t bcm_chimp_read(uintptr_t addr)
{
	return 0;
}
static inline uint32_t bcm_chimp_read_ctrl(uint32_t offset)
{
	return 0;
}
static inline void bcm_chimp_clrbits(uintptr_t addr, uint32_t bits)
{
}
static inline void bcm_chimp_setbits(uintptr_t addr, uint32_t bits)
{
}
static inline int bcm_chimp_is_nic_mode(void)
{
	return 0;
}
static inline void bcm_chimp_fru_prog_done(bool status)
{
}
static inline int bcm_chimp_handshake_done(void)
{
	return 0;
}
static inline int bcm_chimp_wait_handshake(void)
{
	return 0;
}
static inline int bcm_chimp_initiate_fastboot(int fastboot_type)
{
	return 0;
}
#endif /* USE_CHIMP */
#endif
