Building Documentation
======================

To create a rendered copy of this documentation locally you can use the
`Sphinx`_ tool to build and package the plain-text documents into HTML-formatted
pages.

If you are building the documentation for the first time then you will need to
check that you have the required software packages, as described in the
*Prerequisites* section that follows.

.. note::
   An online copy of the documentation is available at
   https://www.trustedfirmware.org/docs/tf-a, if you want to view a rendered
   copy without doing a local build.

Prerequisites
-------------

For building a local copy of the |TF-A| documentation you will need, at minimum:

- Python 3 (3.5 or later)
- PlantUML (1.2017.15 or later)

Optionally, the `Dia`_ application can be installed if you need to edit
existing ``.dia`` diagram files, or create new ones.

You must also install the Python modules that are specified in the
``requirements.txt`` file in the root of the ``docs`` directory. These modules
can be installed using ``pip3`` (the Python Package Installer). Passing this
requirements file as an argument to ``pip3`` automatically installs the specific
module versions required by |TF-A|.

An example set of installation commands for Ubuntu 18.04 LTS follows, assuming
that the working directory is ``docs``:

.. code:: shell

    sudo apt install python3 python3-pip plantuml [dia]
    pip3 install [--user] -r requirements.txt

.. note::
   Several other modules will be installed as dependencies. Please review
   the list to ensure that there will be no conflicts with other modules already
   installed in your environment.

Passing the optional ``--user`` argument to ``pip3`` will install the Python
packages only for the current user. Omitting this argument will attempt to
install the packages globally and this will likely require the command to be run
as root or using ``sudo``.

.. note::
   More advanced usage instructions for *pip* are beyond the scope of this
   document but you can refer to the `pip homepage`_ for detailed guides.

Building rendered documentation
-------------------------------

Documents can be built into HTML-formatted pages from project root directory by
running the following command.

.. code:: shell

   make doc

Output from the build process will be placed in:

::

   docs/build/html/

We also support building documentation in other formats. From the ``docs``
directory of the project, run the following command to see the supported
formats. It is important to note that you will not get the correct result if
the command is run from the project root directory, as that would invoke the
top-level Makefile for |TF-A| itself.

.. code:: shell

   make help

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*

.. _Sphinx: http://www.sphinx-doc.org/en/master/
.. _pip homepage: https://pip.pypa.io/en/stable/
.. _Dia: https://wiki.gnome.org/Apps/Dia
