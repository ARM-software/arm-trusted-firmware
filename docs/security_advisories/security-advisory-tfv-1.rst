Advisory TFV-1 (CVE-2016-10319)
===============================

+----------------+-------------------------------------------------------------+
| Title          | Malformed Firmware Update SMC can result in copy of         |
|                | unexpectedly large data into secure memory                  |
+================+=============================================================+
| CVE ID         | `CVE-2016-10319`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | 18 Oct 2016                                                 |
+----------------+-------------------------------------------------------------+
| Versions       | v1.2 and v1.3 (since commit `48bfb88`_)                     |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | Platforms that use AArch64 BL1 plus untrusted normal world  |
| Affected       | firmware update code executing before BL31                  |
+----------------+-------------------------------------------------------------+
| Impact         | Copy of unexpectedly large data into the free secure memory |
|                | reported by BL1 platform code                               |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #783`_                                        |
+----------------+-------------------------------------------------------------+
| Credit         | IOActive                                                    |
+----------------+-------------------------------------------------------------+

Generic Trusted Firmware (TF) BL1 code contains an SMC interface that is briefly
available after cold reset to support the Firmware Update (FWU) feature (also
known as recovery mode). This allows most FWU functionality to be implemented in
the normal world, while retaining the essential image authentication
functionality in BL1. When cold boot reaches the EL3 Runtime Software (for
example, BL31 on AArch64 systems), the FWU SMC interface is replaced by the EL3
Runtime SMC interface. Platforms may choose how much of this FWU functionality
to use, if any.

The BL1 FWU SMC handling code, currently only supported on AArch64, contains
several vulnerabilities that may be exploited when *all* the following
conditions apply:

1. Platform code uses TF BL1 with the ``TRUSTED_BOARD_BOOT`` build option
   enabled.

2. Platform code arranges for untrusted normal world FWU code to be executed in
   the cold boot path, before BL31 starts. Untrusted in this sense means code
   that is not in ROM or has not been authenticated or has otherwise been
   executed by an attacker.

3. Platform code copies the insecure pattern described below from the ARM
   platform version of ``bl1_plat_mem_check()``.

The vulnerabilities consist of potential integer overflows in the input
validation checks while handling the ``FWU_SMC_IMAGE_COPY`` SMC. The SMC
implementation is designed to copy an image into secure memory for subsequent
authentication, but the vulnerabilities may allow an attacker to copy
unexpectedly large data into secure memory. Note that a separate vulnerability
is required to leverage these vulnerabilities; for example a way to get the
system to change its behaviour based on the unexpected secure memory contents.

Two of the vulnerabilities are in the function ``bl1_fwu_image_copy()`` in
``bl1/bl1_fwu.c``. These are listed below, referring to the v1.3 tagged version
of the code:

- Line 155:

  .. code:: c

    /*
     * If last block is more than expected then
     * clip the block to the required image size.
     */
    if (image_desc->copied_size + block_size >
         image_desc->image_info.image_size) {
        block_size = image_desc->image_info.image_size -
            image_desc->copied_size;
        WARN("BL1-FWU: Copy argument block_size > remaining image size."
            " Clipping block_size\n");
    }

    /* Make sure the image src/size is mapped. */
    if (bl1_plat_mem_check(image_src, block_size, flags)) {
        WARN("BL1-FWU: Copy arguments source/size not mapped\n");
        return -ENOMEM;
    }

    INFO("BL1-FWU: Continuing image copy in blocks\n");

    /* Copy image for given block size. */
    base_addr += image_desc->copied_size;
    image_desc->copied_size += block_size;
    memcpy((void *)base_addr, (const void *)image_src, block_size);
    ...

  This code fragment is executed when the image copy operation is performed in
  blocks over multiple SMCs. ``block_size`` is an SMC argument and therefore
  potentially controllable by an attacker. A very large value may result in an
  integer overflow in the 1st ``if`` statement, which would bypass the check,
  allowing an unclipped ``block_size`` to be passed into
  ``bl1_plat_mem_check()``. If ``bl1_plat_mem_check()`` also passes, this may
  result in an unexpectedly large copy of data into secure memory.

- Line 206:

  .. code:: c

    /* Make sure the image src/size is mapped. */
    if (bl1_plat_mem_check(image_src, block_size, flags)) {
        WARN("BL1-FWU: Copy arguments source/size not mapped\n");
        return -ENOMEM;
    }

    /* Find out how much free trusted ram remains after BL1 load */
    mem_layout = bl1_plat_sec_mem_layout();
    if ((image_desc->image_info.image_base < mem_layout->free_base) ||
         (image_desc->image_info.image_base + image_size >
          mem_layout->free_base + mem_layout->free_size)) {
        WARN("BL1-FWU: Memory not available to copy\n");
        return -ENOMEM;
    }

    /* Update the image size. */
    image_desc->image_info.image_size = image_size;

    /* Copy image for given size. */
    memcpy((void *)base_addr, (const void *)image_src, block_size);
    ...

  This code fragment is executed during the 1st invocation of the image copy
  operation. Both ``block_size`` and ``image_size`` are SMC arguments. A very
  large value of ``image_size`` may result in an integer overflow in the 2nd
  ``if`` statement, which would bypass the check, allowing execution to proceed.
  If ``bl1_plat_mem_check()`` also passes, this may result in an unexpectedly
  large copy of data into secure memory.

If the platform's implementation of ``bl1_plat_mem_check()`` is correct then it
may help prevent the above 2 vulnerabilities from being exploited. However, the
ARM platform version of this function contains a similar vulnerability:

- Line 88 of ``plat/arm/common/arm_bl1_fwu.c`` in function of
  ``bl1_plat_mem_check()``:

  .. code:: c

    while (mmap[index].mem_size) {
        if ((mem_base >= mmap[index].mem_base) &&
            ((mem_base + mem_size)
            <= (mmap[index].mem_base +
            mmap[index].mem_size)))
            return 0;

        index++;
    }
    ...

  This function checks that the passed memory region is within one of the
  regions mapped in by ARM platforms. Here, ``mem_size`` may be the
  ``block_size`` passed from ``bl1_fwu_image_copy()``. A very large value of
  ``mem_size`` may result in an integer overflow and the function to incorrectly
  return success. Platforms that copy this insecure pattern will have the same
  vulnerability.

.. _CVE-2016-10319: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2016-10319
.. _48bfb88: https://github.com/ARM-software/arm-trusted-firmware/commit/48bfb88
.. _Pull Request #783: https://github.com/ARM-software/arm-trusted-firmware/pull/783
