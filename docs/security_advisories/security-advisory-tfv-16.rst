Advisory TFV-16 (CVE-2026-0995)
================================

+----------------+-----------------------------------------------------------------+
| Title          | SME erratum in C1-Pro means memory accesses from the SME unit   |
|                | can remain outstanding after another CPU issues TLBI+DSB        |
+================+=================================================================+
| CVE ID         | `CVE-2026-0995`                                                 |
+----------------+-----------------------------------------------------------------+
| Date           | Reported on 23 September 2025                                   |
+----------------+-----------------------------------------------------------------+
| Versions       | TF-A version from v2.10 onwards                                 |
| Affected       |                                                                 |
+----------------+-----------------------------------------------------------------+
| Configurations | Arm C1-Pro prior to r1p2                                        |
| Affected       |                                                                 |
+----------------+-----------------------------------------------------------------+
| Impact         | SME can access memory after it has been re-allocated,           |
|                | potentially overwriting the new owner's data.                   |
+----------------+-----------------------------------------------------------------+
| Fix Version    | `Gerrit topic #gr/CVE-2026-0995`_                               |
|                | Also see mitigation guidance in the `Official Arm Advisory`_    |
+----------------+-----------------------------------------------------------------+
| Credit         | Arm                                                             |
+----------------+-----------------------------------------------------------------+

.. _c1_pro_CVE-2026-0995:

C1-Pro / CME CVE-2026-0995 Workaround
---------------------------------------

Overview
--------

C1-Pro/CME **CVE-2026-0995** affects CPUs implementing SME and
Streaming mode. Under specific micro-architectural conditions, a
``TLBI + DSB`` sequence performed on one CPU (PE1) may not guarantee
completion of certain in-flight memory accesses performed on another CPU
(PE0). As a result, those accesses may complete after translation
changes have taken effect, potentially resulting in memory accesses
outside the expected translation or privilege boundaries.

To ensure architectural correctness, all affected CPUs must execute a
local ``DSB`` whenever any CPU performs TLB maintenance. The TF-A
workaround provides a coordinated EL3 mechanism that guarantees this
synchronisation across all online C1-Pro CPUs.

Erratum Status
--------------

This erratum applies to C1-Pro multi-core configurations with ``NUM_CME != 0`` and (per TF-A runtime checks) to C1-Pro revisions up to **r1p2** (inclusive).
The erratum is **mitigated in software** through EL3
coordination.

Erratum Description
-------------------

**TLBI + DSB might fail to ensure completion of memory accesses caused by
FP/SIMD, SVE, and SME instructions while in Streaming mode, and by
LDR/STR ZA/ZT0 instructions.**

Description
------------

A ``TLBI + DSB`` sequence executed on **PE1** may fail to ensure
completion of some memory accesses on **PE0** associated with:

* ``LDR`` / ``STR`` to or from ``ZA`` or ``ZT0``.
* FP/SIMD, SVE, or SME memory accesses while in **Streaming mode**.

Configurations Affected
-----------------------

The erratum affects:

* All **multi-core** configurations
* Where ``NUM_CME != 0`` (i.e., systems using a CME complex)

Conditions
----------

The erratum occurs when all of the following are true:

1. **PE0** executes:
   * ``LDR``/``STR`` to/from ZA or ZT0, or
   * Memory accesses tied to FP/SIMD, SVE, or SME execution while in Streaming mode.
2. **PE1** performs:
   * A TLB invalidate instruction affecting a page used by PE0's memory accesses, followed by A ``DSB`` instruction.
3. Complex micro-architectural timing conditions occur.

Implications
------------

If the above conditions are met:

* The ``DSB`` on **PE1** may complete **before** certain affected memory
  accesses on **PE0**, even though those accesses are architecturally
  in scope for PE1’s TLBI.
* As a result, stale or incomplete memory accesses may occur after
  translation changes have taken effect.
* This can lead to memory being accessed outside the expected
  translation or privilege boundaries, depending on the software
  context.

Enabling the Workaround in TF-A
-------------------------------

Support for CVE-2026-0995 is build-time selectable and must be enabled
by the platform.

To enable the workaround, the platform must:

* Set ``WORKAROUND_CVE_2026_0995=1``.
* Include the C1-Pro workaround source in ``BL31_SOURCES``.
* Include the CPU service sources used by the SMC interface.

For example, a platform that contains affected C1-Pro CPUs should add:

.. code-block::

    WORKAROUND_CVE_2026_0995 := 1

    ifeq (${WORKAROUND_CVE_2026_0995},1)
    BL31_SOURCES += lib/cpus/aarch64/c1_pro_pubsub.c \
                    ${CPU_SVC_SRCS}
    endif

If the option is not enabled, or if the platform does not include the
workaround source and CPU service sources, TF-A will not mitigate this
erratum.

Why the Workaround Must Be Implemented in EL3
---------------------------------------------

Due to interaction of Linux, pKVM, and GIC security states:

* The non-secure world cannot reliably deliver SGIs without races.
* A CPU entering ``CPU_OFF`` may be unable to receive SGIs without
  violating PSCI rules.
