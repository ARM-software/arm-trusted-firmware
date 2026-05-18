Runtime Instrumentation Testing - N1SDP
=======================================

For this test we used the N1 System Development Platform (`N1SDP`_), which
contains an SoC consisting of two dual-core Arm N1 clusters. Testing was done
after the rc0 tag.

Please see the Runtime Instrumentation :ref:`Testing Methodology
<Runtime Instrumentation Methodology>` page for more details. The tests were ran
using the
`tf-psci-lava-instr/n1sdp-runtime-instrumentation,n1sdp-runtime-instrumentation:n1sdp-fip.tftf-firmware`
configuration in CI.

Results
-------

``CPU_SUSPEND`` to deepest power level
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to deepest power level in parallel (current release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/current/test_rt_instr_susp_deep_parallel.csv

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to deepest power level in parallel (previous release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/previous/test_rt_instr_susp_deep_parallel.csv

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to deepest power level in serial (current release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/current/test_rt_instr_susp_deep_serial.csv

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to deepest power level in serial (previous release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/previous/test_rt_instr_susp_deep_serial.csv

``CPU_SUSPEND`` to power level 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to power level 0 in parallel (current release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/current/test_rt_instr_cpu_susp_parallel.csv

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to power level 0 in parallel (previous release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/previous/test_rt_instr_cpu_susp_parallel.csv

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to power level 0 in serial (current release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/current/test_rt_instr_cpu_susp_serial.csv

.. csv-table:: ``CPU_SUSPEND`` latencies (ns) to power level 0 in serial (previous release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/previous/test_rt_instr_cpu_susp_serial.csv

``CPU_OFF`` on all non-lead CPUs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CPU_OFF`` on all non-lead CPUs in sequence then, ``CPU_SUSPEND`` on the lead
core to the deepest power level.

.. csv-table:: ``CPU_OFF`` latencies (ns) on all non-lead CPUs (current release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/current/test_rt_instr_cpu_off_serial.csv

.. csv-table:: ``CPU_OFF`` latencies (ns) on all non-lead CPUs (previous release)
    :header: Cluster, Core, Powerdown, Wakeup, Cache Flush
    :width: 95%
    :widths: 1, 1, 3, 3, 3
    :file: data/n1sdp/previous/test_rt_instr_cpu_off_serial.csv

``CPU_VERSION`` in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. csv-table:: ``CPU_VERSION`` latency (ns) in parallel on all cores (current release)
    :header: Cluster, Core, Latency
    :width: 95%
    :widths: 1, 1, 2
    :file: data/n1sdp/current/test_rt_instr_psci_version_parallel.csv

.. csv-table:: ``CPU_VERSION`` latency (ns) in parallel on all cores (previous release)
    :header: Cluster, Core, Latency
    :width: 95%
    :widths: 1, 1, 2
    :file: data/n1sdp/previous/test_rt_instr_psci_version_parallel.csv

--------------

*Copyright (c) 2023-2026, Arm Limited. All rights reserved.*

.. _user guide: https://gitlab.arm.com/arm-reference-solutions/arm-reference-solutions-docs/-/blob/master/docs/n1sdp/user-guide.rst
.. _N1SDP: https://developer.arm.com/documentation/101489/latest
