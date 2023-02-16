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

For building a local copy of the |TF-A| documentation you will need:

- Python 3 (3.8 or later)
- PlantUML (1.2017.15 or later)
- Poetry Python dependency and package manager
- Python modules specified in ``pyproject.toml``
- Optionally, the `Dia`_ application can be installed if you need to edit
  existing ``.dia`` diagram files, or create new ones.


Poetry will handle the creation of a virtual build environment, either creating
a new environment or re-using one created by the user, and installing all
dependencies herein. This ensures that the Python environment is isolated from
your system environment.

An example set of installation commands for Ubuntu follows:

.. code:: shell

    sudo apt install python3 python3-pip plantuml [dia]
    curl -sSL https://install.python-poetry.org | python3 -
    poetry install

Building rendered documentation
-------------------------------

Documents can be built into HTML-formatted pages from project root directory by
running the following command.

.. code:: shell

   poetry run make doc

Output from the build process will be placed in:

::

   docs/build/html

We also support building documentation in other formats. From the ``docs``
directory of the project, run the following command to see the supported
formats. It is important to note that you will not get the correct result if
the command is run from the project root directory, as that would invoke the
top-level Makefile for |TF-A| itself.

.. code:: shell

   poetry run make help

.. note::

   The ``run`` command used above executes ``make`` in the projects virtual
   environment. To spawn a shell in this environment, use ``poetry
   shell``. For other use cases, please see the official `Poetry`_
   documentation.

Building rendered documentation from a container
------------------------------------------------

There may be cases where you can not either install or upgrade required
dependencies to generate the documents, so in this case, one way to
create the documentation is through a docker container. The first step is
to check if `docker`_ is installed in your host, otherwise check main docker
page for installation instructions. Once installed, run the following script
from project root directory

.. code:: shell

   docker run --rm -v $PWD:/TF sphinxdoc/sphinx \
          bash -c 'cd /TF && \
          poetry install && poetry run make doc'

The above command fetches the ``sphinxdoc/sphinx`` container from `docker
hub`_, launches the container, installs documentation requirements and finally
creates the documentation. Once done, exit the container and output from the
build process will be placed in:

::

   docs/build/html

--------------

*Copyright (c) 2019-2023, Arm Limited. All rights reserved.*

.. _Sphinx: http://www.sphinx-doc.org/en/master/
.. _Poetry: https://python-poetry.org/docs/cli/
.. _pip homepage: https://pip.pypa.io/en/stable/
.. _Dia: https://wiki.gnome.org/Apps/Dia
.. _docker: https://www.docker.com/
.. _docker hub: https://hub.docker.com/repository/docker/sphinxdoc/sphinx
