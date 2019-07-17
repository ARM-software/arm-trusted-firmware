Performing an Initial Build
===========================

-  Before building TF-A, the environment variable ``CROSS_COMPILE`` must point
   to the Linaro cross compiler.

   For AArch64:

   .. code:: shell

       export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf-

   For AArch32:

   .. code:: shell

       export CROSS_COMPILE=<path-to-aarch32-gcc>/bin/arm-none-eabi-

   It is possible to build TF-A using Clang or Arm Compiler 6. To do so
   ``CC`` needs to point to the clang or armclang binary, which will
   also select the clang or armclang assembler. Be aware that for Arm Compiler,
   the GNU linker is used by default. However for Clang LLVM linker (LLD)
   is used by default. In case of being needed the linker can be overridden
   using the ``LD`` variable. LLVM linker (LLD) version 9 is
   known to work with TF-A.

   In both cases ``CROSS_COMPILE`` should be set as described above.

   Arm Compiler 6 will be selected when the base name of the path assigned
   to ``CC`` matches the string 'armclang'.

   For AArch64 using Arm Compiler 6:

   .. code:: shell

       export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf-
       make CC=<path-to-armclang>/bin/armclang PLAT=<platform> all

   Clang will be selected when the base name of the path assigned to ``CC``
   contains the string 'clang'. This is to allow both clang and clang-X.Y
   to work.

   For AArch64 using clang:

   .. code:: shell

       export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf-
       make CC=<path-to-clang>/bin/clang PLAT=<platform> all

-  Change to the root directory of the TF-A source tree and build.

   For AArch64:

   .. code:: shell

       make PLAT=<platform> all

   For AArch32:

   .. code:: shell

       make PLAT=<platform> ARCH=aarch32 AARCH32_SP=sp_min all

   Notes:

   -  If ``PLAT`` is not specified, ``fvp`` is assumed by default. See the
      :ref:`Build Options` document for more information on available build
      options.

   -  (AArch32 only) Currently only ``PLAT=fvp`` is supported.

   -  (AArch32 only) ``AARCH32_SP`` is the AArch32 EL3 Runtime Software and it
      corresponds to the BL32 image. A minimal ``AARCH32_SP``, sp_min, is
      provided by TF-A to demonstrate how PSCI Library can be integrated with
      an AArch32 EL3 Runtime Software. Some AArch32 EL3 Runtime Software may
      include other runtime services, for example Trusted OS services. A guide
      to integrate PSCI library with AArch32 EL3 Runtime Software can be found
      at :ref:`PSCI Library Integration guide for Armv8-A AArch32 systems`.

   -  (AArch64 only) The TSP (Test Secure Payload), corresponding to the BL32
      image, is not compiled in by default. Refer to the
      :ref:`Test Secure Payload (TSP) and Dispatcher (TSPD)` document for
      details on building the TSP.

   -  By default this produces a release version of the build. To produce a
      debug version instead, refer to the "Debugging options" section below.

   -  The build process creates products in a ``build`` directory tree, building
      the objects and binaries for each boot loader stage in separate
      sub-directories. The following boot loader binary files are created
      from the corresponding ELF files:

      -  ``build/<platform>/<build-type>/bl1.bin``
      -  ``build/<platform>/<build-type>/bl2.bin``
      -  ``build/<platform>/<build-type>/bl31.bin`` (AArch64 only)
      -  ``build/<platform>/<build-type>/bl32.bin`` (mandatory for AArch32)

      where ``<platform>`` is the name of the chosen platform and ``<build-type>``
      is either ``debug`` or ``release``. The actual number of images might differ
      depending on the platform.

-  Build products for a specific build variant can be removed using:

   .. code:: shell

       make DEBUG=<D> PLAT=<platform> clean

   ... where ``<D>`` is ``0`` or ``1``, as specified when building.

   The build tree can be removed completely using:

   .. code:: shell

       make realclean

--------------

*Copyright (c) 2020, Arm Limited. All rights reserved.*
