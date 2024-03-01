/* SPDX-License-Identifier: BSD-3-Clause */
/**
 * Copyright 2019-2024 NXP
 *
 * KEYWORDS: micro-power uPower driver API
 * -----------------------------------------------------------------------------
 * PURPOSE: uPower driver API
 * -----------------------------------------------------------------------------
 * PARAMETERS:
 * PARAM NAME RANGE:DESCRIPTION:       DEFAULTS:                           UNITS
 * -----------------------------------------------------------------------------
 * REUSE ISSUES: no reuse issues
 */
#ifndef UPWR_API_H
#define UPWR_API_H

#include "upmu.h"
#include "upower_soc_defs.h"
/******************************************************************************
 * uPower API Overview and Concepts
 *
 * This API is intended to be used by the OS drivers (Linux, FreeRTOS etc)
 * as well as bare metal drivers to command and use services from the uPower.
 * It aims to be OS-independent.
 *
 * The API functions fall in 3 categories:
 *  - initialization/start-up
 *  - service requests
 *  - auxiliary
 *
 * The communication with the uPower is mostly made through the Message Unit
 * (MU) IP. uPower provides one MU for each CPU cluster in a different
 * power domain. An API instance runs on each CPU cluster.
 *
 * The API assumes each SoC power domain/CPU cluster receives 2 interrupts
 * from the uPower MU:
 *  1. Tx/Rx, which is issued on both transmission and reception
 *  2. Exception interrupt, to handle critical alams, catastrophic errors, etc.
 *     This interrupt should have a high priority, preferably an NMI.
 *
 * The normal uPower operation is done by service requests. There is an API
 * function for each service request, and all service requests send back a
 * response, at least to indicate success/failure.
 * The service request functions are non-blocking, and their completion can be
 * tracked in two ways:
 *  1. by a callback, registered when the service request call is made by
 *     passing the callback function pointer; a NULL pointer may be passed,
 *     in which case no callback is made.
 *  2. by polling, using the auxiliary functions upwr_req_status or
 *     upwr_poll_req_status;
 *     polling must be used if no callback is registered, but callbacks and
 *     polling are completely independent.
 *
 * Note: a service request must not be started from a callback.
 *
 * uPower service requests are classified in Service Groups.
 * Each Service Group has a set of related functions, named upwr_XXX_,
 * where XXX is a 3-letter service group mnemonic. The service groups are:
 *  - Exception Service Group - upwr_xcp_*
 *     ~ gathers functions that deal with errors and other processes outside
 *       the functional scope.
 *  - Power Management Service Group - upwr_pwm_*
 *     ~ functions to control switches, configure power modes, set internal voltage etc
 *  - Delay Measurement Service Group - upwr_dlm_*
 *     ~ delay measurements function using the process monitor and delay meter
 *  - Voltage Measurement Service Group - upwr_vtm_*
 *     ~ functions for voltage measurements, comparisons, alarms, power meter, set PMIC rail voltage
 *  - Temperature Measurement Service Group - upwr_tpm_*
 *     ~ functions for temperature measurements, comparisons, alarms
 *  - Current Measurement Service Group  - upwr_crm_*
 *     ~ functions for current and charge measurement
 *  - Diagnostic Service Group - upwr_dgn_*
 *     ~ functions for log configuration and statistics collecting
 *
 * Service requests follow this "golden rule":
 * *** No two requests run simultaneously for the same service group,
 *     on the same domain ***
 * They can run simultaneously on different domains (RTD/APD), and can also run
 * simultaneously if belong to different service groups (even on same domain).
 * Therefore, requests to the same service group on the same domain must be
 * serialized. A service request call returns error if there is another request
 * on the same service group pending, waiting a response (on the same domain).
 *
 * A request for continuous service does not block the service group.
 * For instance, a request to "measure the temperature each 10 miliseconds"
 * responds quickly, unlocks the service group, and the temperature
 * continues to be measured as requested, every 10 miliseconds from then on.
 *
 * Service Groups have a fixed priority in the API, from higher to lower:
 *  1. Exception
 *  2. Power Management
 *  3. Delay Measurement
 *  4. Voltage Measurement
 *  5. Current Measurement
 *  6. Temperature Measurement
 *  7. Diagnostics
 *
 * The priority above only affects the order in which requests are sent to the
 * uPower firmware: request to the higher priority Service Group is sent first,
 * even if the call was made later, if there is an MU transmission pending,
 * blocking it. The service priorities in the firmware depend on other factors.
 *
 * Services are requested using API functions. A service function returns with
 * no error if a request was successfully made, but it doesn't mean the service
 * was completed. The service is executed asynchronously, and returns a result
 * (at least success/fail) via a callback or polling for service status.
 * The possible service response codes are:
 * - UPWR_RESP_OK = 0,     : no error
 * - UPWR_RESP_SG_BUSY     : service group is busy
 * - UPWR_RESP_SHUTDOWN    : services not up or shutting down
 * - UPWR_RESP_BAD_REQ     : invalid request (usually invalid argumnents)
 * - UPWR_RESP_BAD_STATE   : system state doesn't allow perform the request
 * - UPWR_RESP_UNINSTALLD  : service or function not installed
 * - UPWR_RESP_UNINSTALLED : service or function not installed (alias)
 * - UPWR_RESP_RESOURCE    : resource not available
 * - UPWR_RESP_TIMEOUT     : service timeout
 */

/**
 * upwr_callb()-generic function pointer for a request return callback;
 * @sg: request service group
 * @func: service request function id.
 * @errcode: error code.
 * @ret: return value, if any. Note that a request may return a value even if
 * service error is returned (errcode != UPWR_RESP_OK); that is dependent on
 * the specific service.
 *
 * Context: no sleep, no locks taken/released.
 * Return: none (void)
 */
typedef void (*upwr_callb)(upwr_sg_t sg, uint32_t func,
			   upwr_resp_t errcode, ...);

