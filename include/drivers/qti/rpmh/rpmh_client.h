/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RPMh command service interface.
 *
 * This is the interface used by resource owners (clock, regulator, bus, ...)
 * to issue commands to the RPMh hardware blocks through the APSS RSC. On the
 * TZ DRV the commands are issued synchronously as AMCs (Active Mode Commands)
 * and the issuing call blocks until the command set has been applied.
 */

#ifndef QTI_RPMH_CLIENT_H
#define QTI_RPMH_CLIENT_H

#include <stdbool.h>
#include <stdint.h>

/* Number of commands that fit in a single TCS. */
#ifndef IMAGE_TCS_SIZE
#define IMAGE_TCS_SIZE	16U
#endif

/* The "set" a request belongs to. */
enum rpmh_set {
	RPMH_SET_ACTIVE = 0,
	RPMH_SET_SLEEP,
	RPMH_SET_WAKE,
	RPMH_NUM_SETS,
};

/*
 * RSC SW DRV mapping. This is the SW numbering scheme shared between the RPMh
 * driver and the RSC HW. Only the TZ DRV is driven from TF-A.
 */
#define RSC_DRV_TZ	0U

/* A single RPMh command. */
struct rpmh_command {
	uint32_t	address;	/* {slaveID, offset} */
	uint32_t	data;
	bool		completion;	/* true: completion, false: fire & forget */
};

/* A set of commands issued to RPMh in a single TCS. */
struct rpmh_command_set {
	enum rpmh_set	set;
	uint32_t	num_commands;
	uint32_t	reserved;
	struct rpmh_command	commands[IMAGE_TCS_SIZE];
};

/* Opaque per-client handle. */
struct rpmh_client;

/*
 * rpmh_client_init - one time initialisation of the RPMh driver.
 *
 * Must be called once before any other RPMh API is used.
 */
void rpmh_client_init(void);

/*
 * rpmh_create_handle - create a client handle for the given RSC+DRV mapping.
 *
 * @drv_id:      the RSC+DRV mapping for the client
 * @client_name: a unique string identifying the client
 *
 * Returns an opaque handle to be used with the other RPMh APIs.
 */
struct rpmh_client *rpmh_create_handle(uint32_t drv_id,
				      const char *client_name);

/*
 * rpmh_issue_command - issue a single command.
 *
 * @handle:     client handle
 * @set:        set associated with the command
 * @completion: true for a completion request, false for fire and forget
 * @address:    {slaveID, offset}
 * @data:       data to send
 *
 * Returns a request id usable with the barrier APIs.
 */
uint32_t rpmh_issue_command(struct rpmh_client *handle, enum rpmh_set set,
			    bool completion, uint32_t address, uint32_t data);

/*
 * rpmh_issue_command_set - issue multiple commands in parallel (one TCS).
 *
 * @handle:      client handle
 * @command_set: the command set to issue
 *
 * Returns a request id usable with the barrier APIs.
 */
uint32_t rpmh_issue_command_set(struct rpmh_client *handle,
				struct rpmh_command_set *command_set);

/*
 * rpmh_barrier_single - block until the given request id has finished.
 *
 * @handle: client handle associated with the request id
 * @req_id: request id to wait for
 */
void rpmh_barrier_single(struct rpmh_client *handle, uint32_t req_id);

/*
 * rpmh_barrier_all - block until the given request id and every prior request
 * on the handle have finished.
 *
 * @handle: client handle associated with the request id
 * @req_id: request id to wait for
 */
void rpmh_barrier_all(struct rpmh_client *handle, uint32_t req_id);

#endif /* QTI_RPMH_CLIENT_H */
