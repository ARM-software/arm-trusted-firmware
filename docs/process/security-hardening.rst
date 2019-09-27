Secure Development Guidelines
=============================

This page contains guidance on what to check for additional security measures,
including build options that can be modified to improve security or catch issues
early in development.

Security considerations
-----------------------

Part of the security of a platform is handling errors correctly, as described in
the previous section. There are several other security considerations covered in
this section.

Do not leak secrets to the normal world
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The secure world **must not** leak secrets to the normal world, for example in
response to an SMC.

Handling Denial of Service attacks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The secure world **should never** crash or become unusable due to receiving too
many normal world requests (a *Denial of Service* or *DoS* attack). It should
have a mechanism for throttling or ignoring normal world requests.

Preventing Secure-world timing information leakage via PMU counters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Secure world needs to implement some defenses to prevent the Non-secure
world from making it leak timing information. In general, higher privilege
levels must defend from those below when the PMU is treated as an attack
vector.

Refer to the :ref:`Performance Monitoring Unit` guide for detailed information
on the PMU registers.

Timing leakage attacks from the Non-secure world
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since the Non-secure world has access to the ``PMCR`` register, it can
configure the PMU to increment counters at any exception level and in both
Secure and Non-secure state. Thus, it attempts to leak timing information from
the Secure world.

Shown below is an example of such a configuration:

-  ``PMEVTYPER0_EL0`` and ``PMCCFILTR_EL0``:

   -  Set ``P`` to ``0``.
   -  Set ``NSK`` to ``1``.
   -  Set ``M`` to ``0``.
   -  Set ``NSH`` to ``0``.
   -  Set ``SH`` to ``1``.

-  ``PMCNTENSET_EL0``:

   -  Set ``P[0]`` to ``1``.
   -  Set ``C`` to ``1``.

-  ``PMCR_EL0``:

   -  Set ``DP`` to ``0``.
   -  Set ``E`` to ``1``.

This configuration instructs ``PMEVCNTR0_EL0`` and ``PMCCNTR_EL0`` to increment
at Secure EL1, Secure EL2 (if implemented) and EL3.

Since the Non-secure world has fine-grained control over where (at which
exception levels) it instructs counters to increment, obtaining event counts
would allow it to carry out side-channel timing attacks against the Secure
world. Examples include Spectre, Meltdown, as well as extracting secrets from
cryptographic algorithms with data-dependent variations in their execution
time.

Secure world mitigation strategies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``MDCR_EL3`` register allows EL3 to configure the PMU (among other things).
The `Arm ARM`_ details all of the bit fields in this register, but for the PMU
there are two bits which determine the permissions of the counters:

-  ``SPME`` for the programmable counters.
-  ``SCCD`` for the cycle counter.

Depending on the implemented features, the Secure world can prohibit counting
in AArch64 state via the following:

-  ARMv8.2-Debug not implemented:

   -  Prohibit general event counters and the cycle counter:
      ``MDCR_EL3.SPME == 0 && PMCR_EL0.DP == 1 && !ExternalSecureNoninvasiveDebugEnabled()``.

      -  ``MDCR_EL3.SPME`` resets to ``0``, so by default general events should
         not be counted in the Secure world.
      -  The ``PMCR_EL0.DP`` bit therefore needs to be set to ``1`` when EL3 is
         entered and ``PMCR_EL0`` needs to be saved and restored in EL3.
      -  ``ExternalSecureNoninvasiveDebugEnabled()`` is an authentication
         interface which is implementation-defined unless ARMv8.4-Debug is
         implemented. The `Arm ARM`_ has detailed information on this topic.

   -  The only other way is to disable the ``PMCR_EL0.E`` bit upon entering
      EL3, which disables counting altogether.

-  ARMv8.2-Debug implemented:

   -  Prohibit general event counters: ``MDCR_EL3.SPME == 0``.
   -  Prohibit cycle counter: ``MDCR_EL3.SPME == 0 && PMCR_EL0.DP == 1``.
      ``PMCR_EL0`` therefore needs to be saved and restored in EL3.

-  ARMv8.5-PMU implemented:

   -  Prohibit general event counters: as in ARMv8.2-Debug.
   -  Prohibit cycle counter: ``MDCR_EL3.SCCD == 1``

In Aarch32 execution state the ``MDCR_EL3`` alias is the ``SDCR`` register,
which has some of the bit fields of ``MDCR_EL3``, most importantly the ``SPME``
and ``SCCD`` bits.

Build options
-------------

Several build options can be used to check for security issues. Refer to the
:ref:`Build Options` for detailed information on these.

- The ``BRANCH_PROTECTION`` build flag can be used to enable Pointer
  Authentication and Branch Target Identification.

- The ``ENABLE_STACK_PROTECTOR`` build flag can be used to identify buffer
  overflows.

- The ``W`` build flag can be used to enable a number of compiler warning
  options to detect potentially incorrect code.

  - W=0 (default value)

    The ``Wunused`` with ``Wno-unused-parameter``, ``Wdisabled-optimization``
    and ``Wvla`` flags are enabled.

    The ``Wunused-but-set-variable``, ``Wmaybe-uninitialized`` and
    ``Wpacked-bitfield-compat`` are GCC specific flags that are also enabled.

  - W=1

    Adds ``Wextra``, ``Wmissing-format-attribute``, ``Wmissing-prototypes``,
    ``Wold-style-definition`` and ``Wunused-const-variable``.

  - W=2

    Adds ``Waggregate-return``, ``Wcast-align``, ``Wnested-externs``,
    ``Wshadow``, ``Wlogical-op``.

  - W=3

    Adds ``Wbad-function-cast``, ``Wcast-qual``, ``Wconversion``, ``Wpacked``,
    ``Wpointer-arith``, ``Wredundant-decls`` and
    ``Wswitch-default``.

  Refer to the GCC or Clang documentation for more information on the individual
  options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html and
  https://clang.llvm.org/docs/DiagnosticsReference.html.

  NB: The ``Werror`` flag is enabled by default in TF-A and can be disabled by
  setting the ``E`` build flag to 0.

.. rubric:: References

-  `Arm ARM`_

--------------

*Copyright (c) 2019-2020, Arm Limited. All rights reserved.*

.. _Arm ARM: https://developer.arm.com/docs/ddi0487/latest
