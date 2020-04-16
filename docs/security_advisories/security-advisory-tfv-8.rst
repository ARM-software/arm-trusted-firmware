Advisory TFV-8 (CVE-2018-19440)
===============================

+----------------+-------------------------------------------------------------+
| Title          | Not saving x0 to x3 registers can leak information from one |
|                | Normal World SMC client to another                          |
+================+=============================================================+
| CVE ID         | `CVE-2018-19440`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | 27 Nov 2018                                                 |
+----------------+-------------------------------------------------------------+
| Versions       | All                                                         |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | Multiple normal world SMC clients calling into AArch64 BL31 |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Leakage of SMC return values from one normal world SMC      |
|                | client to another                                           |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #1710`_                                       |
+----------------+-------------------------------------------------------------+
| Credit         | Secmation                                                   |
+----------------+-------------------------------------------------------------+

When taking an exception to EL3, BL31 saves the CPU context. The aim is to
restore it before returning into the lower exception level software that called
into the firmware. However, for an SMC exception, the general purpose registers
``x0`` to ``x3`` are not part of the CPU context saved on the stack.

As per the `SMC Calling Convention`_, up to 4 values may be returned to the
caller in registers ``x0`` to ``x3``. In TF-A, these return values are written
into the CPU context, typically using one of the ``SMC_RETx()`` macros provided
in the ``include/lib/aarch64/smccc_helpers.h`` header file.

Before returning to the caller, the ``restore_gp_registers()`` function is
called. It restores the values of all general purpose registers taken from the
CPU context stored on the stack. This includes registers ``x0`` to ``x3``, as
can be seen in the ``lib/el3_runtime/aarch64/context.S`` file at line 339
(referring to the version of the code as of `commit c385955`_):

::

    /*
     * This function restores all general purpose registers except x30 from the
     * CPU context. x30 register must be explicitly restored by the caller.
     */
    func restore_gp_registers
        ldp x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
        ldp x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]

In the case of an SMC handler that does not use all 4 return values, the
remaining ones are left unchanged in the CPU context. As a result,
``restore_gp_registers()`` restores the stale values saved by a previous SMC
request (or asynchronous exception to EL3) that used these return values.

In the presence of multiple normal world SMC clients, this behaviour might leak
some of the return values from one client to another. For example, if a victim
client first sends an SMC that returns 4 values, a malicious client may then
send a second SMC expecting no return values (for example, a
``SDEI_EVENT_COMPLETE`` SMC) to get the 4 return values of the victim client.

In general, the responsibility for mitigating threats due to the presence of
multiple normal world SMC clients lies with EL2 software.  When present, EL2
software must trap SMC calls from EL1 software to ensure secure behaviour.

For this reason, TF-A does not save ``x0`` to ``x3`` in the CPU context on an
SMC synchronous exception. It has behaved this way since the first version.

We can confirm that at least upstream KVM-based systems mitigate this threat,
and are therefore unaffected by this issue. Other EL2 software should be audited
to assess the impact of this threat.

EL2 software might find mitigating this threat somewhat onerous, because for all
SMCs it would need to be aware of which return registers contain valid data, so
it can sanitise any unused return registers. On the other hand, mitigating this
in EL3 is relatively easy and cheap. Therefore, TF-A will now ensure that no
information is leaked through registers ``x0`` to ``x3``, by preserving the
register state over the call.

Note that AArch32 TF-A is not affected by this issue. The SMC handling code in
``SP_MIN`` already saves all general purpose registers - including ``r0`` to
``r3``, as can be seen in the ``include/lib/aarch32/smccc_macros.S`` file at
line 19 (referring to the version of the code as of `commit c385955`_):

.. code:: c

    /*
     * Macro to save the General purpose registers (r0 - r12), the banked
     * spsr, lr, sp registers and the `scr` register to the SMC context on entry
     * due a SMC call. The `lr` of the current mode (monitor) is expected to be
     * already saved. The `sp` must point to the `smc_ctx_t` to save to.
     * Additionally, also save the 'pmcr' register as this is updated whilst
     * executing in the secure world.
     */
        .macro smccc_save_gp_mode_regs
        /* Save r0 - r12 in the SMC context */
        stm sp, {r0-r12}

.. _CVE-2018-19440: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-19440
.. _commit c385955: https://github.com/ARM-software/arm-trusted-firmware/commit/c385955
.. _SMC Calling Convention: https://developer.arm.com/docs/den0028/latest
.. _Pull Request #1710: https://github.com/ARM-software/arm-trusted-firmware/pull/1710
