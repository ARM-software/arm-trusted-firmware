Coding Guidelines
=================

This document provides some additional guidelines to consider when writing
|TF-A| code. These are not intended to be strictly-enforced rules like the
contents of the :ref:`Coding Style`.

Automatic Editor Configuration
------------------------------

Many of the rules given below (such as indentation size, use of tabs, and
newlines) can be set automatically using the `EditorConfig`_ configuration file
in the root of the repository: ``.editorconfig``. With a supported editor, the
rules set out in this file can be automatically applied when you are editing
files in the |TF-A| repository.

Several editors include built-in support for EditorConfig files, and many others
support its functionality through plugins.

Use of the EditorConfig file is suggested but is not required.

.. _automatic-compliance-checking:

Automatic Compliance Checking
-----------------------------

To assist with coding style compliance, the project Makefile contains two
targets which both utilise the `checkpatch.pl` script that ships with the Linux
source tree. The project also defines certain *checkpatch* options in the
``.checkpatch.conf`` file in the top-level directory.

.. note::
   Checkpatch errors will gate upstream merging of pull requests.
   Checkpatch warnings will not gate merging but should be reviewed and fixed if
   possible.

To check the entire source tree, you must first download copies of
``checkpatch.pl``, ``spelling.txt`` and ``const_structs.checkpatch`` available
in the `Linux master tree`_ *scripts* directory, then set the ``CHECKPATCH``
environment variable to point to ``checkpatch.pl`` (with the other 2 files in
the same directory) and build the `checkcodebase` target:

.. code:: shell

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkcodebase

To just check the style on the files that differ between your local branch and
the remote master, use:

.. code:: shell

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkpatch

If you wish to check your patch against something other than the remote master,
set the ``BASE_COMMIT`` variable to your desired branch. By default,
``BASE_COMMIT`` is set to ``origin/master``.

Ignored Checkpatch Warnings
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Some checkpatch warnings in the TF codebase are deliberately ignored. These
include:

- ``**WARNING: line over 80 characters**``: Although the codebase should
  generally conform to the 80 character limit this is overly restrictive in some
  cases.

- ``**WARNING: Use of volatile is usually wrong``: see
  `Why the “volatile” type class should not be used`_ . Although this document
  contains some very useful information, there are several legimate uses of the
  volatile keyword within the TF codebase.

Performance considerations
--------------------------

Avoid printf and use logging macros
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``debug.h`` provides logging macros (for example, ``WARN`` and ``ERROR``)
which wrap ``tf_log`` and which allow the logging call to be compiled-out
depending on the ``make`` command. Use these macros to avoid print statements
being compiled unconditionally into the binary.

Each logging macro has a numerical log level:

.. code:: c

  #define LOG_LEVEL_NONE    0
  #define LOG_LEVEL_ERROR   10
  #define LOG_LEVEL_NOTICE  20
  #define LOG_LEVEL_WARNING 30
  #define LOG_LEVEL_INFO    40
  #define LOG_LEVEL_VERBOSE 50

By default, all logging statements with a log level ``<= LOG_LEVEL_INFO`` will
be compiled into debug builds and all statements with a log level
``<= LOG_LEVEL_NOTICE`` will be compiled into release builds. This can be
overridden from the command line or by the platform makefile (although it may be
necessary to clean the build directory first).

For example, to enable ``VERBOSE`` logging on FVP:

.. code:: shell

  make PLAT=fvp LOG_LEVEL=50 all

Use const data where possible
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For example, the following code:

.. code:: c

  struct my_struct {
          int arg1;
          int arg2;
  };

  void init(struct my_struct *ptr);

  void main(void)
  {
          struct my_struct x;
          x.arg1 = 1;
          x.arg2 = 2;
          init(&x);
  }

is better written as:

.. code:: c

  struct my_struct {
          int arg1;
          int arg2;
  };

  void init(const struct my_struct *ptr);

  void main(void)
  {
          const struct my_struct x = { 1, 2 };
          init(&x);
  }

This allows the linker to put the data in a read-only data section instead of a
writeable data section, which may result in a smaller and faster binary. Note
that this may require dependent functions (``init()`` in the above example) to
have ``const`` arguments, assuming they don't need to modify the data.

Libc functions that are banned or to be used with caution
---------------------------------------------------------

