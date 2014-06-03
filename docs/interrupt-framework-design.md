ARM Trusted Firmware Interrupt Management Design guide
======================================================

Contents :

1.  Introduction
    *    Assumptions
    *    Concepts
         -    Interrupt Types
         -    Routing Model
         -    Valid Routing Models
              +    Secure-EL1 Interrupts
              +    Non-secure Interrupts
         -    Mapping of Interrupt Type to Signal

2.  Interrupt Management
    *    Software Components
    *    Interrupt Registration
         -    EL3 Runtime Firmware
         -    Secure Payload Dispatcher
              +    Test Secure Payload Dispatcher behavior
         -    Secure Payload
              +    Secure Payload IHF design w.r.t Secure-EL1 interrupts
              +    Secure Payload IHF design w.r.t Non-secure interrupts
              +    Test Secure Payload behavior
    *    Interrupt Handling
         -    EL3 Runtime Firmware
         -    Secure Payload Dispatcher
              +    Interrupt Entry
              +    Interrupt Exit
              +    Test Secure Payload Dispatcher behavior
         -    Secure Payload
              +    Test Secure Payload behavior


1.  Introduction
----------------
This document describes the design of the Interrupt management framework in ARM
Trusted Firmware. This section briefly describes the requirements from this
framework. It also briefly explains some concepts and assumptions. They will
help in understanding the implementation of the framework explained in
subsequent sections.

This framework is responsible for managing interrupts routed to EL3. It also
allows EL3 software to configure the interrupt routing behavior. Its main
objective is to implement the following two requirements.

1.  It should be possible to route interrupts meant to be handled by secure
    software (Secure interrupts) to EL3, when execution is in non-secure state
    (normal world). The framework should then take care of handing control of
    the interrupt to either software in EL3 or Secure-EL1 depending upon the
    software configuration and the GIC implementation. This requirement ensures
    that secure interrupts are under the control of the secure software with
    respect to their delivery and handling without the possibility of
    intervention from non-secure software.

2.  It should be possible to route interrupts meant to be handled by
  non-secure software (Non-secure interrupts) to the last executed exception
    level in the normal world when the execution is in secure world at
    exception levels lower than EL3. This could be done with or without the
    knowledge of software executing in Secure-EL1/Secure-EL0. The choice of
    approach should be governed by the secure software. This requirement
    ensures that non-secure software is able to execute in tandem with the
    secure software without overriding it.

### 1.1 Assumptions
The framework makes the following assumptions to simplify its implementation.

1.  All secure interrupts are handled in Secure-EL1. They can be delivered to
    Secure-EL1 via EL3 but they cannot be handled in EL3. It will be possible
    to extend the framework to handle secure interrupts in EL3 in the future.

2.  Interrupt exceptions (`PSTATE.I` and `F` bits) are masked during execution
    in EL3.

### 1.2 Concepts

#### 1.2.1 Interrupt types
The framework categorises an interrupt to be one of the following depending upon
the exception level(s) it is handled in.

1.  Secure EL1 interrupt. This type of interrupt can be routed to EL3 or
    Secure-EL1 depending upon the security state of the current execution
    context. It is always handled in Secure-EL1.

2.  Non-secure interrupt. This type of interrupt can be routed to EL3,
    Secure-EL1, Non-secure EL1 or EL2 depending upon the security state of the
    current execution context. It is always handled in either Non-secure EL1
    or EL2.

3.  EL3 interrupt. This type of interrupt can be routed to EL3 or Secure-EL1
    depending upon the security state of the current execution context. It is
    always handled in EL3.

In the current implementation of the framework, all secure interrupts are
treated as Secure EL1 interrupts. It will be possible for EL3 software to
configure a secure interrupt as an EL3 interrupt in future implementations.  The
following constants define the various interrupt types in the framework
implementation.

    #define INTR_TYPE_S_EL1      0
    #define INTR_TYPE_EL3        1
    #define INTR_TYPE_NS         2


#### 1.2.2 Routing model
A type of interrupt can be either generated as an FIQ or an IRQ. The target
exception level of an interrupt type is configured through the FIQ and IRQ bits
in the Secure Configuration Register at EL3 (`SCR_EL3.FIQ` and `SCR_EL3.IRQ`
bits). When `SCR_EL3.FIQ`=1, FIQs are routed to EL3. Otherwise they are routed
to the First Exception Level (FEL) capable of handling interrupts. When
`SCR_EL3.IRQ`=1, IRQs are routed to EL3. Otherwise they are routed to the
FEL. This register is configured independently by EL3 software for each security
state prior to entry into a lower exception level in that security state.

A routing model for a type of interrupt (generated as FIQ or IRQ) is defined as
its target exception level for each security state. It is represented by a
single bit for each security state. A value of `0` means that the interrupt
should be routed to the FEL. A value of `1` means that the interrupt should be
routed to EL3. A routing model is applicable only when execution is not in EL3.

