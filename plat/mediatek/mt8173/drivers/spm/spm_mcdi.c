/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

#define WAKE_SRC_FOR_MCDI \
	(WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT |		\
	 WAKE_SRC_MD32 | WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN |	\
	 WAKE_SRC_AFE | WAKE_SRC_THERM | WAKE_SRC_CIRQ |	\
	 WAKE_SRC_SYSPWREQ | WAKE_SRC_CPU_IRQ)
#define PCM_MCDI_HANDSHAKE_SYNC	0xbeefbeef
#define PCM_MCDI_HANDSHAKE_ACK	0xdeaddead
#define PCM_MCDI_UPDATE_INFORM	0xabcdabcd
#define PCM_MCDI_CKECK_DONE	0x12345678
#define PCM_MCDI_ALL_CORE_AWAKE	0x0
#define PCM_MCDI_OFFLOADED	0xaa55aa55
#define PCM_MCDI_CA72_CPUTOP_PWRCTL	(0x1 << 16)
#define PCM_MCDI_CA53_CPUTOP_PWRCTL	(0x1 << 17)
#define PCM_MCDI_CA72_PWRSTA_SHIFT	16
#define PCM_MCDI_CA53_PWRSTA_SHIFT	9

static const unsigned int mcdi_binary[] = {
	0x1a10001f, 0x10006b04, 0x1890001f, 0x10006b6c, 0x1a40001f, 0x10006210,
	0x18d0001f, 0x10006210, 0x81002001, 0xd82001c4, 0x17c07c1f, 0xa0900402,
	0xc2401540, 0x17c07c1f, 0x81052001, 0xd8200284, 0x17c07c1f, 0xa0950402,
	0xc2401b80, 0x17c07c1f, 0x1a40001f, 0x10006230, 0x18d0001f, 0x10006230,
	0x8100a001, 0xd82003c4, 0x17c07c1f, 0xa0908402, 0xc2401540, 0x17c07c1f,
	0x8105a001, 0xd8200484, 0x17c07c1f, 0xa0958402, 0xc2401b80, 0x17c07c1f,
	0x1a40001f, 0x10006238, 0x18d0001f, 0x10006238, 0x81012001, 0xd82005c4,
	0x17c07c1f, 0xa0910402, 0xc2401540, 0x17c07c1f, 0x81062001, 0xd8200684,
	0x17c07c1f, 0xa0960402, 0xc2401b80, 0x17c07c1f, 0x1a40001f, 0x1000623c,
	0x18d0001f, 0x1000623c, 0x8101a001, 0xd82007c4, 0x17c07c1f, 0xa0918402,
	0xc2401540, 0x17c07c1f, 0x8106a001, 0xd8200884, 0x17c07c1f, 0xa0968402,
	0xc2401b80, 0x17c07c1f, 0x1a40001f, 0x10006298, 0x18d0001f, 0x10006298,
	0x81022001, 0xd82009c4, 0x17c07c1f, 0xa0920402, 0xc2401540, 0x17c07c1f,
	0x81072001, 0xd8200a84, 0x17c07c1f, 0xa0970402, 0xc2401b80, 0x17c07c1f,
	0x1a40001f, 0x1000629c, 0x18d0001f, 0x1000629c, 0x8102a001, 0xd8200bc4,
	0x17c07c1f, 0xa0928402, 0xc2401540, 0x17c07c1f, 0x8107a001, 0xd8200c84,
	0x17c07c1f, 0xa0978402, 0xc2401b80, 0x17c07c1f, 0x1a40001f, 0x100062c4,
	0x18d0001f, 0x100062c4, 0x81032001, 0xd8200dc4, 0x17c07c1f, 0xa0930402,
	0xc2401540, 0x17c07c1f, 0x81082001, 0xd8200e84, 0x17c07c1f, 0xa0980402,
	0xc2401b80, 0x17c07c1f, 0x1a40001f, 0x100062c0, 0x18d0001f, 0x100062c0,
	0x8103a001, 0xd8200fc4, 0x17c07c1f, 0xa0938402, 0xc2401540, 0x17c07c1f,
	0x8108a001, 0xd8201084, 0x17c07c1f, 0xa0988402, 0xc2401b80, 0x17c07c1f,
	0x1a40001f, 0x10006214, 0x18d0001f, 0x10006214, 0x81042001, 0xd82011c4,
	0x17c07c1f, 0xa0940402, 0xc2401540, 0x17c07c1f, 0x81092001, 0xd8201284,
	0x17c07c1f, 0xa0990402, 0xc2401b80, 0x17c07c1f, 0x1a40001f, 0x100062cc,
	0x18d0001f, 0x100062cc, 0x8104a001, 0xd82013c4, 0x17c07c1f, 0xa0948402,
	0xc2401540, 0x17c07c1f, 0x8109a001, 0xd8201484, 0x17c07c1f, 0xa0998402,
	0xc2401b80, 0x17c07c1f, 0x1900001f, 0x10006b6c, 0x80802002, 0xe1000002,
	0xf0000000, 0x17c07c1f, 0xa8c00003, 0x00000004, 0xe2400003, 0xa8c00003,
	0x00000008, 0xe2400003, 0x1b80001f, 0x00000020, 0x88c00003, 0xffffffef,
	0xe2400003, 0x88c00003, 0xfffffffd, 0xe2400003, 0xa8c00003, 0x00000001,
	0xe2400003, 0x88c00003, 0xfffff0ff, 0xe2400003, 0x1b80001f, 0x20000080,
	0x1a90001f, 0x10001220, 0x69200009, 0x1000623c, 0xd8001984, 0x17c07c1f,
	0x69200009, 0x10006214, 0xd8001a64, 0x17c07c1f, 0xd0001b00, 0x17c07c1f,
	0x1900001f, 0x10001220, 0x8a80000a, 0xfffffff9, 0xe100000a, 0xd0001b00,
	0x17c07c1f, 0x1900001f, 0x10001220, 0x8a80000a, 0xff1fbfff, 0xe100000a,
	0x1b80001f, 0x20000080, 0xf0000000, 0x17c07c1f, 0x1a90001f, 0x10001220,
	0x69200009, 0x1000623c, 0xd8001d04, 0x17c07c1f, 0x69200009, 0x10006214,
	0xd8001de4, 0x17c07c1f, 0xd0001e80, 0x17c07c1f, 0x1900001f, 0x10001220,
	0xaa80000a, 0x00000006, 0xe100000a, 0xd0001e80, 0x17c07c1f, 0x1900001f,
	0x10001220, 0xaa80000a, 0x00e04000, 0xe100000a, 0x1b80001f, 0x20000080,
	0x69200009, 0x10006214, 0xd8001fe4, 0x17c07c1f, 0xa8c00003, 0x00000f00,
	0xe2400003, 0xd0002040, 0x17c07c1f, 0xa8c00003, 0x00003f00, 0xe2400003,
	0x1b80001f, 0x20000080, 0xa8c00003, 0x00000002, 0xe2400003, 0x88c00003,
	0xfffffffe, 0xe2400003, 0xa8c00003, 0x00000010, 0xe2400003, 0x88c00003,
	0xfffffffb, 0xe2400003, 0x88c00003, 0xfffffff7, 0xe2400003, 0xf0000000,
	0x17c07c1f, 0xe2e00036, 0xe2e0003e, 0x1b80001f, 0x00000020, 0xe2e0003c,
	0xe8208000, 0x10006244, 0x00000000, 0x1b80001f, 0x20000080, 0xe2e0007c,
	0x1b80001f, 0x20000003, 0xe2e0005c, 0xe2e0004c, 0xe2e0004d, 0xf0000000,
	0x17c07c1f, 0xe2e0004f, 0xe2e0006f, 0xe2e0002f, 0xe8208000, 0x10006244,
	0x00000001, 0x1b80001f, 0x20000080, 0xe2e0002e, 0xe2e0003e, 0xe2e0003a,
	0xe2e00032, 0x1b80001f, 0x00000020, 0xf0000000, 0x17c07c1f, 0xe2e00036,
	0xe2e0003e, 0x1b80001f, 0x00000020, 0xe2e0003c, 0xe2a00000, 0x1b80001f,
	0x20000080, 0xe2e0007c, 0x1b80001f, 0x20000003, 0xe2e0005c, 0xe2e0004c,
	0xe2e0004d, 0xf0000000, 0x17c07c1f, 0xe2e0004f, 0xe2e0006f, 0xe2e0002f,
	0xe2a00001, 0x1b80001f, 0x20000080, 0xe2e0002e, 0xe2e0003e, 0xe2e0003a,
	0xe2e00032, 0xf0000000, 0x17c07c1f, 0xe2e00026, 0xe2e0002e, 0x1b80001f,
	0x00000020, 0x1a00001f, 0x100062b4, 0x1910001f, 0x100062b4, 0x81322804,
	0xe2000004, 0x81202804, 0xe2000004, 0x1b80001f, 0x20000080, 0xe2e0000e,
	0xe2e0000c, 0xe2e0000d, 0xf0000000, 0x17c07c1f, 0xe2e0002d, 0x1a00001f,
	0x100062b4, 0x1910001f, 0x100062b4, 0xa1002804, 0xe2000004, 0xa1122804,
	0xe2000004, 0x1b80001f, 0x20000080, 0xe2e0002f, 0xe2e0002b, 0xe2e00023,
	0x1b80001f, 0x00000020, 0xe2e00022, 0xf0000000, 0x17c07c1f, 0x1910001f,
	0x1000660c, 0x1a10001f, 0x10006610, 0xa2002004, 0x89000008, 0x00030000,
	0xd80036c4, 0x17c07c1f, 0x8207a001, 0xd82036c8, 0x17c07c1f, 0x1900001f,
	0x1020020c, 0x1a10001f, 0x1020020c, 0xaa000008, 0x00000001, 0xe1000008,
	0x1910001f, 0x1020020c, 0x81001001, 0xd8203184, 0x17c07c1f, 0x1910001f,
	0x10006720, 0x820c9001, 0xd8203228, 0x17c07c1f, 0x1900001f, 0x10001220,
	0x1a10001f, 0x10001220, 0xa21f0408, 0xe1000008, 0x1b80001f, 0x20000080,
	0xe2e0006d, 0xe2e0002d, 0x1a00001f, 0x100062b8, 0x1910001f, 0x100062b8,
	0xa9000004, 0x00000001, 0xe2000004, 0x1b80001f, 0x20000080, 0xe2e0002c,
	0xe2e0003c, 0xe2e0003e, 0xe2e0003a, 0xe2e00032, 0x1b80001f, 0x00000020,
	0x1900001f, 0x10006404, 0x1a10001f, 0x10006404, 0xa2168408, 0xe1000008,
	0xf0000000, 0x17c07c1f, 0x1a10001f, 0x10006610, 0x8207a001, 0xd8003e68,
	0x17c07c1f, 0x1a10001f, 0x10006918, 0x8a000008, 0x00003030, 0xb900010c,
	0x01000001, 0xd8203e64, 0x17c07c1f, 0x1900001f, 0x10006404, 0x1a10001f,
	0x10006404, 0x8a000008, 0x0000dfff, 0xe1000008, 0xe2e00036, 0xe2e0003e,
	0x1b80001f, 0x00000020, 0xe2e0002e, 0x1a00001f, 0x100062b8, 0x1910001f,
	0x100062b8, 0x89000004, 0x0000fffe, 0xe2000004, 0x1b80001f, 0x20000080,
	0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0xe2e0004d, 0x1900001f, 0x10001220,
	0x1a10001f, 0x10001220, 0x8a000008, 0xbfffffff, 0xe1000008, 0x1b80001f,
	0x20000080, 0x1900001f, 0x1020020c, 0x1a10001f, 0x1020020c, 0x8a000008,
	0xfffffffe, 0xe1000008, 0x1910001f, 0x1020020c, 0x81001001, 0xd8003dc4,
	0x17c07c1f, 0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
	0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001, 0x11407c1f, 0xe8208000,
	0x10006310, 0x0b160008, 0x1900001f, 0x000f7bde, 0x1a00001f, 0x10200268,
	0xe2000004, 0xe8208000, 0x10006600, 0x00000000, 0x69200006, 0xbeefbeef,
	0xd8204584, 0x17c07c1f, 0x1910001f, 0x10006358, 0x810b1001, 0xd8004244,
	0x17c07c1f, 0x1980001f, 0xdeaddead, 0x69200006, 0xabcdabcd, 0xd8204324,
	0x17c07c1f, 0x88900001, 0x10006814, 0x1910001f, 0x10006400, 0x81271002,
	0x1880001f, 0x10006600, 0xe0800004, 0x1910001f, 0x10006358, 0x810b1001,
	0xd80044a4, 0x17c07c1f, 0x1980001f, 0x12345678, 0x60a07c05, 0x89100002,
	0x10006600, 0x80801001, 0xd8007bc2, 0x17c07c1f, 0x1890001f, 0x10006b00,
	0x82090801, 0xc8800008, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x8a00000c,
	0x3fffe7ff, 0xd82041c8, 0x17c07c1f, 0x1b80001f, 0xd0010000, 0x1a10001f,
	0x10006720, 0x82002001, 0x82201408, 0xd8204988, 0x17c07c1f, 0x1a40001f,
	0x10006200, 0x1a80001f, 0x1000625c, 0xc24028e0, 0x17c07c1f, 0xa1400405,
	0x1a10001f, 0x10006720, 0x8200a001, 0x82209408, 0xd8204b28, 0x17c07c1f,
	0x1a40001f, 0x10006218, 0x1a80001f, 0x10006264, 0xc24028e0, 0x17c07c1f,
	0xa1508405, 0x1a10001f, 0x10006720, 0x82012001, 0x82211408, 0xd8204cc8,
	0x17c07c1f, 0x1a40001f, 0x1000621c, 0x1a80001f, 0x1000626c, 0xc24028e0,
	0x17c07c1f, 0xa1510405, 0x1a10001f, 0x10006720, 0x8201a001, 0x82219408,
	0xd8204e68, 0x17c07c1f, 0x1a40001f, 0x10006220, 0x1a80001f, 0x10006274,
	0xc24028e0, 0x17c07c1f, 0xa1518405, 0x1a10001f, 0x10006720, 0x82022001,
	0x82221408, 0xd8204fe8, 0x17c07c1f, 0x1a40001f, 0x100062a0, 0x1280041f,
	0xc2402cc0, 0x17c07c1f, 0xa1520405, 0x1a10001f, 0x10006720, 0x8202a001,
	0x82229408, 0xd8205168, 0x17c07c1f, 0x1a40001f, 0x100062a4, 0x1290841f,
	0xc2402cc0, 0x17c07c1f, 0xa1528405, 0x1a10001f, 0x10006720, 0x82032001,
	0x82231408, 0xd8205248, 0x17c07c1f, 0xa1530405, 0x1a10001f, 0x10006720,
	0x8203a001, 0x82239408, 0xd8205328, 0x17c07c1f, 0xa1538405, 0x1a10001f,
	0x10006b00, 0x8108a001, 0xd8205e84, 0x17c07c1f, 0x1910001f, 0x1000660c,
	0x1a10001f, 0x10006610, 0xa2002004, 0x89000008, 0x00001e00, 0xd8005944,
	0x17c07c1f, 0x82042001, 0xd8205948, 0x17c07c1f, 0x1900001f, 0x1020002c,
	0x1a10001f, 0x1020002c, 0xaa000008, 0x00000010, 0xe1000008, 0x1910001f,
	0x10006720, 0x820c1001, 0xd8205628, 0x17c07c1f, 0x1900001f, 0x10001250,
	0x1a10001f, 0x10001250, 0xa2110408, 0xe1000008, 0x1b80001f, 0x20000080,
	0x1900001f, 0x10001220, 0x1a10001f, 0x10001220, 0xa21e8408, 0xe1000008,
	0x1b80001f, 0x20000080, 0x1a40001f, 0x10006208, 0xc24024e0, 0x17c07c1f,
	0x1a10001f, 0x10006610, 0x82042001, 0xd8005e88, 0x17c07c1f, 0x1a10001f,
	0x10006918, 0x8a000008, 0x00000f0f, 0xba00010c, 0x1fffe7ff, 0xd8205e88,
	0x17c07c1f, 0x1a40001f, 0x10006208, 0xc24022a0, 0x17c07c1f, 0x1900001f,
	0x10001250, 0x1a10001f, 0x10001250, 0x8a000008, 0xfffffffb, 0xe1000008,
	0x1b80001f, 0x20000080, 0x1900001f, 0x10001220, 0x1a10001f, 0x10001220,
	0x8a000008, 0xdfffffff, 0xe1000008, 0x1b80001f, 0x20000080, 0x1900001f,
	0x1020002c, 0x1a10001f, 0x1020002c, 0x8a000008, 0xffffffef, 0xe1000008,
	0x1a10001f, 0x10006b00, 0x81082001, 0xd8205fa4, 0x17c07c1f, 0x1a40001f,
	0x100062b0, 0xc2402f20, 0x17c07c1f, 0x1b80001f, 0x20000208, 0xd8207b8c,
	0x17c07c1f, 0x1a40001f, 0x100062b0, 0xc2403700, 0x17c07c1f, 0x81001401,
	0xd8206424, 0x17c07c1f, 0x1a10001f, 0x10006918, 0x81002001, 0xb1042081,
	0xb900008c, 0x1fffe7ff, 0xd8206424, 0x17c07c1f, 0x1a40001f, 0x10006200,
	0x1a80001f, 0x1000625c, 0xc24026e0, 0x17c07c1f, 0x89400005, 0xfffffffe,
	0xe8208000, 0x10006f00, 0x00000000, 0xe8208000, 0x10006b30, 0x00000000,
	0xe8208000, 0x100063e0, 0x00000001, 0x81009401, 0xd82067a4, 0x17c07c1f,
	0x1a10001f, 0x10006918, 0x8100a001, 0xb104a081, 0xb900008c, 0x01000001,
	0xd82067a4, 0x17c07c1f, 0x1a40001f, 0x10006218, 0x1a80001f, 0x10006264,
	0xc24026e0, 0x17c07c1f, 0x89400005, 0xfffffffd, 0xe8208000, 0x10006f04,
	0x00000000, 0xe8208000, 0x10006b34, 0x00000000, 0xe8208000, 0x100063e0,
	0x00000002, 0x81011401, 0xd8206b24, 0x17c07c1f, 0x1a10001f, 0x10006918,
	0x81012001, 0xb1052081, 0xb900008c, 0x01000001, 0xd8206b24, 0x17c07c1f,
	0x1a40001f, 0x1000621c, 0x1a80001f, 0x1000626c, 0xc24026e0, 0x17c07c1f,
	0x89400005, 0xfffffffb, 0xe8208000, 0x10006f08, 0x00000000, 0xe8208000,
	0x10006b38, 0x00000000, 0xe8208000, 0x100063e0, 0x00000004, 0x81019401,
	0xd8206ea4, 0x17c07c1f, 0x1a10001f, 0x10006918, 0x8101a001, 0xb105a081,
	0xb900008c, 0x01000001, 0xd8206ea4, 0x17c07c1f, 0x1a40001f, 0x10006220,
	0x1a80001f, 0x10006274, 0xc24026e0, 0x17c07c1f, 0x89400005, 0xfffffff7,
	0xe8208000, 0x10006f0c, 0x00000000, 0xe8208000, 0x10006b3c, 0x00000000,
	0xe8208000, 0x100063e0, 0x00000008, 0x1a10001f, 0x10006610, 0x8207a001,
	0xd8207608, 0x17c07c1f, 0x81021401, 0xd82072a4, 0x17c07c1f, 0x1a10001f,
	0x10006918, 0x81022001, 0xb1062081, 0xb900008c, 0x01000001, 0xd82072a4,
	0x17c07c1f, 0x1a40001f, 0x100062a0, 0x1280041f, 0xc2402a60, 0x17c07c1f,
	0x89400005, 0xffffffef, 0xe8208000, 0x10006f10, 0x00000000, 0xe8208000,
	0x10006b40, 0x00000000, 0xe8208000, 0x100063e0, 0x00000010, 0x81029401,
	0xd8207604, 0x17c07c1f, 0x1a10001f, 0x10006918, 0x8102a001, 0xb106a081,
	0xb900008c, 0x01000001, 0xd8207604, 0x17c07c1f, 0x1a40001f, 0x100062a4,
	0x1290841f, 0xc2402a60, 0x17c07c1f, 0x89400005, 0xffffffdf, 0xe8208000,
	0x10006f14, 0x00000000, 0xe8208000, 0x10006b44, 0x00000000, 0xe8208000,
	0x100063e0, 0x00000020, 0x81031401, 0xd82078c4, 0x17c07c1f, 0x1a10001f,
	0x10006918, 0x81032001, 0xb1072081, 0xb900008c, 0x01000001, 0xd82078c4,
	0x17c07c1f, 0x89400005, 0xffffffbf, 0xe8208000, 0x10006f18, 0x00000000,
	0xe8208000, 0x10006b48, 0x00000000, 0xe8208000, 0x100063e0, 0x00000040,
	0x81039401, 0xd8207b84, 0x17c07c1f, 0x1a10001f, 0x10006918, 0x8103a001,
	0xb107a081, 0xb900008c, 0x01000001, 0xd8207b84, 0x17c07c1f, 0x89400005,
	0xffffff7f, 0xe8208000, 0x10006f1c, 0x00000000, 0xe8208000, 0x10006b4c,
	0x00000000, 0xe8208000, 0x100063e0, 0x00000080, 0xd00041c0, 0x17c07c1f,
	0xe8208000, 0x10006600, 0x00000000, 0x1ac0001f, 0x55aa55aa, 0x1940001f,
	0xaa55aa55, 0x1b80001f, 0x00001000, 0xf0000000, 0x17c07c1f
};

