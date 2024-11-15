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

- `TF-A v2.12-rc0`_
- `TFTF v2.12-rc0`_

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
        parallel (v2.12)

    +---------+------+-------------------+------------------+--------------------+
    | Cluster | Core |     Powerdown     |      Wakeup      |    Cache Flush     |
    +---------+------+-------------------+------------------+--------------------+
    |    0    |  0   |  244.52 (-65.43%) | 26.92 (-32.60%)  |   5.54 (-96.70%)   |
    +---------+------+-------------------+------------------+--------------------+
    |    0    |  1   | 526.18 (+105.12%) |      416.1       | 138.52 (+2011.59%) |
    +---------+------+-------------------+------------------+--------------------+
    |    1    |  0   |       104.34      | 27.02 (-94.62%)  |        5.32        |
    +---------+------+-------------------+------------------+--------------------+
    |    1    |  1   |       384.98      | 23.06 (-85.40%)  |        4.48        |
    +---------+------+-------------------+------------------+--------------------+
    |    1    |  2   |  812.44 (+45.94%) |      126.78      |        4.54        |
    +---------+------+-------------------+------------------+--------------------+
    |    1    |  3   |       986.84      | 77.22 (+176.58%) |       79.76        |
    +---------+------+-------------------+------------------+--------------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        parallel (v2.11)

    +---------+------+-------------------+--------------------+-------------+
    | Cluster | Core |     Powerdown     |       Wakeup       | Cache Flush |
    +---------+------+-------------------+--------------------+-------------+
    |    0    |  0   |  112.98 (-53.44%) |  26.16 (-89.33%)   |     5.48    |
    +---------+------+-------------------+--------------------+-------------+
    |    0    |  1   |       411.18      | 438.88 (+1572.56%) |    138.54   |
    +---------+------+-------------------+--------------------+-------------+
    |    1    |  0   | 261.82 (+150.88%) | 474.06 (+1649.30%) |     5.6     |
    +---------+------+-------------------+--------------------+-------------+
    |    1    |  1   |  714.76 (+86.84%) |       26.44        |     4.48    |
    +---------+------+-------------------+--------------------+-------------+
    |    1    |  2   |       862.66      |  149.34 (-45.00%)  |     4.38    |
    +---------+------+-------------------+--------------------+-------------+
    |    1    |  3   |      1045.12      |  98.12 (-55.76%)   |    79.74    |
    +---------+------+-------------------+--------------------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        serial (v2.12)

    +---------+------+-----------+-----------------+-------------+
    | Cluster | Core | Powerdown |      Wakeup     | Cache Flush |
    +---------+------+-----------+-----------------+-------------+
    |    0    |  0   |   236.36  | 27.94 (-31.52%) |    138.0    |
    +---------+------+-----------+-----------------+-------------+
    |    0    |  1   |   236.58  | 27.86 (-31.72%) |    138.2    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  0   |   280.68  |      27.02      |     77.6    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  1   |   101.4   |      22.52      |     4.42    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  2   |   100.92  |      22.68      |     4.4     |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  3   |   100.96  |      22.54      |     4.38    |
    +---------+------+-----------+-----------------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        serial (v2.11)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   244.42  | 27.42  |    138.12   |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   245.02  | 27.34  |    138.08   |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   297.66  |  26.2  |    77.68    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   108.02  | 21.94  |     4.52    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   107.48  | 21.88  |     4.46    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   107.52  | 21.86  |     4.46    |
    +---------+------+-----------+--------+-------------+