The default routing model for an interrupt type is to route it to the FEL in
either security state.

#### 1.2.3 Valid routing models
The framework considers certain routing models for each type of interrupt to be
incorrect as they conflict with the requirements mentioned in Section 1. The
following sub-sections describe all the possible routing models and specify
which ones are valid or invalid. Only the Secure-EL1 and Non-secure interrupt
types are considered as EL3 interrupts are currently unsupported (See 1.1). The
terminology used in the following sub-sections is explained below.

1.  __CSS__. Current Security State. `0` when secure and `1` when non-secure

2.  __TEL3__. Target Exception Level 3. `0` when targeted to the FEL. `1` when
    targeted to EL3.


##### 1.2.3.1 Secure-EL1 interrupts

1.  __CSS=0, TEL3=0__. Interrupt is routed to the FEL when execution is in
    secure state. This is a valid routing model as secure software is in
    control of handling secure interrupts.

2.  __CSS=0, TEL3=1__. Interrupt is routed to EL3 when execution is in secure
    state. This is a valid routing model as secure software in EL3 can
    handover the interrupt to Secure-EL1 for handling.

3.  __CSS=1, TEL3=0__. Interrupt is routed to the FEL when execution is in
    non-secure state. This is an invalid routing model as a secure interrupt
    is not visible to the secure software which violates the motivation behind
    the ARM Security Extensions.

4.  __CSS=1, TEL3=1__. Interrupt is routed to EL3 when execution is in secure
    state. This is a valid routing model as secure software in EL3 can
    handover the interrupt to Secure-EL1 for handling.


##### 1.2.3.2 Non-secure interrupts

1.  __CSS=0, TEL3=0__. Interrupt is routed to the FEL when execution is in
    secure state. This allows the secure software to trap non-secure
    interrupts, perform its bookeeping and hand the interrupt to the
    non-secure software through EL3. This is a valid routing model as secure
    software is in control of how its execution is pre-empted by non-secure
    interrupts.

2.  __CSS=0, TEL3=1__. Interrupt is routed to EL3 when execution is in secure
    state. This is a valid routing model as secure software in EL3 can save
    the state of software in Secure-EL1/Secure-EL0 before handing the
    interrupt to non-secure software. This model requires additional
    coordination between Secure-EL1 and EL3 software to ensure that the
    former's state is correctly saved by the latter.

3.  __CSS=1, TEL3=0__. Interrupt is routed to FEL when execution is in
    non-secure state. This is an valid routing model as a non-secure interrupt
    is handled by non-secure software.

4.   __CSS=1, TEL3=1__. Interrupt is routed to EL3 when execution is in
    non-secure state. This is an invalid routing model as there is no valid
    reason to route the interrupt to EL3 software and then hand it back to
    non-secure software for handling.


#### 1.2.4 Mapping of interrupt type to signal
The framework is meant to work with any interrupt controller implemented by a
platform. A interrupt controller could generate a type of interrupt as either an
FIQ or IRQ signal to the CPU depending upon the current security state.The
mapping between the type and signal is known only to the platform. The framework
uses this information to determine whether the IRQ or the FIQ bit should be
programmed in `SCR_EL3` while applying the routing model for a type of
interrupt. The platform provides this information through the
`plat_interrupt_type_to_line()` API (described in the [Porting
Guide]). For example, on the FVP port when the platform uses an ARM GICv2
interrupt controller, Secure-EL1 interrupts are signalled through the FIQ signal
while Non-secure interrupts are signalled through the IRQ signal. This applies
when execution is in either security state.


2. Interrupt management
-----------------------
The following sections describe how interrupts are managed by the interrupt
handling framework. This entails:

1.  Providing an interface to allow registration of a handler and specification
    of the routing model for a type of interrupt.

2.  Implementing support to hand control of an interrupt type to its registered
    handler when the interrupt is generated.

Both aspects of interrupt management involve various components in the secure
software stack spanning from EL3 to Secure-EL1. These components are described
in the section 2.1. The framework stores information associated with each type
of interrupt in the following data structure.

```
typedef struct intr_type_desc {
        interrupt_type_handler_t handler;
        uint32_t flags;
        uint32_t scr_el3[2];
} intr_type_desc_t;
```

The `flags` field stores the routing model for the interrupt type in
bits[1:0]. Bit[0] stores the routing model when execution is in the secure
state. Bit[1] stores the routing model when execution is in the non-secure
state. As mentioned in Section 1.2.2, a value of `0` implies that the interrupt
should be targeted to the FEL. A value of `1` implies that it should be targeted
to EL3. The remaining bits are reserved and SBZ. The helper macro
`set_interrupt_rm_flag()` should be used to set the bits in the `flags`
parameter.

