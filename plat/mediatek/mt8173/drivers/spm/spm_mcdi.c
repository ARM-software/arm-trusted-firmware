/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <arch.h>
#include <debug.h>
#include <mmio.h>
#include <mt8173_def.h>
#include <platform.h>
#include <platform_def.h>
#include <spm.h>
#include <spm_hotplug.h>
#include <spm_mcdi.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware.
 * This driver controls the cpu power in cpu idle power saving state.
 */

#define WAKE_SRC_FOR_MCDI	(WAKE_SRC_SYSPWREQ | WAKE_SRC_CPU_IRQ)
#define PCM_MCDI_HANDSHAKE_SYNC	0xbeefbeef
#define PCM_MCDI_HANDSHAKE_ACK	0xdeaddead
#define PCM_MCDI_UPDATE_INFORM	0xabcdabcd
#define PCM_MCDI_CKECK_DONE	0x12345678
#define PCM_MCDI_ALL_CORE_AWAKE	0x0
#define PCM_MCDI_OFFLOADED	0xaa55aa55

static const unsigned int mcdi_binary[] = {
	0x1212841f, 0xe2e00036, 0xe2e0003e, 0x1380201f, 0xe2e0003c, 0xe2a00000,
	0x1b80001f, 0x20000080, 0xe2e0007c, 0x1b80001f, 0x20000003, 0xe2e0005c,
	0xe2e0004c, 0xe2e0004d, 0xf0000000, 0x17c07c1f, 0xe2e0004f, 0xe2e0006f,
	0xe2e0002f, 0xe2a00001, 0x1b80001f, 0x20000080, 0xe2e0002e, 0xe2e0003e,
	0xe2e00032, 0xf0000000, 0x17c07c1f, 0x1212841f, 0xe2e00026, 0xe2e0002e,
	0x1380201f, 0x1a00001f, 0x100062b4, 0x1910001f, 0x100062b4, 0x81322804,
	0xe2000004, 0x81202804, 0xe2000004, 0x1b80001f, 0x20000034, 0x1910001f,
	0x100062b4, 0x81142804, 0xd8000524, 0x17c07c1f, 0xe2e0000e, 0xe2e0000c,
	0xe2e0000d, 0xf0000000, 0x17c07c1f, 0xe2e0002d, 0x1a00001f, 0x100062b4,
	0x1910001f, 0x100062b4, 0xa1002804, 0xe2000004, 0xa1122804, 0xe2000004,
	0x1b80001f, 0x20000080, 0x1910001f, 0x100062b4, 0x81142804, 0xd82007c4,
	0x17c07c1f, 0xe2e0002f, 0xe2e0002b, 0xe2e00023, 0x1380201f, 0xe2e00022,
	0xf0000000, 0x17c07c1f, 0x18c0001f, 0x10006b6c, 0x1910001f, 0x10006b6c,
	0xa1002804, 0xe0c00004, 0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001, 0x11407c1f, 0xe8208000,
	0x10006b6c, 0xa0000000, 0xe8208000, 0x10006310, 0x0b160008, 0x1900001f,
	0x000f7bde, 0x1a00001f, 0x10200268, 0xe2000004, 0xe8208000, 0x10006600,
	0x00000000, 0xc2800940, 0x1280041f, 0x1b00001f, 0x21000001, 0x1b80001f,
	0xd0010000, 0xc2800940, 0x1290841f, 0x69200006, 0xbeefbeef, 0xd8204764,
	0x17c07c1f, 0xc2800940, 0x1291041f, 0x1910001f, 0x10006358, 0x810b1001,
	0xd80043e4, 0x17c07c1f, 0x1980001f, 0xdeaddead, 0x69200006, 0xabcdabcd,
	0xd82044c4, 0x17c07c1f, 0xc2800940, 0x1291841f, 0x88900001, 0x10006814,
	0x1910001f, 0x10006400, 0x81271002, 0x1880001f, 0x10006600, 0xe0800004,
	0x1910001f, 0x10006358, 0x810b1001, 0xd8004684, 0x17c07c1f, 0x1980001f,
	0x12345678, 0x60a07c05, 0x89100002, 0x10006600, 0x80801001, 0xd8007142,
	0x17c07c1f, 0xc2800940, 0x1292041f, 0x1a10001f, 0x10006720, 0x82002001,
	0x82201408, 0xd8204a08, 0x17c07c1f, 0x1a40001f, 0x10006200, 0x1a80001f,
	0x1000625c, 0xc2400200, 0x17c07c1f, 0xa1400405, 0xc2800940, 0x1292841f,
	0x1a10001f, 0x10006720, 0x8200a001, 0x82209408, 0xd8204be8, 0x17c07c1f,
	0x1a40001f, 0x10006218, 0x1a80001f, 0x10006264, 0xc2400200, 0x17c07c1f,
	0xa1508405, 0xc2800940, 0x1293041f, 0x1a10001f, 0x10006720, 0x82012001,
	0x82211408, 0xd8204dc8, 0x17c07c1f, 0x1a40001f, 0x1000621c, 0x1a80001f,
	0x1000626c, 0xc2400200, 0x17c07c1f, 0xa1510405, 0x1a10001f, 0x10006720,
	0x8201a001, 0x82219408, 0xd8204f68, 0x17c07c1f, 0x1a40001f, 0x10006220,
	0x1a80001f, 0x10006274, 0xc2400200, 0x17c07c1f, 0xa1518405, 0x1a10001f,
	0x10006720, 0x82022001, 0x82221408, 0xd82050e8, 0x17c07c1f, 0x1a40001f,
	0x100062a0, 0x1280041f, 0xc2400660, 0x17c07c1f, 0xa1520405, 0x1a10001f,
	0x10006720, 0x8202a001, 0x82229408, 0xd8205268, 0x17c07c1f, 0x1a40001f,
	0x100062a4, 0x1290841f, 0xc2400660, 0x17c07c1f, 0xa1528405, 0x1a10001f,
	0x10006720, 0x82032001, 0x82231408, 0xd82053e8, 0x17c07c1f, 0x1a40001f,
	0x100062a8, 0x1291041f, 0xc2400660, 0x17c07c1f, 0xa1530405, 0x1a10001f,
	0x10006720, 0x8203a001, 0x82239408, 0xd8205568, 0x17c07c1f, 0x1a40001f,
	0x100062ac, 0x1291841f, 0xc2400660, 0x17c07c1f, 0xa1538405, 0x1b80001f,
	0x20000208, 0xd82070cc, 0x17c07c1f, 0x81001401, 0xd8205964, 0x17c07c1f,
	0x1a10001f, 0x10006918, 0x81002001, 0xb1042081, 0xb1003081, 0xb10c3081,
	0xd8205964, 0x17c07c1f, 0x1a40001f, 0x10006200, 0x1a80001f, 0x1000625c,
	0xc2400000, 0x17c07c1f, 0x89400005, 0xfffffffe, 0xe8208000, 0x10006f00,
	0x00000000, 0xe8208000, 0x10006b30, 0x00000000, 0xe8208000, 0x100063e0,
	0x00000001, 0x81009401, 0xd8205cc4, 0x17c07c1f, 0x1a10001f, 0x10006918,
	0x8100a001, 0xb104a081, 0xb1003081, 0xd8205cc4, 0x17c07c1f, 0x1a40001f,
	0x10006218, 0x1a80001f, 0x10006264, 0xc2400000, 0x17c07c1f, 0x89400005,
	0xfffffffd, 0xe8208000, 0x10006f04, 0x00000000, 0xe8208000, 0x10006b34,
	0x00000000, 0xe8208000, 0x100063e0, 0x00000002, 0x81011401, 0xd8206024,
	0x17c07c1f, 0x1a10001f, 0x10006918, 0x81012001, 0xb1052081, 0xb1003081,
	0xd8206024, 0x17c07c1f, 0x1a40001f, 0x1000621c, 0x1a80001f, 0x1000626c,
	0xc2400000, 0x17c07c1f, 0x89400005, 0xfffffffb, 0xe8208000, 0x10006f08,
	0x00000000, 0xe8208000, 0x10006b38, 0x00000000, 0xe8208000, 0x100063e0,
	0x00000004, 0x81019401, 0xd8206384, 0x17c07c1f, 0x1a10001f, 0x10006918,
	0x8101a001, 0xb105a081, 0xb1003081, 0xd8206384, 0x17c07c1f, 0x1a40001f,
	0x10006220, 0x1a80001f, 0x10006274, 0xc2400000, 0x17c07c1f, 0x89400005,
	0xfffffff7, 0xe8208000, 0x10006f0c, 0x00000000, 0xe8208000, 0x10006b3c,
	0x00000000, 0xe8208000, 0x100063e0, 0x00000008, 0x81021401, 0xd82066c4,
	0x17c07c1f, 0x1a10001f, 0x10006918, 0x81022001, 0xb1062081, 0xb1003081,
	0xd82066c4, 0x17c07c1f, 0x1a40001f, 0x100062a0, 0x1280041f, 0xc2400360,
	0x17c07c1f, 0x89400005, 0xffffffef, 0xe8208000, 0x10006f10, 0x00000000,
	0xe8208000, 0x10006b40, 0x00000000, 0xe8208000, 0x100063e0, 0x00000010,
	0x81029401, 0xd8206a04, 0x17c07c1f, 0x1a10001f, 0x10006918, 0x8102a001,
	0xb106a081, 0xb1003081, 0xd8206a04, 0x17c07c1f, 0x1a40001f, 0x100062a4,
	0x1290841f, 0xc2400360, 0x17c07c1f, 0x89400005, 0xffffffdf, 0xe8208000,
	0x10006f14, 0x00000000, 0xe8208000, 0x10006b44, 0x00000000, 0xe8208000,
	0x100063e0, 0x00000020, 0x81031401, 0xd8206d44, 0x17c07c1f, 0x1a10001f,
	0x10006918, 0x81032001, 0xb1072081, 0xb1003081, 0xd8206d44, 0x17c07c1f,
	0x1a40001f, 0x100062a8, 0x1291041f, 0xc2400360, 0x17c07c1f, 0x89400005,
	0xffffffbf, 0xe8208000, 0x10006f18, 0x00000000, 0xe8208000, 0x10006b48,
	0x00000000, 0xe8208000, 0x100063e0, 0x00000040, 0x81039401, 0xd8207084,
	0x17c07c1f, 0x1a10001f, 0x10006918, 0x8103a001, 0xb107a081, 0xb1003081,
	0xd8207084, 0x17c07c1f, 0x1a40001f, 0x100062ac, 0x1291841f, 0xc2400360,
	0x17c07c1f, 0x89400005, 0xffffff7f, 0xe8208000, 0x10006f1c, 0x00000000,
	0xe8208000, 0x10006b4c, 0x00000000, 0xe8208000, 0x100063e0, 0x00000080,
	0xc2800940, 0x1293841f, 0xd0004260, 0x17c07c1f, 0xc2800940, 0x1294041f,
	0xe8208000, 0x10006600, 0x00000000, 0x1ac0001f, 0x55aa55aa, 0x1940001f,
	0xaa55aa55, 0xc2800940, 0x1294841f, 0x1b80001f, 0x00001000, 0xf0000000,
	0x17c07c1f
};

