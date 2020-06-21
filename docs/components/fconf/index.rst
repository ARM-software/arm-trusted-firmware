Firmware Configuration Framework
================================

This document provides an overview of the |FCONF| framework.

Introduction
~~~~~~~~~~~~

The Firmware CONfiguration Framework (|FCONF|) is an abstraction layer for
platform specific data, allowing a "property" to be queried and a value
retrieved without the requesting entity knowing what backing store is being used
to hold the data.

It is used to bridge new and old ways of providing platform-specific data.
Today, information like the Chain of Trust is held within several, nested
platform-defined tables. In the future, it may be provided as part of a device
blob, along with the rest of the information about images to load.
Introducing this abstraction layer will make migration easier and will preserve
functionality for platforms that cannot / don't want to use device tree.

Accessing properties
~~~~~~~~~~~~~~~~~~~~

Properties defined in the |FCONF| are grouped around namespaces and
sub-namespaces: a.b.property.
Examples namespace can be:

- (|TBBR|) Chain of Trust data: tbbr.cot.trusted_boot_fw_cert
- (|TBBR|) dynamic configuration info: tbbr.dyn_config.disable_auth
- Arm io policies: arm.io_policies.bl2_image
- GICv3 properties: hw_config.gicv3_config.gicr_base

Properties can be accessed with the ``FCONF_GET_PROPERTY(a,b,property)`` macro.

Defining properties
~~~~~~~~~~~~~~~~~~~

Properties composing the |FCONF| have to be stored in C structures. If
properties originate from a different backend source such as a device tree,
then the platform has to provide a ``populate()`` function which essentially
captures the property and stores them into a corresponding |FCONF| based C
structure.

Such a ``populate()`` function is usually platform specific and is associated
with a specific backend source. For example, a populator function which
captures the hardware topology of the platform from the HW_CONFIG device tree.
Hence each ``populate()`` function must be registered with a specific
``config_type`` identifier. It broadly represents a logical grouping of
configuration properties which is usually a device tree file.

Example:
 - FW_CONFIG: properties related to base address, maximum size and image id
   of other DTBs etc.
 - TB_FW: properties related to trusted firmware such as IO policies,
   mbedtls heap info etc.
 - HW_CONFIG: properties related to hardware configuration of the SoC
   such as topology, GIC controller, PSCI hooks, CPU ID etc.

Hence the ``populate()`` callback must be registered to the (|FCONF|) framework
with the ``FCONF_REGISTER_POPULATOR()`` macro. This ensures that the function
would be called inside the generic ``fconf_populate()`` function during
initialization.

::

    int fconf_populate_topology(uintptr_t config)
    {
        /* read hw config dtb and fill soc_topology struct */
    }

    FCONF_REGISTER_POPULATOR(HW_CONFIG, topology, fconf_populate_topology);

Then, a wrapper has to be provided to match the ``FCONF_GET_PROPERTY()`` macro:

::

    /* generic getter */
    #define FCONF_GET_PROPERTY(a,b,property)	a##__##b##_getter(property)

    /* my specific getter */
    #define hw_config__topology_getter(prop) soc_topology.prop

This second level wrapper can be used to remap the ``FCONF_GET_PROPERTY()`` to
anything appropriate: structure, array, function, etc..

To ensure a good interpretation of the properties, this documentation must
explain how the properties are described for a specific backend. Refer to the
:ref:`binding-document` section for more information and example.

Loading the property device tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``fconf_load_config(image_id)`` must be called to load fw_config and
tb_fw_config devices tree containing the properties' values. This must be done
after the io layer is initialized, as the |DTB| is stored on an external
device (FIP).

.. uml:: ../../resources/diagrams/plantuml/fconf_bl1_load_config.puml

Populating the properties
~~~~~~~~~~~~~~~~~~~~~~~~~

Once a valid device tree is available, the ``fconf_populate(config)`` function
can be used to fill the C data structure with the data from the config |DTB|.
This function will call all the ``populate()`` callbacks which have been
registered with ``FCONF_REGISTER_POPULATOR()`` as described above.

.. uml:: ../../resources/diagrams/plantuml/fconf_bl2_populate.puml

Namespace guidance
~~~~~~~~~~~~~~~~~~

As mentioned above, properties are logically grouped around namespaces and
sub-namespaces. The following concepts should be considered when adding new
properties/namespaces.
The framework differentiates two types of properties:

 - Properties used inside common code.
 - Properties used inside platform specific code.

The first category applies to properties being part of the firmware and shared
across multiple platforms. They should be globally accessible and defined
inside the ``lib/fconf`` directory. The namespace must be chosen to reflect the
feature/data abstracted.

Example:
 - |TBBR| related properties: tbbr.cot.bl2_id
 - Dynamic configuration information: dyn_cfg.dtb_info.hw_config_id

The second category should represent the majority of the properties defined
within the framework: Platform specific properties. They must be accessed only
within the platform API and are defined only inside the platform scope. The
namespace must contain the platform name under which the properties defined
belong.

Example:
 - Arm io framework: arm.io_policies.bl31_id

.. _binding-document:

Properties binding information
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. toctree::
  :maxdepth: 1

  fconf_properties
