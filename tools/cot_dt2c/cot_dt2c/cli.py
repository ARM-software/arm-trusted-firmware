#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from pathlib import Path
from cot_dt2c.cot_dt2c import generateMain
from cot_dt2c.cot_dt2c import validateMain
from cot_dt2c.cot_dt2c import visualizeMain
from cot_dt2c.dt_validator import dtValidatorMain

import click

@click.group()
@click.version_option()
def cli():
    pass

@cli.command()
@click.argument("inputfile", type=click.Path(dir_okay=True))
@click.argument("outputfile", type=click.Path(dir_okay=True))
def convert_to_c(inputfile, outputfile):
    generateMain(inputfile, outputfile)

@cli.command()
@click.argument("inputfile", type=click.Path(dir_okay=True))
def validate_cot(inputfile):
    validateMain(inputfile)

@cli.command()
@click.argument("inputfile", type=click.Path(dir_okay=True))
def visualize_cot(inputfile):
    visualizeMain(inputfile)

@cli.command()
@click.argument("inputfiledir", type=click.Path(dir_okay=True))
def validate_dt(inputfiledir):
    dtValidatorMain(inputfiledir)