The `scr_el3[2]` field also stores the routing model but as a mapping of the
model in the `flags` field to the corresponding bit in the `SCR_EL3` for each
security state.

The framework also depends upon the platform port to configure the interrupt
controller to distinguish between secure and non-secure interrupts. The platform
is expected to be aware of the secure devices present in the system and their
associated interrupt numbers. It should configure the interrupt controller to
enable the secure interrupts, ensure that their priority is always higher than
the non-secure interrupts and target them to the primary CPU. It should also
export the interface described in the the [Porting Guide][PRTG] to enable
handling of interrupts.

In the remainder of this document, for the sake of simplicity it is assumed that
the FIQ signal is used to generate Secure-EL1 interrupts and the IRQ signal is
used to generate non-secure interrupts in either security state.

### 2.1 Software components
Roles and responsibilities for interrupt management are sub-divided between the
following components of software running in EL3 and Secure-EL1. Each component is
briefly described below.

1.  EL3 Runtime Firmware. This component is common to all ports of the ARM
    Trusted Firmware.

2.  Secure Payload Dispatcher (SPD) service. This service interfaces with the
    Secure Payload (SP) software which runs in exception levels lower than EL3
    i.e. Secure-EL1/Secure-EL0. It is responsible for switching execution
    between software running in secure and non-secure states at exception
    levels lower than EL3. A switch is triggered by a Secure Monitor Call from
    either state. It uses the APIs exported by the Context management library
    to implement this functionality. Switching execution between the two
    security states is a requirement for interrupt management as well. This
    results in a significant dependency on the SPD service. ARM Trusted
    firmware implements an example Test Secure Payload Dispatcher (TSPD)
    service.

    An SPD service plugs into the EL3 runtime firmware and could be common to
    some ports of the ARM Trusted Firmware.

3.  Secure Payload (SP). On a production system, the Secure Payload corresponds
    to a Secure OS which runs in Secure-EL1/Secure-EL0. It interfaces with the
    SPD service to manage communication with non-secure software. ARM Trusted
    Firmware implements an example secure payload called Test Secure Payload
    (TSP) which runs only in Secure-EL1.

    A Secure payload implementation could be common to some ports of the ARM
    Trusted Firmware just like the SPD service.


### 2.2 Interrupt registration
This section describes in detail the role of each software component (see 2.1)
during the registration of a handler for an interrupt type.


#### 2.2.1 EL3 runtime firmware
This component declares the following prototype for a handler of an interrupt type.

        typedef uint64_t (*interrupt_type_handler_t)(uint32_t id,
					     uint32_t flags,
					     void *handle,
					     void *cookie);

The value of the `id` parameter depends upon the definition of the
`IMF_READ_INTERRUPT_ID` build time flag. When the flag is defined, `id` contains
the number of the highest priority pending interrupt of the type that this
handler was registered for. When the flag is not defined `id` contains
`INTR_ID_UNAVAILABLE`.

The `flags` parameter contains miscellaneous information as follows.

1.  Security state, bit[0]. This bit indicates the security state of the lower
    exception level when the interrupt was generated. A value of `1` means
    that it was in the non-secure state. A value of `0` indicates that it was
    in the secure state. This bit can be used by the handler to ensure that
    interrupt was generated and routed as per the routing model specified
    during registration.

2.  Reserved, bits[31:1]. The remaining bits are reserved for future use.

The `handle` parameter points to the `cpu_context` structure of the current CPU
for the security state specified in the `flags` parameter.

Once the handler routine completes, execution will return to either the secure
or non-secure state. The handler routine should return a pointer to
`cpu_context` structure of the current CPU for the the target security state. It
should treat all error conditions as critical errors and take appropriate action
within its implementation e.g. use assertion failures.

The runtime firmware provides the following API for registering a handler for a
particular type of interrupt. A Secure Payload Dispatcher service should use
this API to register a handler for Secure-EL1 and optionally for non-secure
interrupts. This API also requires the caller to specify the routing model for
the type of interrupt.

    int32_t register_interrupt_type_handler(uint32_t type,
					interrupt_type_handler handler,
					uint64_t flags);


The `type` parameter can be one of the three interrupt types listed above i.e.
`INTR_TYPE_S_EL1`, `INTR_TYPE_NS` & `INTR_TYPE_EL3` (currently unimplemented).
The `flags` parameter is as described in Section 2.

The function will return `0` upon a successful registration. It will return
`-EALREADY` in case a handler for the interrupt type has already been
registered.  If the `type` is unrecognised or the `flags` or the `handler` are
invalid it will return `-EINVAL`. It will return `-ENOTSUP` if the specified
`type` is not supported by the framework i.e. `INTR_TYPE_EL3`.

