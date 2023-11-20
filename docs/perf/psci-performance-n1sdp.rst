Runtime Instrumentation Testing - N1SDP
=======================================

For this test we used the N1 System Development Platform (`N1SDP`_), which
contains an SoC consisting of two dual-core Arm N1 clusters.

The following source trees and binaries were used:

- TF-A [`v2.9-rc0-16-g666aec401`_]
- TFTF [`v2.9-rc0`_]
- SCP/MCP `Prebuilt Images`_

Please see the Runtime Instrumentation :ref:`Testing Methodology
<Runtime Instrumentation Methodology>` page for more details.

Procedure
---------

#. Build TFTF with runtime instrumentation enabled:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=n1sdp \
            TESTS=runtime-instrumentation all

#. Build TF-A with the following build options:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=n1sdp \
            ENABLE_RUNTIME_INSTRUMENTATION=1 fiptool all

#. Fetch the SCP firmware images:

    .. code:: shell

        curl --fail --connect-timeout 5 --retry 5 \
            -sLS -o build/n1sdp/release/scp_rom.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.12.0/n1sdp/release/n1sdp-bl1.bin
        curl --fail --connect-timeout 5 \
            --retry 5 -sLS -o build/n1sdp/release/scp_ram.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.12.0/n1sdp/release/n1sdp-bl2.bin

#. Fetch the MCP firmware images:

    .. code:: shell

        curl --fail --connect-timeout 5 --retry 5 \
            -sLS -o build/n1sdp/release/mcp_rom.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.12.0/n1sdp/release/n1sdp-mcp-bl1.bin
        curl --fail --connect-timeout 5 --retry 5 \
            -sLS -o build/n1sdp/release/mcp_ram.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.12.0/n1sdp/release/n1sdp-mcp-bl2.bin

#. Using the fiptool, create a new FIP package and append the SCP ram image onto
   it.

    .. code:: shell

        ./tools/fiptool/fiptool create --blob \
                uuid=cfacc2c4-15e8-4668-82be-430a38fad705,file=build/n1sdp/release/bl1.bin \
                --scp-fw build/n1sdp/release/scp_ram.bin build/n1sdp/release/scp_fw.bin

#. Append the MCP image to the FIP.

    .. code:: shell

        ./tools/fiptool/fiptool create \
            --blob uuid=54464222-a4cf-4bf8-b1b6-cee7dade539e,file=build/n1sdp/release/mcp_ram.bin \
            build/n1sdp/release/mcp_fw.bin

#. Then, add TFTF as the Non-Secure workload in the FIP image:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=n1sdp \
            ENABLE_RUNTIME_INSTRUMENTATION=1 SCP_BL2=/dev/null \
            BL33=<path/to/tftf.bin>  fip

#. Load the following images onto the development board: ``fip.bin``,
   ``scp_rom.bin``, ``scp_ram.bin``, ``mcp_rom.bin``, and ``mcp_ram.bin``.

.. note::

    These instructions presume you have a complete firmware stack. The N1SDP
    `user guide`_ provides a detailed explanation on how to get setup from
    scratch.

Results
-------

