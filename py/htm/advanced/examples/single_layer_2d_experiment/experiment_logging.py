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

import json

import numpy as np

from single_layer_2d_experiment.runner import SingleLayer2DExperimentMonitor
from collections import defaultdict


class SingleLayer2DExperimentVisualizer(SingleLayer2DExperimentMonitor):
    """
    Logs the state of the world and the state of each layer to a CSV.
    """

    def __init__(self, exp, csvOut):
        self.exp = exp
        self.csvOut = csvOut

        self.locationRepresentations = exp.locations
        self.inputRepresentations = exp.inputRepresentations
        self.objectRepresentations = exp.objectRepresentations

        self.locationLayer = exp.locationLayer
        self.inputLayer = exp.inputLayer
        self.objectLayer = exp.objectLayer

        self.subscriberToken = exp.addMonitor(self)

        # Make it compatible with JSON -- can only use strings as dict keys.
        objects = dict(
            (objectName, list(featureLocationPairs.items()))
            for objectName, featureLocationPairs in exp.objects.items()
        )

        self.csvOut.writerow((exp.diameter,))
        self.csvOut.writerow((json.dumps({"A": "red", "B": "blue", "C": "gray"}),))
        self.csvOut.writerow((json.dumps(objects),))

        self.prevActiveLocationCells = np.array([])

    def __enter__(self, *args):
        pass

    def __exit__(self, *args):
        self.unsubscribe()

    def unsubscribe(self):

        self.exp.removeMonitor(self.subscriberToken)
        self.subscriberToken = None

    def beforeTimestep(self, locationSDR, transitionSDR, featureSDR, egocentricLocation, learn):
        self.csvOut.writerow(("t",))

        self.csvOut.writerow(("input", "newLocation"))
        self.csvOut.writerow([json.dumps(locationSDR.tolist())])
        self.csvOut.writerow(
            [
                json.dumps(
                    [
                        decoding
                        for decoding, sdr in self.exp.locations.items()
                        if np.intersect1d(locationSDR, sdr).size == sdr.size
                    ]
                )
            ]
        )

        self.csvOut.writerow(("input", "deltaLocation"))
        self.csvOut.writerow([json.dumps(transitionSDR.tolist())])
        self.csvOut.writerow(
            [
                json.dumps(
                    [
                        decoding
                        for decoding, sdr in self.exp.transitions.items()
                        if np.intersect1d(transitionSDR, sdr).size == sdr.size
                    ]
                )
            ]
        )

        self.csvOut.writerow(("input", "feature"))
        self.csvOut.writerow([json.dumps(featureSDR.tolist())])
        self.csvOut.writerow(
            [
                json.dumps(
                    [
                        k
                        for k, sdr in self.exp.features.items()
                        if np.intersect1d(featureSDR, sdr).size == sdr.size
                    ]
                )
            ]
        )

        self.csvOut.writerow(("egocentricLocation",))
        self.csvOut.writerow([json.dumps(egocentricLocation)])

    def afterReset(self):
        self.csvOut.writerow(("reset",))
        self.prevActiveLocationCells = np.array([])

    def afterPlaceObjects(self, objectPlacements):
        self.csvOut.writerow(("objectPlacements",))
        self.csvOut.writerow([json.dumps(objectPlacements)])

    def afterLocationCompute(
        self,
        deltaLocation,
        newLocation,
        featureLocationInput,
        featureLocationGrowthCandidates,
        learn,
        ):
        activeCells = self.locationLayer.getActiveCells()

        cells = dict((cell, []) for cell in activeCells.tolist())

        for cell in activeCells.tolist():
            presynapticCell = cell
            if presynapticCell in newLocation:
                segmentData = [["newLocation", [cell]]]
                cells[cell].append(segmentData)

        deltaConnections = self.locationLayer.deltaConnections
        connectedPermanence = self.locationLayer.connectedPermanence

        deltaSegments = deltaConnections.filterSegmentsByCell(self.locationLayer.activeDeltaSegments, activeCells)
        cellForDeltaSegment = deltaConnections.mapSegmentsToCells(deltaSegments)

        activeDeltaSynapsesDict = self.presynapticCellsForPostsynapticCells(deltaLocation, deltaConnections, connectedPermanence)
        activeInternalSynapsesDict = self.presynapticCellsForPostsynapticCells(self.prevActiveLocationCells, self.locationLayer.internalConnections, connectedPermanence)

        for cell in cellForDeltaSegment.tolist():
            activeDeltaSynapses = activeDeltaSynapsesDict[cell]
            activeInternalSynapses = activeInternalSynapsesDict[cell]
                
            segmentData = [
                ["deltaLocation", activeDeltaSynapses],
                ["location", activeInternalSynapses],
                ]
            cells[cell].append(segmentData)

        featureLocationConnections = self.locationLayer.featureLocationConnections

        featureLocationSegments = featureLocationConnections.filterSegmentsByCell(self.locationLayer.activeFeatureLocationSegments, activeCells)
        cellForFeatureLocationSegment = featureLocationConnections.mapSegmentsToCells(featureLocationSegments)

        activeFeatureLocationSynapsesDict = self.presynapticCellsForPostsynapticCells(featureLocationInput, featureLocationConnections, connectedPermanence)

        for cell in cellForFeatureLocationSegment.tolist():
            connectedSynapses = activeFeatureLocationSynapsesDict[cell]                
            segmentData = [["input", connectedSynapses]]
            cells[cell].append(segmentData)
       
        self.csvOut.writerow(("layer", "location"))
        self.csvOut.writerow([json.dumps(list(cells.items()))])

        decodings = [k
            for k, sdr in self.locationRepresentations.items()
            if np.intersect1d(activeCells, sdr).size == sdr.size
            ]
        self.csvOut.writerow([json.dumps(decodings)])

        self.prevActiveLocationCells = activeCells

    def afterInputCompute(
        self,
        activeColumns,
        basalInput,
        apicalInput,
        basalGrowthCandidates=None,
        apicalGrowthCandidates=None,
        learn=True,
        ):
        activeCells = self.inputLayer.getActiveCells()

        cells = dict((cell, []) for cell in activeCells.tolist())

        for cell in activeCells.tolist():
            activeColumn = cell // self.inputLayer.cellsPerColumn
            assert activeColumn in activeColumns
            segmentData = [["feature", [activeColumn]]]
            cells[cell].append(segmentData)

        connectedPermanence = self.inputLayer.connectedPermanence
        basalConnections = self.inputLayer.basalConnections

        basalSegments = basalConnections.filterSegmentsByCell(self.inputLayer.activeBasalSegments, activeCells)
        cellForBasalSegment = basalConnections.mapSegmentsToCells(basalSegments)

        basalSynapsesDict = self.presynapticCellsForPostsynapticCells(basalInput, basalConnections, connectedPermanence)

        for cell in cellForBasalSegment.tolist():
            connectedSynapses = basalSynapsesDict[cell]                
            segmentData = [["location", connectedSynapses]]
            cells[cell].append(segmentData)

        apicalConnections = self.inputLayer.apicalConnections

        apicalSegments = apicalConnections.filterSegmentsByCell(self.inputLayer.activeApicalSegments, activeCells)
        cellForApicalSegment = apicalConnections.mapSegmentsToCells(apicalSegments)

        apicalSynapsesDict = self.presynapticCellsForPostsynapticCells(apicalInput, apicalConnections, connectedPermanence)

        for cell in cellForApicalSegment.tolist():
            connectedSynapses = apicalSynapsesDict[cell]                
            segmentData = [["object", connectedSynapses]]
            cells[cell].append(segmentData)

        self.csvOut.writerow(("layer", "input"))
        self.csvOut.writerow([json.dumps(list(cells.items()))])

        decodings = [k
            for k, sdr in self.inputRepresentations.items()
            if np.intersect1d(activeCells, sdr).size == sdr.size
            ]
        self.csvOut.writerow([json.dumps(decodings)])

    def afterObjectCompute(
        self,
        feedforwardInput,
        lateralInputs=(),
        feedforwardGrowthCandidates=None,
        learn=True,
        ):
        activeCells = self.objectLayer.getActiveCells()

        cells = dict((cell, []) for cell in activeCells.tolist())

        synapsesDict = self.presynapticCellsForPostsynapticCells(
            feedforwardInput, 
            self.objectLayer.proximalPermanences, 
            self.objectLayer.connectedPermanenceProximal)

        for cell in activeCells.tolist():
            connectedSynapses = synapsesDict[cell]                
            segmentData = [["input", connectedSynapses]]
            cells[cell].append(segmentData)

        self.csvOut.writerow(("layer", "object"))
        self.csvOut.writerow([json.dumps(list(cells.items()))])

        decodings = [k
            for k, sdr in self.objectRepresentations.items()
            if np.intersect1d(activeCells, sdr).size == sdr.size
            ]
        self.csvOut.writerow([json.dumps(decodings)])


    @staticmethod
    def presynapticCellsForPostsynapticCells(
        presynapticCells, 
        permanences,
        connectedPermanence
        ):
        """
        Answer a dictionary that for each postsynaptic cell activated by the presynaptic cells, holds a list 
        of the presynaptic cells that caused that active cell to be active.
        The key is the presynaptic cell that was activated.
        """
        # Collect the feedforward cells that trigger active cells and associate between them.
        accummulator = defaultdict(list)
        for presynaptic_cell in presynapticCells:
            synapses = permanences.synapsesForPresynapticCell(presynaptic_cell)
            for synapse in synapses:
                if permanences.permanenceForSynapse(synapse) >= connectedPermanence:
                    segment =  permanences.segmentForSynapse(synapse)
                    cell =  permanences.cellForSegment(segment)
                    accummulator[cell].append(int(presynaptic_cell))

        return accummulator


