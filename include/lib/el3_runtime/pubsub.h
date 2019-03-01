/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PUBSUB_H
#define PUBSUB_H

#ifdef __LINKER__

/* For the linker ... */
#define __pubsub_start_sym(event)	__pubsub_##event##_start
#define __pubsub_end_sym(event)		__pubsub_##event##_end
#define __pubsub_section(event)		__pubsub_##event

/*
 * REGISTER_PUBSUB_EVENT has a different definition between linker and compiler
 * contexts. In linker context, this collects pubsub sections for each event,
 * placing guard symbols around each.
 */
#if defined(USE_ARM_LINK)
#define REGISTER_PUBSUB_EVENT(event) \
	__pubsub_start_sym(event) +0 FIXED \
	{ \
		*(__pubsub_section(event)) \
	} \
	__pubsub_end_sym(event) +0 FIXED EMPTY 0 \
	{ \
		/* placeholder */ \
	}
#else
#define REGISTER_PUBSUB_EVENT(event) \
	__pubsub_start_sym(event) = .; \
	KEEP(*(__pubsub_section(event))); \
	__pubsub_end_sym(event) = .
#endif

#else /* __LINKER__ */

/* For the compiler ... */

#include <assert.h>
#include <cdefs.h>
#include <stddef.h>

#include <arch_helpers.h>

#if defined(USE_ARM_LINK)
#define __pubsub_start_sym(event)	Load$$__pubsub_##event##_start$$Base
#define __pubsub_end_sym(event)		Load$$__pubsub_##event##_end$$Base
#else
#define __pubsub_start_sym(event)	__pubsub_##event##_start
#define __pubsub_end_sym(event)		__pubsub_##event##_end
#endif

#define __pubsub_section(event)		__section("__pubsub_" #event)

/*
 * In compiler context, REGISTER_PUBSUB_EVENT declares the per-event symbols
 * exported by the linker required for the other pubsub macros to work.
 */
#define REGISTER_PUBSUB_EVENT(event) \
	extern pubsub_cb_t __pubsub_start_sym(event)[]; \
	extern pubsub_cb_t __pubsub_end_sym(event)[]

/*
 * Have the function func called back when the specified event happens. This
 * macro places the function address into the pubsub section, which is picked up
 * and invoked by the invoke_pubsubs() function via the PUBLISH_EVENT* macros.
 *
 * The extern declaration is there to satisfy MISRA C-2012 rule 8.4.
 */
#define SUBSCRIBE_TO_EVENT(event, func) \
	extern pubsub_cb_t __cb_func_##func##event __pubsub_section(event); \
	pubsub_cb_t __cb_func_##func##event __pubsub_section(event) = (func)

/*
 * Iterate over subscribed handlers for a defined event. 'event' is the name of
 * the event, and 'subscriber' a local variable of type 'pubsub_cb_t *'.
 */
#define for_each_subscriber(event, subscriber) \
	for (subscriber = __pubsub_start_sym(event); \
			subscriber < __pubsub_end_sym(event); \
			subscriber++)

/*
 * Publish a defined event supplying an argument. All subscribed handlers are
 * invoked, but the return value of handlers are ignored for now.
 */
#define PUBLISH_EVENT_ARG(event, arg) \
	do { \
		pubsub_cb_t *subscriber; \
		for_each_subscriber(event, subscriber) { \
			(*subscriber)(arg); \
		} \
	} while (0)

/* Publish a defined event with NULL argument */
#define PUBLISH_EVENT(event)	PUBLISH_EVENT_ARG(event, NULL)

/* Subscriber callback type */
typedef void* (*pubsub_cb_t)(const void *arg);

#endif	/* __LINKER__ */
#endif /* PUBSUB_H */
