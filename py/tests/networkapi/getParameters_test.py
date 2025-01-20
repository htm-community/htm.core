# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2020, Numenta, Inc.
#
# author: David Keeney, dkeeney@gmail.com, August 2020
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
Unit tests for using NetworkAPI with python. 
 This tests getSpecJSON(), getParameters(), and getParameterJSON( ).
"""

import numpy as np
import unittest
import json

from htm.bindings.sdr import SDR
from htm.bindings.engine_internal import Network,Region
from htm.advanced.support.register_regions import registerAllAdvancedRegions

class NetworkAPI_getParameters_Test(unittest.TestCase):
  """ Unit tests for Network class. """

    
  def testGetSpecJSON(self):
    """
    A test of the Network.getSpecJSON( ) function.
    """
    expected = """{"spec": "RDSEEncoderRegion",
  "parameters": {
    "activeBits": {
      "type": "UInt32",
      "count": 1,
      "access": "Create",
      "defaultValue": "0"
    },
    "category": {
      "type": "Bool",
      "count": 1,
      "access": "Create",
      "defaultValue": "false"
    },
    "noise": {
      "description": "amount of noise to add to the output SDR. 0.01 is 1%",
      "type": "Real32",
      "count": 1,
      "access": "ReadWrite",
      "defaultValue": "0.0"
    },
    "radius": {
      "type": "Real32",
      "count": 1,
      "access": "Create",
      "defaultValue": "0.0"
    },
    "resolution": {
      "type": "Real32",
      "count": 1,
      "access": "Create",
      "defaultValue": "0.0"
    },
    "seed": {
      "type": "UInt32",
      "count": 1,
      "access": "Create",
      "defaultValue": "0"
    },
    "sensedValue": {
      "description": "The value to encode. Overriden by input 'values'.",
      "type": "Real64",
      "count": 1,
      "access": "ReadWrite",
      "defaultValue": "0.0"
    },
    "size": {
      "type": "UInt32",
      "count": 1,
      "access": "Create",
      "defaultValue": "0"
    },
    "sparsity": {
      "type": "Real32",
      "count": 1,
      "access": "Create",
      "defaultValue": "0.0"
    }
  },
  "inputs": {
    "values": {
      "description": "Values to encode. Overrides sensedValue.",
      "type": "Real64",
      "count": 1,
      "required": 0,
      "regionLevel": 1,
      "isDefaultInput": 1
    }
  },
  "outputs": {
    "bucket": {
      "description": "Quantized sample based on the radius. Becomes the title for this sample in Classifier.",
      "type": "Real64",
      "count": 1,
      "regionLevel": 0,
      "isDefaultOutput": 0
    },
    "encoded": {
      "description": "Encoded bits. Not a true Sparse Data Representation (SP does that).",
      "type": "SDR",
      "count": 0,
      "regionLevel": 1,
      "isDefaultOutput": 1
    }
  }
}"""
    net = Network()
    json_str = net.getSpecJSON("RDSEEncoderRegion")
    self.assertEqual(json_str, expected)

  def testGetSpec(self):
    """
    Test of region.getSpec() function. Testing if pybind pointers are correctly handled
    """
    net = Network()
    dateRegion = net.addRegion('dateEncoder', 'DateEncoderRegion',
      str(dict(timeOfDay_width=30,
           timeOfDay_radius=1,
           weekend_width=21)))

    dateRegion.getSpec()# twice times to check if no double free arises
    dateRegion.getSpec()
    
    
  def testGetParameters(self):
    """
    A test of the getParameters( ) and getParameterJSON() functions.
    """
    expected = """{
  "activeBits": 200,
  "category": false,
  "noise": 0.010000,
  "radius": 0.030000,
  "resolution": 0.000150,
  "seed": 2019,
  "sensedValue": 0.000000,
  "size": 1000,
  "sparsity": 0.200000
}"""
    net = Network()
    encoder = net.addRegion("encoder", "RDSEEncoderRegion", "{size: 1000, sparsity: 0.2, radius: 0.03, seed: 2019, noise: 0.01}")
    json_str = encoder.getParameters()
    self.assertEqual(json_str, expected)

    json.loads(json_str)  # test if json package can load it

    json_str = encoder.getParameterJSON("activeBits", False)
    self.assertEqual(json_str, "200")
    json_str = encoder.getParameterJSON("activeBits", True)
    self.assertEqual(json_str, "{\"value\": 200, \"type\": \"UInt32\"}")
    

  def testGetParametersCustomRegions(self):

    Network.cleanup()  # removes all previous registrations
    registerAllAdvancedRegions()
    json_list = Network.getRegistrations()
    #print(json_list)
    y = json.loads(json_list)
    self.assertTrue("py.ColumnPoolerRegion" in y)
    
    net = Network()
    #print(net.getSpecJSON("py.ColumnPoolerRegion"))
    cp = net.addRegion("py.ColumnPoolerRegion", "py.ColumnPoolerRegion",
                            """{ 
	"activationThresholdDistal": 20,
	"cellCount": 4096,
	"connectedPermanenceDistal": 0.5,
	"connectedPermanenceProximal": 0.5,
	"initialDistalPermanence": 0.51,
	"initialProximalPermanence": 0.6,
	"minThresholdProximal": 5,
	"sampleSizeDistal": 30,
	"sampleSizeProximal": 10,
	"sdrSize": 40,
	"synPermDistalDec": 0.001,
	"synPermDistalInc": 0.1,
	"synPermProximalDec": 0.001,
	"synPermProximalInc": 0.1
}""")

    # expected results from getParameters()  (in Spec order)
    expected = """{
  "learningMode": true,
  "onlineLearning": false,
  "cellCount": 4096,
  "inputWidth": 16384,
  "numOtherCorticalColumns": 0,
  "sdrSize": 40,
  "maxSdrSize": 0,
  "minSdrSize": 0,
  "synPermProximalInc": 0.100000,
  "synPermProximalDec": 0.001000,
  "initialProximalPermanence": 0.600000,
  "sampleSizeProximal": 10,
  "minThresholdProximal": 5,
  "connectedPermanenceProximal": 0.500000,
  "predictedInhibitionThreshold": 20.000000,
  "synPermDistalInc": 0.100000,
  "synPermDistalDec": 0.001000,
  "initialDistalPermanence": 0.510000,
  "sampleSizeDistal": 30,
  "activationThresholdDistal": 20,
  "connectedPermanenceDistal": 0.500000,
  "inertiaFactor": 1.000000,
  "seed": 42,
  "defaultOutputType": "active"
}"""

    json_list = cp.getParameters()
    #print(json_list)
    self.assertEqual(json_list, expected)
    

  def testGetParametersGridCell(self):
    # a test of arrays in parameters    
    Network.cleanup()  # removes all previous registrations
    registerAllAdvancedRegions()
    json_list = Network.getRegistrations()
    #print(json_list)
    y = json.loads(json_list)
    self.assertTrue("py.GridCellLocationRegion" in y)
    
    # only provide one orentation to shorten the test.  Full test in location_region_test.py
    net = Network()
    #print(net.getSpecJSON("py.GridCellLocationRegion"))
    cp = net.addRegion("grid", "py.GridCellLocationRegion",
          """{ 
  "moduleCount": 1,
  "cellsPerAxis": 10,
  "scale": [1],
  "orientation": [0.5],
  "anchorInputSize": 1,
  "activeFiringRate": 10 
}""")

    # expected results from getParameters()  (in Spec order)
    expected = """{
  "moduleCount": 1,
  "cellsPerAxis": 10,
  "scale": [1],
  "orientation": [0.5],
  "anchorInputSize": 1,
  "activeFiringRate": 10.000000,
  "bumpSigma": 0.181720,
  "activationThreshold": 10,
  "initialPermanence": 0.210000,
  "connectedPermanence": 0.500000,
  "learningThreshold": 10,
  "sampleSize": 20,
  "permanenceIncrement": 0.100000,
  "permanenceDecrement": 0.000000,
  "maxSynapsesPerSegment": -1,
  "bumpOverlapMethod": "probabilistic",
  "learningMode": false,
  "dualPhase": true,
  "dimensions": 2,
  "seed": 42
}"""

    json_list = cp.getParameters()
    #print(json_list)
    self.assertEqual(json_list, expected)
    
  
