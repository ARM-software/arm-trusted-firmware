Building FIP images with support for Trusted Board Boot
=======================================================

Trusted Board Boot primarily consists of the following two features:

-  Image Authentication, described in :ref:`Trusted Board Boot`, and
-  Firmware Update, described in :ref:`Firmware Update (FWU)`

The following steps should be followed to build FIP and (optionally) FWU_FIP
images with support for these features:

#. Fulfill the dependencies of the ``mbedtls`` cryptographic and image parser
   modules by checking out a recent version of the `mbed TLS Repository`_. It
   is important to use a version that is compatible with TF-A and fixes any
   known security vulnerabilities. See `mbed TLS Security Center`_ for more
   information. See the :ref:`Prerequisites` document for the appropriate
   version of mbed TLS to use.

   The ``drivers/auth/mbedtls/mbedtls_*.mk`` files contain the list of mbed TLS
   source files the modules depend upon.
   ``include/drivers/auth/mbedtls/mbedtls_config.h`` contains the configuration
   options required to build the mbed TLS sources.

   Note that the mbed TLS library is licensed under the Apache version 2.0
   license. Using mbed TLS source code will affect the licensing of TF-A
   binaries that are built using this library.

#. To build the FIP image, ensure the following command line variables are set
   while invoking ``make`` to build TF-A:

   -  ``MBEDTLS_DIR=<path of the directory containing mbed TLS sources>``
   -  ``TRUSTED_BOARD_BOOT=1``
   -  ``GENERATE_COT=1``

   In the case of Arm platforms, the location of the ROTPK hash must also be
   specified at build time. The following locations are currently supported (see
   ``ARM_ROTPK_LOCATION`` build option):

   -  ``ARM_ROTPK_LOCATION=regs``: the ROTPK hash is obtained from the Trusted
      root-key storage registers present in the platform. On Juno, this
      registers are read-only. On FVP Base and Cortex models, the registers
      are read-only, but the value can be specified using the command line
      option ``bp.trusted_key_storage.public_key`` when launching the model.
      On Juno board, the default value corresponds to an ECDSA-SECP256R1 public
      key hash, whose private part is not currently available.

   -  ``ARM_ROTPK_LOCATION=devel_rsa``: use the default hash located in
      plat/arm/board/common/rotpk/arm_rotpk_rsa_sha256.bin. Enforce generation
      of the new hash if ROT_KEY is specified.

   -  ``ARM_ROTPK_LOCATION=devel_ecdsa``: use the default hash located in
      plat/arm/board/common/rotpk/arm_rotpk_ecdsa_sha256.bin. Enforce generation
      of the new hash if ROT_KEY is specified.

   Example of command line using RSA development keys:

   .. code:: shell

       MBEDTLS_DIR=<path of the directory containing mbed TLS sources> \
       make PLAT=<platform> TRUSTED_BOARD_BOOT=1 GENERATE_COT=1        \
       ARM_ROTPK_LOCATION=devel_rsa                                    \
       ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
       BL33=<path-to>/<bl33_image>                                     \
       all fip

   The result of this build will be the bl1.bin and the fip.bin binaries. This
   FIP will include the certificates corresponding to the Chain of Trust
   described in the TBBR-client document. These certificates can also be found
   in the output build directory.

#. The optional FWU_FIP contains any additional images to be loaded from
   Non-Volatile storage during the :ref:`Firmware Update (FWU)` process. To build the
   FWU_FIP, any FWU images required by the platform must be specified on the
   command line. On Arm development platforms like Juno, these are:

   -  NS_BL2U. The AP non-secure Firmware Updater image.
   -  SCP_BL2U. The SCP Firmware Update Configuration image.

   Example of Juno command line for generating both ``fwu`` and ``fwu_fip``
   targets using RSA development:

   ::

       MBEDTLS_DIR=<path of the directory containing mbed TLS sources> \
       make PLAT=juno TRUSTED_BOARD_BOOT=1 GENERATE_COT=1              \
       ARM_ROTPK_LOCATION=devel_rsa                                    \
       ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
       BL33=<path-to>/<bl33_image>                                     \
       SCP_BL2=<path-to>/<scp_bl2_image>                               \
       SCP_BL2U=<path-to>/<scp_bl2u_image>                             \
       NS_BL2U=<path-to>/<ns_bl2u_image>                               \
       all fip fwu_fip

   .. note::
      The BL2U image will be built by default and added to the FWU_FIP.
      The user may override this by adding ``BL2U=<path-to>/<bl2u_image>``
      to the command line above.

   .. note::
      Building and installing the non-secure and SCP FWU images (NS_BL1U,
      NS_BL2U and SCP_BL2U) is outside the scope of this document.

   The result of this build will be bl1.bin, fip.bin and fwu_fip.bin binaries.
   Both the FIP and FWU_FIP will include the certificates corresponding to the
   Chain of Trust described in the TBBR-client document. These certificates
   can also be found in the output build directory.

--------------

*Copyright (c) 2019-2020, Arm Limited. All rights reserved.*

.. _mbed TLS Repository: https://github.com/ARMmbed/mbedtls.git
.. _mbed TLS Security Center: https://tls.mbed.org/security
