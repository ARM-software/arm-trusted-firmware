#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

##* Variables
SHELL := /usr/bin/env bash
PYTHON := python
PYTHONPATH := `pwd`

.PHONY: dist
dist: clean
	poetry build

#* Installation
.PHONY: dev-install
dev-install:
	pip3 install mypy
	pip3 install pytest
	pip install -r requirements.txt
	poetry lock -n && poetry export --without-hashes > requirements.txt
	poetry install -n
	-poetry run mypy --install-types --non-interactive ./

.PHONY: install
install: dist
	pip install mypy
	pip install pytest
	pip install -r requirements.txt
	pip install dist/*.whl

clean-test: ## remove test and coverage artifacts
	rm -fr .tox/
	rm -f .coverage
	rm -fr htmlcov/

clean-pyc: ## remove Python file artifacts
	find . -name '*.pyc' -exec rm -f {} +
	find . -name '*.pyo' -exec rm -f {} +
	find . -name '*~' -exec rm -f {} +
	find . -name '__pycache__' -exec rm -fr {} +
	find . | grep -E ".pytest_cache" | xargs rm -rf
	find . | grep -E ".mypy_cache" | xargs rm -rf


clean-build: ## remove build artifacts
	rm -fr build/
	rm -fr dist/
	rm -fr .eggs/
	find . -name '*.egg-info' -exec rm -fr {} +
	find . -name '*.egg' -exec rm -f {} +

clean-tmp:
	rm -rf ./tmp

#* Cleaning
.PHONY: clean clean-build clean-pyc clean-test
clean: uninstall clean-build clean-pyc clean-test clean-tmp ## remove all build, test, coverage and Python artifacts

uninstall:
	pip uninstall -y cot-dt2c

.PHONY: reinstall
reinstall: clean install

.PHONY: test
test:
	PYTHONPATH=$(PYTHONPATH) poetry run pytest -c pyproject.toml tests/
