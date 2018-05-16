/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SGI_RAS__
#define __SGI_RAS__

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

#endif /* __SGI_RAS__ */