static const struct pcm_desc mcdi_pcm = {
	.version = "pcm_mcdi_v0.5_20140721_mt8173_v03.04_20150507",
	.base = mcdi_binary,
	.size = 919,
	.sess = 2,
	.replace = 0,
};

static struct pwr_ctrl mcdi_ctrl = {
	.wake_src = WAKE_SRC_FOR_MCDI,
	.wake_src_md32 = 0,
	.wfi_op = WFI_OP_OR,
	.mcusys_idle_mask = 1,
	.ca7top_idle_mask = 1,
	.ca15top_idle_mask = 1,
	.disp_req_mask = 1,
	.mfg_req_mask = 1,
	.md32_req_mask = 1,
};

static const struct spm_lp_scen spm_mcdi = {
	.pcmdesc = &mcdi_pcm,
	.pwrctrl = &mcdi_ctrl,
};

void spm_mcdi_cpu_wake_up_event(int wake_up_event, int disable_dormant_power)
{
	if (((mmio_read_32(SPM_SLEEP_CPU_WAKEUP_EVENT) & 0x1) == 1)
	    && ((mmio_read_32(SPM_CLK_CON) & CC_DISABLE_DORM_PWR) == 0)) {
		/* MCDI is offload? */
		INFO("%s: SPM_SLEEP_CPU_WAKEUP_EVENT:%x, SPM_CLK_CON %x",
		     __func__, mmio_read_32(SPM_SLEEP_CPU_WAKEUP_EVENT),
		     mmio_read_32(SPM_CLK_CON));
		return;
	}
	/* Inform SPM that CPU wants to program CPU_WAKEUP_EVENT and
	 * DISABLE_CPU_DROM */
	mmio_write_32(SPM_PCM_REG_DATA_INI, PCM_MCDI_HANDSHAKE_SYNC);
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R6);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);

	/* Wait SPM's response, can't use sleep api */
	while (mmio_read_32(SPM_PCM_REG6_DATA) != PCM_MCDI_HANDSHAKE_ACK)
		;

	if (disable_dormant_power) {
		mmio_setbits_32(SPM_CLK_CON, CC_DISABLE_DORM_PWR);
		while (mmio_read_32(SPM_CLK_CON) !=
			(mmio_read_32(SPM_CLK_CON) | CC_DISABLE_DORM_PWR))
			;

	} else {
		mmio_clrbits_32(SPM_CLK_CON, CC_DISABLE_DORM_PWR);
		while (mmio_read_32(SPM_CLK_CON) !=
			(mmio_read_32(SPM_CLK_CON) & ~CC_DISABLE_DORM_PWR))
			;
	}

	mmio_write_32(SPM_SLEEP_CPU_WAKEUP_EVENT, wake_up_event);

	while (mmio_read_32(SPM_SLEEP_CPU_WAKEUP_EVENT) != wake_up_event)
		;

	/* Inform SPM to see updated setting */
	mmio_write_32(SPM_PCM_REG_DATA_INI, PCM_MCDI_UPDATE_INFORM);
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R6);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);

	while (mmio_read_32(SPM_PCM_REG6_DATA) != PCM_MCDI_CKECK_DONE)
		;
	/* END OF sequence */

	mmio_write_32(SPM_PCM_REG_DATA_INI, 0x0);
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R6);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);
}

