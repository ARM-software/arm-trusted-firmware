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

You can also create a TL from a YAML config file.

.. code ::

    tlc create --from-yaml config.yaml tl.bin

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

YAML Config File Format
~~~~~~~~~~~~~~~~~~~~~~~

Example YAML config file:

.. code::

    execution_state: aarch32
    has_checksum: true
    max_size: 4096
    entries:
            - tag_id: 258  # entry point info
              ep_info:
                      args:
                              - 67112968
                              - 67112960
                              - 0
                              - 0
                              - 0
                              - 0
                              - 0
                              - 0
                      h:
                              attr: 8
                              type: 1
                              version: 2
                      pc: 67239936
                      spsr: 467
            - tag_id: 3  # memory layout
              addr: 8
              size: 8
            - tag_id: 1,  # fdt
              blob_file_path: "fdt.bin",

`max_size` defaults to `0x1000`, `execution_state` defaults to `aarch64`, and `has_checksum`
defaults to `true`.

The fields of the YAML file should match the fields in the specification for the transfer list. You
don't need to give the hdr_size or data_size fields. For example, a memory layout entry would have
an entry like:

.. code::

    tag_id: 3
    addr: 8
    size: 8

You can input blob files by giving paths to the current working directory. You can do this for any
TE type. For example, an FDT layout would have an entry like:

.. code::

    tag_id: 1,
    blob_file_path: "fdt.bin",

You can input C-types by giving its fields. For example, an entry point
info entry would have an entry like:

.. code::

    tag_id: 258
    ep_info:
            args:
                    - 67112968
                    - 67112960
                    - 0
                    - 0
            h:
                    attr: 8
                    type: 1
                    version: 2
            lr_svc: 0
            pc: 67239936
            spsr: 467

You can give the name of the tag instead of the tag id number. The valid tag names are in the
`transfer_entry_formats` dict in `tools/tlc/tlc/tl.py`_. Some examples are:

* empty
* fdt
* hob_block
* hob_list

You can input the attr field of entry_point_info as a string of flag
names separated by `|`. The names are taken from ep_info_exp.h in TF-A.
For example:

.. code::

    has_checksum: true
    max_size: 4096
    entries:
    - tag_id: 0x102
      ep_info:
        args:
        - 67112976
        - 67112960
        - 0
        - 0
        - 0
        - 0
        - 0
        - 0
        h:
          attr: EP_NON_SECURE | EP_ST_ENABLE
          type: 1
          version: 2
        pc: 67239936
        spsr: 965

--------------

*Copyright (c) 2024, Arm Limited. All rights reserved.*

.. _Firmware Handoff specification: https://github.com/FirmwareHandoff/firmware_handoff/
.. _tools/tlc/pyproject.toml: https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/heads/master/tools/tlc/pyproject.toml
.. _tools/tlc/tlc/tl.py: https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/heads/master/tools/tlc/tlc/tl.py
