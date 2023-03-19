/*
 * Copyright (C) 2017-2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ARBEL_CLOCK_H_
#define __ARBEL_CLOCK_H_

struct clk_ctl {
	unsigned int	clken1;
	unsigned int	clksel;
	unsigned int	clkdiv1;
	unsigned int	pllcon0;
	unsigned int	pllcon1;
	unsigned int	swrstr;
	unsigned char	res1[0x8];
	unsigned int	ipsrst1;
	unsigned int	ipsrst2;
	unsigned int	clken2;
	unsigned int	clkdiv2;
	unsigned int	clken3;
	unsigned int	ipsrst3;
	unsigned int	wd0rcr;
	unsigned int	wd1rcr;
	unsigned int	wd2rcr;
	unsigned int	swrstc1;
	unsigned int	swrstc2;
	unsigned int	swrstc3;
	unsigned int	tiprstc;
	unsigned int	pllcon2;
	unsigned int	clkdiv3;
	unsigned int	corstc;
	unsigned int	pllcong;
	unsigned int	ahbckfi;
	unsigned int	seccnt;
	unsigned int	cntr25m;
	unsigned int	clken4;
	unsigned int	ipsrst4;
	unsigned int	busto;
	unsigned int	clkdiv4;
	unsigned int	wd0rcrb;
	unsigned int	wd1rcrb;
	unsigned int	wd2rcrb;
	unsigned int	swrstc1b;
	unsigned int	swrstc2b;
	unsigned int	swrstc3b;
	unsigned int	tiprstcb;
	unsigned int	corstcb;
	unsigned int	ipsrstdis1;
	unsigned int	ipsrstdis2;
	unsigned int	ipsrstdis3;
	unsigned int	ipsrstdis4;
	unsigned char	res2[0x10];
	unsigned int	thrtl_cnt;
};

#endif /* __ARBEL_CLOCK_H_ */