* Interrupt masking and CPU power-down sequences may interfere with SGI
  delivery.

EL3 is the only domain capable of:

* Issuing **secure SGIs**.
* Tracking CPU on/off and suspend/resume transitions.
* Ensuring correct ordering during secure world entry/exit.
* Avoiding SGI interference during CPU power-down.

Workaround Mechanism
--------------------

The TF-A workaround ensures that:

* Every affected CPU performs a **local DSB** whenever another CPU
  performs TLB maintenance.
* A coordinated, EL3-managed SGI rendezvous ensures all online C1-Pro
  CPUs participate.

The mechanism uses:

* A **global atomic counter** incremented by the SMC caller.
* **Local counters** on each CPU tracking participation in each epoch.
* **Secure SGIs** sent to all active C1-Pro CPUs.

  The reference implementation uses the EL3 secure SGI number ``ARM_IRQ_SEC_SGI_6``.
* A **wait-for-completion loop** ensuring all CPUs have executed the
  mitigation before returning from SMC.

Global Counter and Ordering
---------------------------

Each SMC caller performs:

1. ``atomic_inc_return(global_counter)``
2. A barrier ensuring visibility before SGIs:
   ``dmbish``

The returned counter value is treated as that caller’s **deadline**:
all CPUs must update their local counter to at least this value.

SGI Rendezvous
--------------

After incrementing the counter:

1. The caller sends a secure SGI to all online C1-Pro CPUs.
2. Receivers execute:
   * A local ``DSB`` (the mitigation)
   * Update their local counter from the global counter
3. The caller waits until all CPUs have reached its deadline.

Handling PSCI ``CPU_OFF`` Races
-------------------------------

SGIs may race with CPUs powering down.

To avoid violating PSCI semantics:

1. The caller sends a secure SGI to all online C1-Pro CPUs.
2. Receivers execute the local mitigation sequence, which:
   * May include a ``DSB`` if required by architectural configuration (for example, when ``SCTLR_EL3.IESB`` is not set).
   * Updates the local counter from the global counter.
3. The caller waits until all CPUs have reached its deadline value.

Tracking Active C1-Pro CPUs
---------------------------

TF-A maintains:

* A per-core signed bytemap tracking whether a C1-Pro CPU is currently *active* (reference count 0/1).
* Per-CPU MPIDRs for SGI targeting.

The implementation supports up to 64 cores (``PLATFORM_CORE_COUNT <= 64``);
the SMC handler uses a 64-bit mask to track which CPUs were sent SGIs.

EL3 updates this information on:

* ``psci_cpu_on_finish``
* ``psci_cpu_off_finish``
* ``psci_suspend_pwrdown_start`` / ``finish``
* ``cm_entering_secure_world`` / ``exited_secure_world``

Memory Ordering Requirements
----------------------------

Strong ordering is required to make the SGI rendezvous reliable across cores and
across PSCI/world-switch transitions:

``dsb()``
    Drains outstanding memory accesses. On C1-Pro, the workaround performs a
    local ``dsb`` **only when** ``SCTLR_EL3.IESB`` is **not** set; if
    ``SCTLR_EL3.IESB`` is set, taking an exception to EL3 is sufficient and the
    explicit ``dsb`` can be skipped.

``dmbish``
    Ensures ordering/visibility between:
    * the atomic increment of ``global_counter`` and subsequent SGI delivery, and
    * updates to the active-core bytemap before using it to decide who must participate in a rendezvous.

``isb``
    Used in the SGI handler to ensure the counter load occurs after interrupt
    acknowledge (prevents stale speculative loads if SGIs merge).

Summary
-------

CVE-2026-0995 allows **stale SME/SIMD/SVE memory accesses to persist
beyond TLBI + DSB**, violating architectural expectations and potentially
compromising security.

The TF-A mitigation:

* Ensures **every active C1-Pro CPU** drains outstanding accesses.
* Coordinates CPUs via **secure SGIs** and **atomic counters**.
* Handles PSCI races safely in EL3.
* Is enabled at build time via: ``WORKAROUND_CVE_2026_0995=1``

The result is a robust, race-free mitigation suitable for systems deploying any
security-sensitive workloads.

OS Coordination Requirement
---------------------------

Full mitigation of CVE-2026-0995 requires coordinated updates in both:

* Trusted Firmware-A (EL3) — this advisory
* The Operating System

The EL3 implementation provides a secure SGI-based rendezvous mechanism. The Operating System must
invoke the defined SMC interface when performing affected TLB maintenance operations to ensure
architectural ordering across all C1-Pro CPUs.

Deploying only one side of the mitigation is insufficient to guarantee architectural ordering
and full protection.

For further information, affected CPUs, and detailed guidance, refer to the full
`Official Arm Advisory`_.

.. _Gerrit topic #gr/CVE-2026-0995: https://review.trustedfirmware.org/q/topic:gr/CVE-2026-0095
.. _Official Arm Advisory: https://developer.arm.com/documentation/111823/latest/
