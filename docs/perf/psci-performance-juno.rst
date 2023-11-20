PSCI Performance Measurements on Arm Juno Development Platform
==============================================================

This document summarises the findings of performance measurements of key
operations in the Trusted Firmware-A Power State Coordination Interface (PSCI)
implementation, using the in-built Performance Measurement Framework (PMF) and
runtime instrumentation timestamps.

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

Given that runtime instrumentation using PMF is invasive, there is a small
(unquantified) overhead on the results. PMF uses the generic counter for
timestamps, which runs at 50MHz on Juno.

The following source trees and binaries were used:

- TF-A [`v2.9-rc0`_]
- TFTF [`v2.9-rc0`_]

Please see the Runtime Instrumentation :ref:`Testing Methodology
<Runtime Instrumentation Methodology>`
page for more details.

Procedure
---------

#. Build TFTF with runtime instrumentation enabled:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=juno \
            TESTS=runtime-instrumentation all

#. Fetch Juno's SCP binary from TF-A's archive:

    .. code:: shell

        curl --fail --connect-timeout 5 --retry 5 -sLS -o scp_bl2.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.12.0/juno/release/juno-bl2.bin

#. Build TF-A with the following build options:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=juno \
            BL33="/path/to/tftf.bin" SCP_BL2="scp_bl2.bin" \
            ENABLE_RUNTIME_INSTRUMENTATION=1 fiptool all fip

#. Load the following images onto the development board: ``fip.bin``,
   ``scp_bl2.bin``.

Results
-------