Below is a list of functions that present security risks and either must not be
used (Banned) or are discouraged from use and must be used with care (Caution).

+------------------------+-----------+--------------------------------------+
|    libc function       | Status    | Comments                             |
+========================+===========+======================================+
| ``strcpy, wcscpy``,    | Banned    | use strlcpy instead                  |
| ``strncpy``            |           |                                      |
+------------------------+-----------+--------------------------------------+
| ``strcat, wcscat``,    | Banned    | use strlcat instead                  |
| ``strncat``            |           |                                      |
+------------------------+-----------+--------------------------------------+
| ``sprintf, vsprintf``  | Banned    | use snprintf, vsnprintf              |
|                        |           | instead                              |
+------------------------+-----------+--------------------------------------+
| ``snprintf``           | Caution   | ensure result fits in buffer         |
|                        |           | i.e : snprintf(buf,size...) < size   |
+------------------------+-----------+--------------------------------------+
| ``vsnprintf``          | Caution   | inspect va_list match types          |
|                        |           | specified in format string           |
+------------------------+-----------+--------------------------------------+
| ``strtok``             | Banned    | use strtok_r or strsep instead       |
+------------------------+-----------+--------------------------------------+
| ``strtok_r, strsep``   | Caution   | inspect for terminated input buffer  |
+------------------------+-----------+--------------------------------------+
| ``ato*``               | Banned    | use equivalent strto* functions      |
+------------------------+-----------+--------------------------------------+
| ``*toa``               | Banned    | Use snprintf instead                 |
+------------------------+-----------+--------------------------------------+

The `libc` component in the codebase will not add support for the banned APIs.

Error handling and robustness
-----------------------------

Using CASSERT to check for compile time data errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Where possible, use the ``CASSERT`` macro to check the validity of data known at
compile time instead of checking validity at runtime, to avoid unnecessary
runtime code.

For example, this can be used to check that the assembler's and compiler's views
of the size of an array is the same.

.. code:: c

  #include <cassert.h>

  define MY_STRUCT_SIZE 8 /* Used by assembler source files */

  struct my_struct {
      uint32_t arg1;
      uint32_t arg2;
  };

  CASSERT(MY_STRUCT_SIZE == sizeof(struct my_struct), assert_my_struct_size_mismatch);


If ``MY_STRUCT_SIZE`` in the above example were wrong then the compiler would
emit an error like this:

::

  my_struct.h:10:1: error: size of array ‘assert_my_struct_size_mismatch’ is negative


Using assert() to check for programming errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In general, each secure world TF image (BL1, BL2, BL31 and BL32) should be
treated as a tightly integrated package; the image builder should be aware of
and responsible for all functionality within the image, even if code within that
image is provided by multiple entities. This allows us to be more aggressive in
interpreting invalid state or bad function arguments as programming errors using
``assert()``, including arguments passed across platform porting interfaces.
This is in contrast to code in a Linux environment, which is less tightly
integrated and may attempt to be more defensive by passing the error back up the
call stack.

Where possible, badly written TF code should fail early using ``assert()``. This
helps reduce the amount of untested conditional code. By default these
statements are not compiled into release builds, although this can be overridden
using the ``ENABLE_ASSERTIONS`` build flag.

Examples:

- Bad argument supplied to library function
- Bad argument provided by platform porting function
- Internal secure world image state is inconsistent


Handling integration errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Each secure world image may be provided by a different entity (for example, a
Trusted Boot vendor may provide the BL2 image, a TEE vendor may provide the BL32
image and the OEM/SoC vendor may provide the other images).

An image may contain bugs that are only visible when the images are integrated.
The system integrator may not even have access to the debug variants of all the
images in order to check if asserts are firing. For example, the release variant
of BL1 may have already been burnt into the SoC. Therefore, TF code that detects
an integration error should _not_ consider this a programming error, and should
always take action, even in release builds.

If an integration error is considered non-critical it should be treated as a
recoverable error. If the error is considered critical it should be treated as
an unexpected unrecoverable error.

Handling recoverable errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The secure world **must not** crash when supplied with bad data from an external
source. For example, data from the normal world or a hardware device. Similarly,
the secure world **must not** crash if it detects a non-critical problem within
itself or the system. It must make every effort to recover from the problem by
emitting a ``WARN`` message, performing any necessary error handling and
continuing.