Interrupt routing is governed by the configuration of the `SCR_EL3.FIQ/IRQ` bits
prior to entry into a lower exception level in either security state. The
context management library maintains a copy of the `SCR_EL3` system register for
each security state in the `cpu_context` structure of each CPU. It exports the
following APIs to let EL3 Runtime Firmware program and retrieve the routing
model for each security state for the current CPU. The value of `SCR_EL3` stored
in the `cpu_context` is used by the `el3_exit()` function to program the
`SCR_EL3` register prior to returning from the EL3 exception level.

        uint32_t cm_get_scr_el3(uint32_t security_state);
        void cm_write_scr_el3_bit(uint32_t security_state,
                                  uint32_t bit_pos,
                                  uint32_t value);

`cm_get_scr_el3()` returns the value of the `SCR_EL3` register for the specified
security state of the current CPU. `cm_write_scr_el3()` writes a `0` or `1` to
the bit specified by `bit_pos`. `register_interrupt_type_handler()` invokes
`set_routing_model()` API which programs the `SCR_EL3` according to the routing
model using the `cm_get_scr_el3()` and `cm_write_scr_el3_bit()` APIs.

It is worth noting that in the current implementation of the framework, the EL3
runtime firmware is responsible for programming the routing model. The SPD is
responsible for ensuring that the routing model has been adhered to upon
receiving an interrupt.

#### 2.2.2 Secure payload dispatcher
A SPD service is responsible for determining and maintaining the interrupt
routing model supported by itself and the Secure Payload. It is also responsible
for ferrying interrupts between secure and non-secure software depending upon
the routing model. It could determine the routing model at build time or at
runtime. It must use this information to register a handler for each interrupt
type using the `register_interrupt_type_handler()` API in EL3 runtime firmware.

If the routing model is not known to the SPD service at build time, then it must
be provided by the SP as the result of its initialisation. The SPD should
program the routing model only after SP initialisation has completed e.g. in the
SPD initialisation function pointed to by the `bl32_init` variable.

The SPD should determine the mechanism to pass control to the Secure Payload
after receiving an interrupt from the EL3 runtime firmware. This information
could either be provided to the SPD service at build time or by the SP at
runtime.

#### 2.2.2.1 Test secure payload dispatcher behavior
The TSPD only handles Secure-EL1 interrupts and is provided with the following
routing model at build time.

*   Secure-EL1 interrupts are routed to EL3 when execution is in non-secure
    state and are routed to the FEL when execution is in the secure state
    i.e __CSS=0, TEL3=0__ & __CSS=1, TEL3=1__ for Secure-EL1 interrupts

*   The default routing model is used for non-secure interrupts i.e they are
    routed to the FEL in either security state i.e __CSS=0, TEL3=0__ &
    __CSS=1, TEL3=0__ for Non-secure interrupts

It performs the following actions in the `tspd_init()` function to fulfill the
requirements mentioned earlier.

1.  It passes control to the Test Secure Payload to perform its
    initialisation. The TSP provides the address of the vector table
    `tsp_vectors` in the SP which also includes the handler for Secure-EL1
    interrupts in the `fiq_entry` field. The TSPD passes control to the TSP at
    this address when it receives a Secure-EL1 interrupt.

    The handover agreement between the TSP and the TSPD requires that the TSPD
    masks all interrupts (`PSTATE.DAIF` bits) when it calls
    `tsp_fiq_entry()`. The TSP has to preserve the callee saved general
    purpose, SP_EL1/Secure-EL0, LR, VFP and system registers. It can use
    `x0-x18` to enable its C runtime.

2.  The TSPD implements a handler function for Secure-EL1 interrupts. It
    registers it with the EL3 runtime firmware using the
    `register_interrupt_type_handler()` API as follows

        /* Forward declaration */
        interrupt_type_handler tspd_secure_el1_interrupt_handler;
        int32_t rc, flags = 0;
        set_interrupt_rm_flag(flags, NON_SECURE);
        rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
                                         tspd_secure_el1_interrupt_handler,
                                         flags);
        assert(rc == 0);

#### 2.2.3 Secure payload
A Secure Payload must implement an interrupt handling framework at Secure-EL1
(Secure-EL1 IHF) to support its chosen interrupt routing model. Secure payload
execution will alternate between the below cases.

1.  In the code where IRQ, FIQ or both interrupts are enabled, if an interrupt
    type is targeted to the FEL, then it will be routed to the Secure-EL1
    exception vector table. This is defined as the asynchronous model of
    handling interrupts. This mode applies to both Secure-EL1 and non-secure
    interrupts.

