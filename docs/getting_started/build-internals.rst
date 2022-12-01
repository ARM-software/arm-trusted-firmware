Internal Build Options
======================

|TF-A| internally uses certain options that are not exposed directly through
:ref:`build-options <build options>` but enabled or disabled indirectly and
depends on certain options to be enabled or disabled.

.. _build_options_internal:

-  ``CTX_INCLUDE_EL2_REGS``: This boolean option provides context save/restore
   operations when entering/exiting an EL2 execution context. This is of primary
   interest when Armv8.4-SecEL2 or RME extension is implemented.
   Default is 0 (disabled). This option will be set to 1 (enabled) when ``SPD=spmd``
   and ``SPMD_SPM_AT_SEL2`` is set or when ``ENABLE_RME`` is set to 1 (enabled).
