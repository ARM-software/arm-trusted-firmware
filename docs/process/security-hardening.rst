Security hardening
==================

This page contains guidance on what to check for additional security measures,
including build options that can be modified to improve security or catch issues
early in development.

Build options
-------------

Several build options can be used to check for security issues. Refer to the
:ref:`User Guide` for detailed information on the specific build options.

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

    Adds ``Wextra``, ``Wmissing-declarations``, ``Wmissing-format-attribute``,
    ``Wmissing-prototypes``, ``Wold-style-definition`` and
    ``Wunused-const-variable``.

  - W=2

    Adds ``Waggregate-return``, ``Wcast-align``, ``Wnested-externs``,
    ``Wshadow``, ``Wlogical-op``, ``Wmissing-field-initializers`` and
    ``Wsign-compare``.

  - W=3

    Adds ``Wbad-function-cast``, ``Wcast-qual``, ``Wconversion``, ``Wpacked``,
    ``Wpadded``, ``Wpointer-arith``, ``Wredundant-decls`` and
    ``Wswitch-default``.

  Refer to the GCC or Clang documentation for more information on the individual
  options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html and
  https://clang.llvm.org/docs/DiagnosticsReference.html.

  NB: The ``Werror`` flag is enabled by default in TF-A and can be disabled by
  setting the ``E`` build flag to 0.

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*