2.  In the code where both interrupts are disabled, if an interrupt type is
    targeted to the FEL, then execution will eventually migrate to the
    non-secure state. Any non-secure interrupts will be handled as described
    in the routing model where __CSS=1 and TEL3=0__. Secure-EL1 interrupts
    will be routed to EL3 (as per the routing model where __CSS=1 and
    TEL3=1__) where the SPD service will hand them to the SP. This is defined
    as the synchronous mode of handling interrupts.

The interrupt handling framework implemented by the SP should support one or
both these interrupt handling models depending upon the chosen routing model.

The following list briefly describes how the choice of a valid routing model
(See 1.2.3) effects the implementation of the Secure-EL1 IHF. If the choice of
the interrupt routing model is not known to the SPD service at compile time,
then the SP should pass this information to the SPD service at runtime during
its initialisation phase.

As mentioned earlier, it is assumed that the FIQ signal is used to generate
Secure-EL1 interrupts and the IRQ signal is used to generate non-secure
interrupts in either security state.

##### 2.2.3.1 Secure payload IHF design w.r.t secure-EL1 interrupts
1.  __CSS=0, TEL3=0__. If `PSTATE.F=0`, Secure-EL1 interrupts will be
    trigerred at one of the Secure-EL1 FIQ exception vectors. The Secure-EL1
    IHF should implement support for handling FIQ interrupts asynchronously.

    If `PSTATE.F=1` then Secure-EL1 interrupts will be handled as per the
    synchronous interrupt handling model. The SP could implement this scenario
    by exporting a seperate entrypoint for Secure-EL1 interrupts to the SPD
    service during the registration phase. The SPD service would also need to
    know the state of the system, general purpose and the `PSTATE` registers
    in which it should arrange to return execution to the SP. The SP should
    provide this information in an implementation defined way during the
    registration phase if it is not known to the SPD service at build time.

2.  __CSS=1, TEL3=1__. Interrupts are routed to EL3 when execution is in
    non-secure state. They should be handled through the synchronous interrupt
    handling model as described in 1. above.

3.  __CSS=0, TEL3=1__. Secure interrupts are routed to EL3 when execution is in
    secure state. They will not be visible to the SP. The `PSTATE.F` bit in
    Secure-EL1/Secure-EL0 will not mask FIQs. The EL3 runtime firmware will
    call the handler registered by the SPD service for Secure-EL1
    interrupts. Secure-EL1 IHF should then handle all Secure-EL1 interrupt
    through the synchronous interrupt handling model described in 1. above.


##### 2.2.3.2 Secure payload IHF design w.r.t non-secure interrupts
1.  __CSS=0, TEL3=0__. If `PSTATE.I=0`, non-secure interrupts will be
    trigerred at one of the Secure-EL1 IRQ exception vectors . The Secure-EL1
    IHF should co-ordinate with the SPD service to transfer execution to the
    non-secure state where the interrupt should be handled e.g the SP could
    allocate a function identifier to issue a SMC64 or SMC32 to the SPD
    service which indicates that the SP execution has been pre-empted by a
    non-secure interrupt. If this function identifier is not known to the SPD
    service at compile time then the SP could provide it during the
    registration phase.

    If `PSTATE.I=1` then the non-secure interrupt will pend until execution
    resumes in the non-secure state.

2.  __CSS=0, TEL3=1__.  Non-secure interrupts are routed to EL3. They will not
    be visible to the SP. The `PSTATE.I` bit in Secure-EL1/Secure-EL0 will
    have not effect. The SPD service should register a non-secure interrupt
    handler which should save the SP state correctly and resume execution in
    the non-secure state where the interrupt will be handled. The Secure-EL1
    IHF does not need to take any action.

3.  __CSS=1, TEL3=0__.  Non-secure interrupts are handled in the FEL in
    non-secure state (EL1/EL2) and are not visible to the SP. This routing
    model does not affect the SP behavior.


A Secure Payload must also ensure that all Secure-EL1 interrupts are correctly
configured at the interrupt controller by the platform port of the EL3 runtime
firmware. It should configure any additional Secure-EL1 interrupts which the EL3
runtime firmware is not aware of through its platform port.

#### 2.2.3.3 Test secure payload behavior
The routing model for Secure-EL1 and non-secure interrupts chosen by the TSP is
described in Section 2.2.2. It is known to the TSPD service at build time.

The TSP implements an entrypoint (`tsp_fiq_entry()`) for handling Secure-EL1
interrupts taken in non-secure state and routed through the TSPD service
(synchronous handling model). It passes the reference to this entrypoint via
`tsp_vectors` to the TSPD service.

The TSP also replaces the default exception vector table referenced through the
`early_exceptions` variable, with a vector table capable of handling FIQ and IRQ
exceptions taken at the same (Secure-EL1) exception level. This table is
referenced through the `tsp_exceptions` variable and programmed into the
VBAR_EL1. It caters for the asynchronous handling model.

