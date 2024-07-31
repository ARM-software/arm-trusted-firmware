Transfer List Compiler
======================

The Transfer List Compiler (tlc) is a host tool used by TF-A to generate transfer
lists compliant with the v0.9 of the `Firmware Handoff specification`_. It enables
developers to statically generate transfer list blobs containing any number of
transfer entries.

Getting Started
~~~~~~~~~~~~~~~

``tlc`` is installed by default with TF-A's poetry environment. All of it's
dependencies are listed in `tools/tlc/pyproject.toml`_.

To install ``tlc`` seperately, run the following command:

.. code::

    make -C tools/tlc install

Creating a Transfer List
~~~~~~~~~~~~~~~~~~~~~~~~

To create an empty TL, you can use the ``create`` command.

.. code::

    tlc create tl.bin

This commands generates a binary blob representing an empty TL, shown in the
hexdump below.

.. code::

    $ hexdump tl.bin | head
    0000000 b10b 4a0f 01a6 0318 0018 0000 1000 0000
    0000010 0001 0000 0000 0000

A common use-case this tool supports is the addition of TE's via the option
``--entry``. This takes as input the tag ID and path to a binary blob to be
included in the transfer list. The snippet below shows how to include an FDT in
the TL.

.. code::

    tlc create --entry 1 fdt.dtb tl.bin

Alternatively, addition of a device tree is supported through the option
``--fdt``. This has the same effect as passing the device tree and it's tag ID
through the ``--entry`` option.

.. code::

    tlc create --fdt fdt.dtb tl.bin

.. note::

    ``tlc`` makes no effort to verify the contents of a binary blob against the
    provided tag ID. It only checks that the tags provided as input are within
    range and that there is sufficient memory to include their TE's.

Printing the contents of a TL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Support is provided for dumping the contents of a TL via the ``info`` command.
This prints the header of the TL and all included TE's.

.. code::

    $ tlc info tl.bin
    signature  0x4a0fb10b
    checksum   0xe1
    version    0x1
    hdr_size   0x18
    alignment  0x3
    size       0x2a6f
    total_size 0x4e20
    flags      0x1
    ----
    id         0x1
    data_size  0x2a47
    hdr_size   0x8
    offset     0x18
    ----
    id         0x0
    data_size  0x0
    hdr_size   0x8
    offset     0x2a68

The example above shows the dump produced by ``tlc`` for a 20Kb TL containing a
device tree (tag_id=1) and a NULL entry (tag_id=0).

Modifying the contents of an existing TL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`tlc` supports removal of one or more entries from a TL through the ``remove``
command. It takes as argument the filename, and one or more tag ID's, passed
through the ``--tags`` option.  It produces a valid TL blob without those
entries.


For example, using the same blob as in the section above, we can remove the FDT
TE with the command.

.. code::

    $ tlc remove --tags 1 tl.bin

Using the ``info`` command, shows the the TE has been remove:

.. code::

    $ tlc info tl.bin

    signature  0x4a0fb10b
    checksum   0x38
    version    0x1
    hdr_size   0x18
    alignment  0x3
    size       0x20
    total_size 0x4e20
    flags      0x1
    ----
    id         0x0
    data_size  0x0
    hdr_size   0x8
    offset     0x18

Note that more than one entry can be removed at a time. The ``--tags`` option
accepts multiple tag ID's.

Conversely, TE's can be added to an existing TL. This is achieved through the
`add` command.

.. code::

    $ tlc add --entry 1 fdt.dtb tl.bin


The result of this modification is shown below:

.. code::

    $ tlc info tl.bin

    signature  0x4a0fb10b
    checksum   0xe1
    version    0x1
    hdr_size   0x18
    alignment  0x3
    size       0x2a6f
    total_size 0x4e20
    flags      0x1
    ----
    id         0x0
    data_size  0x0
    hdr_size   0x8
    offset     0x18
    ----
    id         0x1
    data_size  0x2a47
    hdr_size   0x8
    offset     0x20

Unpacking a Transfer List
~~~~~~~~~~~~~~~~~~~~~~~~~

Given a transfer list, ``tlc`` also provides a mechanism for extracting TE data.
Running the command ``unpack``, yields binary files containing data from all the TE's.

.. code::

    $ tlc create --size 20000 --fdt build/fvp/debug/fdts/fvp-base-gicv3-psci.dtb tl.bin
    $ tlc unpack tl.bin
    $ file te_1.bin
    te_1.bin: Device Tree Blob version 17, size=10823, boot CPU=0, string block size=851, DT structure block size=9900

Validate a Transfer List
~~~~~~~~~~~~~~~~~~~~~~~~

``tlc validate`` provides a quick and simple mechanism for checking wether the TL
is compliant with version of the specification supported by the tool. It
performs the following checks:

#. Validates the signature.
#. Ensures that the specified version is greater than or equal to the tool’s current version.
#. Verifies alignment criteria for all TE’s.

--------------

*Copyright (c) 2024, Arm Limited. All rights reserved.*

.. _Firmware Handoff specification: https://github.com/FirmwareHandoff/firmware_handoff/
.. _tools/tlc/pyproject.toml: https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/heads/master/tools/tlc/pyproject.toml