``CPU_SUSPEND`` to deepest power level
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        parallel (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   104.58  | 241.20 |     5.26    |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   384.24  | 22.50  |    138.76   |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   244.56  | 22.18  |     5.16    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   670.56  | 18.58  |     4.44    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   809.36  | 269.28 |     4.44    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   984.96  | 219.70 |    79.62    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        parallel (v2.10)

    +---------+------+-------------------+--------+-------------+
    | Cluster | Core |     Powerdown     | Wakeup | Cache Flush |
    +---------+------+-------------------+--------+-------------+
    |    0    |  0   | 242.66 (+132.03%) | 245.1  |     5.4     |
    +---------+------+-------------------+--------+-------------+
    |    0    |  1   |  522.08 (+35.87%) | 26.24  |    138.32   |
    +---------+------+-------------------+--------+-------------+
    |    1    |  0   |  104.36 (-57.33%) |  27.1  |     5.32    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  1   |  382.56 (-42.95%) | 23.34  |     4.42    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  2   |       807.74      | 271.54 |     4.64    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  3   |       981.36      | 221.8  |    79.48    |
    +---------+------+-------------------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        serial (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   236.56  | 23.24  |    138.18   |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   236.86  | 23.28  |    138.10   |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   281.04  | 22.80  |    77.24    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   100.28  | 18.52  |     4.54    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   100.12  | 18.78  |     4.50    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   100.36  | 18.94  |     4.44    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        serial (v2.10)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   236.84  |  27.1  |    138.36   |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   236.96  |  27.1  |    138.32   |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   280.06  | 26.94  |     77.5    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   100.76  | 23.42  |     4.36    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   100.02  | 23.42  |     4.44    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   100.08  |  23.2  |     4.4     |
    +---------+------+-----------+--------+-------------+

``CPU_SUSPEND`` to power level 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in
        parallel (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   662.34  | 15.22  |     8.08    |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   802.00  | 15.50  |     8.16    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   385.22  | 15.74  |     7.88    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   106.16  | 16.06  |     7.44    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   524.38  | 15.64  |     7.34    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   246.00  | 15.78  |     7.72    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in
        parallel (v2.10)

    +---------+------+-------------------+--------+-------------+
    | Cluster | Core |     Powerdown     | Wakeup | Cache Flush |
    +---------+------+-------------------+--------+-------------+
    |    0    |  0   |       801.04      | 18.66  |     8.22    |
    +---------+------+-------------------+--------+-------------+
    |    0    |  1   |       661.28      | 19.08  |     7.88    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  0   |  105.9 (-72.51%)  |  20.3  |     7.58    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  1   | 383.58 (+261.32%) |  20.4  |     7.42    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  2   |       523.52      |  20.1  |     7.74    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  3   |       244.5       | 20.16  |     7.56    |
    +---------+------+-------------------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   99.80   | 15.94  |     5.42    |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   99.76   | 15.80  |     5.24    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   278.26  | 16.16  |     4.58    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   96.88   | 16.00  |     4.52    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   96.80   | 16.12  |     4.54    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   96.88   | 16.12  |     4.54    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.10)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   99.84   | 18.86  |     5.54    |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   100.2   | 18.82  |     5.66    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   278.12  | 20.56  |     4.48    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   96.68   | 20.62  |     4.3     |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   96.94   | 20.14  |     4.42    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   96.68   | 20.46  |     4.32    |
    +---------+------+-----------+--------+-------------+

``CPU_OFF`` on all non-lead CPUs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CPU_OFF`` on all non-lead CPUs in sequence then, ``CPU_SUSPEND`` on the lead
core to the deepest power level.

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   235.76  | 26.14  |    137.80   |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   235.40  | 25.72  |    137.62   |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   174.70  | 22.40  |    77.26    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   100.92  | 24.04  |     4.52    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   100.68  | 22.44  |     4.36    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   101.36  | 22.70  |     4.52    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.10)

    +---------------------------------------------------+
    |       test_rt_instr_cpu_off_serial (latest)       |
    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   236.04  | 30.02  |    137.9    |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   235.38  |  29.7  |    137.72   |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   175.18  | 26.96  |    77.26    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   100.56  | 28.34  |     4.32    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   100.38  | 26.82  |     4.3     |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   100.86  | 26.98  |     4.42    |
    +---------+------+-----------+--------+-------------+

``CPU_VERSION`` in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (2.9)

    +-------------+--------+-------------+
    |   Cluster   |  Core  |   Latency   |
    +-------------+--------+-------------+
    |      0      |   0    |     1.48    |
    +-------------+--------+-------------+
    |      0      |   1    |     1.04    |
    +-------------+--------+-------------+
    |      1      |   0    |     0.56    |
    +-------------+--------+-------------+
    |      1      |   1    |     0.92    |
    +-------------+--------+-------------+
    |      1      |   2    |     0.96    |
    +-------------+--------+-------------+
    |      1      |   3    |     0.96    |
    +-------------+--------+-------------+

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (2.10)

    +-------------+--------+----------------------+
    |   Cluster   |  Core  |       Latency        |
    +-------------+--------+----------------------+
    |      0      |   0    |    1.1 (-25.68%)     |
    +-------------+--------+----------------------+
    |      0      |   1    |         1.06         |
    +-------------+--------+----------------------+
    |      1      |   0    |         0.58         |
    +-------------+--------+----------------------+
    |      1      |   1    |         0.88         |
    +-------------+--------+----------------------+
    |      1      |   2    |         0.92         |
    +-------------+--------+----------------------+
    |      1      |   3    |         0.9          |
    +-------------+--------+----------------------+

Annotated Historic Results
--------------------------

The following results are based on the upstream `TF master as of 31/01/2017`_.
TF-A was built using the same build instructions as detailed in the procedure
above.

In the results below, CPUs 0-3 refer to CPUs in the little cluster (A53) and
CPUs 4-5 refer to CPUs in the big cluster (A57). In all cases CPU 4 is the lead
CPU.

``PSCI_ENTRY`` corresponds to the powerdown latency, ``PSCI_EXIT`` the wakeup latency, and
``CFLUSH_OVERHEAD`` the latency of the cache flush operation.

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

The ``CFLUSH_OVERHEAD`` times for lead CPU 4 and all CPUs in the non-lead cluster
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

--------------

*Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.*

.. _Juno R1 platform: https://developer.arm.com/documentation/100122/latest/
.. _TF master as of 31/01/2017: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/?id=c38b36d
.. _v2.9-rc0: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/?h=v2.9-rc0
