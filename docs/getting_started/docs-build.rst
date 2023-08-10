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
- `Poetry`_ (Python dependency manager)
- Optionally, the `Dia`_ application can be installed if you need to edit
  existing ``.dia`` diagram files, or create new ones.


Below is an example set of instructions to get a working environment (tested on
Ubuntu):

.. code:: shell

    sudo apt install python3 python3-pip plantuml [dia]
    curl -sSL https://install.python-poetry.org | python3 -

Building rendered documentation
-------------------------------

To install Python dependencies using Poetry:

.. code:: shell

    poetry install

Poetry will create a new virtual environment and install all dependencies listed
in ``pyproject.toml``. You can get information about this environment, such as
its location and the Python version, with the command:

.. code:: shell

    poetry env info

If you have already sourced a virtual environment, Poetry will respect this and
install dependencies there.

Once all dependencies are installed, the documentation can be compiled into
HTML-formatted pages from the project root directory by running:

.. code:: shell

   poetry run make doc

Output from the build process will be placed in: ``docs/build/html``.

Other Output Formats
~~~~~~~~~~~~~~~~~~~~

We also support building documentation in other formats. From the ``docs``
directory of the project, run the following command to see the supported
formats.

.. code:: shell

   poetry run make -C docs help

To build the documentation in PDF format, additionally ensure that the following
packages are installed:

- FreeSerif font
- latexmk
- librsvg2-bin
- xelatex
- xindy

Below is an example set of instructions to install the required packages
(tested on Ubuntu):

.. code:: shell

	sudo apt install fonts-freefont-otf latexmk librsvg2-bin texlive-xetex xindy

Once all the dependencies are installed, run the command ``poetry run make -C
docs latexpdf`` to build the documentation. Output from the build process
(``trustedfirmware-a.pdf``) can be found in ``docs/build/latex``.

Building rendered documentation from Poetry's virtual environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The command ``poetry run`` used in the steps above executes the input command
from inside the project's virtual environment. The easiest way to activate this
virtual environment is with the ``poetry shell`` command.

Running ``poetry shell`` from the directory containing this project, activates
the same virtual environment. This creates a sub-shell through which you can
build the documentation directly with ``make``.

.. code:: shell

    poetry shell
    make doc

Type ``exit`` to deactivate the virtual environment and exit this new shell. For
other use cases, please see the official `Poetry`_ documentation.

Building rendered documentation from a container
------------------------------------------------

There may be cases where you can not either install or upgrade required
dependencies to generate the documents, so in this case, one way to
create the documentation is through a docker container. The first step is
to check if `docker`_ is installed in your host, otherwise check main docker
page for installation instructions. Once installed, run the following script
from project root directory

.. code:: shell

   docker run --rm -v $PWD:/tf-a sphinxdoc/sphinx \
        bash -c 'cd /tf-a &&
            apt-get update && apt-get install -y curl plantuml &&
            curl -sSL https://install.python-poetry.org | python3 - &&
            ~/.local/bin/poetry install && ~/.local/bin/poetry run make doc'

The above command fetches the ``sphinxdoc/sphinx`` container from `docker
hub`_, launches the container, installs documentation requirements and finally
creates the documentation. Once done, exit the container and output from the
build process will be placed in: ``docs/build/html``.

--------------

*Copyright (c) 2019-2023, Arm Limited. All rights reserved.*

.. _Sphinx: http://www.sphinx-doc.org/en/master/
.. _Poetry: https://python-poetry.org/docs/
.. _pip homepage: https://pip.pypa.io/en/stable/
.. _Dia: https://wiki.gnome.org/Apps/Dia
.. _docker: https://www.docker.com/
.. _docker hub: https://hub.docker.com/repository/docker/sphinxdoc/sphinx
