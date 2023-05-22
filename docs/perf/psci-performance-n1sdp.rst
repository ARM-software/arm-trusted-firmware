Runtime Instrumentation Testing - N1SDP
=======================================

For this test we used the N1 System Development Platform (`N1SDP`_), which
contains an SoC consisting of two dual-core Arm N1 clusters.

The following source trees and binaries were used:

- TF-A [`v2.9-rc0-16-g666aec401`_]
- TFTF [`v2.9-rc0`_]
- SCP/MCP `Prebuilt Images`_

Please see the Runtime Instrumentation `Testing Methodology`_ page for more
details.

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
        parallel

    +---------+------+-----------+---------+-------------+
    | Cluster | Core | Powerdown | Wakekup | Cache Flush |
    +=========+======+===========+=========+=============+
    |    0    |  0   |    3.44   |  10.04  |     0.4     |
    +---------+------+-----------+---------+-------------+
    |    0    |  1   |    4.98   |  12.72  |     0.16    |
    +---------+------+-----------+---------+-------------+
    |    1    |  0   |    3.58   |  15.42  |     0.2     |
    +---------+------+-----------+---------+-------------+
    |    1    |  1   |    5.24   |  17.78  |     0.18    |
    +---------+------+-----------+---------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to deepest power level in
        serial

    +---------+------+-----------+---------+-------------+
    | Cluster | Core | Powerdown | Wakekup | Cache Flush |
    +=========+======+===========+=========+=============+
    |    0    |  0   |    1.82   |   9.98  |     0.32    |
    +---------+------+-----------+---------+-------------+
    |    0    |  1   |    1.96   |   9.96  |     0.18    |
    +---------+------+-----------+---------+-------------+
    |    1    |  0   |    2.0    |   10.5  |     0.16    |
    +---------+------+-----------+---------+-------------+
    |    1    |  1   |    2.22   |  10.56  |     0.16    |
    +---------+------+-----------+---------+-------------+

``CPU_SUSPEND`` to power level 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in
        parallel

    +---------+------+-----------+---------+-------------+
    | Cluster | Core | Powerdown | Wakekup | Cache Flush |
    +=========+======+===========+=========+=============+
    |    0    |  0   |    1.52   |  11.84  |     0.34    |
    +---------+------+-----------+---------+-------------+
    |    0    |  1   |    1.1    |  13.66  |     0.14    |
    +---------+------+-----------+---------+-------------+
    |    1    |  0   |    2.18   |   9.48  |     0.18    |
    +---------+------+-----------+---------+-------------+
    |    1    |  1   |    2.06   |   14.4  |     0.16    |
    +---------+------+-----------+---------+-------------+

.. table:: ``CPU_SUSPEND`` latencies (µs) to power level 0 in serial

    +---------+------+-----------+---------+-------------+
    | Cluster | Core | Powerdown | Wakekup | Cache Flush |
    +=========+======+===========+=========+=============+
    |    0    |  0   |    1.54   |   9.34  |     0.3     |
    +---------+------+-----------+---------+-------------+
    |    0    |  1   |    1.88   |   9.5   |     0.16    |
    +---------+------+-----------+---------+-------------+
    |    1    |  0   |    1.86   |   9.86  |     0.2     |
    +---------+------+-----------+---------+-------------+
    |    1    |  1   |    2.02   |   9.64  |     0.18    |
    +---------+------+-----------+---------+-------------+

``CPU_OFF`` on all non-lead CPUs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CPU_OFF`` on all non-lead CPUs in sequence then, ``CPU_SUSPEND`` on the lead
core to the deepest power level.

.. table:: ``CPU_OFF`` latencies (µs) on all non-lead CPUs

    +---------+------+-----------+---------+-------------+
    | Cluster | Core | Powerdown | Wakekup | Cache Flush |
    +=========+======+===========+=========+=============+
    |    0    |  0   |    1.86   |   9.88  |     0.32    |
    +---------+------+-----------+---------+-------------+
    |    0    |  1   |    21.1   |  12.44  |     0.42    |
    +---------+------+-----------+---------+-------------+
    |    1    |  0   |   21.22   |   13.2  |     0.32    |
    +---------+------+-----------+---------+-------------+
    |    1    |  1   |   21.56   |  13.18  |     0.54    |
    +---------+------+-----------+---------+-------------+

``CPU_VERSION`` in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_VERSION`` latency (µs) in parallel on all cores

    +-------------+--------+--------------+
    |   Cluster   |  Core  |   Latency    |
    +=============+========+==============+
    |      0      |   0    |     0.08     |
    +-------------+--------+--------------+
    |      0      |   1    |     0.22     |
    +-------------+--------+--------------+
    |      1      |   0    |     0.28     |
    +-------------+--------+--------------+
    |      1      |   1    |     0.26     |
    +-------------+--------+--------------+

--------------

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _v2.9-rc0-16-g666aec401: https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/heads/v2.9-rc0-16-g666aec401
.. _v2.9-rc0: https://review.trustedfirmware.org/plugins/gitiles/TF-A/tf-a-tests/+/refs/tags/v2.9-rc0
.. _user guide: https://gitlab.arm.com/arm-reference-solutions/arm-reference-solutions-docs/-/blob/master/docs/n1sdp/user-guide.rst
.. _Prebuilt Images:  https://downloads.trustedfirmware.org/tf-a/css_scp_2.11.0/n1sdp/release/
.. _N1SDP: https://developer.arm.com/documentation/101489/latest
.. _Testing Methodology: ../perf/psci-performance-methodology.html