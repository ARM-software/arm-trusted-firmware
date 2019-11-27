Test Secure Payload (TSP) and Dispatcher (TSPD)
===============================================

Building the Test Secure Payload
--------------------------------

The TSP is coupled with a companion runtime service in the BL31 firmware,
called the TSPD. Therefore, if you intend to use the TSP, the BL31 image
must be recompiled as well. For more information on SPs and SPDs, see the
:ref:`firmware_design_sel1_spd` section in the :ref:`Firmware Design`.

First clean the TF-A build directory to get rid of any previous BL31 binary.
Then to build the TSP image use:

.. code:: shell

    make PLAT=<platform> SPD=tspd all

An additional boot loader binary file is created in the ``build`` directory:

::

    build/<platform>/<build-type>/bl32.bin

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*