/**---------------------------------------------------------------
 * INITIALIZATION, CONFIGURATION
 *
 * A reference uPower initialization sequence goes as follows:
 *
 * 1. host CPU calls upwr_init.
 * 2. (optional) host checks the ROM version and SoC code calling upwr_vers(...)
 *    and optionally performs any configuration or workaround accordingly.
 * 3. host CPU calls upwr_start to start the uPower services, passing a
 *    service option number.
 *    If no RAM code is loaded or it has no service options, the launch option
 *    number passed must be 0, which will start the services available in ROM.
 *    upwr_start also receives a pointer to a callback called by the API
 *    when the firmware is ready to receive service requests.
 *    The callback may be replaced by polling, calling upwr_req_status in a loop
 *    or upwr_poll_req_status; in this case the callback pointer may be NULL.
 *    A host may call upwr_start even if the services were already started by
 *    any host: if the launch option is the same, the response will be ok,
 *    but will indicate error if the services were already started with a
 *    different launch option.
 * 4. host waits for the callback calling, or polling finishing;
 *    if no error is returned, it can start making service calls using the API.
 *
 * Variations on that reference sequence are possible:
 *  - the uPower services can be started using the ROM code only, which includes
 *    the basic Power Management services, among others, with launch option
 *    number = 0.
 *    The code RAM can be loaded while these services are running and,
 *    when the loading is done, the services can be re-started with these 2
 *    requests executed in order: upwr_xcp_shutdown and upwr_start,
 *    using the newly loaded RAM code (launch option > 0).
 *
 * NOTE: the initialization call upwr_init is not effective and
 *       returns error when called after the uPower services are started.
 */

/**
 * upwr_init() - API initialization; must be the first API call after reset.
 * @domain: SoC-dependent CPU domain id; identifier used by the firmware in
 * many services. Defined by SoC-dependent type soc_domain_t found in
 * upower_soc_defs.h.
 * @muptr: pointer to the MU instance.
 * @mallocptr: pointer to the memory allocation function
 * @physaddrptr: pointer to the function to convert pointers to
 * physical addresses. If NULL, no conversion is made (pointer=physical address)
 * @isrinstptr: pointer to the function to install the uPower ISR callbacks;
 * the function receives the pointers to the MU tx/rx and Exception ISRs
 * callbacks, which must be called from the actual system ISRs.
 * The function pointed by isrinstptr must also enable the interrupt at the
 * core/interrupt controller, but must not enable the interrupt at the MU IP.
 * The system ISRs are responsible for dealing with the interrupt controller,
 * performing any other context save/restore, and any other housekeeping.
 * @lockptr: pointer to a function that prevents MU interrupts (if argrument=1)
 * or allows it (if argument=0). The API calls this function to make small
 * specific code portions thread safe. Only MU interrupts must be avoided,
 * the code may be suspended for other reasons.
 * If no MU interrupts can happen during the execution of an API call or
 * callback, even if enabled, for some other reason (e.g. interrupt priority),
 * then this argument may be NULL.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if failed to allocate memory, or use some other resource.
 *        -2 if any argument is invalid.
 *        -3 if failed to send the ping message.
 *        -4 if failed to receive the initialization message, or was invalid
 */

/* malloc function ptr */
typedef void* (*upwr_malloc_ptr_t)(unsigned int size);

/* pointer->physical address conversion function ptr */
typedef void* (*upwr_phyadr_ptr_t)(const void *addr);

typedef uint32_t upwr_api_state_t;

extern volatile upwr_api_state_t api_state;

/*
 * upwr_lock_ptr_t: pointer to a function that prevents MU interrupts
 * (if argrument lock=1) or allows it (if argument lock=0).
 * The API calls this function to make small specific code portions thread safe.
 * Only MU interrupts must be avoided, the code may be suspended for other
 * reasons.
 */
typedef void  (*upwr_lock_ptr_t)(int lock);

typedef void (*upwr_isr_callb)(void);

typedef void (*upwr_inst_isr_ptr_t)(upwr_isr_callb txrx_isr,
				    upwr_isr_callb excp_isr);
void upwr_start_callb(void);

int upwr_init(soc_domain_t domain, struct MU_t *muptr,
	      const upwr_malloc_ptr_t mallocptr,
	      const upwr_phyadr_ptr_t phyadrptr,
	      const upwr_inst_isr_ptr_t isrinstptr,
	      const upwr_lock_ptr_t lockptr);

/**
 * upwr_start() - Starts the uPower services.
 * @launchopt: a number to select between multiple launch options,
 * that may define, among other things, which services will be started,
 * or which services implementations, features etc.
 * launchopt = 0 selects a subset of services implemented in ROM;
 * any other number selects service sets implemented in RAM, launched
 * by the firmware function ram_launch; if an invalid launchopt value is passed,
 * no services are started, and the callback returns error (see below).
 * @rdycallb: pointer to the callback to be called when the uPower is ready
 * to receive service requests. NULL if no callback needed.
 * The callback receives as arguments the RAM firmware version numbers.
 * If all 3 numbers (vmajor, vminor, vfixes) are 0, that means the
 * service launching failed.
 * Firmware version numbers will be the same as ROM if launchopt = 0,
 * selecting the ROM services.
 *
 * upwr_start can be called by any domain even if the services are already
 * started: it has no effect, returning success, if the launch option is the
 * same as the one that actually started the service, and returns error if
 * called with a different option.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if a resource failed,
 *        -2 if the domain passed is the same as the caller,
 *        -3 if called in an invalid API state
 */

extern void upwr_txrx_isr(void);

typedef void (*upwr_rdy_callb)(uint32_t vmajor, uint32_t vminor, uint32_t vfixes);

int upwr_start(uint32_t launchopt, const upwr_rdy_callb rdycallb);


/**---------------------------------------------------------------
 * EXCEPTION SERVICE GROUP
 */

/**
 * upwr_xcp_config() - Applies general uPower configurations.
 * @config: pointer to the uPower SoC-dependent configuration struct
 * upwr_xcp_config_t defined in upower_soc_defs.h. NULL may be passed, meaning
 * a request to read the configuration, in which case it appears in the callback
 * argument ret, or can be pointed by argument retptr in the upwr_req_status and
 * upwr_poll_req_status calls, casted to upwr_xcp_config_t.
 * @callb: pointer to the callback to be called when the uPower has finished
 * the configuration, or NULL if no callback needed (polling used instead).
 *
 * Some configurations are targeted for a specific domain (see the struct
 * upwr_xcp_config_t definition in upower_soc_defs.h); this call has implicit
 * domain target (the same domain from which is called).
 *
 * The return value is always the current configuration value, either in a
 * read-only request (config = NULL) or after setting a new configuration
 * (non-NULL config).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */

int upwr_xcp_config(const upwr_xcp_config_t *config, const upwr_callb callb);

/**
 * upwr_xcp_sw_alarm() - Makes uPower issue an alarm interrupt to given domain.
 * @domain: identifier of the domain to alarm. Defined by SoC-dependent type
 * soc_domain_t found in upower_soc_defs.h.
 * @code: alarm code. Defined by SoC-dependent type upwr_alarm_t found in
 * upower_soc_defs.h.
 * @callb: pointer to the callback to be called when the uPower has finished
 * the alarm, or NULL if no callback needed (polling used instead).
 *
 * The function requests the uPower to issue an alarm of the given code as if
 * it had originated internally. This service is useful mainly to test the
 * system response to such alarms, or to make the system handle a similar alarm
 * situation detected externally to uPower.
 *
 * The system ISR/code handling the alarm may retrieve the alarm code by calling
 * the auxiliary function upwr_alarm_code.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */

int upwr_xcp_sw_alarm(soc_domain_t domain, upwr_alarm_t code,
		      const upwr_callb callb);

/**
 * upwr_xcp_set_ddr_retention() - M33/A35 can use this API to set/clear ddr retention
 * @domain: identifier of the caller domain.
 * soc_domain_t found in upower_soc_defs.h.
 * @enable: true, means that set ddr retention, false clear ddr retention.
 * @callb: NULL
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */

int upwr_xcp_set_ddr_retention(soc_domain_t domain, uint32_t enable,
			       const upwr_callb callb);

/**
 * upwr_xcp_set_mipi_dsi_ena() - M33/A35 can use this API to set/clear mipi dsi ena
 * @domain: identifier of the caller domain.
 * soc_domain_t found in upower_soc_defs.h.
 * @enable: true, means that set ddr retention, false clear ddr retention.
 * @callb: NULL
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */

int upwr_xcp_set_mipi_dsi_ena(soc_domain_t domain, uint32_t enable,
			      const upwr_callb callb);

/**
 * upwr_xcp_get_mipi_dsi_ena() - M33/A35 can use this API to get mipi dsi ena status
 * @domain: identifier of the caller domain.
 * soc_domain_t found in upower_soc_defs.h.
 * @callb: NULL
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */

int upwr_xcp_get_mipi_dsi_ena(soc_domain_t domain, const upwr_callb callb);

/**
 * upwr_xcp_set_osc_mode() - M33/A35 can use this API to set uPower OSC mode
 * @domain: identifier of the caller domain.
 * soc_domain_t found in upower_soc_defs.h.
 * @osc_mode, 0 means low frequency, not 0 means high frequency.
 * @callb: NULL
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */
int upwr_xcp_set_osc_mode(soc_domain_t domain, uint32_t osc_mode,
			  const upwr_callb callb);

/**
 * upwr_xcp_set_rtd_use_ddr() - M33 call this API to inform uPower, M33 is using ddr
 * @domain: identifier of the caller domain.
 * soc_domain_t found in upower_soc_defs.h.
 * @is_use_ddr: not 0, true, means that RTD is using ddr. 0, false, means that, RTD
 * is not using ddr.
 * @callb: NULL
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */
int upwr_xcp_set_rtd_use_ddr(soc_domain_t domain, uint32_t is_use_ddr,
			     const upwr_callb callb);

/**
 * upwr_xcp_set_rtd_apd_llwu() - M33/A35 can use this API to set/clear rtd_llwu apd_llwu
 * @domain: set which domain (RTD_DOMAIN, APD_DOMAIN) LLWU.
 * soc_domain_t found in upower_soc_defs.h.
 * @enable: true, means that set rtd_llwu or apd_llwu, false clear rtd_llwu or apd_llwu.
 * @callb: NULL
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */
int upwr_xcp_set_rtd_apd_llwu(soc_domain_t domain, uint32_t enable,
			      const upwr_callb callb);
/**
 * upwr_xcp_shutdown() - Shuts down all uPower services and power mode tasks.
 * @callb: pointer to the callback to be called when the uPower has finished
 * the shutdown, or NULL if no callback needed
 * (polling used instead).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * At the callback the uPower/API is back to initialization/start-up phase,
 * so service request calls return error.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */
int upwr_xcp_shutdown(const upwr_callb callb);

/**
 * upwr_xcp_i2c_access() - Performs an access through the uPower I2C interface.
 * @addr: I2C slave address, up to 10 bits.
 * @data_size: determines the access direction and data size in bytes, up to 4;
 * negetive data_size determines a read  access with size -data_size;
 * positive data_size determines a write access with size  data_size;
 * data_size=0 is invalid, making the service return error UPWR_RESP_BAD_REQ.
 * @subaddr_size: size of the sub-address in bytes, up to 4; if subaddr_size=0,
 * no subaddress is used.
 * @subaddr: sub-address, only used if subaddr_size > 0.
 * @wdata: write data, up to 4 bytes; ignored if data_size < 0 (read)
 * @callb: pointer to the callback to be called when the uPower has finished
 * the access, or NULL if no callback needed
 * (polling used instead).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_EXCEPT as the service group argument.
 *
 * The service performs a read (data_size < 0) or a write (data_size > 0) of
 * up to 4 bytes on the uPower I2C interface. The data read from I2C comes via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 *
 * Sub-addressing is supported, with sub-address size determined by the argument
 * subaddr_size, up to 4 bytes. Sub-addressing is not used if subaddr_size=0.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */
int upwr_xcp_i2c_access(uint16_t addr, int8_t data_size, uint8_t subaddr_size,
			uint32_t subaddr, uint32_t wdata,
			const upwr_callb callb);


