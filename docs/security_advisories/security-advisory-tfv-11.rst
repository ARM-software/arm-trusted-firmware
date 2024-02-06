Advisory TFV-11 (CVE-2023-49100)
================================

+----------------+-------------------------------------------------------------+
| Title          | A Malformed SDEI SMC can cause out of bound memory read.    |
+================+=============================================================+
| CVE ID         | `CVE-2023-49100`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | Reported on 12 Oct 2023                                     |
+----------------+-------------------------------------------------------------+
| Versions       | TF-A releases v1.5 to v2.9                                  |
| Affected       | LTS releases  lts-v2.8.0 to lts-v2.8.11                     |
+----------------+-------------------------------------------------------------+
| Configurations | Platforms with SDEI support                                 |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Impact         | Denial of Service (secure world panic)                      |
+----------------+-------------------------------------------------------------+
| Fix Version    | `a7eff3477`_ "fix(sdei): ensure that interrupt ID is valid" |
+----------------+-------------------------------------------------------------+
| Credit         | Christian Lindenmeier `@_chli_`_                            |
|                | Marcel Busch `@0ddc0de`_                                    |
|                | `IT Security Infrastructures Lab`_                          |
+----------------+-------------------------------------------------------------+

This security advisory describes a vulnerability in the SDEI services, where a
rogue Non-secure caller invoking a SDEI_INTERRUPT_BIND SMC call with an invalid
interrupt ID causes out of bound memory read.

SDEI_INTERRUPT_BIND is used to bind any physical interrupt into a normal
priority SDEI event. The interrupt can be a private peripheral interrupt
(PPI) or a shared peripheral interrupt (SPI).
Refer to SDEI_INTERRUPT_BIND in the `SDEI Specification`_ for further details.

The vulnerability exists when the SDEI client passes an interrupt ID which
is not implemented by the GIC. This will result in a data abort exception
or a EL3 panic depending on the GIC version used in the system.

- **GICv2 systems:**

.. code:: c

  Call stack:
        sdei_interrupt_bind(interrupt ID)
         -> plat_ic_get_interrupt_type(interrupt ID)
           -> gicv2_get_interrupt_group(interrupt ID)
             -> gicd_get_igroupr(distributor base, interrupt ID)
               -> gicd_read_igroupr(distributor base, interrupt ID).

  gicd_read_igroupr() will eventually do a MMIO read to an unimplemented IGROUPR
  register. Which may cause a data abort or an access to a random EL3 memory region.

- **GICv3 systems:**

.. code:: c

   Call stack:
        sdei_interrupt_bind(interrupt ID)
          -> plat_ic_get_interrupt_type(interrupt ID)
            -> gicv3_get_interrupt_group(interrupt ID, core ID)
              -> is_sgi_ppi(interrupt ID)

   is_sgi_ppi() will end up in an EL3 panic on encountering an invalid interrupt ID.

The vulnerability is fixed by ensuring that the Interrupt ID provided by the
SDEI client is a valid PPI or SPI, otherwise return an error code indicating
that the parameter is invalid.

.. code:: c

   /* Bind an SDEI event to an interrupt */
   static int sdei_interrupt_bind(unsigned int intr_num)
   {
        sdei_ev_map_t *map;
        bool retry = true, shared_mapping;

        /* Interrupt must be either PPI or SPI */
        if (!(plat_ic_is_ppi(intr_num) || plat_ic_is_spi(intr_num)))
              return SDEI_EINVAL;

.. _CVE-2023-49100: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2023-49100
.. _a7eff3477: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=a7eff3477dcf3624c74f5217419b1a27b7ebd2aa
.. _IT Security Infrastructures Lab: https://www.cs1.tf.fau.de/
.. _SDEI Specification: https://developer.arm.com/documentation/den0054/latest/
.. _@_chli_: https://twitter.com/_chli_
.. _@0ddc0de: https://twitter.com/0ddc0de
