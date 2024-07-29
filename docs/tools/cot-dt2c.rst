TF-A CoT dt2c Tool
==================

This tool is used to automatically generate the corresponding c file for a
CoT DT file. Since currently TF-A support two type of CoT file: static c file
and CoT DT binding. This is error prone and hard to maintain, therefore this
tool can generate the c file for the platform that does not support CoT DT
binding, given the CoT DT file so the c file can be deprecated.

Prerequisites
~~~~~~~~~~~~~

#. Python (3.8 or later)
#. `Poetry`_ Python package manager


Getting Started
~~~~~~~~~~~~~~~

#. Install the tool

    .. code::

        make install


#. Verify that the tool runs correctly

    .. code::

        make test


#. Usage of the tool

    .. code::

        cot-dt2c

    This command will output the following as usage for this command

    .. code-block:: text

        Usage: cot-dt2c [OPTIONS] COMMAND [ARGS]...

        Options:
        --version  Show the version and exit.
        --help     Show this message and exit.

        Commands:
        convert-to-c
        validate-cot
        visualize-cot
        validate-dt

#. Uninstall the tool
    .. code::

        make uninstall

    This command will uninstall the tool


#. Uninstall the tool and clean all the build file
    .. code::

        make clean

    This command will clean all the build file and implicitly uninstall the tool


#. Call the make file from TF-A root directory
    .. code::

        make -C tools/cot-dt2c install
        make -C tools/cot-dt2c uninstall
        make -C tools/cot-dt2c clean

Convert CoT descriptors to C file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To convert the CoT descriptors

This command is for the platform that does not use CoT DT parser,
which can generate the C file given the CoT descriptors. Before
the conversion to C file, the tool will do an implicit checks on
the validity of the CoT DT file.

.. code::

    cot-dt2c convert-to-c [INPUT DTB PATH] [OUTPUT C PATH]
    cot-dt2c convert-to-c fdts/tbbr_cot_descriptors.dtsi test.c


Validate CoT descriptors
~~~~~~~~~~~~~~~~~~~~~~~~~

To validate the certificate

The tests folder in the tool folder provides some bad-example of the
DT file, and the tool will print out "not a valid CoT DT file" on console.

The command will check the format of the CoT file

#. The open bracket
#. The open ifdef macro
#. The missing mandatory attribute
#. Malformed DT file (cert missing parent, missing root certs. etc.)

Currently the validation is specifically for checking the CoT DT file

.. code::

    cot-dt2c validate-cot [INPUT DTB PATH]
    cot-dt2c validate-cot fdts/tbbr_cot_descriptors.dtsi


Visualize CoT descriptors
~~~~~~~~~~~~~~~~~~~~~~~~~

This command create a HTML to visualize the relationship between
the certificates and the image of a CoT DT file.

.. code::

    cot-dt2c visualize-cot [INPUT DTB PATH]
    cot-dt2c visualize-cot fdts/tbbr_cot_descriptors.dtsi


Validate Other DT files
~~~~~~~~~~~~~~~~~~~~~~~

The command will transform the dtsi/dts file into a more standard
dtsi/dts file inside /tmp folder that can be used as input to dt-schema
for further validation. Currently the tool will perform some basic validation
for the file (syntax) and dt-schema can be used for advance checks. dt-schema
is not installed along with the tool.

.. code::

    cot-dt2c validate-dt [INPUT DTS PATH or INPUT DTS folder]
    cot-dt2c validate-dt fdts/
    cot-dt2c validate-dt fdts/fvp-bsae-gicv3.dtsi

--------------

*Copyright (c) 2024, Arm Limited. All rights reserved.*

.. _Poetry: https://python-poetry.org/docs/
