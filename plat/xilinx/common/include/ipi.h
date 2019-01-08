/*
 * Copyright (c) 2018, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Xilinx IPI management configuration data and macros */

#ifndef IPI_H
#define IPI_H

#include <stdint.h>

/*********************************************************************
 * Struct definitions
 ********************************************************************/

/* structure to maintain IPI configuration information */
struct ipi_config {
	unsigned int ipi_bit_mask;
	unsigned int ipi_reg_base;
	unsigned char secure_only;
};

#endif /* IPI_H */
