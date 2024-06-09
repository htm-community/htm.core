# ------------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2019, Frederick C. Rotbart
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU Affero Public License version 3 as published by the Free
# Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Affero Public License for more details.
#
# You should have received a copy of the GNU Affero Public License along with
# this program.  If not, see http://www.gnu.org/licenses.
# ------------------------------------------------------------------------------

from htm.bindings.algorithms import Connections as CPPConnections
import numpy as np

class Connections(CPPConnections):
    
    def __init__(self, *args):
        super().__init__(*args)
        self._presynaptic_cells_for_segment = {}
    
    def presynapticCellsForSegment(self, segment):
        """
        Gets the index of this segment on its respective cell 
        from the cache until the presynaptic cells change due to learning.
        """
        if segment in self._presynaptic_cells_for_segment:
            cells = self._presynaptic_cells_for_segment[segment]

        else:
            # Not a cached value so access and cache
            cells = super().presynapticCellsForSegment(segment)
            self._presynaptic_cells_for_segment[segment] = cells

        return cells

    def createSynapse(self, segment, cell, initialPermanence):
        """
        Creates a synapse on the specified segment that connects to the presynaptic cell.
        This also clears the presynaptic cells cache sice the presynaptic cells
        may have changed.
        """
        super().createSynapse(segment, cell, initialPermanence)
        self._presynaptic_cells_for_segment = {}

    def adaptSegment(self, segment, activeInput, permanenceIncrement, permanenceDecrement, pruneZeroSynapses):
        """
        The primary method in charge of learning.   Adapts the permanence values of
        the synapses based on the input SDR.  Learning is applied to a single
        segment.  Permanence values are increased for synapses connected to input
        bits that are turned on, and decreased for synapses connected to inputs
        bits that are turned off.
        *
        @param segment  Index of segment to apply learning to.  Is returned by 
               method getSegment.
        @param inputVector  An SDR
        @param increment  Change in permanence for synapses with active presynapses.
        @param decrement  Change in permanence for synapses with inactive presynapses.
        @param pruneZeroSynapses (default Talse) If set, synapses that reach minPermanence(aka. "zero")
               are removed. This is used in TemporalMemory.
               If true, then the presynaptic cells cache is cleared since synapses may be removed.
        @param segmentThreshold (optional) (default 0) Minimum number of connected synapses for a segment
               to be considered active. @see raisePermenencesToThreshold(). Equivalent to `SP.stimulusThreshold`.
               If `pruneZeroSynapses` is used and synapses are removed, if the amount of synapses drops below 
               `segmentThreshold`, we'll remove the segment as it can never become active again. See `destroySegment`.

        """
        super().adaptSegment(segment, activeInput, permanenceIncrement, permanenceDecrement, pruneZeroSynapses)
        
        if pruneZeroSynapses:
            # Clear the cache
            self._presynaptic_cells_for_segment = {}

    def numConnectedSynapsesForCells(self, cells):
        """
        Return the number of connected synapses in the connection for the list of cells.
        """
        n = 0
        for cell in cells:
            segments = self.segmentsForCell(cell)
            for segment in segments:
                n += self.numConnectedSynapses(segment)
        return n
    
    def numSynapsesForCells(self, cells):
        """
        Return the number of connected synapses in the connection for the list of cells.
        """
        n = 0
        for cell in cells:
            segments = self.segmentsForCell(cell)
            for segment in segments:
                n += self.numSynapses(segment)
        return n
    
    def numSegmentsWithSynapses(self, cells):
        """
        Return the number of segments in the connection that have at least one synapse for the list of cells.
        """
        n = 0
        for cell in cells:
            segments = self.segmentsForCell(cell)
            for segment in segments:
                if self.numSynapses(segment) > 0:
                    n += 1
        return n
    
    def sortSegmentsByCell(self, segments):
        """
        Sort an array of segments by cell in increasing order.
        
        @param segments
            The segment array.
            
        @return:
            A sorted segment array
        """
        cells = [self.cellForSegment(s) for s in segments]
        cells_args = np.argsort(cells)
        return segments[cells_args]
    
    def filterSegmentsByCell(self, segments, cells, assumeSorted=False):
        """
        Return the subset of segments that are on the provided cells.
        
        @param segments
            The segments to filter. Must be sorted by cell.
        
        @param cells
            The cells whose segments we want to keep. Must be sorted.
        
        """
        if not assumeSorted:
            segments = self.sortSegmentsByCell(segments)

        mask = np.isin([self.cellForSegment(s) for s in segments], cells)
        return segments[mask]   

    def mapSegmentsToCells(self, segments):
        """
        Get the cell for each provided segment.
        
        @param segments
            The segments to query
        
        @param cells
            Output array with the same length as 'segments'
        """
        return np.array([self.cellForSegment(s) for s in segments], dtype=np.uint32)
    
    def getSegmentCounts(self, cells):
        """
        Get the number of segments on each of the provided cells.
        
        @param cells
        The cells to check
        
        @param counts
        Output array with the same length as 'cells'
        """
        return np.array([self.numSegments(cell) for cell in cells], dtype=np.uint32)
        
    def computeActiveSegments(self, presynapticCells, activationThreshold):
        """
        Compute the segments whose number of active synapses is greater or equal to activationThreshold
        for a vector of active presynaptic cells
        
        @param SDR presynapticCells
        The cells to check
        
        @param int activationThreshold
        The threshold that number of synapses must reach
        
        @return list
        List of segments with greater or equal number of sysnapses to activationThreshold 
        """
        overlaps = self.computeActivity(presynapticCells, False)
        return np.flatnonzero(overlaps >= activationThreshold)

    def growSynapsesToSample(
        self, 
        newSegmentCells,
        inputs,
        sampleSize,
        initialPermanence,
        rng,
        maxSegmentsPerCell=255
        ):
        """
        For each specified segments, grow synapses to a random subset of the
        inputs that aren't already connected to the segment.

        @param newSegmentCells
        The cells that need new segements to modify

        @param inputs
        The inputs to sample

        @param sampleSize
        The number of synapses to attempt to grow per segment

        @param initialPermanence
        The permanence for each added synapse

        @param maxSegmentsPerCell
        The maximum number of segments per cell.

        @param rng
        Random number generator

        Note that the arguments are slightly different to those of 
        nupic.research.core
        """
        for cell in  newSegmentCells:
            newSegment = self.createSegment(cell, maxSegmentsPerCell)
            self.growSynapses(newSegment, inputs, initialPermanence, rng, maxNew=sampleSize)

    def presynapticCellsForPostsynapticCells(self, presynapticCells, connectedPermanence):
        """
        Answer a dictionary that for each postsynaptic cell activated by the presynaptic cells, holds a list 
        of the presynaptic cells that caused that active cell to be active.
        The key is the presynaptic cell that was activated.

        Note that this is slow and really should be implemented in htm.core C++ code.
        """
        # Collect the feedforward cells that trigger active cells and associate between them.
        accummulator = defaultdict(list)
        for presynaptic_cell in presynapticCells:
            synapses = self.synapsesForPresynapticCell(presynaptic_cell)
            for synapse in synapses:
                if self.permanenceForSynapse(synapse) >= connectedPermanence:
                    segment =  self.segmentForSynapse(synapse)
                    cell =  self.cellForSegment(segment)
                    accummulator[cell].append(int(presynaptic_cell))

        return accummulator