``CPU_SUSPEND`` to deepest power level
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        parallel (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    2.80   | 10.08  |     0.80    |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    4.14   | 15.92  |     0.16    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    3.68   | 12.96  |     0.16    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    3.36   | 18.58  |     0.18    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        parallel (v2.10)

    +---------+------+----------------+------------------+-----------------+
    | Cluster | Core |   Powerdown    |      Wakeup      |   Cache Flush   |
    +---------+------+----------------+------------------+-----------------+
    |    0    |  0   |      2.12      | 23.94 (+137.50%) |  0.42 (-47.50%) |
    +---------+------+----------------+------------------+-----------------+
    |    0    |  0   |      3.52      | 42.08 (+164.32%) |  0.26 (+62.50%) |
    +---------+------+----------------+------------------+-----------------+
    |    1    |  0   | 2.76 (-25.00%) | 38.3 (+195.52%)  |  0.26 (+62.50%) |
    +---------+------+----------------+------------------+-----------------+
    |    1    |  0   |      2.64      | 44.56 (+139.83%) | 0.36 (+100.00%) |
    +---------+------+----------------+------------------+-----------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        serial (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    1.86   |  9.92  |     0.32    |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    2.70   | 10.48  |     0.36    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    1.78   |  9.72  |     0.16    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    1.94   | 10.44  |     0.16    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        serial (v2.10)

    +---------+------+-----------+------------------+----------------+
    | Cluster | Core | Powerdown |      Wakeup      |  Cache Flush   |
    +---------+------+-----------+------------------+----------------+
    |    0    |  0   |    1.74   | 23.7 (+138.91%)  |      0.3       |
    +---------+------+-----------+------------------+----------------+
    |    0    |  0   |    2.08   | 23.96 (+128.63%) | 0.26 (-27.78%) |
    +---------+------+-----------+------------------+----------------+
    |    1    |  0   |    1.9    | 23.62 (+143.00%) | 0.28 (+75.00%) |
    +---------+------+-----------+------------------+----------------+
    |    1    |  0   |    2.06   | 23.92 (+129.12%) | 0.26 (+62.50%) |
    +---------+------+-----------+------------------+----------------+

``CPU_SUSPEND`` to power level 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in
        parallel (v2.9)

    +---------------------------------------------------+
    |          test_rt_instr_cpu_susp_parallel          |
    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    0.88   | 12.32  |     0.26    |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    2.12   | 14.62  |     0.26    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    1.86   | 14.14  |     0.16    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    1.92   |  9.44  |     0.18    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in
        parallel (v2.10)

    +---------+------+---------------+------------------+----------------+
    | Cluster | Core |   Powerdown   |      Wakeup      |  Cache Flush   |
    +---------+------+---------------+------------------+----------------+
    |    0    |  0   | 1.5 (+70.45%) | 35.02 (+184.25%) |      0.24      |
    +---------+------+---------------+------------------+----------------+
    |    0    |  0   |      1.92     | 38.12 (+160.74%) |      0.28      |
    +---------+------+---------------+------------------+----------------+
    |    1    |  0   |      1.88     | 38.1 (+169.45%)  | 0.26 (+62.50%) |
    +---------+------+---------------+------------------+----------------+
    |    1    |  0   |      2.04     | 23.1 (+144.70%)  |      0.24      |
    +---------+------+---------------+------------------+----------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.9)

    +---------------------------------------------------+
    |           test_rt_instr_cpu_susp_serial           |
    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    1.52   |  9.40  |     0.30    |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    1.92   |  9.80  |     0.18    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    2.20   |  9.60  |     0.14    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    1.82   |  9.78  |     0.18    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.10)

    +---------+------+-----------+------------------+-----------------+
    | Cluster | Core | Powerdown |      Wakeup      |   Cache Flush   |
    +---------+------+-----------+------------------+-----------------+
    |    0    |  0   |    1.52   | 23.08 (+145.53%) |       0.3       |
    +---------+------+-----------+------------------+-----------------+
    |    0    |  0   |    1.98   | 23.68 (+141.63%) |  0.28 (+55.56%) |
    +---------+------+-----------+------------------+-----------------+
    |    1    |  0   |    1.84   | 23.86 (+148.54%) | 0.28 (+100.00%) |
    +---------+------+-----------+------------------+-----------------+
    |    1    |  0   |    1.98   | 23.68 (+142.13%) |  0.28 (+55.56%) |
    +---------+------+-----------+------------------+-----------------+

``CPU_OFF`` on all non-lead CPUs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CPU_OFF`` on all non-lead CPUs in sequence then, ``CPU_SUSPEND`` on the lead
core to the deepest power level.

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.9)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    1.84   |  9.94  |     0.32    |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |   14.20   | 13.10  |     0.50    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   13.88   | 12.36  |     0.42    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |   14.40   | 13.26  |     0.52    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.10)

    +---------+------+-----------+------------------+----------------+
    | Cluster | Core | Powerdown |      Wakeup      |  Cache Flush   |
    +---------+------+-----------+------------------+----------------+
    |    0    |  0   |    1.78   | 23.7 (+138.43%)  |      0.3       |
    +---------+------+-----------+------------------+----------------+
    |    0    |  0   |   13.96   | 31.16 (+137.86%) | 0.34 (-32.00%) |
    +---------+------+-----------+------------------+----------------+
    |    1    |  0   |   13.54   | 30.24 (+144.66%) | 0.26 (-38.10%) |
    +---------+------+-----------+------------------+----------------+
    |    1    |  0   |   14.46   | 31.12 (+134.69%) | 0.7 (+34.62%)  |
    +---------+------+-----------+------------------+----------------+

``CPU_VERSION`` in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (v2.9)

    +------------------------------------+
    | test_rt_instr_psci_version_parallel|
    +-------------+--------+-------------+
    |   Cluster   |  Core  |   Latency   |
    +-------------+--------+-------------+
    |      0      |   0    |     0.08    |
    +-------------+--------+-------------+
    |      0      |   0    |     0.26    |
    +-------------+--------+-------------+
    |      1      |   0    |     0.20    |
    +-------------+--------+-------------+
    |      1      |   0    |     0.26    |
    +-------------+--------+-------------+

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (v2.10)

    +----------------------------------------------+
    | test_rt_instr_psci_version_parallel (latest) |
    +-------------+--------+-----------------------+
    |   Cluster   |  Core  |        Latency        |
    +-------------+--------+-----------------------+
    |      0      |   0    |     0.14 (+75.00%)    |
    +-------------+--------+-----------------------+
    |      0      |   0    |          0.22         |
    +-------------+--------+-----------------------+
    |      1      |   0    |          0.2          |
    +-------------+--------+-----------------------+
    |      1      |   0    |          0.26         |
    +-------------+--------+-----------------------+

--------------

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _v2.9-rc0-16-g666aec401: https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/heads/v2.9-rc0-16-g666aec401
.. _v2.9-rc0: https://review.trustedfirmware.org/plugins/gitiles/TF-A/tf-a-tests/+/refs/tags/v2.9-rc0
.. _user guide: https://gitlab.arm.com/arm-reference-solutions/arm-reference-solutions-docs/-/blob/master/docs/n1sdp/user-guide.rst
.. _Prebuilt Images:  https://downloads.trustedfirmware.org/tf-a/css_scp_2.11.0/n1sdp/release/
.. _N1SDP: https://developer.arm.com/documentation/101489/latest
