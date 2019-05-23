Advisory TFV-4 (CVE-2017-9607)
==============================

+----------------+-------------------------------------------------------------+
| Title          | Malformed Firmware Update SMC can result in copy or         |
|                | authentication of unexpected data in secure memory in       |
|                | AArch32 state                                               |
+================+=============================================================+
| CVE ID         | `CVE-2017-9607`_                                            |
+----------------+-------------------------------------------------------------+
| Date           | 20 Jun 2017                                                 |
+----------------+-------------------------------------------------------------+
| Versions       | None (only between 22 May 2017 and 14 June 2017)            |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | Platforms that use AArch32 BL1 plus untrusted normal world  |
| Affected       | firmware update code executing before BL31                  |
+----------------+-------------------------------------------------------------+
| Impact         | Copy or authentication of unexpected data in the secure     |
|                | memory                                                      |
+----------------+-------------------------------------------------------------+
| Fix Version    | `Pull Request #979`_ (merged on 14 June 2017)               |
+----------------+-------------------------------------------------------------+
| Credit         | ARM                                                         |
+----------------+-------------------------------------------------------------+

The ``include/lib/utils_def.h`` header file provides the
``check_uptr_overflow()`` macro, which aims at detecting arithmetic overflows
that may occur when computing the sum of a base pointer and an offset. This
macro evaluates to 1 if the sum of the given base pointer and offset would
result in a value large enough to wrap around, which may lead to unpredictable
behaviour.

The macro code is at line 52, referring to the version of the code as of `commit
c396b73`_:

.. code:: c

    /*
     * Evaluates to 1 if (ptr + inc) overflows, 0 otherwise.
     * Both arguments must be unsigned pointer values (i.e. uintptr_t).
     */
    #define check_uptr_overflow(ptr, inc)       \
        (((ptr) > UINTPTR_MAX - (inc)) ? 1 : 0)

This macro does not work correctly for AArch32 images. It fails to detect
overflows when the sum of its two parameters fall into the ``[2^32, 2^64 - 1]``
range. Therefore, any AArch32 code relying on this macro to detect such integer
overflows is actually not protected.

The buggy code has been present in ARM Trusted Firmware (TF) since `Pull Request
#678`_ was merged (on 18 August 2016). However, the upstream code was not
vulnerable until `Pull Request #939`_ was merged (on 22 May 2017), which
introduced AArch32 support for the Trusted Board Boot (TBB) feature. Before
then, the ``check_uptr_overflow()`` macro was not used in AArch32 code.

The vulnerability resides in the BL1 FWU SMC handling code and it may be
exploited when *all* the following conditions apply:

- Platform code uses TF BL1 with the ``TRUSTED_BOARD_BOOT`` build option.

- Platform code uses the Firmware Update (FWU) code provided in
  ``bl1/bl1_fwu.c``, which is part of the TBB support.

- TF BL1 is compiled with the ``ARCH=aarch32`` build option.

In this context, the AArch32 BL1 image might fail to detect potential integer
overflows in the input validation checks while handling the
``FWU_SMC_IMAGE_COPY`` and ``FWU_SMC_IMAGE_AUTH`` SMCs.

The ``FWU_SMC_IMAGE_COPY`` SMC handler is designed to copy an image into secure
memory for subsequent authentication. This is implemented by the
``bl1_fwu_image_copy()`` function, which has the following function prototype:

.. code:: c

     static int bl1_fwu_image_copy(unsigned int image_id,
                        uintptr_t image_src,
                        unsigned int block_size,
                        unsigned int image_size,
                        unsigned int flags)

``image_src`` is an SMC argument and therefore potentially controllable by an
attacker. A very large 32-bit value, for example ``2^32 -1``, may result in the
sum of ``image_src`` and ``block_size`` overflowing a 32-bit type, which
``check_uptr_overflow()`` will fail to detect.  Depending on its implementation,
the platform-specific function ``bl1_plat_mem_check()`` might get defeated by
these unsanitized values and allow the following memory copy operation, that
would wrap around.  This may allow an attacker to copy unexpected data into
secure memory if the memory is mapped in BL1's address space, or cause a fatal
exception if it's not.

The ``FWU_SMC_IMAGE_AUTH`` SMC handler is designed to authenticate an image
resident in secure memory. This is implemented by the ``bl1_fwu_image_auth()``
function, which has the following function prototype:

.. code:: c

    static int bl1_fwu_image_auth(unsigned int image_id,
                        uintptr_t image_src,
                        unsigned int image_size,
                        unsigned int flags)

Similarly, if an attacker has control over the ``image_src`` or ``image_size``
arguments through the SMC interface and injects high values whose sum overflows,
they might defeat the ``bl1_plat_mem_check()`` function and make the
authentication module read data outside of what's normally allowed by the
platform code or crash the platform.

Note that in both cases, a separate vulnerability is required to leverage this
vulnerability; for example a way to get the system to change its behaviour based
on the unexpected secure memory accesses.  Moreover, the normal world FWU code
would need to be compromised in order to send a malformed FWU SMC that triggers
an integer overflow.

The vulnerability is known to affect all ARM standard platforms when enabling
the ``TRUSTED_BOARD_BOOT`` and ``ARCH=aarch32`` build options.  Other platforms
may also be affected if they fulfil the above conditions.

.. _CVE-2017-9607: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9607
.. _commit c396b73: https://github.com/ARM-software/arm-trusted-firmware/commit/c396b73
.. _Pull Request #678: https://github.com/ARM-software/arm-trusted-firmware/pull/678
.. _Pull Request #939: https://github.com/ARM-software/arm-trusted-firmware/pull/939
.. _Pull Request #979: https://github.com/ARM-software/arm-trusted-firmware/pull/979