/**---------------------------------------------------------------
 * POWER MANAGEMENT SERVICE GROUP
 */

/**
 * upwr_pwm_dom_power_on() - Commands uPower to power on the platform of other
 * domain (not necessarily its core(s)); does not release the core reset.
 * @domain: identifier of the domain to power on. Defined by SoC-dependent type
 * soc_domain_t found in upower_soc_defs.h.
 * @boot_start: must be 1 to start the domain core(s) boot(s), releasing
 * its (their) resets, or 0 otherwise.
 * @pwroncallb: pointer to the callback to be called when the uPower has
 * finished the power on procedure, or NULL if no callback needed
 * (polling used instead).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -2 if the domain passed is the same as the caller,
 *        -3 if called in an invalid API state
 */
int upwr_pwm_dom_power_on(soc_domain_t domain, int boot_start,
			  const upwr_callb pwroncallb);

/**
 * upwr_pwm_boot_start() - Commands uPower to release the reset of other CPU(s),
 * starting their boots.
 * @domain: identifier of the domain to release the reset. Defined by
 * SoC-dependent type soc_domain_t found in upower_soc_defs.h.
 * @bootcallb: pointer to the callback to be called when the uPower has finished
 * the boot start procedure, or NULL if no callback needed
 * (polling used instead).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * The callback calling doesn't mean the CPUs boots have finished:
 * it only indicates that uPower released the CPUs resets, and can receive
 * other power management service group requests.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -2 if the domain passed is the same as the caller,
 *        -3 if called in an invalid API state
 */
int upwr_pwm_boot_start(soc_domain_t domain, const upwr_callb  bootcallb);

/**
 * upwr_pwm_param() - Changes Power Management parameters.
 * @param: pointer to a parameter structure upwr_pwm_param_t, SoC-dependent,
 * defined in upwr_soc_defines.h. NULL may be passed, meaning
 * a request to read the parameter set, in which case it appears in the callback
 * argument ret, or can be pointed by argument retptr in the upwr_req_status and
 * upwr_poll_req_status calls, casted to upwr_pwm_param_t.
 * @callb: response callback pointer; NULL if no callback needed.
 *
 * The return value is always the current parameter set value, either in a
 * read-only request (param = NULL) or after setting a new parameter
 * (non-NULL param).
 *
 * Some parameters may be targeted for a specific domain (see the struct
 * upwr_pwm_param_t definition in upower_soc_defs.h); this call has implicit
 * domain target (the same domain from which is called).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded or
 * not.
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */
int upwr_pwm_param(upwr_pwm_param_t *param, const upwr_callb callb);

