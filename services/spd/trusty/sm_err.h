/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __LIB_SM_SM_ERR_H
#define __LIB_SM_SM_ERR_H

/* Errors from the secure monitor */
#define SM_ERR_UNDEFINED_SMC		0xFFFFFFFF /* Unknown SMC (defined by ARM DEN 0028A(0.9.0) */
#define SM_ERR_INVALID_PARAMETERS	-2
#define SM_ERR_INTERRUPTED		-3	/* Got interrupted. Call back with restart SMC */
#define SM_ERR_UNEXPECTED_RESTART	-4	/* Got an restart SMC when we didn't expect it */
#define SM_ERR_BUSY			-5	/* Temporarily busy. Call back with original args */
#define SM_ERR_INTERLEAVED_SMC		-6	/* Got a trusted_service SMC when a restart SMC is required */
#define SM_ERR_INTERNAL_FAILURE		-7	/* Unknown error */
#define SM_ERR_NOT_SUPPORTED		-8
#define SM_ERR_NOT_ALLOWED		-9	/* SMC call not allowed */
#define SM_ERR_END_OF_INPUT		-10

#endif
