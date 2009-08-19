#!/usr/bin/env python
# encoding: utf-8
"""
setup.py

Setup the Keyring Lib for Python.
"""

from distutils.core import setup, Extension
from extensions import get_extensions

setup(name = 'keyring',
      version = "0.1",
      description = "Store and access your passwords safely.",
      url = "http://keyring-python.org/",
      maintainer = "Kang Zhang",
      maintainer_email = "jobo.zh@gmail.com",
      license="PSF",
      long_description = open('README.txt').read(),
      platforms = ["Many"],
      packages = ['keyring'],
      ext_modules = get_extensions()
    )