/**
 * upwr_pwm_chng_reg_voltage() - Changes the voltage at a given regulator.
 * @reg: regulator id.
 * @volt: voltage value; value unit is SoC-dependent, converted from mV by the
 * macro UPWR_VOLT_MILIV, or from micro-Volts by the macro UPWR_VOLT_MICROV,
 * both macros in upower_soc_defs.h
 * @callb: response callback pointer; NULL if no callback needed.
 *
 * The function requests uPower to change the voltage of the given regulator.
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_chng_reg_voltage(uint32_t reg, uint32_t volt, upwr_callb callb);

/**
 * upwr_pwm_freq_setup() - Determines the next frequency target for a given
 *                         domain and current frequency.
 * @domain: identifier of the domain to change frequency. Defined by
 * SoC-dependent type soc_domain_t found in upower_soc_defs.h.
 * @rail: the pmic regulator number for the target domain.
 * @stage: DVA adjust stage
 * refer to upower_defs.h "DVA adjust stage"
 * @target_freq: the target adjust frequency, accurate to MHz
 *
 * refer to upower_defs.h structure definition upwr_pwm_freq_msg
 *
 * @callb: response callback pointer; NULL if no callback needed.
 *
 * The DVA algorithm is broken down into two phases.
 * The first phase uses a look up table to get a safe operating voltage
 * for the requested frequency.
 * This voltage is guaranteed to work over process and temperature.
 *
 * The second step of the second phase is to measure the temperature
 * using the uPower Temperature Sensor module.
 * This is accomplished by doing a binary search of the TSEL bit field
 * in the Temperature Measurement Register (TMR).
 * The search is repeated until the THIGH bit fields in the same register change value.
 * There are 3 temperature sensors in 8ULP (APD, AVD, and RTD).
 *
 *
 * The second phase is the fine adjust of the voltage.
 * This stage is entered only when the new frequency requested
 * by application was already set as well as the voltage for that frequency.
 * The first step of the fine adjust is to find what is the current margins
 * for the monitored critical paths, or, in other words,
 * how many delay cells will be necessary to generate a setup-timing violation.
 * The function informs uPower that the given domain frequency has changed or
 * will change to the given value. uPower firmware will then adjust voltage and
 * bias to cope with the new frequency (if decreasing) or prepare for it
 * (if increasing). The function must be called after decreasing the frequency,
 * and before increasing it. The actual increase in frequency must not occur
 * before the service returns its response.
 *
 * So, for increase clock frequency case, user need to call this API twice,
 * the first stage gross adjust and the second stage fine adjust.
 *
 * for reduce clock frequency case, user can only call this API once,
 * full stage (combine gross stage and fine adjust)
 *
 * The request is executed if arguments are within range.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_freq_setup(soc_domain_t domain, uint32_t rail, uint32_t stage,
			uint32_t target_freq, upwr_callb callb);

/**
 * upwr_pwm_power_on()- Powers on (not off) one or more switches and ROM/RAMs.
 * @swton: pointer to an array of words that tells which power switches to
 *  turn on. Each word in the array has 1 bit for each switch.
 *  A bit=1 means the respective switch must be turned on,
 *  bit = 0 means it will stay unchanged (on or off).
 *  The pointer may be set to NULL, in which case no switch will be changed,
 *  unless a memory that it feeds must be turned on.
 *  WARNING: swton must not point to the first shared memory address.
 * @memon: pointer to an array of words that tells which memories to turn on.
 *  Each word in the array has 1 bit for each switch.
 *  A bit=1 means the respective memory must be turned on, both array and
 *  periphery logic;
 *  bit = 0 means it will stay unchanged (on or off).
 *  The pointer may be set to NULL, in which case no memory will be changed.
 *  WARNING: memon must not point to the first shared memory address.
 * @callb: pointer to the callback called when configurations are applyed.
 * NULL if no callback is required.
 *
 * The function requests uPower to turn on the PMC and memory array/peripheral
 * switches that control their power, as specified above.
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate memory power state related to overall system state.
 *
 * If a memory is requested to turn on, but the power switch that feeds that
 * memory is not, the power switch will be turned on anyway, if the pwron
 * array is not provided (that is, if pwron is NULL).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Callback or polling may return error if the service contends for a resource
 * already being used by a power mode transition or an ongoing service in
 * another domain.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -2 if a pointer conversion to physical address failed,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_power_on(const uint32_t swton[], const uint32_t memon[],
		      upwr_callb callb);

/**
 * upwr_pwm_power_off()- Powers off (not on) one or more switches and ROM/RAMs.
 * @swtoff: pointer to an array of words that tells which power switches to
 *  turn off. Each word in the array has 1 bit for each switch.
 *  A bit=1 means the respective switch must be turned off,
 *  bit = 0 means it will stay unchanged (on or off).
 *  The pointer may be set to NULL, in which case no switch will be changed.
 *  WARNING: swtoff must not point to the first shared memory address.
 * @memoff: pointer to an array of words that tells which memories to turn off.
 *  Each word in the array has 1 bit for each switch.
 *  A bit=1 means the respective memory must be turned off, both array and
 *  periphery logic;
 *  bit = 0 means it will stay unchanged (on or off).
 *  The pointer may be set to NULL, in which case no memory will be changed,
 *  but notice it may be turned off if the switch that feeds it is powered off.
 *  WARNING: memoff must not point to the first shared memory address.
 * @callb: pointer to the callback called when configurations are applyed.
 * NULL if no callback is required.
 *
 * The function requests uPower to turn off the PMC and memory array/peripheral
 * switches that control their power, as specified above.
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate memory power state related to overall system state.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Callback or polling may return error if the service contends for a resource
 * already being used by a power mode transition or an ongoing service in
 * another domain.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -2 if a pointer conversion to physical address failed,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_power_off(const uint32_t swtoff[], const uint32_t memoff[],
		       upwr_callb callb);

/**
 * upwr_pwm_mem_retain()- Configures one or more memory power switches to
 * retain its contents, having the power array on, while its peripheral logic
 * is turned off.
 * @mem: pointer to an array of words that tells which memories to put in a
 *  retention state. Each word in the array has 1 bit for each memory.
 *  A bit=1 means the respective memory must be put in retention state,
 *  bit = 0 means it will stay unchanged (retention, fully on or off).
 * @callb: pointer to the callback called when configurations are applyed.
 * NULL if no callback is required.
 *
 * The function requests uPower to turn off the memory peripheral and leave
 * its array on, as specified above.
 * The request is executed if arguments are within range.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Callback or polling may return error if the service contends for a resource
 * already being used by a power mode transition or an ongoing service in
 * another domain.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -2 if a pointer conversion to physical address failed,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_mem_retain(const uint32_t mem[], upwr_callb callb);

/**
 * upwr_pwm_chng_switch_mem() - Turns on/off power on one or more PMC switches
 * and memories, including their array and peripheral logic.
 * @swt: pointer to a list of PMC switches to be opened/closed.
 *  The list is structured as an array of struct upwr_switch_board_t
 *  (see upower_defs.h), each one containing a word for up to 32 switches,
 *  one per bit. A bit = 1 means switch closed, bit = 0 means switch open.
 *  struct upwr_switch_board_t also specifies a mask with 1 bit for each
 *  respective switch: mask bit = 1 means the open/close action is applied,
 *  mask bit = 0 means the switch stays unchanged.
 *  The pointer may be set to NULL, in which case no switch will be changed,
 *  unless a memory that it feeds must be turned on.
 *  WARNING: swt must not point to the first shared memory address.
 * @mem: pointer to a list of switches to be turned on/off.
 *  The list is structured as an array of struct upwr_mem_switches_t
 *  (see upower_defs.h), each one containing 2 word for up to 32 switches,
 *  one per bit, one word for the RAM array power switch, other for the
 *  RAM peripheral logic power switch. A bit = 1 means switch closed,
 *  bit = 0 means switch open.
 *  struct upwr_mem_switches_t also specifies a mask with 1 bit for each
 *  respective switch: mask bit = 1 means the open/close action is applied,
 *  mask bit = 0 means the switch stays unchanged.
 *  The pointer may be set to NULL, in which case no memory switch will be
 *  changed, but notice it may be turned off if the switch that feeds it is
 *  powered off.
 *  WARNING: mem must not point to the first shared memory address.
 * @callb: pointer to the callback called when the configurations are applied.
 * NULL if no callback is required.
 *
 * The function requests uPower to change the PMC switches and/or memory power
 * as specified above.
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate switch combinations and overall system state.
 *
 * If a memory is requested to turn on, but the power switch that feeds that
 * memory is not, the power switch will be turned on anyway, if the swt
 * array is not provided (that is, if swt is NULL).
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Callback or polling may return error if the service contends for a resource
 * already being used by a power mode transition or an ongoing service in
 * another domain.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy.
 *        -2 if a pointer conversion to physical address failed,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_chng_switch_mem(const struct upwr_switch_board_t swt[],
			     const struct upwr_mem_switches_t mem[],
			     upwr_callb callb);

/**
 * upwr_pwm_pmode_config() - Configures a given power mode in a given domain.
 * @domain: identifier of the domain to which the power mode belongs.
 * Defined by SoC-dependent type soc_domain_t found in upower_soc_defs.h.
 * @pmode: SoC-dependent power mode identifier defined by type abs_pwr_mode_t
 * found in upower_soc_defs.h.
 * @config: pointer to an SoC-dependent struct defining the power mode
 * configuration, found in upower_soc_defs.h.
 * @callb: pointer to the callback called when configurations are applied.
 * NULL if no callback is required.
 *
 * The function requests uPower to change the power mode configuration as
 * specified above. The request is executed if arguments are within range,
 * and complies with SoC-dependent restrictions on value combinations.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -2 if the pointer conversion to physical address failed,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_pmode_config(soc_domain_t domain, abs_pwr_mode_t pmode,
			  const void *config, upwr_callb callb);



/**
 * upwr_pwm_reg_config() - Configures the uPower internal regulators.
 * @config: pointer to the struct defining the regulator configuration;
 * the struct upwr_reg_config_t is defined in the file upower_defs.h.
 * @callb: pointer to the callback called when configurations are applied.
 * NULL if no callback is required.
 *
 * The function requests uPower to change/define the configurations of the
 * internal regulators.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * The service may fail with error UPWR_RESP_RESOURCE if a power mode transition
 * or the same service (called from another domain) is executing simultaneously.
 * This error should be interpreted as a "try later" response, as the service
 * will succeed once those concurrent executions are done, and no other is
 * started.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -2 if the pointer conversion to physical address failed,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_pwm_reg_config(const struct upwr_reg_config_t *config,
			upwr_callb callb);

/**
 * upwr_pwm_chng_dom_bias() - Changes the domain bias.
 * @bias: pointer to a domain bias configuration struct (see upower_soc_defs.h).
 * @callb: pointer to the callback called when configurations are applied.
 * NULL if no callback is required.
 *
 * The function requests uPower to change the domain bias configuration as
 * specified above. The request is executed if arguments are within range,
 * with no protections regarding the adequate value combinations and
 * overall system state.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */

