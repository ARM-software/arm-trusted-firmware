/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __BL2_H__
#define __BL2_H__

/******************************************
 * Data declarations
 *****************************************/
extern unsigned long long bl2_entrypoint;

/******************************************
 * Forward declarations
 *****************************************/
struct meminfo;
struct bl31_args;

/******************************************
 * Function prototypes
 *****************************************/
extern void bl2_platform_setup(void);
extern struct meminfo *bl2_plat_sec_mem_layout(void);

/*******************************************************************************
 * This function returns a pointer to the shared memory that the platform has
 * kept aside to pass trusted firmware related information that BL3-1
 * could need
 ******************************************************************************/
extern struct bl31_params *bl2_plat_get_bl31_params(void);

/*******************************************************************************
 * This function returns a pointer to the shared memory that the platform has
 * kept aside to pass platform related information that BL3-1 could need
 ******************************************************************************/
extern struct bl31_plat_params *bl2_plat_get_bl31_plat_params(void);

/*******************************************************************************
 * This function returns a pointer to the shared memory that the platform
 * has kept to point to entry point information of BL31 to BL2
 ******************************************************************************/
extern struct el_change_info *bl2_plat_get_bl31_ep(void);


/************************************************************************
 * This function flushes to main memory all the params that are
 * passed to BL3-1
 **************************************************************************/
extern void bl2_plat_flush_bl31_params(void);


#endif /* __BL2_H__ */