Examples:

- Secure world receives SMC from normal world with bad arguments.
- Secure world receives SMC from normal world at an unexpected time.
- BL31 receives SMC from BL32 with bad arguments.
- BL31 receives SMC from BL32 at unexpected time.
- Secure world receives recoverable error from hardware device. Retrying the
  operation may help here.
- Non-critical secure world service is not functioning correctly.
- BL31 SPD discovers minor configuration problem with corresponding SP.

Handling unrecoverable errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In some cases it may not be possible for the secure world to recover from an
error. This situation should be handled in one of the following ways:

1. If the unrecoverable error is unexpected then emit an ``ERROR`` message and
   call ``panic()``. This will end up calling the platform-specific function
   ``plat_panic_handler()``.
2. If the unrecoverable error is expected to occur in certain circumstances,
   then emit an ``ERROR`` message and call the platform-specific function
   ``plat_error_handler()``.

Cases 1 and 2 are subtly different. A platform may implement
``plat_panic_handler`` and ``plat_error_handler`` in the same way (for example,
by waiting for a secure watchdog to time-out or by invoking an interface on the
platform's power controller to reset the platform). However,
``plat_error_handler`` may take additional action for some errors (for example,
it may set a flag so the platform resets into a different mode). Also,
``plat_panic_handler()`` may implement additional debug functionality (for
example, invoking a hardware breakpoint).

Examples of unexpected unrecoverable errors:

- BL32 receives an unexpected SMC response from BL31 that it is unable to
  recover from.
- BL31 Trusted OS SPD code discovers that BL2 has not loaded the corresponding
  Trusted OS, which is critical for platform operation.
- Secure world discovers that a critical hardware device is an unexpected and
  unrecoverable state.
- Secure world receives an unexpected and unrecoverable error from a critical
  hardware device.
- Secure world discovers that it is running on unsupported hardware.

Examples of expected unrecoverable errors:

- BL1/BL2 fails to load the next image due to missing/corrupt firmware on disk.
- BL1/BL2 fails to authenticate the next image due to an invalid certificate.
- Secure world continuously receives recoverable errors from a hardware device
  but is unable to proceed without a valid response.

Handling critical unresponsiveness
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the secure world is waiting for a response from an external source (for
example, the normal world or a hardware device) which is critical for continued
operation, it must not wait indefinitely. It must have a mechanism (for example,
a secure watchdog) for resetting itself and/or the external source to prevent
the system from executing in this state indefinitely.

Examples:

- BL1 is waiting for the normal world to raise an SMC to proceed to the next
  stage of the secure firmware update process.
- A Trusted OS is waiting for a response from a proxy in the normal world that
  is critical for continued operation.
- Secure world is waiting for a hardware response that is critical for continued
  operation.

Use of built-in *C* and *libc* data types
-----------------------------------------

The |TF-A| codebase should be kept as portable as possible, especially since
both 64-bit and 32-bit platforms are supported. To help with this, the following
data type usage guidelines should be followed:

- Where possible, use the built-in *C* data types for variable storage (for
  example, ``char``, ``int``, ``long long``, etc) instead of the standard *C99*
  types. Most code is typically only concerned with the minimum size of the
  data stored, which the built-in *C* types guarantee.

- Avoid using the exact-size standard *C99* types in general (for example,
  ``uint16_t``, ``uint32_t``, ``uint64_t``, etc) since they can prevent the
  compiler from making optimizations. There are legitimate uses for them,
  for example to represent data of a known structure. When using them in struct
  definitions, consider how padding in the struct will work across architectures.
  For example, extra padding may be introduced in |AArch32| systems if a struct
  member crosses a 32-bit boundary.

- Use ``int`` as the default integer type - it's likely to be the fastest on all
  systems. Also this can be assumed to be 32-bit as a consequence of the
  `Procedure Call Standard for the Arm Architecture`_ and the `Procedure Call
  Standard for the Arm 64-bit Architecture`_ .

- Avoid use of ``short`` as this may end up being slower than ``int`` in some
  systems. If a variable must be exactly 16-bit, use ``int16_t`` or
  ``uint16_t``.

- Avoid use of ``long``. This is guaranteed to be at least 32-bit but, given
  that `int` is 32-bit on Arm platforms, there is no use for it. For integers of
  at least 64-bit, use ``long long``.

- Use ``char`` for storing text. Use ``uint8_t`` for storing other 8-bit data.

- Use ``unsigned`` for integers that can never be negative (counts,
  indices, sizes, etc). TF intends to comply with MISRA "essential type" coding
  rules (10.X), where signed and unsigned types are considered different
  essential types. Choosing the correct type will aid this. MISRA static
  analysers will pick up any implicit signed/unsigned conversions that may lead
  to unexpected behaviour.

- For pointer types:

  - If an argument in a function declaration is pointing to a known type then
    simply use a pointer to that type (for example: ``struct my_struct *``).

  - If a variable (including an argument in a function declaration) is pointing
    to a general, memory-mapped address, an array of pointers or another
    structure that is likely to require pointer arithmetic then use
    ``uintptr_t``. This will reduce the amount of casting required in the code.
    Avoid using ``unsigned long`` or ``unsigned long long`` for this purpose; it
    may work but is less portable.

  - For other pointer arguments in a function declaration, use ``void *``. This
    includes pointers to types that are abstracted away from the known API and
    pointers to arbitrary data. This allows the calling function to pass a
    pointer argument to the function without any explicit casting (the cast to
    ``void *`` is implicit). The function implementation can then do the
    appropriate casting to a specific type.

  - Avoid pointer arithmetic generally (as this violates MISRA C 2012 rule
    18.4) and especially on void pointers (as this is only supported via
    language extensions and is considered non-standard). In TF-A, setting the
    ``W`` build flag to ``W=3`` enables the *-Wpointer-arith* compiler flag and
    this will emit warnings where pointer arithmetic is used.

  - Use ``ptrdiff_t`` to compare the difference between 2 pointers.

- Use ``size_t`` when storing the ``sizeof()`` something.

- Use ``ssize_t`` when returning the ``sizeof()`` something from a function that
  can also return an error code; the signed type allows for a negative return
  code in case of error. This practice should be used sparingly.

- Use ``u_register_t`` when it's important to store the contents of a register
  in its native size (32-bit in |AArch32| and 64-bit in |AArch64|). This is not a
  standard *C99* type but is widely available in libc implementations,
  including the FreeBSD version included with the TF codebase. Where possible,
  cast the variable to a more appropriate type before interpreting the data. For
  example, the following struct in ``ep_info.h`` could use this type to minimize
  the storage required for the set of registers:

.. code:: c

    typedef struct aapcs64_params {
            u_register_t arg0;
            u_register_t arg1;
            u_register_t arg2;
            u_register_t arg3;
            u_register_t arg4;
            u_register_t arg5;
            u_register_t arg6;
            u_register_t arg7;
    } aapcs64_params_t;

If some code wants to operate on ``arg0`` and knows that it represents a 32-bit
unsigned integer on all systems, cast it to ``unsigned int``.

These guidelines should be updated if additional types are needed.

Favor C language over assembly language
---------------------------------------

Generally, prefer code written in C over assembly. Assembly code is less
portable, harder to understand, maintain and audit security wise. Also, static
analysis tools generally don't analyze assembly code.

There are, however, legitimate uses of assembly language. These include:

  - Early boot code executed before the C runtime environment is setup.

  - Exception handling code.

  - Low-level code where the exact sequence of instructions executed on the CPU
    matters, such as CPU reset sequences.

  - Low-level code where specific system-level instructions must be used, such
    as cache maintenance operations.

--------------

*Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.*

.. _`Linux master tree`: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/
.. _`Procedure Call Standard for the Arm Architecture`: https://developer.arm.com/docs/ihi0042/latest/
.. _`Procedure Call Standard for the Arm 64-bit Architecture`: https://developer.arm.com/docs/ihi0055/latest/
.. _`EditorConfig`: http://editorconfig.org/
.. _`Why the “volatile” type class should not be used`: https://www.kernel.org/doc/html/latest/process/volatile-considered-harmful.html
.. _`MISRA C:2012 Guidelines`: https://www.misra.org.uk/Activities/MISRAC/tabid/160/Default.aspx
.. _`a spreadsheet`: https://developer.trustedfirmware.org/file/download/lamajxif3w7c4mpjeoo5/PHID-FILE-fp7c7acszn6vliqomyhn/MISRA-and-TF-Analysis-v1.3.ods
