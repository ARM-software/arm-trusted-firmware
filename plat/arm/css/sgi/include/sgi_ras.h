/*
 * Copyright (c) 2018-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI_RAS_H
#define SGI_RAS_H

/* Platform specific SMC FID's used for DMC-620 RAS error handling */
#define SP_DMC_ERROR_OVERFLOW_EVENT_AARCH64	0xC4000043
#define SP_DMC_ERROR_ECC_EVENT_AARCH64		0xC4000044

/*
 * Mapping the RAS interrupt with SDEI event number and the event
 * id used with Standalone MM code
 */
struct sgi_ras_ev_map {
	int ras_ev_num;		/* RAS Event number */
	int sdei_ev_num;	/* SDEI Event number */
	int intr;		/* Physical intr number */
};

int sgi_ras_intr_handler_setup(void);

#endif /* SGI_RAS_H */