int upwr_pwm_chng_dom_bias(const struct upwr_dom_bias_cfg_t *bias,
			   upwr_callb callb);

/**
 * upwr_pwm_chng_mem_bias()- Changes a ROM/RAM power bias.
 * @domain: identifier of the domain upon which the bias is applied.
 * Defined by SoC-dependent type soc_domain_t found in upower_soc_defs.h.
 * @bias: pointer to a memory bias configuration struct (see upower_soc_defs.h).
 * @callb: pointer to the callback called when configurations are applied.
 * NULL if no callback is required.
 *
 * The function requests uPower to change the memory bias configuration as
 * specified above. The request is executed if arguments are within range,
 * with no protections regarding the adequate value combinations and
 * overall system state.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */

int upwr_pwm_chng_mem_bias(soc_domain_t domain,
			   const struct upwr_mem_bias_cfg_t *bias,
			   upwr_callb callb);

/**---------------------------------------------------------------
 * VOLTAGE MANAGEMENT SERVICE GROUP
 */

/**
 * upwr_vtm_pmic_cold_reset() -request cold reset the pmic.
 * pmic will power cycle all the regulators
 * @callb: response callback pointer; NULL if no callback needed.
 *
 * The function requests uPower to cold reset the pmic.
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_VOLTM as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_vtm_pmic_cold_reset(upwr_callb callb);

/**
 * upwr_vtm_set_pmic_mode() -request uPower set pmic mode
 * @pmic_mode: the target mode need to be set
 * @callb: response callback pointer; NULL if no callback needed.
 *
 * The function requests uPower to set pmic mode
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_VOLTM as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_vtm_set_pmic_mode(uint32_t pmic_mode, upwr_callb callb);

/**
 * upwr_vtm_chng_pmic_voltage() - Changes the voltage of a given rail.
 * @rail: pmic rail id.
 * @volt: the target voltage of the given rail, accurate to uV
 * If pass volt value 0, means that power off this rail.
 * @callb: response callback pointer; NULL if no callback needed.
 *
 * The function requests uPower to change the voltage of the given rail.
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_VOLTM as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_vtm_chng_pmic_voltage(uint32_t rail, uint32_t volt, upwr_callb callb);

/**
 * upwr_vtm_get_pmic_voltage() - Get the voltage of a given ral.
 * @rail: pmic rail id.
 * @callb: response callback pointer; NULL if no callback needed.
 * (polling used instead)
 *
 * The function requests uPower to get the voltage of the given rail.
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_VOLTM as the service group argument.
 *
 * The voltage data read from uPower via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_vtm_get_pmic_voltage(uint32_t rail, upwr_callb callb);


/**
 * upwr_vtm_power_measure() - request uPower to measure power consumption
 * @ssel: This field determines which power switches will have their currents
 * sampled to be accounted for a
 * current/power measurement. Support 0~7

 * SSEL bit #	Power Switch
 * 0	M33 core complex/platform/peripherals
 * 1	Fusion Core and Peripherals
 * 2	A35[0] core complex
 * 3	A35[1] core complex
 * 4	3DGPU
 * 5	HiFi4
 * 6	DDR Controller (PHY and PLL NOT included)
 * 7	PXP, EPDC
 *
 * @callb: response callback pointer; NULL if no callback needed.
 * (polling used instead)
 *
 * The function requests uPower to measure power consumption
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_VOLTM as the service group argument.
 *
 * The power consumption data read from uPower via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 * upower fw needs support cocurrent request from M33 and A35.
 *
 * Accurate to uA
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_vtm_power_measure(uint32_t ssel, upwr_callb callb);

/**
 * upwr_vtm_vmeter_measure() - request uPower to measure voltage
 * @vdetsel: Voltage Detector Selector, support 0~3
 * 00b - RTD sense point
 * 01b - LDO output
 * 10b - APD domain sense point
 * 11b - AVD domain sense point
 * Refer to upower_defs.h
 * @callb: response callback pointer; NULL if no callback needed.
 * (polling used instead)
 *
 * The function requests uPower to use vmeter to measure voltage
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_VOLTM as the service group argument.
 *
 * The voltage data read from uPower via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 * upower fw needs support cocurrent request from M33 and A35.
 *
 * Refer to RM COREREGVL (Core Regulator Voltage Level)
 * uPower return VDETLVL to user, user can calculate the real voltage:
 *
 * 0b000000(0x00) - 0.595833V
 * 0b100110(0x26) - 1.007498V
 * <value> - 0.595833V + <value>x10.8333mV
 * 0b110010(0x32) - 1.138V
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_vtm_vmeter_measure(uint32_t vdetsel, upwr_callb callb);

/**
 * upwr_vtm_pmic_config() - Configures the SoC PMIC (Power Management IC).
 * @config: pointer to a PMIC-dependent struct defining the PMIC configuration.
 * @size:   size of the struct pointed by config, in bytes.
 * @callb: pointer to the callback called when configurations are applied.
 * NULL if no callback is required.
 *
 * The function requests uPower to change/define the PMIC configuration.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_PWRMGMT as the service group argument.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok, -1 if service group is busy,
 *        -2 if the pointer conversion to physical address failed,
 *        -3 if called in an invalid API state.
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_vtm_pmic_config(const void *config, uint32_t size, upwr_callb callb);

/**---------------------------------------------------------------
 * TEMPERATURE MANAGEMENT SERVICE GROUP
 */

