# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2015, Numenta, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Affero Public License for more details.
#
# You should have received a copy of the GNU Affero Public License
# along with this program.  If not, see http://www.gnu.org/licenses.
# ----------------------------------------------------------------------
"""
Based on: http://stackoverflow.com/questions/5317672/pip-not-finding-setup-file
"""

import os

from setuptools.command import egg_info

REPO_DIR = os.path.dirname(os.path.realpath(__file__))
filename = os.path.basename(__file__)

os.chdir(os.path.join(REPO_DIR, "bindings","py","packaging"))
setupdir = os.getcwd()

egginfo = "pip-egg-info"

__file__ = os.path.join(setupdir, filename)

def replacement_run(self):
  print("setup.py::replacement_run()\n")
  self.mkpath(self.egg_info)

  installer = self.distribution.fetch_build_egg

  for ep in egg_info.iter_entry_points('egg_info.writers'):
    # require=False is the change we're making from pip
    writer = ep.load(require=False)

    if writer:
      writer(self, ep.name, egg_info.os.path.join(self.egg_info,ep.name))

  self.find_sources()

egg_info.egg_info.run = replacement_run
print("setup.py: Calling {}\n".format(__file__))
import runpy
runpy.run_path(__file__, run_name=__name__)
