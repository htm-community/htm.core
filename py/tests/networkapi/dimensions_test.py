# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2020, Numenta, Inc.
#
# author: David Keeney, dkeeney@gmail.com, July 2020
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
Unit tests for Dimensions object with python. 
"""

import datetime
import numpy as np
import unittest

from htm.bindings.sdr import SDR
from htm.bindings.engine_internal import Dimensions

class NetworkAPI_Dimensions_Test(unittest.TestCase):
  """ Unit tests for Dimensions class. """

  def testDimensionsMembers(self):
    # test of the status conditions of a Dimensions object  
    dim1 = Dimensions()
    self.assertEqual(dim1.empty(), True)
    self.assertEqual(dim1.size(), 0)
    self.assertEqual(dim1.isUnspecified(), True)
    self.assertEqual(dim1.toString(), "[unspecified]")
    
    dim2 = Dimensions(0)
    self.assertEqual(dim2.size(), 1)
    self.assertEqual(dim2.isDontcare(), True)
    self.assertEqual(dim2.isSpecified(), False)
    self.assertEqual(dim2.toString(), "[dontcare]")
    
    dim3 = Dimensions(1)
    self.assertEqual(dim3.size(), 1)
    self.assertEqual(dim3.getCount(), 1)
    self.assertEqual(dim3.isSpecified(), True)
    self.assertEqual(dim3.toString(), "[1] ")

    dim4 = Dimensions(0, 0)
    self.assertEqual(dim4.size(), 2)
    self.assertEqual(dim4.isInvalid(), True)
    self.assertEqual(dim4.toString(), "[0,0] ")
    
    dim5 = Dimensions(5, 25)
    self.assertEqual(dim5.size(), 2)
    self.assertEqual(dim5.getCount(), 125)
    self.assertEqual(dim5.isInvalid(), False)
    self.assertEqual(dim5.isSpecified(), True)
    self.assertEqual(dim5.toString(), "[5,25] ")
    
    dim5.resize(3)
    self.assertEqual(dim5.size(), 3)
    self.assertEqual(dim5.toString(), "[5,25,0] ")
    self.assertEqual(dim5.isInvalid(), True)
    
    dim5.clear()
    self.assertEqual(dim5.isUnspecified(), True)
    
    
    
  def testAsVector(self):
    # test of Dimension.asVector()
    dim = Dimensions()
    dim.push_front(1)
    dim.push_back(2)
    self.assertEqual(dim.getCount(), 2)
    v = dim.asVector()
    expected = np.array([1,2])
    np.testing.assert_array_equal(v, expected)
    
    s = SDR(v)
    self.assertEqual(s, SDR(dim.asVector()))
    
    
