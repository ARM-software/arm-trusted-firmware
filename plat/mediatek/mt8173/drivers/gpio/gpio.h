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
#ifndef __PLAT_DRIVER_GPIO_H__
#define __PLAT_DRIVER_GPIO_H__

#include <stdint.h>

enum {
	GPIOEXT_BASE = 0xC000,
};

/* Error Code No. */
enum {
	RSUCCESS = 0,
	ERACCESS,
	ERINVAL,
	ERWRAPPER,
};

enum {
	GPIO_UNSUPPORTED = -1,

	GPIO0,   GPIO1,   GPIO2,   GPIO3,   GPIO4,   GPIO5,   GPIO6,   GPIO7,
	GPIO8,   GPIO9,   GPIO10,  GPIO11,  GPIO12,  GPIO13,  GPIO14,  GPIO15,
	GPIO16,  GPIO17,  GPIO18,  GPIO19,  GPIO20,  GPIO21,  GPIO22,  GPIO23,
	GPIO24,  GPIO25,  GPIO26,  GPIO27,  GPIO28,  GPIO29,  GPIO30,  GPIO31,
	GPIO32,  GPIO33,  GPIO34,  GPIO35,  GPIO36,  GPIO37,  GPIO38,  GPIO39,
	GPIO40,  GPIO41,  GPIO42,  GPIO43,  GPIO44,  GPIO45,  GPIO46,  GPIO47,
	GPIO48,  GPIO49,  GPIO50,  GPIO51,  GPIO52,  GPIO53,  GPIO54,  GPIO55,
	GPIO56,  GPIO57,  GPIO58,  GPIO59,  GPIO60,  GPIO61,  GPIO62,  GPIO63,
	GPIO64,  GPIO65,  GPIO66,  GPIO67,  GPIO68,  GPIO69,  GPIO70,  GPIO71,
	GPIO72,  GPIO73,  GPIO74,  GPIO75,  GPIO76,  GPIO77,  GPIO78,  GPIO79,
	GPIO80,  GPIO81,  GPIO82,  GPIO83,  GPIO84,  GPIO85,  GPIO86,  GPIO87,
	GPIO88,  GPIO89,  GPIO90,  GPIO91,  GPIO92,  GPIO93,  GPIO94,  GPIO95,
	GPIO96,  GPIO97,  GPIO98,  GPIO99,  GPIO100, GPIO101, GPIO102, GPIO103,
	GPIO104, GPIO105, GPIO106, GPIO107, GPIO108, GPIO109, GPIO110, GPIO111,
	GPIO112, GPIO113, GPIO114, GPIO115, GPIO116, GPIO117, GPIO118, GPIO119,
	GPIO120, GPIO121, GPIO122, GPIO123, GPIO124, GPIO125, GPIO126, GPIO127,
	GPIO128, GPIO129, GPIO130, GPIO131, GPIO132, GPIO133, GPIO134,

	GPIOEXT0,  GPIOEXT1,  GPIOEXT2,  GPIOEXT3,  GPIOEXT4,  GPIOEXT5,
	GPIOEXT6,  GPIOEXT7,  GPIOEXT8,  GPIOEXT9,  GPIOEXT10, GPIOEXT11,
	GPIOEXT12, GPIOEXT13, GPIOEXT14, GPIOEXT15, GPIOEXT16, GPIOEXT17,
	GPIOEXT18, GPIOEXT19, GPIOEXT20, GPIOEXT21, GPIOEXT22, GPIOEXT23,
	GPIOEXT24, GPIOEXT25, GPIOEXT26, GPIOEXT27, GPIOEXT28, GPIOEXT29,
	GPIOEXT30, GPIOEXT31, GPIOEXT32, GPIOEXT33, GPIOEXT34, GPIOEXT35,
	GPIOEXT36, GPIOEXT37, GPIOEXT38, GPIOEXT39, GPIOEXT40,

	GPIO_MAX
};

#define MAX_GPIO_PIN		GPIO_MAX

#define GPIO_EXTEND_START	GPIOEXT0

/* GPIO DIRECTION */
enum {
	GPIO_DIR_UNSUPPORTED = -1,
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT = 1,
	GPIO_DIR_MAX,
	GPIO_DIR_DEFAULT = GPIO_DIR_IN,
};

/* GPIO OUTPUT */
enum {
	GPIO_OUT_UNSUPPORTED = -1,
	GPIO_OUT_ZERO = 0,
	GPIO_OUT_ONE = 1,
	GPIO_OUT_MAX,
	GPIO_OUT_DEFAULT = GPIO_OUT_ZERO,
	GPIO_DATA_OUT_DEFAULT = GPIO_OUT_ZERO,	/* compatible with DCT */
};

struct val_regs {
	uint16_t val;
	uint16_t _align1;
	uint16_t set;
	uint16_t _align2;
	uint16_t rst;
	uint16_t _align3[3];
};

