PSCI Performance Measurements on Arm Juno Development Platform
==============================================================

This document summarises the findings of performance measurements of key
operations in the ARM Trusted Firmware (TF) Power State Coordination Interface
(PSCI) implementation, using the in-built Performance Measurement Framework
(PMF) and runtime instrumentation timestamps.

Method
------

We used the `Juno R1 platform`_ for these tests, which has 4 x Cortex-A53 and 2
x Cortex-A57 clusters running at the following frequencies:

+-----------------+--------------------+
| Domain          | Frequency (MHz)    |
+=================+====================+
| Cortex-A57      | 900 (nominal)      |
+-----------------+--------------------+
| Cortex-A53      | 650 (underdrive)   |
+-----------------+--------------------+
| AXI subsystem   | 533                |
+-----------------+--------------------+

Juno supports CPU, cluster and system power down states, corresponding to power
levels 0, 1 and 2 respectively. It does not support any retention states.

We used the upstream `TF master as of 31/01/2017`_, building the platform using
the ``ENABLE_RUNTIME_INSTRUMENTATION`` option:

.. code:: shell

    make PLAT=juno ENABLE_RUNTIME_INSTRUMENTATION=1 \
        SCP_BL2=<path/to/scp-fw.bin>                \
        BL33=<path/to/test-fw.bin>                  \
        all fip

When using the debug build of TF, there was no noticeable difference in the
results.

The tests are based on an ARM-internal test framework. The release build of this
framework was used because the results in the debug build became skewed; the
console output prevented some of the tests from executing in parallel.

The tests consist of both parallel and sequential tests, which are broadly
described as follows:

- **Parallel Tests** This type of test powers on all the non-lead CPUs and
  brings them and the lead CPU to a common synchronization point.  The lead CPU
  then initiates the test on all CPUs in parallel.

- **Sequential Tests** This type of test powers on each non-lead CPU in
  sequence. The lead CPU initiates the test on a non-lead CPU then waits for the
  test to complete before proceeding to the next non-lead CPU. The lead CPU then
  executes the test on itself.

In the results below, CPUs 0-3 refer to CPUs in the little cluster (A53) and
CPUs 4-5 refer to CPUs in the big cluster (A57). In all cases CPU 4 is the lead
CPU.

``PSCI_ENTRY`` refers to the time taken from entering the TF PSCI implementation
to the point the hardware enters the low power state (WFI). Referring to the TF
runtime instrumentation points, this corresponds to:
``(RT_INSTR_ENTER_HW_LOW_PWR - RT_INSTR_ENTER_PSCI)``.

``PSCI_EXIT`` refers to the time taken from the point the hardware exits the low
power state to exiting the TF PSCI implementation. This corresponds to:
``(RT_INSTR_EXIT_PSCI - RT_INSTR_EXIT_HW_LOW_PWR)``.

``CFLUSH_OVERHEAD`` refers to the part of ``PSCI_ENTRY`` taken to flush the
caches. This corresponds to: ``(RT_INSTR_EXIT_CFLUSH - RT_INSTR_ENTER_CFLUSH)``.

Note there is very little variance observed in the values given (~1us), although
the values for each CPU are sometimes interchanged, depending on the order in
which locks are acquired. Also, there is very little variance observed between
executing the tests sequentially in a single boot or rebooting between tests.

Given that runtime instrumentation using PMF is invasive, there is a small
(unquantified) overhead on the results. PMF uses the generic counter for
timestamps, which runs at 50MHz on Juno.

Results and Commentary
----------------------

``CPU_SUSPEND`` to deepest power level on all CPUs in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-------+---------------------+--------------------+--------------------------+
| CPU   | ``PSCI_ENTRY`` (us) | ``PSCI_EXIT`` (us) | ``CFLUSH_OVERHEAD`` (us) |
+=======+=====================+====================+==========================+
| 0     | 27                  | 20                 | 5                        |
+-------+---------------------+--------------------+--------------------------+
| 1     | 114                 | 86                 | 5                        |
+-------+---------------------+--------------------+--------------------------+
| 2     | 202                 | 58                 | 5                        |
+-------+---------------------+--------------------+--------------------------+
| 3     | 375                 | 29                 | 94                       |
+-------+---------------------+--------------------+--------------------------+
| 4     | 20                  | 22                 | 6                        |
+-------+---------------------+--------------------+--------------------------+
| 5     | 290                 | 18                 | 206                      |
+-------+---------------------+--------------------+--------------------------+