/**
 * upwr_tpm_get_temperature() - request uPower to get temperature of one temperature sensor
 * @sensor_id: temperature sensor ID, support 0~2
 * @callb: response callback pointer; NULL if no callback needed.
 * (polling used instead)
 *
 * The function requests uPower to measure temperature
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_TEMPM as the service group argument.
 *
 * The temperature data read from uPower via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 *
 * uPower return TSEL to the caller (M33 or A35), caller calculate the real temperature
 * Tsh = 0.000002673049*TSEL[7:0]^3 + 0.0003734262*TSEL[7:0]^2 +
0.4487042*TSEL[7:0] - 46.98694
 *
 * upower fw needs support cocurrent request from M33 and A35.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_tpm_get_temperature(uint32_t sensor_id, upwr_callb callb);

/**---------------------------------------------------------------
 * DELAY MANAGEMENT SERVICE GROUP
 */

/**
 * upwr_dlm_get_delay_margin() - request uPower to get delay margin
 * @path: The critical path
 * @index: Use whitch delay meter
 * @callb: response callback pointer; NULL if no callback needed.
 * (polling used instead)
 *
 * The function requests uPower to get delay margin
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_DELAYM as the service group argument.
 *
 * The delay margin data read from uPower via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 * upower fw needs support cocurrent request from M33 and A35.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_dlm_get_delay_margin(uint32_t path, uint32_t index, upwr_callb callb);

/**
 * upwr_dlm_set_delay_margin() - request uPower to set delay margin
 * @path: The critical path
 * @index: Use whitch delay meter
 * @delay_margin: the value of delay margin
 * @callb: response callback pointer; NULL if no callback needed.
 * (polling used instead)
 *
 * The function requests uPower to set delay margin
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_DELAYM as the service group argument.
 *
 * The result of the corresponding critical path,  failed or not  read from uPower via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 * upower fw needs support cocurrent request from M33 and A35.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_dlm_set_delay_margin(uint32_t path, uint32_t index, uint32_t delay_margin, upwr_callb callb);

/**
 * upwr_dlm_process_monitor() - request uPower to do process monitor
 * @chain_sel: Chain Cell Type Selection
 * Select the chain to be used for the clock signal generation.
 * Support two types chain cell, 0~1
0b - P4 type delay cells selected
1b - P16 type delay cells selected
 * @callb: response callback pointer; NULL if no callback needed.
 * (polling used instead)
 *
 * The function requests uPower to do process monitor
 * The request is executed if arguments are within range, with no protections
 * regarding the adequate voltage value for the given domain process,
 * temperature and frequency.
 *
 * A callback can be optionally registered, and will be called upon the arrival
 * of the request response from the uPower firmware, telling if it succeeded
 * or not.
 *
 * A callback may not be registered (NULL pointer), in which case polling has
 * to be used to check the response, by calling upwr_req_status or
 * upwr_poll_req_status, using UPWR_SG_DELAYM as the service group argument.
 *
 * The result of process monitor,  failed or not  read from uPower via
 * the callback argument ret, or written to the variable pointed by retptr,
 * if polling is used (calls upwr_req_status or upwr_poll_req_status).
 * ret (or *retptr) also returns the data written on writes.
 * upower fw needs support cocurrent request from M33 and A35.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 * Note that this is not the error response from the request itself:
 * it only tells if the request was successfully sent to the uPower.
 */
int upwr_dlm_process_monitor(uint32_t chain_sel, upwr_callb callb);

/**---------------------------------------------------------------
 * DIAGNOSE SERVICE GROUP
 */

/**
 * upwr_dgn_mode() - Sets the diagnostic mode.
 * @mode:  diagnostic mode, which can be:
 *  - UPWR_DGN_NONE:   no diagnostic recorded
 *  - UPWR_DGN_TRACE:  warnings, errors, service, internal activity recorded
 *  - UPWR_DGN_SRVREQ: warnings, errors, service activity recorded
 *  - UPWR_DGN_WARN:   warnings and errors recorded
 *  - UPWR_DGN_ALL:    trace, service, warnings, errors, task state recorded
 *  - UPWR_DGN_ERROR:  only errors recorded
 *  - UPWR_DGN_ALL2ERR: record all until an error occurs,
 *    freeze recording on error
 *  - UPWR_DGN_ALL2HLT: record all until an error occurs,
 *    executes an ebreak on error, which halts the core if enabled through
 *    the debug interface
 * @callb: pointer to the callback called when mode is changed.
 * NULL if no callback is required.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok,
 *        -1 if service group is busy,
 *        -3 if called in an invalid API state
 */
int upwr_dgn_mode(upwr_dgn_mode_t mode, const upwr_callb callb);

/**---------------------------------------------------------------
 * AUXILIARY CALLS
 */

/**
 * upwr_rom_version() - informs the ROM firwmware version.
 * @vmajor: pointer to the variable to get the firmware major version number.
 * @vminor: pointer to the variable to get the firmware minor version number.
 * @vfixes: pointer to the variable to get the firmware fixes number.
 *
 * Context: no sleep, no locks taken/released.
 * Return: SoC id.
 */
uint32_t upwr_rom_version(uint32_t *vmajor, uint32_t *vminor, uint32_t *vfixes);

