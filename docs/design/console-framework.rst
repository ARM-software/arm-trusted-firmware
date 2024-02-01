Console Framework
=================

The TF-A console framework is used to register consoles for different boot states
so that user's output can be displayed on physical consoles throughout the different
boot stages. The framework also supports debug mode for general debugging purposes.

The console framework supports a number of different UARTs, it is highly likely
that the driver of the UART that is needed is already implemented. If not, a driver
will need to be written for the new UART in TF-A. Current supported UARTs are:

* Amlogic Meson
* Arm PL011
* Cadence CDNS
* Coreboot CBMEM
* Marvell A3700
* NXP
    * i.MX LPUART
    * i.MX UART
    * Linflex
* Nvidia SPE
* Qualcomm UARTDM
* Renesas RCAR
* STMicroelectronics STM32
* Texas Instruments 16550

    .. note::
        The supported UART list is non-exhaustive. Check if the UART driver has
        already been written before writing a new one.

::

    Console scopes and flags

    Scope   : Flag
    BOOT    : CONSOLE_FLAG_BOOT
    RUNTIME : CONSOLE_FLAG_RUNTIME
    CRASH   : CONSOLE_FLAG_CRASH

The console framework supports multiple consoles. Multiple instances of a UART
can be registered at any given moment. Any registered console can have a single
scope or multiple scopes. In single scope for example, setting three different
consoles with each having BOOT, RUNTIME, and CRASH states respectively, the boot
console will display only boot logs, the runtime console will display only the
runtime output, while the crash console will be used to print the crash log in the
event of a crash. Similarly, a console with all three scopes will display any and
all output destined for BOOT, RUNTIME, or CRASH consoles.

These multiple scopes can be useful in many ways, for example:

* Having different consoles for Boot and Runtime messages
* Having a single console for both Runtime and Boot messages
* Having no runtime console at all and just having a single Boot console.
* Having a separate console for crash reporting when debugging.

.. Registering a console:

Registering a console
---------------------
To register a console in TF-A check if the hardware (UART) that is going to be used
is already defined, if not we will need to define it, for example, the **PL011**
UART driver API is defined in ``include/drivers/arm/pl011.h``.

A skeleton console driver (assembly) is provided in TF-A ``drivers/console/aarch64/
skeleton_console.S``, this skeleton sets the rules for writing a new console_driver.
Have a look at ``drivers/arm/pl011/aarch64/pl011_console.S`` for an actual
implementation using this skeleton.

Function : console_xxx_register
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : console_t *, ...
    Return   : int

This ASM Function is used to initialize and register a console. The caller needs
to pass an empty ``console_t`` struct which *MUST* be allocated in persistent
memory (e.g. a global or static local variable, *NOT* on the stack).

This function takes a ``console_t`` struct placed in x0 and additional
arguments placed in x1 - x7. It returns x0 with either a 0 on failure or 1
on success.

See ``console_pl011_register`` ASM function for an implementation of this
function.

    .. note::
        The ``xxx`` in the function name is replaced with the console driver
        name, for example, ``console_xxx_register`` becomes
        ``console_pl011_register`` in the driver for pl011.

Function : console_xxx_putc
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int, console_t *
    Return   : int

This ASM function is used to send a character to the UART's Transmit FIFO. It takes
two arguments, a character as int stored in w0, and the ``console_t`` struct pointer
stored in x1. It returns w0 with either the character on successs or a negative
value on error. In a crash context this function must only clobber x0 - x2, x16 - x17.

See ``console_pl011_putc`` ASM function for an implementation.

    .. note::
        Avoid the direct use of this function for printing to the console, instead use
        the ``debug.h`` print macros, such as: VERBOSE(...), INFO(...), WARN(...),
        NOTICE(...) and ERROR(...).

Function : console_xxx_getc
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : console_t *
    Return   : int

This ASM function is used to read a character from the receive FIFO. It takes a pointer
to the console_struct as an argument and returns a character on success or a negative
value below -2 on failure. This function is dependent on the ``ENABLE_CONSOLE_GETC`` flag,
which is optional and is left to the platform because there may be security implications.

See ``console_pl011_getc`` ASM function for an implementation.

Function : console_xxx_flush
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : console_t *
    Return   : void

This ASM function flushes any characters, that are still in the Transmit FIFO but
haven't been printed yet to the console. It takes a pointer to the console_struct
but doesn't return any value. In a crash context this function must only clobber
x0 - x5, x16 - x17.

See ``console_pl011_flush`` ASM function for an implementation.

Macro : finish_console_register xxx putc=1 getc=ENABLE_CONSOLE_GETC flush=1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Callbacks
    xxx      : name of the console driver
    putc     : 0 for off, 1 to turn on putc function
    getc     : 0 for off, ENABLE_CONSOLE_GETC to turn on the getc function
    flush    : 0 for off, 1 to turn on flush function