void spm_mcdi_wakeup_all_cores(void)
{
	if (is_mcdi_ready() == 0)
		return;

	spm_mcdi_cpu_wake_up_event(1, 1);
	while (mmio_read_32(SPM_PCM_REG5_DATA) != PCM_MCDI_ALL_CORE_AWAKE)
		;
	spm_mcdi_cpu_wake_up_event(1, 0);
	while (mmio_read_32(SPM_PCM_REG5_DATA) != PCM_MCDI_OFFLOADED)
		;

	spm_clean_after_wakeup();
	clear_all_ready();
}

void spm_mcdi_wfi_sel_enter(unsigned long mpidr)
{
	int core_id_val = mpidr & MPIDR_CPU_MASK;
	int cluster_id = (mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	/* SPM WFI Select by core number */
	if (cluster_id) {
		switch (core_id_val) {
		case 0:
			mmio_write_32(SPM_CA15_CPU0_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA15_WFI0_EN, 1);
			break;
		case 1:
			mmio_write_32(SPM_CA15_CPU1_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA15_WFI1_EN, 1);
			break;
		case 2:
			mmio_write_32(SPM_CA15_CPU2_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA15_WFI2_EN, 1);
			break;
		case 3:
			mmio_write_32(SPM_CA15_CPU3_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA15_WFI3_EN, 1);
			break;
		default:
			break;
		}
	} else {
		switch (core_id_val) {
		case 0:
			mmio_write_32(SPM_CA7_CPU0_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, 1);
			break;
		case 1:
			mmio_write_32(SPM_CA7_CPU1_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, 1);
			break;
		case 2:
			mmio_write_32(SPM_CA7_CPU2_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, 1);
			break;
		case 3:
			mmio_write_32(SPM_CA7_CPU3_IRQ_MASK, 1);
			mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, 1);
			break;
		default:
			break;
		}
	}
}