The TSP also programs the Secure Physical Timer in the ARM Generic Timer block
to raise a periodic interrupt (every half a second) for the purpose of testing
interrupt management across all the software components listed in 2.1


### 2.3 Interrupt handling
This section describes in detail the role of each software component (see
Section 2.1) in handling an interrupt of a particular type.

#### 2.3.1 EL3 runtime firmware
The EL3 runtime firmware populates the IRQ and FIQ exception vectors referenced
by the `runtime_exceptions` variable as follows.

1.  IRQ and FIQ exceptions taken from the current exception level with
    `SP_EL0` or `SP_EL3` are reported as irrecoverable error conditions. As
    mentioned earlier, EL3 runtime firmware always executes with the
    `PSTATE.I` and `PSTATE.F` bits set.

2.  The following text describes how the IRQ and FIQ exceptions taken from a
    lower exception level using AArch64 or AArch32 are handled.

When an interrupt is generated, the vector for each interrupt type is
responsible for:

1.  Saving the entire general purpose register context (x0-x30) immediately
    upon exception entry. The registers are saved in the per-cpu `cpu_context`
    data structure referenced by the `SP_EL3`register.

2.  Saving the `ELR_EL3`, `SP_EL0` and `SPSR_EL3` system registers in the
    per-cpu `cpu_context` data structure referenced by the `SP_EL3` register.

3.  Switching to the C runtime stack by restoring the `CTX_RUNTIME_SP` value
    from the per-cpu `cpu_context` data structure in `SP_EL0` and
    executing the `msr spsel, #0` instruction.

4.  Determining the type of interrupt. Secure-EL1 interrupts will be signalled
    at the FIQ vector. Non-secure interrupts will be signalled at the IRQ
    vector. The platform should implement the following API to determine the
    type of the pending interrupt.

        uint32_t plat_ic_get_interrupt_type(void);

    It should return either `INTR_TYPE_S_EL1` or `INTR_TYPE_NS`.

5.  Determining the handler for the type of interrupt that has been generated.
    The following API has been added for this purpose.

        interrupt_type_handler get_interrupt_type_handler(uint32_t interrupt_type);

    It returns the reference to the registered handler for this interrupt
    type. The `handler` is retrieved from the `intr_type_desc_t` structure as
    described in Section 2. `NULL` is returned if no handler has been
    registered for this type of interrupt. This scenario is reported as an
    irrecoverable error condition.

6.  Calling the registered handler function for the interrupt type generated.
    The firmware also determines the interrupt id if the IMF_READ_INTERRUPT_ID
    build time flag is set. The id is set to `INTR_ID_UNAVAILABLE` if the flag
    is not set. The id along with the current security state and a reference to
    the `cpu_context_t` structure for the current security state are passed to
    the handler function as its arguments.

    The handler function returns a reference to the per-cpu `cpu_context_t`
    structure for the target security state.

7.  Calling `el3_exit()` to return from EL3 into a lower exception level in
    the security state determined by the handler routine. The `el3_exit()`
    function is responsible for restoring the register context from the
    `cpu_context_t` data structure for the target security state.


#### 2.3.2 Secure payload dispatcher

##### 2.3.2.1 Interrupt entry
The SPD service begins handling an interrupt when the EL3 runtime firmware calls
the handler function for that type of interrupt. The SPD service is responsible
for the following:

1.  Validating the interrupt. This involves ensuring that the interrupt was
    generating according to the interrupt routing model specified by the SPD
    service during registration. It should use the interrupt id and the
    security state of the exception level (passed in the `flags` parameter of
    the handler) where the interrupt was taken from to determine this. If the
    interrupt is not recognised then the handler should treat it as an
    irrecoverable error condition.

    A SPD service can register a handler for Secure-EL1 and/or Non-secure
    interrupts. The following text describes further error scenarios keeping
    this in mind:

    1.  __SPD service has registered a handler for Non-secure interrupts__:
        When an interrupt is received by the handler, it could check its id
        to ensure it has been configured as a non-secure interrupt at the
        interrupt controller. A secure interrupt should never be handed to
        the non-secure interrupt handler. A non-secure interrupt should
        never be routed to EL3 when execution is in non-secure state. The
        handler could check the security state flag to ensure this.

    2.  __SPD service has registered a handler for Secure-EL1 interrupts__:
        When an interrupt is received by the handler, it could check its id
        to ensure it has been configured as a secure interrupt at the
        interrupt controller. A non-secure interrupt should never be handed
        to the secure interrupt handler. If the routing model chosen is such
        that Secure-EL1 interrupts are not routed to EL3 when execution is
        in non-secure state, then a Secure-EL1 interrupt generated in the
        secure state would be invalid. The handler could use the security
        state flag to check this.

    The SPD service should use the platform API:
    `plat_ic_get_interrupt_type()` to determine the type of interrupt for the
    specified id.