This assembly macro function is called by the ``console_xxx_register`` to
encapsulate the common setup that has to be done at the end of a console
driver's register function. It takes ``putc``, ``getc`` and ``flush`` macro
arguments. It will register all of the driver's callbacks in the ``console_t``
struct and initialize the ``flags`` field (by default consoles are enabled for
"boot" and "crash" states, this can be changed after registration using the
``console_set_scope`` function). This macro ends with a tail call that will
include return to the caller.

This macro requires ``console_t`` pointer in x0 and a valid return address in x30.

See ``include/arch/aarch64/console_macros.S``.

Registering a console using C
-----------------------------

A console can be implemented in pure C, which is much easier than using assembly.
Currently there is no C template for implementing a console driver in C but it can
easily be implemented using existing examples. See ``drivers/arm/dcc/dcc_console.c``
for an implementation of a console driver in C.

The assembly functions in `Registering a console`_ section can be written in C when
implementing a console driver using C.

    .. note::
        A crash callback needs to be able to run without a stack. If crash mode
        support is required then the console driver will need to be written in
        Assembly (only the putc and flush functions are needed in a crash
        context).

Multi Console API
-----------------

TF-A uses the multi-console API to manage the registered console instances and the
characters print queue. This can be found in ``drivers/console/multi_console.c``.

The multi-console API stores all registered consoles in a struct list ``console_list``.
Consoles can be removed from the console_list if no longer needed.

Consoles are registered with BOOT and CRASH scopes by default. These scopes can be
changed after registration using ``console_set_scope`` function, as per the platform
requirement.

This API also helps print characters to the specified consoles, characters can also
be retrieved from the receive FIFO (this implementation is disabled by default but can
be enabled if there is a need for it). The API can also help flush the transmit FIFO
to get rid of any lingering characters from the queue when switching from secure world
to the non-secure world.

The following functions are defined in the multi_console API.

Function : console_register()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : console_t*
    Return   : int

This function adds a console to the ``console_list`` declared in
``include/drivers/console.h`` and makes sure that there is only one instance
of a specific console in this list. This function is called by the
``finish_console_register`` asm macro function, at the end of the console
registration process.

This function always return 1. If the console is already present in the
``console_list`` it will return immediately with a value of 1, otherwise
it will add the console to the ``console_list`` and then return 1.

    .. note::
        The ``console_list`` is a list of type ``console_t``, it is an **extern**
        variable declared in ``include/drivers/console.h``.

Function : console_unregister()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : console_t*
    Return   : console_t* or NULL

This function removes a console from the ``console_list``. It will return the
removed console on success or a ``NULL`` character upon failure.

Function : console_set_scope()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : console_t*, int
    Return   : void

This function is used to set the scope of the registered console. A console
can be registered with upto three states (called the scope). These states are

* BOOT      - set using the flag ``CONSOLE_FLAG_BOOT``
* RUNTIME   - set using the flag ``CONSOLE_FLAG_RUNTIME``
* CRASH     - set using the flag ``CONSOLE_FLAG_CRASH``

It takes a pointer to the console and an int value (which is provided as the
FLAG value) as its arguments. This function does not return anything.

Function : console_switch_state()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Return   : void

This function sets the console state (scope) for printing, i.e, TF-A will
start sending all logs (INFO, WARNING, ERROR, NOTICE, VERBOSE) to the consoles
that are registered with this new state (scope). For example, calling
``console_switch_state(CONSOLE_FLAG_RUNTIME)``, TF-A will start sending all log
messages to all consoles marked with the RUNTIME flag. BOOT is the default
console state.

This function takes a console state as the function's only argument. This function
does not return a value.

Function : console_putc()
~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Return   : int

Invoking this function sends a character to the ``console->putc`` (struct
member) function of all consoles registered for the current scope, for example,
BOOT logs will only be printed on consoles set with a BOOT scope. In the PL011
implementation ``console->putc`` call points to the ``console_pl011_putc()``
function.

This function takes the int value of a character as an argument and returns the
int value of the character back on success or a negative int value on error.

    .. note::
        Do not use this function in TF-A release builds, instead use the log
        prefixes, for example, ``INFO("Print information here.")`` to print
        messages on the active console.

Function : console_getc()
~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : int

This function is used to fetch a character from the receive FIFO that has
not been printed to the console yet. This function is disabled by default for
security reasons but can be enabled using the ``ENABLE_CONSOLE_GETC`` macro
if there is a need for it.

This function doesn't take any argument but returns a character as an int.

Function : console_flush()
~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Argument : void
    Return   : void

This function flushes all the characters pending in the transmit FIFO of the
active UART thus removing them from the print queue.

This function has no arguments and do not return a value.

Function : putchar()
~~~~~~~~~~~~~~~~~~~~

::

    Argument : int
    Return   : int

This function overrides the weak implementation of the putchar library. It is
used to send a character to the ``console_putc()`` function to be printed to
the active console.

This function will either return the character on success or an **EOF** character
otherwise.

--------------

*Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.*