# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2019, David McDougall
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

import unittest
import pytest
import sys

from htm.bindings.sdr import SDR
from htm.bindings.math import Random
from htm.bindings.algorithms import Connections

import numpy as np

import pickle

NUM_CELLS = 4096

class ConnectionsTest(unittest.TestCase):
  
  def _getPresynapticCells(self, connections, segment, threshold):
    """
    Return a set of presynaptic cells that have synapses to segment.
    """
    return set([connections.presynapticCellForSynapse(synapse) for synapse in connections.synapsesForSegment(segment) 
                if connections.permanenceForSynapse(synapse) >= threshold])

  def testAdaptShouldNotRemoveSegments(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input = list(range(0, 10))
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input
    
    connections = Connections(NUM_CELLS, 0.51) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
    
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        self.assertEqual(len(segments), 1, "Segments were destroyed.")
        segment = segments[0]
        connections.adaptSegment(segment, inputSDR, 0.1, 0.001, False)

        segments = connections.segmentsForCell(cell)
        self.assertEqual(len(segments), 1, "Segments were destroyed.")
        segment = segments[0]

  def testAdaptShouldRemoveSegments(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input = list(range(0, 10))
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input
    
    connections = Connections(NUM_CELLS, 0.51) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
    
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        self.assertEqual(len(segments), 1, "Segments were prematurely destroyed.")
        segment = segments[0]
        connections.adaptSegment(segment, inputSDR, 0.1, 0.001, True)
        segments = connections.segmentsForCell(cell)
        self.assertEqual(len(segments), 0, "Segments were not destroyed.")

  def testAdaptShouldIncrementSynapses(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input = list(range(0, 10))
    presynaptic_input_set = set(presynaptic_input)
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input
    
    connections = Connections(NUM_CELLS, 0.51) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
    
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        for c in presynaptic_input:
          connections.createSynapse(segment, c, 0.1)          
        connections.adaptSegment(segment, inputSDR, 0.1, 0.001, True)

        presynamptic_cells = self._getPresynapticCells(connections, segment, 0.2)
        self.assertEqual(presynamptic_cells, presynaptic_input_set, "Missing synapses")

        presynamptic_cells = self._getPresynapticCells(connections, segment, 0.3)
        self.assertEqual(presynamptic_cells, set(), "Too many synapses")

  def testAdaptShouldDecrementSynapses(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input = list(range(0, 10))
    presynaptic_input_set = set(presynaptic_input)
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input
    
    connections = Connections(NUM_CELLS, 0.51) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
    
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        for c in presynaptic_input:
          connections.createSynapse(segment, c, 0.1)
          
        connections.adaptSegment(segment, inputSDR, 0.1, 0.0, False)
    
        presynamptic_cells = self._getPresynapticCells(connections, segment, 0.2)
        self.assertEqual(presynamptic_cells, presynaptic_input_set, "Missing synapses")

    presynaptic_input1 = list(range(0, 5))
    presynaptic_input_set1 = set(presynaptic_input1)
    inputSDR.sparse = presynaptic_input1
    
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        connections.adaptSegment(segment, inputSDR, 0.0, 0.1, False)
    

        presynamptic_cells = self._getPresynapticCells(connections, segment, 0.2)
        self.assertEqual(presynamptic_cells, presynaptic_input_set1, "Too many synapses")

        presynamptic_cells = self._getPresynapticCells(connections, segment, 0.1)
        self.assertEqual(presynamptic_cells, presynaptic_input_set, "Missing synapses")


  def testNumSynapses(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input = list(range(0, 10))
    presynaptic_input_set = set(presynaptic_input)
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input
    
    connections = Connections(NUM_CELLS, 0.3) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
      
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        for c in presynaptic_input:
          connections.createSynapse(segment, c, 0.1)
          
        connections.adaptSegment(segment, inputSDR, 0.1, 0.0, False)
    
        num_synapses = connections.numSynapses(segment)
        self.assertEqual(num_synapses, len(presynaptic_input), "Missing synapses")
        
    self.assertEqual(connections.numSynapses(), len(presynaptic_input) * 40, "Missing synapses")
    

  def testNumConnectedSynapses(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input = list(range(0, 10))
    presynaptic_input_set = set(presynaptic_input)
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input
    
    connections = Connections(NUM_CELLS, 0.2) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
    
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        for c in presynaptic_input:
          connections.createSynapse(segment, c, 0.1)
          
        connections.adaptSegment(segment, inputSDR, 0.1, 0.0, False)
    
        connected_synapses = connections.numConnectedSynapses(segment)
        self.assertEqual(connected_synapses, len(presynaptic_input), "Missing synapses")

    presynaptic_input1 = list(range(0, 5))
    presynaptic_input_set1 = set(presynaptic_input1)
    inputSDR.sparse = presynaptic_input1
    
    total_connected = 0

    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        connections.adaptSegment(segment, inputSDR, 0.0, 0.1, False)
    
        connected_synapses = connections.numConnectedSynapses(segment)
        self.assertEqual(connected_synapses, len(presynaptic_input1), "Missing synapses")
        
        total_connected += connected_synapses

        connected_synapses = connections.numSynapses(segment)
        self.assertEqual(connected_synapses, len(presynaptic_input), "Missing synapses")

    self.assertEqual(total_connected, len(presynaptic_input1) * 40, "Missing synapses")

  def testComputeActivity(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input = list(range(0, 10))
    presynaptic_input_set = set(presynaptic_input)
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input
    l = len(presynaptic_input)
    
    connections = Connections(NUM_CELLS, 0.51, False) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
    
    numActiveConnectedSynapsesForSegment = connections.computeActivity(inputSDR, False)
    for count in numActiveConnectedSynapsesForSegment:
      self.assertEqual(count, 0, "Segment should not be active")

    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        for c in presynaptic_input:
          connections.createSynapse(segment, c, 0.1)
        
    numActiveConnectedSynapsesForSegment = connections.computeActivity(inputSDR, False)
    for count in numActiveConnectedSynapsesForSegment:
      self.assertEqual(count, 0, "Segment should not be active")

    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]        
        connections.adaptSegment(segment, inputSDR, 0.5, 0.0, False)
        
    active_cells_set = set(active_cells)
    numActiveConnectedSynapsesForSegment = connections.computeActivity(inputSDR, False)
    for cell, count in enumerate(numActiveConnectedSynapsesForSegment):
      if cell in active_cells_set:
        self.assertEqual(count, l, "Segment should be active")
      else:
        self.assertEqual(count, 0, "Segment should not be active")
        
  def _learn(self, connections, active_cells, presynaptic_input):
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input

    for cell in active_cells:
      segments = connections.segmentsForCell(cell)
      segment = segments[0]
      for c in presynaptic_input:
        connections.createSynapse(segment, c, 0.1)
        
    for cell in active_cells:
        segments = connections.segmentsForCell(cell)
        segment = segments[0]
        connections.adaptSegment(segment, inputSDR, 0.5, 0.0, False)

  def testComputeActivityUnion(self):
    """
    Test that connections are generated on predefined segments.
    """
    random = Random(1981)
    active_cells = np.array(random.sample(np.arange(0, NUM_CELLS, 1, dtype="uint32"), 40), dtype="uint32")
    active_cells.sort()
    
    presynaptic_input1 = list(range(0, 10))
    presynaptic_input1_set = set(presynaptic_input1)
    presynaptic_input2 = list(range(10, 20))
    presynaptic_input2_set = set(presynaptic_input1)
    
    connections = Connections(NUM_CELLS, 0.51, False) 
    for i in range(NUM_CELLS):
      seg = connections.createSegment(i, 1)
    
    self._learn(connections, active_cells, presynaptic_input1)
    self._learn(connections, active_cells, presynaptic_input2)
    
    numSynapses = connections.numSynapses()
    self.assertNotEqual(numSynapses, 40, "There should be a synapse for each presynaptic cell")
    
    active_cells_set = set(active_cells)
    inputSDR = SDR(1024)
    inputSDR.sparse = presynaptic_input1
    
    numActiveConnectedSynapsesForSegment = connections.computeActivity(inputSDR, False)
    for cell, count in enumerate(numActiveConnectedSynapsesForSegment):
      if cell in active_cells_set:
        self.assertNotEqual(count, 0, "Segment should be active")

    inputSDR.sparse = presynaptic_input2
    numActiveConnectedSynapsesForSegment = connections.computeActivity(inputSDR, False)
    for cell, count in enumerate(numActiveConnectedSynapsesForSegment):
      if cell in active_cells_set:
        self.assertNotEqual(count, 0, "Segment should be active")


if __name__ == "__main__":
  unittest.main()
