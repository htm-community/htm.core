# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2017, Numenta, Inc. 
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
# along with this program.    If not, see http://www.gnu.org/licenses.
#
# http://numenta.org/licenses/
# ----------------------------------------------------------------------

"""
A single-layer approach to computing a location
"""

import numpy as np

from htm.bindings.math import Random

from htm.advanced.algorithms.connections import Connections
from htm.advanced.support import numpy_helpers as np2
from htm.bindings.sdr import SDR


class SingleLayerLocationMemory(object):
    """
    A layer of cells which learns how to take a "delta location" (e.g. a motor
    command or a proprioceptive delta) and update its active cells to represent
    the new location.

    Its active cells might represent a union of locations.
    As the location changes, the featureLocationInput causes this union to narrow
    down until the location is inferred.

    This layer receives absolute proprioceptive info as proximal input.
    For now, we assume that there's a one-to-one mapping between absolute
    proprioceptive input and the location SDR. So rather than modeling
    proximal synapses, we'll just relay the proprioceptive SDR. In the future
    we might want to consider a many-to-one mapping of proprioceptive inputs
    to location SDRs.

    After this layer is trained, it no longer needs the proprioceptive input.
    The delta location will drive the layer. The current active cells and the
    other distal connections will work together with this delta location to
    activate a new set of cells.

    When no cells are active, activate a large union of possible locations.
    With subsequent inputs, the union will narrow down to a single location SDR.
    """

    def __init__(
        self,
        cellCount,
        deltaLocationInputSize,
        featureLocationInputSize,
        activationThreshold=13,
        initialPermanence=0.21,
        connectedPermanence=0.50,
        learningThreshold=10,
        sampleSize=20,
        permanenceIncrement=0.1,
        permanenceDecrement=0.1,
        maxSynapsesPerSegment=-1,
        seed=42,
        ):

        # Active cells represent location.
        # Each cell is modelled with two dendrites - a virtual internal/delta dendrite 
        # and a feature/location one.
        # The internal/delta dendrite has virtual segments split into two parts - one on 
        # the internal connections and one on the delta connections..        #
        # For the segment to be active, both parts must be active.
        self.internalConnections = Connections(cellCount, connectedPermanence, False)
        self.prevActiveCellsSDR = SDR(cellCount)
        self.deltaConnections = Connections(cellCount, connectedPermanence, False)
        self.deltaInputSDR = SDR(deltaLocationInputSize)

        # Distal segments that receive input from the layer that represents
        # feature-locations.
        self.featureLocationConnections = Connections(cellCount, connectedPermanence, False)
        self.featureLocationInputSDR = SDR(featureLocationInputSize)

        self.activeCells = np.empty(0, dtype="uint32")
        self.activeDeltaSegments = np.empty(0, dtype="uint32")
        self.activeFeatureLocationSegments = np.empty(0, dtype="uint32")

        self.initialPermanence = initialPermanence
        self.connectedPermanence = connectedPermanence
        self.learningThreshold = learningThreshold
        self.sampleSize = sampleSize
        self.permanenceIncrement = permanenceIncrement
        self.permanenceDecrement = permanenceDecrement
        self.activationThreshold = activationThreshold
        self.maxSynapsesPerSegment = maxSynapsesPerSegment


        self.rng = Random(seed)

    def reset(self):
        """
        Deactivate all cells.
        """

        self.activeCells = np.empty(0, dtype="uint32")
        self.activeDeltaSegments = np.empty(0, dtype="uint32")
        self.activeFeatureLocationSegments = np.empty(0, dtype="uint32")

    def compute(
        self,
        deltaLocation=(),
        newLocation=(),
        featureLocationInput=(),
        featureLocationGrowthCandidates=(),
        learn=True,
        ):
        """
        Run one time step of the Location Memory algorithm.

        @param deltaLocation (sorted numpy array)
        @param newLocation (sorted numpy array)
        @param featureLocationInput (sorted numpy array)
        @param featureLocationGrowthCandidates (sorted numpy array)
        """
        prevActiveCells = self.activeCells
        self.prevActiveCellsSDR.sparse = prevActiveCells
        self.deltaInputSDR.sparse = deltaLocation

        _, potentialOverlaps = self.internalConnections.computeActivityFull(self.prevActiveCellsSDR, False)
        _, deltaPotentialOverlaps = self.deltaConnections.computeActivityFull(self.deltaInputSDR, False)
        
        # Active delta segments occur when both the internal and its corresponding delta segment are
        # active together. That is, where the previous location and the current delta input were learnt 
        # together and hence associated with each other.
        self.activeDeltaSegments = np.where(
            (potentialOverlaps >= self.activationThreshold)
            & (deltaPotentialOverlaps >= self.activationThreshold)
            )[0]

        # When we're moving, the feature-location input has no effect.
        self.featureLocationInputSDR.sparse = featureLocationInput
        _, featurePotentialOverlaps = self.featureLocationConnections.computeActivityFull(self.featureLocationInputSDR, False)

        if len(deltaLocation) == 0:
            # Not moving
            self.activeFeatureLocationSegments = np.where(featurePotentialOverlaps >= self.activationThreshold)[0]
        else:
            self.activeFeatureLocationSegments = np.empty(0, dtype="uint32")

        if len(newLocation) > 0:
            # Drive activations by relaying this location SDR.
            # If not learning, ignore the delta and function/location input!
            self.activeCells = newLocation

            # We only learn when there is a new location, whether we are moving or not
            if learn:
                self._learnTransition(prevActiveCells, deltaLocation, newLocation, potentialOverlaps, deltaPotentialOverlaps)

                # Learn the featureLocationInput.
                self._learnFeatureLocationPair(newLocation, featureLocationInput, featureLocationGrowthCandidates, featurePotentialOverlaps)

        elif len(prevActiveCells) > 0:
            # We have an active location or feature
            if len(deltaLocation) > 0:
                # We are moving!
                # Drive activations by applying the deltaLocation to the current
                # location. Completely ignore the featureLocationInput. It's
                # outdated, associated with the previous location.

                cellsForDeltaSegments = self.internalConnections.mapSegmentsToCells(self.activeDeltaSegments)

                self.activeCells = np.unique(cellsForDeltaSegments)
            else:
                # We are not moving so keep previous active cells active.
                # Modulate with the featureLocationInput.

                if len(self.activeFeatureLocationSegments) > 0:

                    cellsForFeatureLocationSegments = (self.featureLocationConnections.mapSegmentsToCells(self.activeFeatureLocationSegments))
                    self.activeCells = np.intersect1d(prevActiveCells, cellsForFeatureLocationSegments)
                else:
                    # Not moving and no change in the feature/location so just keep 
                    # the current state
                    self.activeCells = prevActiveCells

        elif len(featureLocationInput) > 0:
            # No active cells, not moving and no location input, so drive activations 
            # with the featureLocationInput. 
            # A location was associated with this feature/location in the past 
            # so revive it.
            cellsForFeatureLocationSegments = (self.featureLocationConnections.mapSegmentsToCells(self.activeFeatureLocationSegments))
            self.activeCells = np.unique(cellsForFeatureLocationSegments)

    def _learnTransition(self, prevActiveCells, deltaLocation, newLocation, prevLocationPotentialOverlaps, deltaPotentialOverlaps):
        """
        For each cell in the newLocation SDR, learn the transition of prevLocation
        (i.e. prevActiveCells) + deltaLocation.

        The transition might be already known. In that case, just reinforce the
        existing segments.
        """
        # Matching delta segments occur where the previous location and the current delta input started to learnt together
        # when associated with each other.
        matchingDeltaSegments = np.where(
            (prevLocationPotentialOverlaps >= self.learningThreshold)
            & (deltaPotentialOverlaps >= self.learningThreshold)
            )[0]

        # Cells with a active segment pair: reinforce the segment
        cellsForActiveSegments = self.internalConnections.mapSegmentsToCells(self.activeDeltaSegments)
        # Cells that are active from the current location and are part of the new location SDR
        # should be reinforced.
        learningActiveDeltaSegments = self.activeDeltaSegments[np.in1d(cellsForActiveSegments, newLocation)]
        # Find cells that are newly active and not part of the previous location
        remainingCells = np.setdiff1d(newLocation, cellsForActiveSegments)

        # Remaining cells with a matching segment pair: reinforce the best matching
        # segment pair.
        candidateSegments = self.internalConnections.filterSegmentsByCell(matchingDeltaSegments, remainingCells)
        cellsForCandidateSegments = self.internalConnections.mapSegmentsToCells(candidateSegments)
        candidateSegments = matchingDeltaSegments[np.in1d(cellsForCandidateSegments, remainingCells)]

        onePerCellFilter = np2.argmaxMulti(
            prevLocationPotentialOverlaps[candidateSegments]
            + deltaPotentialOverlaps[candidateSegments],
            cellsForCandidateSegments,
            )
        learningMatchingDeltaSegments = candidateSegments[onePerCellFilter]

        # newDeltaSegmentCells are cells still without suitable segements to learn on
        newDeltaSegmentCells = np.setdiff1d(remainingCells, cellsForCandidateSegments)

        for learningSegments in (learningActiveDeltaSegments,learningMatchingDeltaSegments):
            # Strengthen or grow synapses to existing parallel segments in order to learn 
            # to associate the previous location and the delta input.
            self._learn(
                self.internalConnections,
                self.rng,
                learningSegments,
                self.prevActiveCellsSDR,
                prevActiveCells,
                prevLocationPotentialOverlaps,
                self.initialPermanence,
                self.sampleSize,
                self.permanenceIncrement,
                self.permanenceDecrement,
                self.maxSynapsesPerSegment,
                )
            self._learn(
                self.deltaConnections,
                self.rng,
                learningSegments,
                self.deltaInputSDR,
                deltaLocation,
                deltaPotentialOverlaps,
                self.initialPermanence,
                self.sampleSize,
                self.permanenceIncrement,
                self.permanenceDecrement,
                self.maxSynapsesPerSegment,
                )

        numNewLocationSynapses = len(prevActiveCells)
        numNewDeltaSynapses = len(deltaLocation)

        if self.sampleSize != -1:
            numNewLocationSynapses = min(numNewLocationSynapses, self.sampleSize)
            numNewDeltaSynapses = min(numNewDeltaSynapses, self.sampleSize)

        if self.maxSynapsesPerSegment != -1:
            numNewLocationSynapses = min(numNewLocationSynapses, self.maxSynapsesPerSegment)
            numNewDeltaSynapses = min(numNewLocationSynapses, self.maxSynapsesPerSegment)

        # Add new segments to cells that do not have any for the previous location and the new
        # delta and grow synapses on these new segments in order to learn to associate the previous 
        # location and the new delta input
        self.internalConnections.growSynapsesToSample(
            newDeltaSegmentCells,
            prevActiveCells,
            numNewLocationSynapses,
            self.initialPermanence,
            self.rng,
            )
        self.deltaConnections.growSynapsesToSample(
            newDeltaSegmentCells,
            deltaLocation,
            numNewDeltaSynapses,
            self.initialPermanence,
            self.rng,
            )
    
    def _learnFeatureLocationPair(self, newLocation, featureLocationInput, featureLocationGrowthCandidates, potentialOverlaps):
        """
        Grow / reinforce synapses between the location layer's dendrites and the
        input layer's active cells.
        """
        matchingSegments = np.where(potentialOverlaps > self.learningThreshold)[0]

        # Cells with a active segment pair: reinforce the segment
        cellsForActiveSegments = self.featureLocationConnections.mapSegmentsToCells(self.activeFeatureLocationSegments)
        learningActiveSegments = self.activeFeatureLocationSegments[np.in1d(cellsForActiveSegments, newLocation)]
        remainingCells = np.setdiff1d(newLocation, cellsForActiveSegments)

        # Remaining cells with a matching segment pair: reinforce the best matching
        # segment pair.
        candidateSegments = self.featureLocationConnections.filterSegmentsByCell(matchingSegments, remainingCells)
        cellsForCandidateSegments = self.featureLocationConnections.mapSegmentsToCells(candidateSegments)
        candidateSegments = candidateSegments[np.in1d(cellsForCandidateSegments, remainingCells)]
        onePerCellFilter = np2.argmaxMulti(potentialOverlaps[candidateSegments], cellsForCandidateSegments)
        learningMatchingSegments = candidateSegments[onePerCellFilter]

        newSegmentCells = np.setdiff1d(remainingCells, cellsForCandidateSegments)

        for learningSegments in (learningActiveSegments, learningMatchingSegments):
            # Strengthen or grow synapses to existing function/location segments.
            self._learn(
                self.featureLocationConnections,
                self.rng,
                learningSegments,
                self.featureLocationInputSDR,
                featureLocationGrowthCandidates,
                potentialOverlaps,
                self.initialPermanence,
                self.sampleSize,
                self.permanenceIncrement,
                self.permanenceDecrement,
                self.maxSynapsesPerSegment,
                )

        numNewSynapses = len(featureLocationInput)

        if self.sampleSize != -1:
            numNewSynapses = min(numNewSynapses, self.sampleSize)

        if self.maxSynapsesPerSegment != -1:
            numNewSynapses = min(numNewSynapses, self.maxSynapsesPerSegment)

        # Add new segments to cells that do not have any for the new function/location
        # and grow synapses on these new segments.
        self.featureLocationConnections.growSynapsesToSample(
            newSegmentCells,
            featureLocationGrowthCandidates,
            numNewSynapses,
            self.initialPermanence,
            self.rng,
            )

    @staticmethod
    def _learn(
        connections,
        rng,
        learningSegments,
        activeInput,
        growthCandidates,
        potentialOverlaps,
        initialPermanence,
        sampleSize,
        permanenceIncrement,
        permanenceDecrement,
        maxSynapsesPerSegment,
        ):
        """
        Adjust synapse permanences, grow new synapses, and grow new segments.

        @param learningActiveSegments (numpy array)
        @param learningMatchingSegments (numpy array)
        @param segmentsToPunish (numpy array)
        @param activeInput (numpy array)
        @param growthCandidates (numpy array)
        @param potentialOverlaps (numpy array)
        """

        # Learn on existing segments
        for segment in learningSegments:
            connections.adaptSegment(segment, activeInput, permanenceIncrement, permanenceDecrement, False)

            # Grow new synapses. Calculate "maxNew", the maximum number of synapses to
            # grow per segment. "maxNew" might be a number or it might be a list of
            # numbers.
            if sampleSize == -1:
                maxNew = len(growthCandidates)
            else:
                maxNew = sampleSize - potentialOverlaps[segment]
    
            if maxSynapsesPerSegment != -1:
                synapseCounts = connections.numSynapses(segment)
                numSynapsesToReachMax = maxSynapsesPerSegment - synapseCounts
                maxNew = min(maxNew, numSynapsesToReachMax)
            if maxNew > 0:
                connections.growSynapses(segment, growthCandidates, initialPermanence, rng, maxNew)


    def getActiveCells(self):
        return self.activeCells
