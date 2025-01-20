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
Unit tests for using NetworkAPI with python. 
Simular to the C++ example napi_hello.cpp
"""

import datetime
import numpy as np
import math
import unittest
import json

from htm.bindings.sdr import SDR
from htm.bindings.engine_internal import Network,Region

EPOCHS = 3

class NetworkAPI_SineWave_Test(unittest.TestCase):
  """ Unit tests for Network class. """

  def testNetwork(self):
    """
    A simple NetworkAPI example with three regions.
    ///////////////////////////////////////////////////////////////
    //
    //                          .------------------.
    //                         |    encoder        |
    //        sinewave data--->|(RDSEEncoderRegion)|
    //                         |                   |
    //                         `-------------------'
    //                                 |
    //                         .-----------------.
    //                         |     sp          |
    //                         |  (SPRegion)     |
    //                         |                 |
    //                         `-----------------'
    //                                 |
    //                         .-----------------.
    //                         |      tm         |
    //                         |   (TMRegion)    |---->anomaly score
    //                         |                 |
    //                         `-----------------'
    //
    //////////////////////////////////////////////////////////////////
    """
    
    """ Creating network instance. """
    config = """
        {network: [
            {addRegion: {name: "encoder", type: "RDSEEncoderRegion", params: {size: 1000, sparsity: 0.2, radius: 0.03, seed: 2019, noise: 0.01}}},
            {addRegion: {name: "sp", type: "SPRegion", params: {columnCount: 2048, globalInhibition: true}}},
            {addRegion: {name: "tm", type: "TMRegion", params: {cellsPerColumn: 8, orColumnOutputs: true}}},
            {addLink:   {src: "encoder.encoded", dest: "sp.bottomUpIn"}},
            {addLink:   {src: "sp.bottomUpOut", dest: "tm.bottomUpIn"}}
        ]}"""
    net = Network()
    net.configure(config)
    
    # iterate EPOCHS times
    x = 0.00
    for e in range(EPOCHS):
      # Data is a sine wave,    (Note: first iteration is for x=0.01, not 0)
      x += 0.01  # advance one step, 0.01 radians
      s = math.sin(x)   # compute current sine as data.
      #  feed data to RDSE encoder via its "sensedValue" parameter.
      net.getRegion('encoder').setParameterReal64('sensedValue', s)
      net.run(1)  # Execute one iteration of the Network object

    # Retreive the final anomaly score from the TM object's 'anomaly' output. (as a single element numpy array)
    score = np.array(net.getRegion('tm').getOutputArray('anomaly'))
    self.assertEqual(score, [1])
    #Note: Anomaly score will be 1 until there have been enough iterations to 'learn' the pattern.
    

    
