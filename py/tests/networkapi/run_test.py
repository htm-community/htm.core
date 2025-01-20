# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2020, Numenta, Inc.
#
# author: David Keeney, dkeeney@gmail.com, Aug 2021
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
"""

import datetime
import numpy as np
import math
import unittest
import json

from htm.bindings.sdr import SDR
from htm.bindings.engine_internal import Network,Region


class NetworkAPI_Run_Test(unittest.TestCase):
  """ Unit tests for Network class. """

  def testRun(self):
    """
    A simple NetworkAPI example with three regions.
    ///////////////////////////////////////////////////////////////
    //
    //                          .------------------.
    //                         |    encoder        |          
    //                         |(RDSEEncoderRegion)|<------ inital input
    //                         |                   |          (INPUT.begin)
    //                         `-------------------'   
    //                                 | --------------. sp.bottomUpIn
    //                                 |/              | 
    //                         .-----------------.     |
    //                         |     sp          |     |
    //                         |  (SPRegion)     |     |
    //                         |                 |     |
    //                         `-----------------'     |
    //                                 |               ^
    //                         .-----------------.     |
    //                         |      tm         |     |
    //                         |   (TMRegion)    |-----'  (tm.bottomUpOut)
    //                         |                 |
    //                         `-----------------'
    //
    //////////////////////////////////////////////////////////////////
    """
    
    """ Creating network instance. """
    config = """
     {network: [
         {addRegion: {name: "encoder", type: "RDSEEncoderRegion", params: {size: 1000, sparsity: 0.2, radius: 0.03, seed: 2019, noise: 0.01}, phase: [1]}},
         {addRegion: {name: "sp", type: "SPRegion", params: {columnCount: 1000, globalInhibition: true}, phase: [2]}},
         {addRegion: {name: "tm", type: "TMRegion", params: {cellsPerColumn: 8, orColumnOutputs: true}, phase: [2]}},
         {addLink:   {src: "INPUT.begin", dest: "encoder.values", dim: [1]}},
         {addLink:   {src: "encoder.encoded", dest: "sp.bottomUpIn", mode: "overwrite"}},
         {addLink:   {src: "sp.bottomUpOut", dest: "tm.bottomUpIn"}},
         {addLink:   {src: "tm.bottomUpOut", dest: "sp.bottomUpIn"}}
      ]}"""
    net = Network()
    net.configure(config)
    
    net.initialize()
    # for debugging: print(net.getExecutionMap())
    
    """ Force initial data. """
    net.setInputData("begin", np.array([10]))

    """ Execute encoder once, the loop (sp and tm) 4 times """
    net.run(1, [1])      # execute initial entry (phase 1) 
    net.run(4, [2])      # execute loop 4 times  (phase 2)
    
    # Here is how you access the buffers
    sp_input_buffer = np.array(net.getRegion('sp').getInputArray('bottomUpIn'))
    tn_output_buffer = np.array(net.getRegion('tm').getOutputArray('bottomUpOut'))
    self.assertEqual(sp_input_buffer.size, tn_output_buffer.size)

    


    
  
