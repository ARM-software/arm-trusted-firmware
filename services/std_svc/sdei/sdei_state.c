/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <lib/cassert.h>

#include "sdei_private.h"

/* Aliases for SDEI handler states: 'R'unning, 'E'nabled, and re'G'istered */
#define r_		0U
#define R_		(1u << SDEI_STATF_RUNNING)

#define e_		0U
#define E_		(1u << SDEI_STATF_ENABLED)

#define g_		0U
#define G_		(1u << SDEI_STATF_REGISTERED)

/* All possible composite handler states */
#define reg_		(r_ | e_ | g_)
#define reG_		(r_ | e_ | G_)
#define rEg_		(r_ | E_ | g_)
#define rEG_		(r_ | E_ | G_)
#define Reg_		(R_ | e_ | g_)
#define ReG_		(R_ | e_ | G_)
#define REg_		(R_ | E_ | g_)
#define REG_		(R_ | E_ | G_)

#define MAX_STATES	(REG_ + 1u)

/* Invalid state */
#define	SDEI_STATE_INVALID	((sdei_state_t) (-1))

/* No change in state */
#define	SDEI_STATE_NOP		((sdei_state_t) (-2))

#define X___		SDEI_STATE_INVALID
#define NOP_		SDEI_STATE_NOP

/* Ensure special states don't overlap with valid ones */
CASSERT(X___ > REG_, sdei_state_overlap_invalid);
CASSERT(NOP_ > REG_, sdei_state_overlap_nop);

/*
 * SDEI handler state machine: refer to sections 6.1 and 6.1.2 of the SDEI v1.0
 * specification (ARM DEN0054A).
 *
 * Not all calls contribute to handler state transition. This table is also used
 * to validate whether a call is permissible at a given handler state:
 *
 *  - X___ denotes a forbidden transition;
 *  - NOP_ denotes a permitted transition, but there's no change in state;
 *  - Otherwise, XXX_ gives the new state.
 *
 * DISP[atch] is a transition added for the implementation, but is not mentioned
 * in the spec.
 *
 * Those calls that the spec mentions as can be made any time don't picture in
 * this table.
 */

static const sdei_state_t sdei_state_table[MAX_STATES][DO_MAX] = {
/*
 *	Action:		REG     REL	ENA	DISA	UREG	ROUT	CTX	COMP	COMPR	DISP
 *	Notes:			[3]			[1]	[3]	[3][4]			[2]
 */
	/* Handler unregistered, disabled, and not running. This is the default state. */
/* 0 */	[reg_] = {	reG_,	NOP_,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	},

	/* Handler unregistered and running */
/* 4 */	[Reg_] = {	X___,	X___,	X___,	X___,	X___,	X___,	NOP_,	reg_,	reg_,	X___,	},

	/* Handler registered */
/* 1 */	[reG_] = {	X___,	X___,	rEG_,	NOP_,	reg_,	NOP_,	X___,	X___,	X___,	X___,	},

	/* Handler registered and running */
/* 5 */	[ReG_] = {	X___,	X___,	REG_,	NOP_,	Reg_,	X___,	NOP_,	reG_,	reG_,	X___,	},

	/* Handler registered and enabled */
/* 3 */	[rEG_] = {	X___,	X___,	NOP_,	reG_,	reg_,	X___,	X___,	X___,	X___,	REG_,	},

	/* Handler registered, enabled, and running */
/* 7 */	[REG_] = {	X___,	X___,	NOP_,	ReG_,	Reg_,	X___,	NOP_,	rEG_,	rEG_,	X___,	},

	/*
	 * Invalid states: no valid transition would leave the handler in these
	 * states; and no transition from these states is possible either.
	 */

	/*
	 * Handler can't be enabled without being registered. I.e., XEg is
	 * impossible.
	 */
/* 2 */	[rEg_] = {	X___,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	},
/* 6 */	[REg_] = {	X___,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	X___,	},
};

/*
 * [1] Unregister will always also disable the event, so the new state will have
 *     Xeg.
 * [2] Event is considered for dispatch only when it's both registered and
 *     enabled.
 * [3] Never causes change in state.
 * [4] Only allowed when running.
 */

/*
 * Given an action, transition the state of an event by looking up the state
 * table above:
 *
 *  - Return false for invalid transition;
 *  - Return true for valid transition that causes no change in state;
 *  - Otherwise, update state and return true.
 *
 * This function assumes that the caller holds necessary locks. If the
 * transition has constrains other than the state table describes, the caller is
 * expected to restore the previous state. See sdei_event_register() for
 * example.
 */
bool can_sdei_state_trans(sdei_entry_t *se, sdei_action_t act)
{
	sdei_state_t next;

	assert(act < DO_MAX);
	if (se->state >= MAX_STATES) {
		WARN(" event state invalid: %x\n", se->state);
		return false;
	}

	next = sdei_state_table[se->state][act];
	switch (next) {
	case SDEI_STATE_INVALID:
		return false;

	case SDEI_STATE_NOP:
		return true;

	default:
		/* Valid transition. Update state. */
		SDEI_LOG(" event state 0x%x => 0x%x\n", se->state, next);
		se->state = next;

		return true;
	}
}
