Runtime Instrumentation Testing - N1SDP
=======================================

For this test we used the N1 System Development Platform (`N1SDP`_), which
contains an SoC consisting of two dual-core Arm N1 clusters.

The following source trees and binaries were used:

- `TF-A v2.12-rc0`_
- `TFTF v2.12-rc0`_
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

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in parallel (v2.12)

    +---------+------+----------------+--------+----------------+
    | Cluster | Core |   Powerdown    | Wakeup |  Cache Flush   |
    +---------+------+----------------+--------+----------------+
    |    0    |  0   |      2.58      | 24.14  | 0.28 (-69.57%) |
    +---------+------+----------------+--------+----------------+
    |    0    |  0   | 4.24 (-32.27%) |  40.1  |      0.3       |
    +---------+------+----------------+--------+----------------+
    |    1    |  0   |      3.58      | 35.54  |      0.28      |
    +---------+------+----------------+--------+----------------+
    |    1    |  0   |      3.28      | 42.36  |      0.3       |
    +---------+------+----------------+--------+----------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in parallel (v2.11)

    +---------+------+----------------+--------+----------------+
    | Cluster | Core |   Powerdown    | Wakeup |  Cache Flush   |
    +---------+------+----------------+--------+----------------+
    |    0    |  0   | 3.0 (+41.51%)  | 23.14  | 1.2 (+185.71%) |
    +---------+------+----------------+--------+----------------+
    |    0    |  0   |      4.6       | 35.86  |      0.3       |
    +---------+------+----------------+--------+----------------+
    |    1    |  0   | 3.68 (+33.33%) | 33.36  |      0.3       |
    +---------+------+----------------+--------+----------------+
    |    1    |  0   | 3.7 (+40.15%)  |  38.1  |      0.28      |
    +---------+------+----------------+--------+----------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in serial (v2.12)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    1.9    |  23.8  |     0.36    |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    2.26   | 23.86  |     0.34    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    2.02   |  23.4  |     0.36    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    2.24   | 23.84  |     0.36    |
    +---------+------+-----------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in serial (v2.11)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    1.7    | 22.46  |     0.3     |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    2.28   |  22.5  |     0.3     |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    2.14   |  21.5  |     0.32    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    2.24   | 22.66  |     0.3     |
    +---------+------+-----------+--------+-------------+

``CPU_SUSPEND`` to power level 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in parallel (v2.12)

    +---------+------+-----------+--------+----------------+
    | Cluster | Core | Powerdown | Wakeup |  Cache Flush   |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |    1.46   |  31.7  |      0.32      |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |    2.06   |  35.5  | 0.48 (+60.00%) |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |    1.96   |  35.7  |      0.32      |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |    2.08   | 23.38  |      0.28      |
    +---------+------+-----------+--------+----------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in parallel (v2.11)

    +---------+------+----------------+--------+-------------+
    | Cluster | Core |   Powerdown    | Wakeup | Cache Flush |
    +---------+------+----------------+--------+-------------+
    |    0    |  0   | 0.94 (-37.33%) | 30.36  |     0.3     |
    +---------+------+----------------+--------+-------------+
    |    0    |  0   |      2.12      | 33.12  |     0.28    |
    +---------+------+----------------+--------+-------------+
    |    1    |  0   |      2.08      | 32.56  |     0.3     |
    +---------+------+----------------+--------+-------------+
    |    1    |  0   |      2.14      | 21.92  |     0.28    |
    +---------+------+----------------+--------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.12)

    +---------+------+-----------+--------+----------------+
    | Cluster | Core | Powerdown | Wakeup |  Cache Flush   |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |    1.66   | 23.22  |      0.36      |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |    2.58   | 23.72  | 0.78 (+85.71%) |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |    2.02   | 23.84  |      0.38      |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |    2.16   | 23.92  |      0.34      |
    +---------+------+-----------+--------+----------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial (v2.11)

    +---------+------+-----------+--------+-------------+
    | Cluster | Core | Powerdown | Wakeup | Cache Flush |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    1.64   | 21.88  |     0.34    |
    +---------+------+-----------+--------+-------------+
    |    0    |  0   |    2.42   | 21.76  |     0.34    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    2.02   | 21.14  |     0.32    |
    +---------+------+-----------+--------+-------------+
    |    1    |  0   |    2.18   |  22.3  |     0.34    |
    +---------+------+-----------+--------+-------------+

``CPU_OFF`` on all non-lead CPUs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CPU_OFF`` on all non-lead CPUs in sequence then, ``CPU_SUSPEND`` on the lead
core to the deepest power level.

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.12)

    +---------+------+-----------+--------+----------------+
    | Cluster | Core | Powerdown | Wakeup |  Cache Flush   |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |    1.84   | 23.82  |      0.36      |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |   14.18   | 31.78  | 0.56 (+86.67%) |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |   13.64   | 30.54  |      0.36      |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |   14.18   | 31.82  |      0.68      |
    +---------+------+-----------+--------+----------------+

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs (v2.11)

    +---------+------+-----------+--------+----------------+
    | Cluster | Core | Powerdown | Wakeup |  Cache Flush   |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |    1.96   | 22.44  |      0.38      |
    +---------+------+-----------+--------+----------------+
    |    0    |  0   |   13.76   | 30.34  |      0.26      |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |   13.46   | 28.28  |      0.24      |
    +---------+------+-----------+--------+----------------+
    |    1    |  0   |   13.84   | 30.06  | 0.28 (-60.00%) |
    +---------+------+-----------+--------+----------------+

``CPU_VERSION`` in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (v2.12)

    +----------+------+-------------------+
    | Cluster  | Core |      Latency      |
    +----------+------+-------------------+
    |    0     |  0   |        0.14       |
    +----------+------+-------------------+
    |    0     |  0   |   0.2 (-28.57%)   |
    +----------+------+-------------------+
    |    1     |  0   |        0.2        |
    +----------+------+-------------------+
    |    1     |  0   |        0.26       |
    +----------+------+-------------------+

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores (v2.11)

    +-------------+--------+--------------+
    |   Cluster   |  Core  |   Latency    |
    +-------------+--------+--------------+
    |      0      |   0    |     0.12     |
    +-------------+--------+--------------+
    |      0      |   0    |     0.24     |
    +-------------+--------+--------------+
    |      1      |   0    |     0.2      |
    +-------------+--------+--------------+
    |      1      |   0    |     0.26     |
    +-------------+--------+--------------+

--------------

*Copyright (c) 2023-2024, Arm Limited. All rights reserved.*

.. _TF-A v2.12-rc0: https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.12-rc0
.. _TFTF v2.12-rc0: https://review.trustedfirmware.org/plugins/gitiles/TF-A/tf-a-tests/+/refs/tags/v2.12-rc0
.. _user guide: https://gitlab.arm.com/arm-reference-solutions/arm-reference-solutions-docs/-/blob/master/docs/n1sdp/user-guide.rst
.. _Prebuilt Images:  https://downloads.trustedfirmware.org/tf-a/css_scp_2.12.0/n1sdp/release/
.. _N1SDP: https://developer.arm.com/documentation/101489/latest