/**
 * upwr_ram_version() - informs the RAM firwmware version.
 * @vminor: pointer to the variable to get the firmware minor version number.
 * @vfixes: pointer to the variable to get the firmware fixes number.
 *
 * The 3 values returned are 0 if no RAM firmwmare was loaded and initialized.
 *
 * Context: no sleep, no locks taken/released.
 * Return: firmware major version number.
 */
uint32_t upwr_ram_version(uint32_t *vminor, uint32_t *vfixes);

/**
 * upwr_req_status() - tells the status of the service group request, and
 *                     returns a request return value, if any.
 * @sg: service group of the request
 * @sgfptr: pointer to the variable that will hold the function id of
 * the last request completed; can be NULL, in which case it is not used.
 * @errptr: pointer to the variable that will hold the error code;
 * can be NULL, in which case it is not used.
 * @retptr: pointer to the variable that will hold the value returned
 * by the last request completed (invalid if the last request completed didn't
 * return any value); can be NULL, in which case it is not used.
 * Note that a request may return a value even if service error is returned
 * (*errptr != UPWR_RESP_OK): that is dependent on the specific service.
 *
 * This call can be used in a poll loop of a service request completion in case
 * a callback was not registered.
 *
 * Context: no sleep, no locks taken/released.
 * Return: service request status: succeeded, failed, or ongoing (busy)
 */

/* service request status */
typedef enum {
	UPWR_REQ_OK,     /* request succeeded */
	UPWR_REQ_ERR,    /* request failed */
	UPWR_REQ_BUSY    /* request execution ongoing */
} upwr_req_status_t;

upwr_req_status_t upwr_req_status(upwr_sg_t sg,
				  uint32_t *sgfptr,
				  upwr_resp_t *errptr,
				  int *retptr);

/**
 * upwr_poll_req_status() - polls the status of the service group request, and
 *                          returns a request return value, if any.
 * @sg: service group of the request
 * @sgfptr: pointer to the variable that will hold the function id of
 * the last request completed; can be NULL, in which case it is not used.
 * @errptr: pointer to the variable that will hold the error code;
 * can be NULL, in which case it is not used.
 * @retptr: pointer to the variable that will hold the value returned
 * by the last request completed (invalid if the last request completed didn't
 * return any value); can be NULL, in which case it is not used.
 * Note that a request may return a value even if service error is returned
 * (*errptr != UPWR_RESP_OK): that is dependent on the specific service.
 * @attempts: maximum number of polling attempts; if attempts > 0 and is
 * reached with no service response received, upwr_poll_req_status returns
 * UPWR_REQ_BUSY and variables pointed by sgfptr, retptr and errptr are not
 * updated; if attempts = 0, upwr_poll_req_status waits "forever".
 *
 * This call can be used to poll a service request completion in case a
 * callback was not registered.
 *
 * Context: no sleep, no locks taken/released.
 * Return: service request status: succeeded, failed, or ongoing (busy)
 */
upwr_req_status_t upwr_poll_req_status(upwr_sg_t sg,
				       uint32_t *sgfptr,
				       upwr_resp_t *errptr,
				       int *retptr,
				       uint32_t attempts);

/**
 * upwr_alarm_code() - returns the alarm code of the last alarm occurrence.
 *
 * The value returned is not meaningful if no alarm was issued by uPower.
 *
 * Context: no sleep, no locks taken/released.
 * Return: alarm code, as defined by the type upwr_alarm_t in upwr_soc_defines.h
 */
upwr_alarm_t upwr_alarm_code(void);

/**---------------------------------------------------------------
 * TRANSMIT/RECEIVE PRIMITIVES
 * ---------------------------------------------------------------
 */

typedef void (*UPWR_TX_CALLB_FUNC_T)(void);
typedef void (*UPWR_RX_CALLB_FUNC_T)(void);

/**
 * upwr_tx() - queues a message for transmission.
 * @msg : pointer to the message sent.
 * @size: message size in 32-bit words
 * @callback: pointer to a function to be called when transmission done;
 *            can be NULL, in which case no callback is done.
 *
 * This is an auxiliary function used by the rest of the API calls.
 * It is normally not called by the driver code, unless maybe for test purposes.
 *
 * Context: no sleep, no locks taken/released.
 * Return: number of vacant positions left in the transmission queue, or
 *         -1 if the queue was already full when upwr_tx was called, or
 *         -2 if any argument is invalid (like size off-range)
 */
int upwr_tx(const uint32_t *msg, unsigned int size,
	    UPWR_TX_CALLB_FUNC_T callback);

/**
 * upwr_rx() - unqueues a received message from the reception queue.
 * @msg: pointer to the message destination buffer.
 * @size: pointer to variable to hold message size in 32-bit words.
 *
 * This is an auxiliary function used by the rest of the API calls.
 * It is normally not called by the driver code, unless maybe for test purposes.
 *
 * Context: no sleep, no locks taken/released.
 * Return: number of messages remaining in the reception queue, or
 *         -1 if the queue was already empty when upwr_rx was called, or
 *         -2 if any argument is invalid (like mu off-range)
 */
int upwr_rx(char *msg, unsigned int *size);

/**
 * upwr_rx_callback() - sets up a callback for a message receiving event.
 * @callback: pointer to a function to be called when a message arrives;
 *            can be NULL, in which case no callback is done.
 *
 * This is an auxiliary function used by the rest of the API calls.
 * It is normally not called by the driver code, unless maybe for test purposes.
 *
 * Context: no sleep, no locks taken/released.
 * Return: 0 if ok; -2 if any argument is invalid (mu off-range).
 */
int upwr_rx_callback(UPWR_RX_CALLB_FUNC_T callback);

/**
 * msg_copy() - copies a message.
 * @dest: pointer to the destination message.
 * @src : pointer to the source message.
 * @size: message size in words.
 *
 * This is an auxiliary function used by the rest of the API calls.
 * It is normally not called by the driver code, unless maybe for test purposes.
 *
 * Context: no sleep, no locks taken/released.
 * Return: none (void)
 */
void msg_copy(char *dest, char *src, unsigned int size);

#endif /* UPWR_API_H */