void spm_mcdi_wfi_sel_leave(unsigned long mpidr)
{
	int core_id_val = mpidr & MPIDR_CPU_MASK;
	int cluster_id = (mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	/* SPM WFI Select by core number */
	if (cluster_id) {
		switch (core_id_val) {
		case 0:
			mmio_write_32(SPM_SLEEP_CA15_WFI0_EN, 0);
			mmio_write_32(SPM_CA15_CPU0_IRQ_MASK, 0);
			break;
		case 1:
			mmio_write_32(SPM_SLEEP_CA15_WFI1_EN, 0);
			mmio_write_32(SPM_CA15_CPU1_IRQ_MASK, 0);
			break;
		case 2:
			mmio_write_32(SPM_SLEEP_CA15_WFI2_EN, 0);
			mmio_write_32(SPM_CA15_CPU2_IRQ_MASK, 0);
			break;
		case 3:
			mmio_write_32(SPM_SLEEP_CA15_WFI3_EN, 0);
			mmio_write_32(SPM_CA15_CPU3_IRQ_MASK, 0);
			break;
		default:
			break;
		}
	} else {
		switch (core_id_val) {
		case 0:
			mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, 0);
			mmio_write_32(SPM_CA7_CPU0_IRQ_MASK, 0);
			break;
		case 1:
			mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, 0);
			mmio_write_32(SPM_CA7_CPU1_IRQ_MASK, 0);
			break;
		case 2:
			mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, 0);
			mmio_write_32(SPM_CA7_CPU2_IRQ_MASK, 0);
			break;
		case 3:
			mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, 0);
			mmio_write_32(SPM_CA7_CPU3_IRQ_MASK, 0);
			break;
		default:
			break;
		}
	}
}

void spm_mcdi_prepare(unsigned long mpidr)
{
	const struct pcm_desc *pcmdesc = spm_mcdi.pcmdesc;
	struct pwr_ctrl *pwrctrl = spm_mcdi.pwrctrl;

	spm_lock_get();
	if (is_mcdi_ready() == 0) {
		if (is_hotplug_ready() == 1)
			spm_clear_hotplug();
		set_pwrctrl_pcm_flags(pwrctrl, 0);
		spm_reset_and_init_pcm();
		spm_kick_im_to_fetch(pcmdesc);
		spm_set_power_control(pwrctrl);
		spm_set_wakeup_event(pwrctrl);
		spm_kick_pcm_to_run(pwrctrl);
		set_mcdi_ready();
	}
	spm_mcdi_wfi_sel_enter(mpidr);
	spm_lock_release();
}

void spm_mcdi_finish(unsigned long mpidr)
{
	unsigned long linear_id = platform_get_core_pos(mpidr);

	spm_lock_get();
	spm_mcdi_wfi_sel_leave(mpidr);
	mmio_write_32(SPM_PCM_SW_INT_CLEAR, (0x1 << linear_id));
	spm_lock_release();
}
