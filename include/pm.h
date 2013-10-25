/*
 * Copyright (c) 2013, ARM Limited. All rights reserved.
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

#ifndef __PM_H__
#define __PM_H__

#ifndef __ASSEMBLY__

/*******************************************************************************
 * Structure populated by platform specific code to export routines which
 * perform common low level pm functions
 ******************************************************************************/
typedef struct {
	int (*cpu_on)(unsigned long);
	int (*cpu_off)(unsigned long);
	int (*cpu_suspend)(unsigned long);
	int (*affinity_info)(unsigned long, unsigned int);
} pm_frontend_ops;

/*******************************************************************************
 * Structure populated by a generic power management api implementation e.g.
 * psci to perform api specific bits after a cpu has been turned on.
 ******************************************************************************/
typedef struct {
	unsigned long (*cpu_off_finisher)(unsigned long);
	unsigned long (*cpu_suspend_finisher)(unsigned long);
} pm_backend_ops;

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
extern pm_frontend_ops *get_pm_frontend_ops(void);
extern pm_backend_ops *get_pm_backend_ops(void);
extern void set_pm_frontend_ops(pm_frontend_ops *);
extern void set_pm_backend_ops(pm_backend_ops *);

#endif /*__ASSEMBLY__*/

#endif /* __PM_H__ */
