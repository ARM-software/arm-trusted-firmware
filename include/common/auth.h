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

#ifndef AUTH_H_
#define AUTH_H_

#include <stddef.h>
#include <stdint.h>

/*
 * Authentication infrastructure for Trusted Boot
 *
 * This infrastructure provides an API to access the authentication module. This
 * module will implement the required operations for Trusted Boot by creating an
 * instance of the structure 'auth_mod_t'. This instance must be called
 * 'auth_mod' and must provide the functions to initialize the module and
 * verify the authenticity of the images.
 */

/* Objects (images and certificates) involved in the TBB process */
enum {
	AUTH_BL2_IMG_CERT,
	AUTH_BL2_IMG,
	AUTH_TRUSTED_KEY_CERT,
	AUTH_BL30_KEY_CERT,
	AUTH_BL30_IMG_CERT,
	AUTH_BL30_IMG,
	AUTH_BL31_KEY_CERT,
	AUTH_BL31_IMG_CERT,
	AUTH_BL31_IMG,
	AUTH_BL32_KEY_CERT,
	AUTH_BL32_IMG_CERT,
	AUTH_BL32_IMG,
	AUTH_BL33_KEY_CERT,
	AUTH_BL33_IMG_CERT,
	AUTH_BL33_IMG,
	AUTH_NUM_OBJ
};

/* Authentication module structure */
typedef struct auth_mod_s {
	/* [mandatory] Module name. Printed to the log during initialization */
	const char *name;

	/* [mandatory] Initialize the authentication module */
	int (*init)(void);

	/* [mandatory] This function will be called to authenticate a new
	 * object loaded into memory. The obj_id corresponds to one of the
	 * values in the enumeration above */
	int (*verify)(unsigned int obj_id, uintptr_t obj_buf, size_t len);
} auth_mod_t;

/* This variable must be instantiated by the authentication module */
extern const auth_mod_t auth_mod;

/* Public functions */
void auth_init(void);
int auth_verify_obj(unsigned int obj_id, uintptr_t obj_buf, size_t len);

#endif /* AUTH_H_ */