2.  Determining whether the security state of the exception level for handling
    the interrupt is the same as the security state of the exception level
    where the interrupt was generated. This depends upon the routing model and
    type of the interrupt. The SPD should use this information to determine if
    a context switch is required. The following two cases would require a
    context switch from secure to non-secure or vice-versa.

    1.  A Secure-EL1 interrupt taken from the non-secure state should be
        routed to the Secure Payload.

    2.  A non-secure interrupt taken from the secure state should be routed
        to the last known non-secure exception level.

    The SPD service must save the system register context of the current
    security state. It must then restore the system register context of the
    target security state. It should use the `cm_set_next_eret_context()` API
    to ensure that the next `cpu_context` to be restored is of the target
    security state.

    If the target state is secure then execution should be handed to the SP as
    per the synchronous interrupt handling model it implements. A Secure-EL1
    interrupt can be routed to EL3 while execution is in the SP. This implies
    that SP execution can be preempted while handling an interrupt by a
    another higher priority Secure-EL1 interrupt (or a EL3 interrupt in the
    future). The SPD service should manage secure interrupt priorities before
    handing control to the SP to prevent this type of preemption which can
    leave the system in an inconsistent state.

3.  Setting the return value of the handler to the per-cpu `cpu_context` if
    the interrupt has been successfully validated and ready to be handled at a
    lower exception level.

The routing model allows non-secure interrupts to be taken to Secure-EL1 when in
secure state. The SPD service and the SP should implement a mechanism for
routing these interrupts to the last known exception level in the non-secure
state. The former should save the SP context, restore the non-secure context and
arrange for entry into the non-secure state so that the interrupt can be
handled.

##### 2.3.2.2 Interrupt exit
When the Secure Payload has finished handling a Secure-EL1 interrupt, it could
return control back to the SPD service through a SMC32 or SMC64. The SPD service
should handle this secure monitor call so that execution resumes in the
exception level and the security state from where the Secure-EL1 interrupt was
originally taken.

##### 2.3.2.1 Test secure payload dispatcher behavior
The example TSPD service registers a handler for Secure-EL1 interrupts taken
from the non-secure state. Its handler `tspd_secure_el1_interrupt_handler()`
takes the following actions upon being invoked.

1.  It uses the `id` parameter to query the interrupt controller to ensure
    that the interrupt is a Secure-EL1 interrupt. It asserts if this is not
    the case.

2.  It uses the security state provided in the `flags` parameter to ensure
    that the secure interrupt originated from the non-secure state. It asserts
    if this is not the case.

3.  It saves the system register context for the non-secure state by calling
    `cm_el1_sysregs_context_save(NON_SECURE);`.

4.  It sets the `ELR_EL3` system register to `tsp_fiq_entry` and sets the
    `SPSR_EL3.DAIF` bits in the secure CPU context. It sets `x0` to
    `TSP_HANDLE_FIQ_AND_RETURN`. If the TSP was in the middle of handling a
    standard SMC, then the `ELR_EL3` and `SPSR_EL3` registers in the secure CPU
    context are saved first.

5.  It restores the system register context for the secure state by calling
    `cm_el1_sysregs_context_restore(SECURE);`.

6.  It ensures that the secure CPU context is used to program the next
    exception return from EL3 by calling `cm_set_next_eret_context(SECURE);`.

7.  It returns the per-cpu `cpu_context` to indicate that the interrupt can
    now be handled by the SP. `x1` is written with the value of `elr_el3`
    register for the non-secure state. This information is used by the SP for
    debugging purposes.

The figure below describes how the interrupt handling is implemented by the TSPD
when a Secure-EL1 interrupt is generated when execution is in the non-secure
state.

![Image 1](diagrams/sec-int-handling.png?raw=true)

The TSP issues an SMC with `TSP_HANDLED_S_EL1_FIQ` as the function identifier to
signal completion of interrupt handling.

The TSP issues an SMC with `TSP_PREEMPTED` as the function identifier to signal
generation of a non-secure interrupt in Secure-EL1.

The TSPD service takes the following actions in `tspd_smc_handler()` function
upon receiving an SMC with `TSP_HANDLED_S_EL1_FIQ` and `TSP_PREEMPTED` as the
function identifiers:

1.  It ensures that the call originated from the secure state otherwise
    execution returns to the non-secure state with `SMC_UNK` in `x0`.

2.  If the function identifier is `TSP_HANDLED_S_EL1_FIQ`, it restores the
    saved `ELR_EL3` and `SPSR_EL3` system registers back to the secure CPU
    context (see step 4 above) in case the TSP had been preempted by a non
    secure interrupt earlier.  It does not save the secure context since the
    TSP is expected to preserve it (see Section 2.2.2.1)

