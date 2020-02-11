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

Properties can be accessed with the ``FCONF_GET_PROPERTY(a,b,property)`` macro.

Defining properties
~~~~~~~~~~~~~~~~~~~

Properties composing the |FCONF| have to be stored in C structures. If another
backing store is wanted to be used, the platform has to provide a ``populate()``
function to fill the corresponding C structure.

The ``populate()`` function must be registered to the |FCONF| framework with
the ``FCONF_REGISTER_POPULATOR()`` macro. This ensures that the function would
be called inside the generic ``fconf_populate()`` function during
initialization.

::

    int fconf_populate_tbbr_dyn_config(uintptr_t config)
    {
        /* read dtb and fill tbbr_dyn_config struct */
    }

    FCONF_REGISTER_POPULATOR(fconf_populate_tbbr_dyn_config);

Then, a wrapper has to be provided to match the ``FCONF_GET_PROPERTY()`` macro:

::

    /* generic getter */
    #define FCONF_GET_PROPERTY(a,b,property)	a##__##b##_getter(property)

    /* my specific getter */
    #define tbbr__dyn_config_getter(id)	tbbr_dyn_config.id

This second level wrapper can be used to remap the ``FCONF_GET_PROPERTY()`` to
anything appropriate: structure, array, function, etc..

Loading the property device tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``fconf_load_config()`` must be called to load the device tree containing
the properties' values. This must be done after the io layer is initialized, as
the |DTB| is stored on an external device (FIP).

.. uml:: ../resources/diagrams/plantuml/fconf_bl1_load_config.puml

Populating the properties
~~~~~~~~~~~~~~~~~~~~~~~~~

Once a valid device tree is available, the ``fconf_populate(config)`` function
can be used to fill the C data structure with the data from the config |DTB|.
This function will call all the ``populate()`` callbacks which have been
registered with ``FCONF_REGISTER_POPULATOR()``.

.. uml:: ../resources/diagrams/plantuml/fconf_bl2_populate.puml
