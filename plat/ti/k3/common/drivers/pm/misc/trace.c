/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/trace.h>

void pm_trace_debug(uint32_t action, uint32_t val)
{
	if (action & TRACE_PM_ACTION_FAIL) {
		action &= ~TRACE_PM_ACTION_FAIL;
		/* There are few traces with 0x7x values but are
		 * not ACTION FAIL scenarios
		 */
		if (action < 0x31)
			VERBOSE("ACTION FAIL\n");
	}

	switch (action) {
	case TRACE_PM_ACTION_DEVICE_ON:
		VERBOSE("DEVICE_ON\n");
		VERBOSE("Device ID = %d\n", val);
		break;
	case TRACE_PM_ACTION_DEVICE_OFF:
		VERBOSE("DEVICE_OFF\n");
		VERBOSE("Device ID = %d\n", val);
		break;
	case TRACE_PM_ACTION_CLOCK_ENABLE:
		VERBOSE("CLOCK_ENABLE\n");
		VERBOSE("Clock ID = %d\n", val);
		break;
	case TRACE_PM_ACTION_CLOCK_DISABLE:
		VERBOSE("CLOCK_DISABLE\n");
		VERBOSE("Clock ID = %d\n", val);
		break;
	case TRACE_PM_ACTION_CLOCK_SET_RATE:
		VERBOSE("CLOCK_SET_RATE\n");
		VERBOSE("Clock ID = %d\n",
			((val & TRACE_PM_VAL_CLOCK_ID_MASK) >> TRACE_PM_VAL_CLOCK_ID_SHIFT));
		VERBOSE("Clock Frequency{significand} = %d\n",
			((val & TRACE_PM_VAL_CLOCK_VAL_MASK)  >> TRACE_PM_VAL_CLOCK_VAL_SHIFT));
		VERBOSE("Clock Frequency{Exponent} = %d\n",
			((val & TRACE_PM_VAL_CLOCK_EXP_SHIFT)  >> TRACE_PM_VAL_CLOCK_EXP_SHIFT));
		break;
	case TRACE_PM_ACTION_CLOCK_SET_PARENT:
		VERBOSE("CLOCK_SET_PARENT\n");
		VERBOSE("Clock ID = %d\n",
			((val & TRACE_PM_VAL_CLOCK_ID_MASK) >> TRACE_PM_VAL_CLOCK_ID_SHIFT));
		VERBOSE("New Parent ID = %d\n",
			((val & TRACE_PM_VAL_CLOCK_VAL_MASK)  >> TRACE_PM_VAL_CLOCK_VAL_SHIFT));
		break;
	case TRACE_PM_ACTION_MSG_RECEIVED:
		VERBOSE("MSG_RECEIVED\n");
		switch (val) {
		case TISCI_MSG_SET_CLOCK:
			VERBOSE("TISCI_MSG_SET_CLOCK\n");
			break;
		case TISCI_MSG_GET_CLOCK:
			VERBOSE("TISCI_MSG_GET_CLOCK\n");
			break;
		case TISCI_MSG_SET_CLOCK_PARENT:
			VERBOSE("TISCI_MSG_SET_CLOCK_PARENT\n");
			break;
		case TISCI_MSG_GET_CLOCK_PARENT:
			VERBOSE("TISCI_MSG_GET_CLOCK_PARENT\n");
			break;
		case TISCI_MSG_GET_NUM_CLOCK_PARENTS:
			VERBOSE("TISCI_MSG_GET_NUM_CLOCK_PARENTS\n");
			break;
		case TISCI_MSG_SET_FREQ:
			VERBOSE("TISCI_MSG_SET_FREQ\n");
			break;
		case TISCI_MSG_QUERY_FREQ:
			VERBOSE("TISCI_MSG_QUERY_FREQ\n");
			break;
		case TISCI_MSG_GET_FREQ:
			VERBOSE("TISCI_MSG_GET_FREQ\n");
			break;
		case TISCI_MSG_SET_DEVICE:
			VERBOSE("TISCI_MSG_SET_DEVICE\n");
			break;
		case TISCI_MSG_GET_DEVICE:
			VERBOSE("TISCI_MSG_GET_DEVICE\n");
			break;
		case TISCI_MSG_SET_DEVICE_RESETS:
			VERBOSE("TISCI_MSG_SET_DEVICE_RESETS\n");
			break;
		case TISCI_MSG_DEVICE_DROP_POWERUP_REF:
			VERBOSE("TISCI_MSG_DEVICE_DROP_POWERUP_REF\n");
			break;
		default:
			VERBOSE("Message not handled\n");
		}
		break;
	case TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID:
		VERBOSE("MSG_PARAM_DEV_CLK_ID\n");
		VERBOSE("Clock ID = %d\n",
			val >> TRACE_PM_MSG_CLK_ID_SHIFT);
		VERBOSE("Device ID = %d\n",
			val & TRACE_PM_VAL_DEVICE_ID_MASK);
		break;
	case TRACE_PM_ACTION_MSG_PARAM_VAL:
		VERBOSE("MSG_PARAM_VAL\n");
		VERBOSE("value = %d\n", val);
		break;
	case TRACE_PM_ACTION_WAKE_ARM:
		VERBOSE("WAKE_ARM\n");
		break;
	case TRACE_PM_ACTION_WAKE_HANDLER:
		VERBOSE("WAKE_HANDLER\n");
		break;
	case TRACE_PM_ACTION_PD_GET:
		VERBOSE("PD_GET\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("Power domain ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("PD Usage Count = %d\n",
			(val & TRACE_PM_VAL_MAX_PSC_DATA));
		break;
	case TRACE_PM_ACTION_PD_PUT:
		VERBOSE("PD_PUT\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("Power domain ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("PD Usage Count = %d\n",
			(val & TRACE_PM_VAL_MAX_PSC_DATA));
		break;
	case TRACE_PM_ACTION_SET_LOCAL_RESET:
		VERBOSE("SET_LOCAL_RESET\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("LPSC ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Enable(1)/Disable(0) = %d\n",
			(val & 0x1U));
		break;
	case TRACE_PM_ACTION_MODULE_GET:
		VERBOSE("MODULE_GET\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("LPSC ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Module Use Count = %d\n",
			(val & TRACE_PM_VAL_MAX_PSC_DATA));
		break;
	case TRACE_PM_ACTION_MODULE_PUT:
		VERBOSE("MODULE_PUT\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("LPSC ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Module Use Count = %d\n",
			(val & TRACE_PM_VAL_MAX_PSC_DATA));
		break;
	case TRACE_PM_ACTION_RETENTION_GET:
		VERBOSE("RETENTION_GET\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("LPSC ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Module Retention Count = %d\n",
			(val & TRACE_PM_VAL_MAX_PSC_DATA));
		break;
	case TRACE_PM_ACTION_RETENTION_PUT:
		VERBOSE("RETENTION_PUT\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("LPSC ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Module Retention Count = %d\n",
			(val & TRACE_PM_VAL_MAX_PSC_DATA));
		break;
	case TRACE_PM_ACTION_PD_INIT:
		VERBOSE("PD_INIT\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("LPSC ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Module Retention Count = %d\n",
			(val & TRACE_PM_VAL_MAX_PSC_DATA));
		break;
	case TRACE_PM_ACTION_INVALID_PSC_DATA:
		VERBOSE("INVALID_PSC_DATA\n");
		break;
	case TRACE_PM_ACTION_PSC_TRANSITION_TIMEOUT:
		VERBOSE("PSC_TRANSITION_TIMEOUT\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("Power domain ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Internal position = %d\n",
			(val & 0x7U));
		break;
	case TRACE_PM_ACTION_PSC_INVALID_DEP_DATA:
		VERBOSE("PSC_INVALID_DEP_DATA\n");
		VERBOSE("Dependent Power domain ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("Power domain ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Internal position = %d\n",
			(val & 0x7U));
		break;
	case TRACE_PM_ACTION_PSC_RST_DONE_TIMEOUT:
		VERBOSE("PSC_RST_DONE_TIMEOUT\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("Power domain ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Internal position = %d\n",
			(val & 0x7U));
		break;
	case TRACE_PM_ACTION_SET_MODULE_RESET:
		VERBOSE("SET_MODULE_RESET\n");
		VERBOSE("SET_LOCAL_RESET\n");
		VERBOSE("PSC ID = %d\n",
			(val & TRACE_PM_VAL_PSC_MASK) >> TRACE_PM_VAL_PSC_SHIFT);
		VERBOSE("LPSC ID = %d\n",
			(val & TRACE_PM_VAL_PD_MASK) >> TRACE_PM_VAL_PD_SHIFT);
		VERBOSE("Enable(1)/Disable(0) = %d\n",
			(val & 0x1U));
		break;
	case TRACE_PM_ACTION_INIT:
		VERBOSE("INIT\n");
		break;
	case TRACE_PM_ACTION_DEV_INIT:
		VERBOSE("DEV_INIT\n");
		break;
	case TRACE_PM_ACTION_SYSRESET:
		VERBOSE("SYSRESET\n");
		break;
	case TRACE_PM_ACTION_LPM_SEQUENCE:
		VERBOSE("LPM_SEQUENCE\n");
		break;
	case TRACE_PM_ACTION_EXCLUSIVE_BUSY & ~TRACE_PM_ACTION_FAIL:
		VERBOSE("EXCLUSIVE_BUSY\n");
		VERBOSE("Device ID = %d",
			(val & TRACE_PM_VAL_EXCLUSIVE_BUSY_DEVICE_ID_MASK)
			>> TRACE_PM_VAL_EXCLUSIVE_BUSY_DEVICE_ID_SHIFT);
		VERBOSE("Requester Host ID = %d",
			(val & TRACE_PM_VAL_EXCLUSIVE_BUSY_RHOST_ID_MASK)
			>> TRACE_PM_VAL_EXCLUSIVE_BUSY_RHOST_ID_SHIFT);
		VERBOSE("Holder Host ID = %d",
			(val & TRACE_PM_VAL_EXCLUSIVE_BUSY_EHOST_ID_MASK)
			>> TRACE_PM_VAL_EXCLUSIVE_BUSY_EHOST_ID_SHIFT);
		break;
	case TRACE_PM_ACTION_EXCLUSIVE_DEVICE & ~TRACE_PM_ACTION_FAIL:
		VERBOSE("EXCLUSIVE_DEVICE\n");
		VERBOSE("Device ID = %d",
			(val & TRACE_PM_VAL_EXCLUSIVE_DEVICE_ID_MASK)
			>> TRACE_PM_VAL_EXCLUSIVE_DEVICE_ID_SHIFT);
		VERBOSE("Holder Host ID = %d",
			(val & TRACE_PM_VAL_EXCLUSIVE_HOST_ID_MASK)
			>> TRACE_PM_VAL_EXCLUSIVE_HOST_ID_SHIFT);
		break;
	case TRACE_PM_ACTION_INVALID_STATE & ~TRACE_PM_ACTION_FAIL:
		VERBOSE("INVALID_STATE\n");
		VERBOSE("state = %d\n", val);
		break;
	case TRACE_PM_ACTION_BAD_DEVICE & ~TRACE_PM_ACTION_FAIL:
		VERBOSE("BAD_DEVICE\n");
		VERBOSE("Device ID = %d\n", val);
		break;
	case TRACE_PM_ACTION_MSG_PARAM_LATENCY:
		VERBOSE("MSG_PARAM_LATENCY\n");
		break;
	default:
		VERBOSE("Trace not handled\n");
	}
}