struct gpio_regs {
	struct val_regs dir[9];		/* 0x0000 ~ 0x008F: 144 bytes */
	uint8_t rsv00[112];		/* 0x0090 ~ 0x00FF: 112 bytes */
	struct val_regs pullen[9];	/* 0x0100 ~ 0x018F: 144 bytes */
	uint8_t rsv01[112];		/* 0x0190 ~ 0x01FF: 112 bytes */
	struct val_regs pullsel[9];	/* 0x0200 ~ 0x028F: 144 bytes */
	uint8_t rsv02[112];		/* 0x0290 ~ 0x02FF: 112 bytes */
	uint8_t rsv03[256];		/* 0x0300 ~ 0x03FF: 256 bytes */
	struct val_regs dout[9];	/* 0x0400 ~ 0x048F: 144 bytes */
	uint8_t rsv04[112];		/* 0x0490 ~ 0x04FF: 112 bytes */
	struct val_regs din[9];		/* 0x0500 ~ 0x058F: 114 bytes */
	uint8_t rsv05[112];		/* 0x0590 ~ 0x05FF: 112 bytes */
	struct val_regs mode[27];	/* 0x0600 ~ 0x07AF: 432 bytes */
	uint8_t rsv06[336];		/* 0x07B0 ~ 0x08FF: 336 bytes */
	struct val_regs ies[3];		/* 0x0900 ~ 0x092F:  48 bytes */
	struct val_regs smt[3];		/* 0x0930 ~ 0x095F:  48 bytes */
	uint8_t rsv07[160];		/* 0x0960 ~ 0x09FF: 160 bytes */
	struct val_regs tdsel[8];	/* 0x0A00 ~ 0x0A7F: 128 bytes */
	struct val_regs rdsel[6];	/* 0x0A80 ~ 0x0ADF:  96 bytes */
	uint8_t rsv08[32];		/* 0x0AE0 ~ 0x0AFF:  32 bytes */
	struct val_regs drv_mode[10];	/* 0x0B00 ~ 0x0B9F: 160 bytes */
	uint8_t rsv09[96];		/* 0x0BA0 ~ 0x0BFF:  96 bytes */
	struct val_regs msdc0_ctrl0;	/* 0x0C00 ~ 0x0C0F:  16 bytes */
	struct val_regs msdc0_ctrl1;	/* 0x0C10 ~ 0x0C1F:  16 bytes */
	struct val_regs msdc0_ctrl2;	/* 0x0C20 ~ 0x0C2F:  16 bytes */
	struct val_regs msdc0_ctrl5;	/* 0x0C30 ~ 0x0C3F:  16 bytes */
	struct val_regs msdc1_ctrl0;	/* 0x0C40 ~ 0x0C4F:  16 bytes */
	struct val_regs msdc1_ctrl1;	/* 0x0C50 ~ 0x0C5F:  16 bytes */
	struct val_regs msdc1_ctrl2;	/* 0x0C60 ~ 0x0C6F:  16 bytes */
	struct val_regs msdc1_ctrl5;	/* 0x0C70 ~ 0x0C7F:  16 bytes */
	struct val_regs msdc2_ctrl0;	/* 0x0C80 ~ 0x0C8F:  16 bytes */
	struct val_regs msdc2_ctrl1;	/* 0x0C90 ~ 0x0C9F:  16 bytes */
	struct val_regs msdc2_ctrl2;	/* 0x0CA0 ~ 0x0CAF:  16 bytes */
	struct val_regs msdc2_ctrl5;	/* 0x0CB0 ~ 0x0CBF:  16 bytes */
	struct val_regs msdc3_ctrl0;	/* 0x0CC0 ~ 0x0CCF:  16 bytes */
	struct val_regs msdc3_ctrl1;	/* 0x0CD0 ~ 0x0CDF:  16 bytes */
	struct val_regs msdc3_ctrl2;	/* 0x0CE0 ~ 0x0CEF:  16 bytes */
	struct val_regs msdc3_ctrl5;	/* 0x0CF0 ~ 0x0CFF:  16 bytes */
	struct val_regs msdc0_ctrl3;	/* 0x0D00 ~ 0x0D0F:  16 bytes */
	struct val_regs msdc0_ctrl4;	/* 0x0D10 ~ 0x0D1F:  16 bytes */
	struct val_regs msdc1_ctrl3;	/* 0x0D20 ~ 0x0D2F:  16 bytes */
	struct val_regs msdc1_ctrl4;	/* 0x0D30 ~ 0x0D3F:  16 bytes */
	struct val_regs msdc2_ctrl3;	/* 0x0D40 ~ 0x0D4F:  16 bytes */
	struct val_regs msdc2_ctrl4;	/* 0x0D50 ~ 0x0D5F:  16 bytes */
	struct val_regs msdc3_ctrl3;	/* 0x0D60 ~ 0x0D6F:  16 bytes */
	struct val_regs msdc3_ctrl4;	/* 0x0D70 ~ 0x0D7F:  16 bytes */
	uint8_t rsv10[64];		/* 0x0D80 ~ 0x0DBF:  64 bytes */
	struct val_regs exmd_ctrl[1];	/* 0x0DC0 ~ 0x0DCF:  16 bytes */
	uint8_t rsv11[48];		/* 0x0DD0 ~ 0x0DFF:  48 bytes */
	struct val_regs kpad_ctrl[2];	/* 0x0E00 ~ 0x0E1F:  32 bytes */
	struct val_regs hsic_ctrl[4];	/* 0x0E20 ~ 0x0E5F:  64 bytes */
};

struct ext_val_regs {
	uint16_t val;
	uint16_t set;
	uint16_t rst;
	uint16_t _align;
};

struct gpioext_regs {
	struct ext_val_regs dir[4];	/* 0x0000 ~ 0x001F: 32 bytes */
	struct ext_val_regs pullen[4];	/* 0x0020 ~ 0x003F: 32 bytes */
	struct ext_val_regs pullsel[4];	/* 0x0040 ~ 0x005F: 32 bytes */
	struct ext_val_regs dinv[4];	/* 0x0060 ~ 0x007F: 32 bytes */
	struct ext_val_regs dout[4];	/* 0x0080 ~ 0x009F: 32 bytes */
	struct ext_val_regs din[4];	/* 0x00A0 ~ 0x00BF: 32 bytes */
	struct ext_val_regs mode[10];	/* 0x00C0 ~ 0x010F: 80 bytes */
};

/* GPIO Driver interface */
void gpio_set(uint32_t gpio, int32_t value);

#endif /* __PLAT_DRIVER_GPIO_H__ */