A large variance in ``PSCI_ENTRY`` and ``PSCI_EXIT`` times across CPUs is
observed due to TF PSCI lock contention. In the worst case, CPU 3 has to wait
for the 3 other CPUs in the cluster (0-2) to complete ``PSCI_ENTRY`` and release
the lock before proceeding.

The ``CFLUSH_OVERHEAD`` times for CPUs 3 and 5 are higher because they are the
last CPUs in their respective clusters to power down, therefore both the L1 and
L2 caches are flushed.

The ``CFLUSH_OVERHEAD`` time for CPU 5 is a lot larger than that for CPU 3
because the L2 cache size for the big cluster is lot larger (2MB) compared to
the little cluster (1MB).

``CPU_SUSPEND`` to power level 0 on all CPUs in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-------+---------------------+--------------------+--------------------------+
| CPU   | ``PSCI_ENTRY`` (us) | ``PSCI_EXIT`` (us) | ``CFLUSH_OVERHEAD`` (us) |
+=======+=====================+====================+==========================+
| 0     | 116                 | 14                 | 8                        |
+-------+---------------------+--------------------+--------------------------+
| 1     | 204                 | 14                 | 8                        |
+-------+---------------------+--------------------+--------------------------+
| 2     | 287                 | 13                 | 8                        |
+-------+---------------------+--------------------+--------------------------+
| 3     | 376                 | 13                 | 9                        |
+-------+---------------------+--------------------+--------------------------+
| 4     | 29                  | 15                 | 7                        |
+-------+---------------------+--------------------+--------------------------+
| 5     | 21                  | 15                 | 8                        |
+-------+---------------------+--------------------+--------------------------+

There is no lock contention in TF generic code at power level 0 but the large
variance in ``PSCI_ENTRY`` times across CPUs is due to lock contention in Juno
platform code. The platform lock is used to mediate access to a single SCP
communication channel. This is compounded by the SCP firmware waiting for each
AP CPU to enter WFI before making the channel available to other CPUs, which
effectively serializes the SCP power down commands from all CPUs.

On platforms with a more efficient CPU power down mechanism, it should be
possible to make the ``PSCI_ENTRY`` times smaller and consistent.

The ``PSCI_EXIT`` times are consistent across all CPUs because TF does not
require locks at power level 0.

The ``CFLUSH_OVERHEAD`` times for all CPUs are small and consistent since only
the cache associated with power level 0 is flushed (L1).

``CPU_SUSPEND`` to deepest power level on all CPUs in sequence
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-------+---------------------+--------------------+--------------------------+
| CPU   | ``PSCI_ENTRY`` (us) | ``PSCI_EXIT`` (us) | ``CFLUSH_OVERHEAD`` (us) |
+=======+=====================+====================+==========================+
| 0     | 114                 | 20                 | 94                       |
+-------+---------------------+--------------------+--------------------------+
| 1     | 114                 | 20                 | 94                       |
+-------+---------------------+--------------------+--------------------------+
| 2     | 114                 | 20                 | 94                       |
+-------+---------------------+--------------------+--------------------------+
| 3     | 114                 | 20                 | 94                       |
+-------+---------------------+--------------------+--------------------------+
| 4     | 195                 | 22                 | 180                      |
+-------+---------------------+--------------------+--------------------------+
| 5     | 21                  | 17                 | 6                        |
+-------+---------------------+--------------------+--------------------------+

The ``CLUSH_OVERHEAD`` times for lead CPU 4 and all CPUs in the non-lead cluster
are large because all other CPUs in the cluster are powered down during the
test. The ``CPU_SUSPEND`` call powers down to the cluster level, requiring a
flush of both L1 and L2 caches.

The ``CFLUSH_OVERHEAD`` time for CPU 4 is a lot larger than those for the little
CPUs because the L2 cache size for the big cluster is lot larger (2MB) compared
to the little cluster (1MB).

The ``PSCI_ENTRY`` and ``CFLUSH_OVERHEAD`` times for CPU 5 are low because lead
CPU 4 continues to run while CPU 5 is suspended. Hence CPU 5 only powers down to
level 0, which only requires L1 cache flush.