``CPU_SUSPEND`` to power level 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in
        parallel (v2.12)

    +--------------------------------------------------------------------+
    |                  test_rt_instr_cpu_susp_parallel                   |
    +---------+------+-------------------+-----------------+-------------+
    | Cluster | Core |     Powerdown     |      Wakeup     | Cache Flush |
    +---------+------+-------------------+-----------------+-------------+
    |    0    |  0   |       663.12      | 19.66 (-39.21%) |     8.26    |
    +---------+------+-------------------+-----------------+-------------+
    |    0    |  1   |       804.18      | 19.24 (-40.65%) |     8.1     |
    +---------+------+-------------------+-----------------+-------------+
    |    1    |  0   |  105.58 (-58.80%) |      19.68      |     7.42    |
    +---------+------+-------------------+-----------------+-------------+
    |    1    |  1   |  245.02 (-39.67%) |       19.8      |     6.82    |
    +---------+------+-------------------+-----------------+-------------+
    |    1    |  2   |  383.82 (-30.83%) |      18.84      |     7.06    |
    +---------+------+-------------------+-----------------+-------------+
    |    1    |  3   | 523.36 (+391.23%) |       19.0      |     7.3     |
    +---------+------+-------------------+-----------------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in
        parallel (v2.11)

    +---------+------+-------------------+--------+-------------+
    | Cluster | Core |     Powerdown     | Wakeup | Cache Flush |
    +---------+------+-------------------+--------+-------------+
    |    0    |  0   |       704.46      | 19.28  |     7.86    |
    +---------+------+-------------------+--------+-------------+
    |    0    |  1   |       853.66      | 18.78  |     7.82    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  0   | 556.52 (+425.51%) | 19.06  |     7.82    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  1   |  113.28 (-70.47%) | 19.28  |     7.48    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  2   |  260.62 (-50.22%) |  19.8  |     7.26    |
    +---------+------+-------------------+--------+-------------+
    |    1    |  3   |  408.16 (+66.94%) | 19.82  |     7.38    |
    +---------+------+-------------------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.12)

    +---------+------+-----------+-----------------+-------------+
    | Cluster | Core | Powerdown |      Wakeup     | Cache Flush |
    +---------+------+-----------+-----------------+-------------+
    |    0    |  0   |   100.04  | 20.32 (-38.50%) |     5.62    |
    +---------+------+-----------+-----------------+-------------+
    |    0    |  1   |   99.78   |  20.6 (-36.10%) |     5.42    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  0   |   278.28  |      19.52      |     4.32    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  1   |    97.3   |      19.44      |     4.26    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  2   |   97.56   |      19.52      |     4.32    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  3   |   97.52   |      19.46      |     4.26    |
    +---------+------+-----------+-----------------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.11)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   106.78  |  19.2  |     5.32    |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   107.44  | 19.64  |     5.44    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   295.82  | 19.14  |     4.34    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   104.34  | 19.18  |     4.28    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   103.96  | 19.34  |     4.4     |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   104.32  | 19.18  |     4.34    |
    +---------+------+-----------+--------+-------------+

``CPU_OFF`` on all non-lead CPUs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CPU_OFF`` on all non-lead CPUs in sequence then, ``CPU_SUSPEND`` on the lead
core to the deepest power level.

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.12)

    +---------+------+-----------+-----------------+-------------+
    | Cluster | Core | Powerdown |      Wakeup     | Cache Flush |
    +---------+------+-----------+-----------------+-------------+
    |    0    |  0   |   236.3   | 30.88 (-29.30%) |    137.76   |
    +---------+------+-----------+-----------------+-------------+
    |    0    |  1   |   236.66  |  30.5 (-29.23%) |    138.02   |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  0   |   175.9   |       27.0      |    77.86    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  1   |   100.96  |      27.56      |     4.26    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  2   |   101.04  |      26.48      |     4.38    |
    +---------+------+-----------+-----------------+-------------+
    |    1    |  3   |   101.08  |      26.74      |     4.4     |
    +---------+------+-----------+-----------------+-------------+

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.11)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   243.62  | 29.84  |    137.66   |
    +---------+------+-----------+--------+-------------+
    |    0    |  1   |   243.88  | 29.54  |    137.8    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   183.26  | 26.22  |    77.76    |
    +---------+------+-----------+--------+-------------+
    |    1    |  1   |   107.64  | 26.74  |     4.34    |
    +---------+------+-----------+--------+-------------+
    |    1    |  2   |   107.52  |  25.9  |     4.32    |
    +---------+------+-----------+--------+-------------+
    |    1    |  3   |   107.74  |  25.8  |     4.34    |
    +---------+------+-----------+--------+-------------+

``CPU_VERSION`` in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (2.12)

    +-------------+--------+--------------+
    |   Cluster   |  Core  |   Latency    |
    +-------------+--------+--------------+
    |      0      |   0    |     1.0      |
    +-------------+--------+--------------+
    |      0      |   1    |     1.02     |
    +-------------+--------+--------------+
    |      1      |   0    |     0.52     |
    +-------------+--------+--------------+
    |      1      |   1    |     0.94     |
    +-------------+--------+--------------+
    |      1      |   2    |     0.94     |
    +-------------+--------+--------------+
    |      1      |   3    |     0.92     |
    +-------------+--------+--------------+

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (2.11)

    +-------------+--------+--------------+
    |   Cluster   |  Core  |   Latency    |
    +-------------+--------+--------------+
    |      0      |   0    |     1.26     |
    +-------------+--------+--------------+
    |      0      |   1    |     0.96     |
    +-------------+--------+--------------+
    |      1      |   0    |     0.54     |
    +-------------+--------+--------------+
    |      1      |   1    |     0.94     |
    +-------------+--------+--------------+
    |      1      |   2    |     0.92     |
    +-------------+--------+--------------+
    |      1      |   3    |     1.02     |
    +-------------+--------+--------------+

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

*Copyright (c) 2019-2024, Arm Limited and Contributors. All rights reserved.*

.. _Juno R1 platform: https://developer.arm.com/documentation/100122/latest/
.. _TF master as of 31/01/2017: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/?id=c38b36d
.. _TF-A v2.12-rc0: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/?h=v2.12-rc0
.. _TFTF v2.12-rc0: https://git.trustedfirmware.org/TF-A/tf-a-tests.git/tree/?h=v2.12-rc0
