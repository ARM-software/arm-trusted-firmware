/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_GPIO_H
#define MT_GPIO_H

#include <plat/common/common_def.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
//  Error Code No.
#define RSUCCESS        0
#define ERACCESS        1
#define ERINVAL         2
#define ERWRAPPER       3
/*----------------------------------------------------------------------------*/

#define MAX_GPIO_PIN    (MT_GPIO_BASE_MAX)
/******************************************************************************
* Enumeration for GPIO pin
******************************************************************************/

typedef enum GPIO_PIN {
	GPIO_UNSUPPORTED = -1,

	GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7,
	GPIO8, GPIO9, GPIO10, GPIO11, GPIO12, GPIO13, GPIO14, GPIO15,
	GPIO16, GPIO17, GPIO18, GPIO19, GPIO20, GPIO21, GPIO22, GPIO23,
	GPIO24, GPIO25, GPIO26, GPIO27, GPIO28, GPIO29, GPIO30, GPIO31,
	GPIO32, GPIO33, GPIO34, GPIO35, GPIO36, GPIO37, GPIO38, GPIO39,
	GPIO40, GPIO41, GPIO42, GPIO43, GPIO44, GPIO45, GPIO46, GPIO47,
	GPIO48, GPIO49, GPIO50, GPIO51, GPIO52, GPIO53, GPIO54, GPIO55,
	GPIO56, GPIO57, GPIO58, GPIO59, GPIO60, GPIO61, GPIO62, GPIO63,
	GPIO64, GPIO65, GPIO66, GPIO67, GPIO68, GPIO69, GPIO70, GPIO71,
	GPIO72, GPIO73, GPIO74, GPIO75, GPIO76, GPIO77, GPIO78, GPIO79,
	GPIO80, GPIO81, GPIO82, GPIO83, GPIO84, GPIO85, GPIO86, GPIO87,
	GPIO88, GPIO89, GPIO90, GPIO91, GPIO92, GPIO93, GPIO94, GPIO95,
	GPIO96, GPIO97, GPIO98, GPIO99, GPIO100, GPIO101, GPIO102, GPIO103,
	GPIO104, GPIO105, GPIO106, GPIO107, GPIO108, GPIO109, GPIO110, GPIO111,
	GPIO112, GPIO113, GPIO114, GPIO115, GPIO116, GPIO117, GPIO118, GPIO119,
	GPIO120, GPIO121, GPIO122, GPIO123, GPIO124, GPIO125, GPIO126, GPIO127,
	GPIO128, GPIO129, GPIO130, GPIO131, GPIO132, GPIO133, GPIO134, GPIO135,
	GPIO136, GPIO137, GPIO138, GPIO139, GPIO140, GPIO141, GPIO142, GPIO143,
	GPIO144, GPIO145, GPIO146, GPIO147, GPIO148, GPIO149, GPIO150, GPIO151,
	GPIO152, GPIO153, GPIO154, GPIO155, GPIO156, GPIO157, GPIO158, GPIO159,
	GPIO160, GPIO161, GPIO162, GPIO163, GPIO164, GPIO165, GPIO166, GPIO167,
	GPIO168, GPIO169, GPIO170, GPIO171, GPIO172, GPIO173, GPIO174, GPIO175,
	GPIO176, GPIO177, GPIO178, GPIO179,
	MT_GPIO_BASE_MAX
} GPIO_PIN;
/* GPIO MODE CONTROL VALUE*/
typedef enum {
	GPIO_MODE_UNSUPPORTED = -1,
	GPIO_MODE_GPIO  = 0,
	GPIO_MODE_00    = 0,
	GPIO_MODE_01    = 1,
	GPIO_MODE_02    = 2,
	GPIO_MODE_03    = 3,
	GPIO_MODE_04    = 4,
	GPIO_MODE_05    = 5,
	GPIO_MODE_06    = 6,
	GPIO_MODE_07    = 7,

	GPIO_MODE_MAX,
	GPIO_MODE_DEFAULT = GPIO_MODE_00,
} GPIO_MODE;
/*----------------------------------------------------------------------------*/
/* GPIO DIRECTION */
typedef enum {
	GPIO_DIR_UNSUPPORTED = -1,
	GPIO_DIR_OUT    = 0,
	GPIO_DIR_IN     = 1,
	GPIO_DIR_MAX,
	GPIO_DIR_DEFAULT = GPIO_DIR_IN,
} GPIO_DIR;
/*----------------------------------------------------------------------------*/
/* GPIO PULL ENABLE*/
typedef enum {
	GPIO_PULL_EN_UNSUPPORTED = -1,
	GPIO_PULL_DISABLE   = 0,
	GPIO_PULL_ENABLE    = 1,
	GPIO_PULL_ENABLE_R0 = 2,
	GPIO_PULL_ENABLE_R1 = 3,
	GPIO_PULL_ENABLE_R0R1 = 4,

	GPIO_PULL_EN_MAX,
	GPIO_PULL_EN_DEFAULT = GPIO_PULL_ENABLE,
} GPIO_PULL_EN;
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* GPIO PULL-UP/PULL-DOWN*/
typedef enum {
	GPIO_PULL_UNSUPPORTED = -1,
	GPIO_PULL_NONE        = 0,
	GPIO_PULL_UP          = 1,
	GPIO_PULL_DOWN        = 2,
	GPIO_PULL_MAX,
	GPIO_PULL_DEFAULT = GPIO_PULL_DOWN
} GPIO_PULL;
/*----------------------------------------------------------------------------*/
/* GPIO OUTPUT */
typedef enum {
	GPIO_OUT_UNSUPPORTED = -1,
	GPIO_OUT_ZERO = 0,
	GPIO_OUT_ONE  = 1,

	GPIO_OUT_MAX,
	GPIO_OUT_DEFAULT = GPIO_OUT_ZERO,
	GPIO_DATA_OUT_DEFAULT = GPIO_OUT_ZERO,  /*compatible with DCT*/
} GPIO_OUT;
/*----------------------------------------------------------------------------*/
/* GPIO INPUT */
typedef enum {
	GPIO_IN_UNSUPPORTED = -1,
	GPIO_IN_ZERO = 0,
	GPIO_IN_ONE  = 1,

	GPIO_IN_MAX,
} GPIO_IN;
/*----------------------------------------------------------------------------*/
typedef struct {
	uint32_t val;
	uint32_t set;
	uint32_t rst;
	uint32_t _align1;
} VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
	VAL_REGS dir[6];        /*0x0000 ~ 0x005F:  96 bytes */
	uint8_t rsv00[160];          /*0x0060 ~ 0x00FF: 160 bytes */
	VAL_REGS dout[6];       /*0x0100 ~ 0x015F:  96 bytes */
	uint8_t rsv01[160];          /*0x0160 ~ 0x01FF: 160 bytes */
	VAL_REGS din[6];        /*0x0200 ~ 0x025F:  96 bytes */
	uint8_t rsv02[160];          /*0x0260 ~ 0x02FF: 160 bytes */
	VAL_REGS mode[23];      /*0x0300 ~ 0x046F: 368 bytes */
} GPIO_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
	uint32_t no     : 16;
	uint32_t mode   : 3;
	uint32_t pullsel: 1;
	uint32_t din    : 1;
	uint32_t dout   : 1;
	uint32_t pullen : 1;
	uint32_t dir    : 1;
	uint32_t dinv   : 1;
	uint32_t _align : 7;
} GPIO_CFG;

#define DIR_BASE     GPIO_BASE + 0x000
#define DOUT_BASE    GPIO_BASE + 0x100
#define DIN_BASE     GPIO_BASE + 0x200
#define MODE_BASE    GPIO_BASE + 0x300
#define SET          0x4
#define CLR          0x8

/******************************************************************************
* GPIO Driver interface
******************************************************************************/
/*direction*/
void mt_set_gpio_dir(int gpio, int direction);
int mt_get_gpio_dir(int gpio);

/*pull select*/
void mt_set_gpio_pull(int gpio, int pull);
int mt_get_gpio_pull(int gpio);

/*input/output*/
void mt_set_gpio_out(int gpio, int value);
int mt_get_gpio_out(int gpio);
int mt_get_gpio_in(int gpio);

/*mode control*/
void mt_set_gpio_mode(int gpio, int mode);
int mt_get_gpio_mode(int gpio);
#endif /* MCSI_H */
