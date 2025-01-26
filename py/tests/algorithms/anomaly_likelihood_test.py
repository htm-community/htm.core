# ----------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2013-2014, Numenta, Inc.
#               2021, David McDougall
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
Test the anomaly likelihood stuff with specific artificial distributions of
anomaly scores. We want to specifically cover the various situations Jeff drew
on the board.
"""

from htm.algorithms.anomaly_likelihood import AnomalyLikelihood
from unittest import main, skip, TestCase as TestCaseBase


threshold = .33 # Magic threshold for anomaly detection.


class ArtificialAnomalyTest(TestCaseBase):


    @staticmethod
    def _addSampleData(origData=[], numSamples=1440, spikeValue=1.0, spikePeriod=20):
        """
        Add sample anomaly data to the existing data list and return it.
        Note: this does not modify the original data list
        """
        data = list(origData)
        for idx in range(numSamples):
            if (spikePeriod > 0) and ( (idx + 1) % spikePeriod == 0):
                data.append(spikeValue)
            else:
                data.append(0.0)
        return data


    def testCaseSingleSpike(self):
        """
        No anomalies, and then you see a single spike. That spike should be an
        anomaly.
        """
        an = AnomalyLikelihood(100)
        for _ in range(1000):
            an.compute(0)
        anom = an.compute(1)

        self.assertAlmostEqual(anom, 1.0, places=3)


    def testCaseUnusuallyHighSpikeFrequency(self):
        """
        Test B: one anomaly spike every 20 records. Then we suddenly get a bunch
        in a row. The likelihood of those spikes should be high.
        """
        an = AnomalyLikelihood()

        data = self._addSampleData(spikePeriod=20, numSamples=3000)
        anom = [an.compute(x) for x in data]

        # If we continue to see the same distribution, we should get reasonable
        # likelihoods.
        max_anom = max(anom[-100:])
        self.assertTrue(max_anom < threshold)

        # Make 20 spikes in a row.
        anom = [an.compute(1.0) for x in range(20)]
        # Check for anomaly detected.
        self.assertTrue(max(anom) > threshold)


    def testCaseContinuousBunchesOfSpikes(self):
        """
        Test D: bunches of anomalies every 40 records that continue. This should
        not be anomalous.
        """
        an = AnomalyLikelihood()

        # Generate initial data
        data = []
        for _ in range(30):
            data = self._addSampleData(data, spikePeriod=0, numSamples=30)
            data = self._addSampleData(data, spikePeriod=3, numSamples=10)

        anom = [an.compute(x) for x in data[:1000]]

        # Now feed in the same distribution
        data = self._addSampleData(      spikePeriod=0, numSamples=30)
        data = self._addSampleData(data, spikePeriod=3, numSamples=10)
        anom = [an.compute(x) for x in data]

        self.assertTrue(max(anom) < threshold)


    def testCaseIncreasedSpikeFrequency(self):
        """
        Test E: bunches of anomalies every 20 records that become even more
        frequent. This should be anomalous.
        """
        an = AnomalyLikelihood(500)

        # Generate initial data
        data = []
        for _ in range(30):
            data = self._addSampleData(data, spikePeriod=0, numSamples=30)
            data = self._addSampleData(data, spikePeriod=3, numSamples=10)

        anom = [an.compute(x) for x in data[:1000]]

        # Now feed in a more frequent distribution
        data = self._addSampleData(      spikePeriod=0, numSamples=30)
        data = self._addSampleData(data, spikePeriod=1, numSamples=10)
        anom = [an.compute(x) for x in data]

        # The likelihood should become anomalous but only near the end
        self.assertTrue(max(anom[0:30]) < threshold)
        self.assertTrue(max(anom[30:])  > threshold)


    def testCaseIncreasedAnomalyScore(self):
        """
        Test F: small anomaly score every 20 records, but then a large one when you
        would expect a small one. This should be anomalous.
        """
        an = AnomalyLikelihood()

        data = self._addSampleData(spikePeriod=20, spikeValue=0.4, numSamples=3000)
        anom = [an.compute(x) for x in data]

        # Now feed in a larger magnitude distribution.
        data = self._addSampleData(spikePeriod=20, spikeValue=1.0, numSamples=100)
        anom = [an.compute(x) for x in data]

        self.assertTrue(max(anom) > threshold)


