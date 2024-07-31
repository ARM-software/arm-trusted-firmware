#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

##* Variables
SHELL := /usr/bin/env bash
PYTHON := python
PYTHONPATH := `pwd`

#* Docker variables
IMAGE := tlc
VERSION := latest

#* Installation
.PHONY: dist
dist: clean
	poetry build

.PHONY: dev-install
dev-install:
	poetry lock -n --no-update && poetry export --without-hashes > requirements.txt
	poetry install -n
	-poetry run mypy --install-types --non-interactive ./

.PHONY: install
install: dist
	pip install dist/*.whl

.PHONY: pre-commit-install
pre-commit-install:
	poetry run pre-commit install

#* Formatters
.PHONY: codestyle
codestyle:
	poetry run pyupgrade --exit-zero-even-if-changed --py38-plus **/*.py
	poetry run isort --settings-path pyproject.toml ./
	poetry run black --config pyproject.toml ./

.PHONY: formatting
formatting: codestyle

#* Linting
.PHONY: test
test:
	PYTHONPATH=$(PYTHONPATH) poetry run pytest -c pyproject.toml --cov-report=html --cov=tlc tests/
	poetry run coverage-badge -o assets/images/coverage.svg -f

.PHONY: check-codestyle
check-codestyle:
	poetry run isort --diff --check-only --settings-path pyproject.toml ./
	poetry run black --diff --check --config pyproject.toml ./
	poetry run darglint --verbosity 2 tlc tests

.PHONY: mypy
mypy:
	poetry run mypy --config-file pyproject.toml ./

.PHONY: check-safety
check-safety:
	poetry check
	poetry run safety check --full-report
	poetry run bandit -ll --recursive tlc tests

.PHONY: lint
lint: test check-codestyle mypy check-safety

.PHONY: update-dev-deps
update-dev-deps:
	poetry add -D bandit@latest darglint@latest "isort[colors]@latest" mypy@latest pre-commit@latest pydocstyle@latest pylint@latest pytest@latest pyupgrade@latest safety@latest coverage@latest coverage-badge@latest pytest-html@latest pytest-cov@latest
	poetry add -D --allow-prereleases black@latest

#* Docker
.PHONY: docker-build docker-remove
docker-build:
	@echo Building docker $(IMAGE):$(VERSION) ...
	docker build \
		-t $(IMAGE):$(VERSION) . \
		-f ./docker/Dockerfile --no-cache

docker-remove:
	@echo Removing docker $(IMAGE):$(VERSION) ...
	docker rmi -f $(IMAGE):$(VERSION)


#* Cleaning
.PHONY: clean .clean-build clean-pyc clean-test
clean: clean-build clean-pyc clean-test ## remove all build, test, coverage and Python artifacts

clean-build: ## remove build artifacts
	rm -fr build/
	rm -fr dist/
	rm -fr .eggs/
	find . -name '*.egg-info' -exec rm -fr {} +
	find . -name '*.egg' -exec rm -f {} +

clean-pyc: ## remove Python file artifacts
	find . -name '*.pyc' -exec rm -f {} +
	find . -name '*.pyo' -exec rm -f {} +
	find . -name '*~' -exec rm -f {} +
	find . -name '__pycache__' -exec rm -fr {} +
	find . | grep -E ".pytest_cache" | xargs rm -rf
	find . | grep -E ".mypy_cache" | xargs rm -rf

clean-test: ## remove test and coverage artifacts
	rm -fr .tox/
	rm -f .coverage
	rm -fr htmlcov/
