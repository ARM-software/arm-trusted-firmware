Runtime Instrumentation Methodology
===================================

This document outlines steps for undertaking performance measurements of key
operations in the Trusted Firmware-A Power State Coordination Interface (PSCI)
implementation, using the in-built Performance Measurement Framework (PMF) and
runtime instrumentation timestamps.

Framework
~~~~~~~~~

The tests are based on the ``runtime-instrumentation`` test suite provided by
the Trusted Firmware Test Framework (TFTF). The release build of this framework
was used because the results in the debug build became skewed; the console
output prevented some of the tests from executing in parallel.

The tests consist of both parallel and sequential tests, which are broadly
described as follows:

- **Parallel Tests** This type of test powers on all the non-lead CPUs and
  brings them and the lead CPU to a common synchronization point.  The lead CPU
  then initiates the test on all CPUs in parallel.

- **Sequential Tests** This type of test powers on each non-lead CPU in
  sequence. The lead CPU initiates the test on a non-lead CPU then waits for the
  test to complete before proceeding to the next non-lead CPU. The lead CPU then
  executes the test on itself.

Note there is very little variance observed in the values given (~1us), although
the values for each CPU are sometimes interchanged, depending on the order in
which locks are acquired. Also, there is very little variance observed between
executing the tests sequentially in a single boot or rebooting between tests.

Given that runtime instrumentation using PMF is invasive, there is a small
(unquantified) overhead on the results. PMF uses the generic counter for
timestamps, which runs at 50MHz on Juno.

Metrics
~~~~~~~

.. glossary::

   Powerdown Latency
        Time taken from entering the TF PSCI implementation to the point the hardware
        enters the low power state (WFI). Referring to the TF runtime instrumentation points, this
        corresponds to: ``(RT_INSTR_ENTER_HW_LOW_PWR - RT_INSTR_ENTER_PSCI)``.

   Wakeup Latency
        Time taken from the point the hardware exits the low power state to exiting
        the TF PSCI implementation. This corresponds to: ``(RT_INSTR_EXIT_PSCI -
        RT_INSTR_EXIT_HW_LOW_PWR)``.

   Cache Flush Latency
        Time taken to flush the caches during powerdown. This corresponds to:
        ``(RT_INSTR_EXIT_CFLUSH - RT_INSTR_ENTER_CFLUSH)``.