static const struct pcm_desc mcdi_pcm = {
	.version = "pcm_mcdi_mt8173_20160401_v1",
	.base = mcdi_binary,
	.size = 1001,
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

static void spm_mcdi_wfi_sel_enter(unsigned long mpidr)
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

static void spm_mcdi_wfi_sel_leave(unsigned long mpidr)
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

static void spm_mcdi_set_cputop_pwrctrl_for_cluster_off(unsigned long mpidr)
{
	unsigned long cluster_id = mpidr & MPIDR_CLUSTER_MASK;
	unsigned long cpu_id = mpidr & MPIDR_CPU_MASK;
	unsigned int pwr_status, shift, i, flag = 0;

	pwr_status = mmio_read_32(SPM_PWR_STATUS) |
				 mmio_read_32(SPM_PWR_STATUS_2ND);

	if (cluster_id) {
		for (i = 0; i < PLATFORM_CLUSTER1_CORE_COUNT; i++) {
			if (i == cpu_id)
				continue;
			shift = i + PCM_MCDI_CA72_PWRSTA_SHIFT;
			flag |= (pwr_status & (1 << shift)) >> shift;
		}
		if (!flag)
			mmio_setbits_32(SPM_PCM_RESERVE,
					PCM_MCDI_CA72_CPUTOP_PWRCTL);
	} else {
		for (i = 0; i < PLATFORM_CLUSTER0_CORE_COUNT; i++) {
			if (i == cpu_id)
				continue;
			shift = i + PCM_MCDI_CA53_PWRSTA_SHIFT;
			flag |= (pwr_status & (1 << shift)) >> shift;
		}
		if (!flag)
			mmio_setbits_32(SPM_PCM_RESERVE,
					PCM_MCDI_CA53_CPUTOP_PWRCTL);
	}
}

static void spm_mcdi_clear_cputop_pwrctrl_for_cluster_on(unsigned long mpidr)
{
	unsigned long cluster_id = mpidr & MPIDR_CLUSTER_MASK;

	if (cluster_id)
		mmio_clrbits_32(SPM_PCM_RESERVE,
				PCM_MCDI_CA72_CPUTOP_PWRCTL);
	else
		mmio_clrbits_32(SPM_PCM_RESERVE,
				PCM_MCDI_CA53_CPUTOP_PWRCTL);
}

void spm_mcdi_prepare_for_mtcmos(void)
{
	const struct pcm_desc *pcmdesc = spm_mcdi.pcmdesc;
	struct pwr_ctrl *pwrctrl = spm_mcdi.pwrctrl;

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
}

void spm_mcdi_prepare_for_off_state(unsigned long mpidr, unsigned int afflvl)
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
	if (afflvl == MPIDR_AFFLVL1)
		spm_mcdi_set_cputop_pwrctrl_for_cluster_off(mpidr);
	spm_lock_release();
}

void spm_mcdi_finish_for_on_state(unsigned long mpidr, unsigned int afflvl)
{
	unsigned long linear_id;

	linear_id = ((mpidr & MPIDR_CLUSTER_MASK) >> 6) |
			(mpidr & MPIDR_CPU_MASK);

	spm_lock_get();
	spm_mcdi_clear_cputop_pwrctrl_for_cluster_on(mpidr);
	spm_mcdi_wfi_sel_leave(mpidr);
	mmio_write_32(SPM_PCM_SW_INT_CLEAR, (0x1 << linear_id));
	spm_lock_release();
}