``CPU_SUSPEND`` to power level 0 on all CPUs in sequence
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-------+---------------------+--------------------+--------------------------+
| CPU   | ``PSCI_ENTRY`` (us) | ``PSCI_EXIT`` (us) | ``CFLUSH_OVERHEAD`` (us) |
+=======+=====================+====================+==========================+
| 0     | 22                  | 14                 | 5                        |
+-------+---------------------+--------------------+--------------------------+
| 1     | 22                  | 14                 | 5                        |
+-------+---------------------+--------------------+--------------------------+
| 2     | 21                  | 14                 | 5                        |
+-------+---------------------+--------------------+--------------------------+
| 3     | 22                  | 14                 | 5                        |
+-------+---------------------+--------------------+--------------------------+
| 4     | 17                  | 14                 | 6                        |
+-------+---------------------+--------------------+--------------------------+
| 5     | 18                  | 15                 | 6                        |
+-------+---------------------+--------------------+--------------------------+

Here the times are small and consistent since there is no contention and it is
only necessary to flush the cache to power level 0 (L1). This is the best case
scenario.

The ``PSCI_ENTRY`` times for CPUs in the big cluster are slightly smaller than
for the CPUs in little cluster due to greater CPU performance.

The ``PSCI_EXIT`` times are generally lower than in the last test because the
cluster remains powered on throughout the test and there is less code to execute
on power on (for example, no need to enter CCI coherency)

``CPU_OFF`` on all non-lead CPUs in sequence then ``CPU_SUSPEND`` on lead CPU to deepest power level
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The test sequence here is as follows:

1. Call ``CPU_ON`` and ``CPU_OFF`` on each non-lead CPU in sequence.

2. Program wake up timer and suspend the lead CPU to the deepest power level.

3. Call ``CPU_ON`` on non-lead CPU to get the timestamps from each CPU.

+-------+---------------------+--------------------+--------------------------+
| CPU   | ``PSCI_ENTRY`` (us) | ``PSCI_EXIT`` (us) | ``CFLUSH_OVERHEAD`` (us) |
+=======+=====================+====================+==========================+
| 0     | 110                 | 28                 | 93                       |
+-------+---------------------+--------------------+--------------------------+
| 1     | 110                 | 28                 | 93                       |
+-------+---------------------+--------------------+--------------------------+
| 2     | 110                 | 28                 | 93                       |
+-------+---------------------+--------------------+--------------------------+
| 3     | 111                 | 28                 | 93                       |
+-------+---------------------+--------------------+--------------------------+
| 4     | 195                 | 22                 | 181                      |
+-------+---------------------+--------------------+--------------------------+
| 5     | 20                  | 23                 | 6                        |
+-------+---------------------+--------------------+--------------------------+

The ``CFLUSH_OVERHEAD`` times for all little CPUs are large because all other
CPUs in that cluster are powerered down during the test. The ``CPU_OFF`` call
powers down to the cluster level, requiring a flush of both L1 and L2 caches.

The ``PSCI_ENTRY`` and ``CFLUSH_OVERHEAD`` times for CPU 5 are small because
lead CPU 4 is running and CPU 5 only powers down to level 0, which only requires
an L1 cache flush.

The ``CFLUSH_OVERHEAD`` time for CPU 4 is a lot larger than those for the little
CPUs because the L2 cache size for the big cluster is lot larger (2MB) compared
to the little cluster (1MB).

The ``PSCI_EXIT`` times for CPUs in the big cluster are slightly smaller than
for CPUs in the little cluster due to greater CPU performance.  These times
generally are greater than the ``PSCI_EXIT`` times in the ``CPU_SUSPEND`` tests
because there is more code to execute in the "on finisher" compared to the
"suspend finisher" (for example, GIC redistributor register programming).

``PSCI_VERSION`` on all CPUs in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since very little code is associated with ``PSCI_VERSION``, this test
approximates the round trip latency for handling a fast SMC at EL3 in TF.

+-------+-------------------+
| CPU   | TOTAL TIME (ns)   |
+=======+===================+
| 0     | 3020              |
+-------+-------------------+
| 1     | 2940              |
+-------+-------------------+
| 2     | 2980              |
+-------+-------------------+
| 3     | 3060              |
+-------+-------------------+
| 4     | 520               |
+-------+-------------------+
| 5     | 720               |
+-------+-------------------+

The times for the big CPUs are less than the little CPUs due to greater CPU
performance.

We suspect the time for lead CPU 4 is shorter than CPU 5 due to subtle cache
effects, given that these measurements are at the nano-second level.

.. _Juno R1 platform: https://www.arm.com/files/pdf/Juno_r1_ARM_Dev_datasheet.pdf
.. _TF master as of 31/01/2017: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/?id=c38b36d