3.  If the function identifier is `TSP_PREEMPTED`, it saves the system
    register context for the secure state by calling
    `cm_el1_sysregs_context_save(SECURE)`.

4.  It restores the system register context for the non-secure state by
    calling `cm_el1_sysregs_context_restore(NON_SECURE)`. It sets `x0` to
    `SMC_PREEMPTED` if the incoming function identifier is
    `TSP_PREEMPTED`. The Normal World is expected to resume the TSP after the
    non-secure interrupt handling by issuing an SMC with `TSP_FID_RESUME` as
    the function identifier.

5.  It ensures that the non-secure CPU context is used to program the next
    exception return from EL3 by calling
    `cm_set_next_eret_context(NON_SECURE)`.

6.  `tspd_smc_handler()` returns a reference to the non-secure `cpu_context`
    as the return value.

As mentioned in 4. above, if a non-secure interrupt preempts the TSP execution
then the non-secure software issues an SMC with `TSP_FID_RESUME` as the function
identifier to resume TSP execution. The TSPD service takes the following actions
in `tspd_smc_handler()` function upon receiving this SMC:

1.  It ensures that the call originated from the non secure state. An
    assertion is raised otherwise.

2.  Checks whether the TSP needs a resume i.e check if it was preempted. It
    then saves the system register context for the secure state by calling
    `cm_el1_sysregs_context_save(NON_SECURE)`.

3.  Restores the secure context by calling
    `cm_el1_sysregs_context_restore(SECURE)`

4.  It ensures that the secure CPU context is used to program the next
    exception return from EL3 by calling `cm_set_next_eret_context(SECURE)`.

5.  `tspd_smc_handler()` returns a reference to the secure `cpu_context` as the
    return value.

The figure below describes how the TSP/TSPD handle a non-secure interrupt when
it is generated during execution in the TSP with `PSTATE.I` = 0.

![Image 2](diagrams/non-sec-int-handling.png?raw=true)


#### 2.3.3 Secure payload
The SP should implement one or both of the synchronous and asynchronous
interrupt handling models depending upon the interrupt routing model it has
chosen (as described in 2.2.3).

In the synchronous model, it should begin handling a Secure-EL1 interrupt after
receiving control from the SPD service at an entrypoint agreed upon during build
time or during the registration phase. Before handling the interrupt, the SP
should save any Secure-EL1 system register context which is needed for resuming
normal execution in the SP later e.g. `SPSR_EL1, `ELR_EL1`. After handling the
interrupt, the SP could return control back to the exception level and security
state where the interrupt was originally taken from. The SP should use an SMC32
or SMC64 to ask the SPD service to do this.

In the asynchronous model, the Secure Payload is responsible for handling
non-secure and Secure-EL1 interrupts at the IRQ and FIQ vectors in its exception
vector table when `PSTATE.I` and `PSTATE.F` bits are 0. As described earlier,
when a non-secure interrupt is generated, the SP should coordinate with the SPD
service to pass control back to the non-secure state in the last known exception
level. This will allow the non-secure interrupt to be handled in the non-secure
state.

##### 2.3.3.1 Test secure payload behavior
The TSPD hands control of a Secure-EL1 interrupt to the TSP at the
`tsp_fiq_entry()`.  The TSP handles the interrupt while ensuring that the
handover agreement described in Section 2.2.2.1 is maintained. It updates some
statistics by calling `tsp_update_sync_fiq_stats()`. It then calls
`tsp_fiq_handler()` which.

1.  Checks whether the interrupt is the secure physical timer interrupt. It
    uses the platform API `plat_ic_get_pending_interrupt_id()` to get the
    interrupt number.

2.   Handles the interrupt by acknowledging it using the
    `plat_ic_acknowledge_interrupt()` platform API, calling
    `tsp_generic_timer_handler()` to reprogram the secure physical generic
    timer and calling the `plat_ic_end_of_interrupt()` platform API to signal
    end of interrupt processing.

The TSP passes control back to the TSPD by issuing an SMC64 with
`TSP_HANDLED_S_EL1_FIQ` as the function identifier.

The TSP handles interrupts under the asynchronous model as follows.

1.  Secure-EL1 interrupts are handled by calling the `tsp_fiq_handler()`
    function. The function has been described above.

2.  Non-secure interrupts are handled by issuing an SMC64 with `TSP_PREEMPTED`
    as the function identifier. Execution resumes at the instruction that
    follows this SMC instruction when the TSPD hands control to the TSP in
    response to an SMC with `TSP_FID_RESUME` as the function identifier from
    the non-secure state (see section 2.3.2.1).

- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2014, ARM Limited and Contributors. All rights reserved._

[Porting Guide]:             ./porting-guide.md
