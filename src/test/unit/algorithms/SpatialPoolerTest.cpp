/* ---------------------------------------------------------------------
 * Numenta Platform for Intelligent Computing (NuPIC)
 * Copyright (C) 2013, Numenta, Inc.  Unless you have an agreement
 * with Numenta, Inc., for a separate license for this software code, the
 * following terms and conditions apply:
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with this program.  If not, see http://www.gnu.org/licenses.
 *
 * http://numenta.org/licenses/
 * ---------------------------------------------------------------------
 */

/** @file
 * Implementation of unit tests for SpatialPooler
 */

#include <cstring>
#include <fstream>
#include <stdio.h>

#include "gtest/gtest.h"
#include <nupic/algorithms/SpatialPooler.hpp>
#include <nupic/math/StlIo.hpp>
#include <nupic/types/Types.hpp>
#include <nupic/utils/Log.hpp>
#include <nupic/os/Directory.hpp>

using namespace std;
using namespace nupic;
using namespace nupic::algorithms::spatial_pooler;

namespace {
UInt countNonzero(const vector<UInt> &vec) {
  UInt count = 0;

  for (UInt x : vec) {
    if (x > 0) {
      count++;
    }
  }

  return count;
}

bool almost_eq(Real a, Real b) {
  Real diff = a - b;
  return (diff > -1e-5 && diff < 1e-5);
}

bool check_vector_eq(UInt arr[], vector<UInt> vec) {
  for (UInt i = 0; i < vec.size(); i++) {
    if (arr[i] != vec[i]) {
      return false;
    }
  }
  return true;
}

bool check_vector_eq(Real arr[], vector<Real> vec) {
  for (size_t i = 0; i < vec.size(); i++) {
    if (!almost_eq(arr[i], vec[i])) {
      return false;
    }
  }
  return true;
}

bool check_vector_eq(UInt arr1[], UInt arr2[], UInt n) {
  for (UInt i = 0; i < n; i++) {
    if (arr1[i] != arr2[i]) {
      return false;
    }
  }
  return true;
}

bool check_vector_eq(Real arr1[], Real arr2[], UInt n) {
  for (UInt i = 0; i < n; i++) {
    if (!almost_eq(arr1[i], arr2[i])) {
      return false;
    }
  }
  return true;
}

bool check_vector_eq(vector<UInt> vec1, vector<UInt> vec2) {
  if (vec1.size() != vec2.size()) {
    return false;
  }
  for (UInt i = 0; i < vec1.size(); i++) {
    if (vec1[i] != vec2[i]) {
      return false;
    }
  }
  return true;
}

bool check_vector_eq(vector<Real> vec1, vector<Real> vec2) {
  if (vec1.size() != vec2.size()) {
    return false;
  }
  for (UInt i = 0; i < vec1.size(); i++) {
    if (!almost_eq(vec1[i], vec2[i])) {
      return false;
    }
  }
  return true;
}

void check_spatial_eq(SpatialPooler sp1, SpatialPooler sp2) {
  UInt numColumns = sp1.getNumColumns();
  UInt numInputs = sp2.getNumInputs();

  ASSERT_TRUE(sp1.getNumColumns() == sp2.getNumColumns());
  ASSERT_TRUE(sp1.getNumInputs() == sp2.getNumInputs());
  ASSERT_TRUE(sp1.getPotentialRadius() == sp2.getPotentialRadius());
  ASSERT_TRUE(sp1.getPotentialPct() == sp2.getPotentialPct());
  ASSERT_TRUE(sp1.getGlobalInhibition() == sp2.getGlobalInhibition());
  ASSERT_TRUE(sp1.getNumActiveColumnsPerInhArea() == sp2.getNumActiveColumnsPerInhArea());
  ASSERT_TRUE(almost_eq(sp1.getLocalAreaDensity(), sp2.getLocalAreaDensity()));
  ASSERT_TRUE(sp1.getStimulusThreshold() == sp2.getStimulusThreshold());
  ASSERT_TRUE(sp1.getDutyCyclePeriod() == sp2.getDutyCyclePeriod());
  ASSERT_TRUE(almost_eq(sp1.getBoostStrength(), sp2.getBoostStrength()));
  ASSERT_TRUE(sp1.getIterationNum() == sp2.getIterationNum());
  ASSERT_TRUE(sp1.getIterationLearnNum() == sp2.getIterationLearnNum());
  ASSERT_TRUE(sp1.getSpVerbosity() == sp2.getSpVerbosity());
  ASSERT_TRUE(sp1.getWrapAround() == sp2.getWrapAround());
  ASSERT_TRUE(sp1.getUpdatePeriod() == sp2.getUpdatePeriod());
  ASSERT_TRUE(almost_eq(sp1.getSynPermTrimThreshold(), sp2.getSynPermTrimThreshold()));
  cout << "check: " << sp1.getSynPermActiveInc() << " "
       << sp2.getSynPermActiveInc() << endl;
  ASSERT_TRUE(almost_eq(sp1.getSynPermActiveInc(), sp2.getSynPermActiveInc()));
  ASSERT_TRUE(almost_eq(sp1.getSynPermInactiveDec(), sp2.getSynPermInactiveDec()));
  ASSERT_TRUE(almost_eq(sp1.getSynPermBelowStimulusInc(),
                        sp2.getSynPermBelowStimulusInc()));
  ASSERT_TRUE(almost_eq(sp1.getSynPermConnected(), sp2.getSynPermConnected()));
  ASSERT_TRUE(almost_eq(sp1.getMinPctOverlapDutyCycles(),
                        sp2.getMinPctOverlapDutyCycles()));

  auto boostFactors1 = new Real[numColumns];
  auto boostFactors2 = new Real[numColumns];
  sp1.getBoostFactors(boostFactors1);
  sp2.getBoostFactors(boostFactors2);
  ASSERT_TRUE(check_vector_eq(boostFactors1, boostFactors2, numColumns));
  delete[] boostFactors1;
  delete[] boostFactors2;

  auto overlapDutyCycles1 = new Real[numColumns];
  auto overlapDutyCycles2 = new Real[numColumns];
  sp1.getOverlapDutyCycles(overlapDutyCycles1);
  sp2.getOverlapDutyCycles(overlapDutyCycles2);
  ASSERT_TRUE(check_vector_eq(overlapDutyCycles1, overlapDutyCycles2, numColumns));
  delete[] overlapDutyCycles1;
  delete[] overlapDutyCycles2;

  auto activeDutyCycles1 = new Real[numColumns];
  auto activeDutyCycles2 = new Real[numColumns];
  sp1.getActiveDutyCycles(activeDutyCycles1);
  sp2.getActiveDutyCycles(activeDutyCycles2);
  ASSERT_TRUE(check_vector_eq(activeDutyCycles1, activeDutyCycles2, numColumns));
  delete[] activeDutyCycles1;
  delete[] activeDutyCycles2;

  auto minOverlapDutyCycles1 = new Real[numColumns];
  auto minOverlapDutyCycles2 = new Real[numColumns];
  sp1.getMinOverlapDutyCycles(minOverlapDutyCycles1);
  sp2.getMinOverlapDutyCycles(minOverlapDutyCycles2);
  ASSERT_TRUE(check_vector_eq(minOverlapDutyCycles1, minOverlapDutyCycles2, numColumns));
  delete[] minOverlapDutyCycles1;
  delete[] minOverlapDutyCycles2;

  for (UInt i = 0; i < numColumns; i++) {
    auto potential1 = new UInt[numInputs];
    auto potential2 = new UInt[numInputs];
    sp1.getPotential(i, potential1);
    sp2.getPotential(i, potential2);
    ASSERT_TRUE(check_vector_eq(potential1, potential2, numInputs));
    delete[] potential1;
    delete[] potential2;
  }

  for (UInt i = 0; i < numColumns; i++) {
    auto perm1 = new Real[numInputs];
    auto perm2 = new Real[numInputs];
    sp1.getPermanence(i, perm1);
    sp2.getPermanence(i, perm2);
    ASSERT_TRUE(check_vector_eq(perm1, perm2, numInputs));
    delete[] perm1;
    delete[] perm2;
  }

  for (UInt i = 0; i < numColumns; i++) {
    auto con1 = new UInt[numInputs];
    auto con2 = new UInt[numInputs];
    sp1.getConnectedSynapses(i, con1);
    sp2.getConnectedSynapses(i, con2);
    ASSERT_TRUE(check_vector_eq(con1, con2, numInputs));
    delete[] con1;
    delete[] con2;
  }

  auto conCounts1 = new UInt[numColumns];
  auto conCounts2 = new UInt[numColumns];
  sp1.getConnectedCounts(conCounts1);
  sp2.getConnectedCounts(conCounts2);
  ASSERT_TRUE(check_vector_eq(conCounts1, conCounts2, numColumns));
  delete[] conCounts1;
  delete[] conCounts2;
}

void setup(SpatialPooler &sp, UInt numInputs, UInt numColumns) {
  vector<UInt> inputDim;
  vector<UInt> columnDim;
  inputDim.push_back(numInputs);
  columnDim.push_back(numColumns);
  sp.initialize(inputDim, columnDim);
}



////////////////////////////////////////////////////////

TEST(SpatialPoolerTest, testUpdateInhibitionRadius) {
  SpatialPooler sp;
  vector<UInt> colDim, inputDim;
  colDim.push_back(57);
  colDim.push_back(31);
  colDim.push_back(2);
  inputDim.push_back(1);
  inputDim.push_back(1);
  inputDim.push_back(1);

  sp.initialize(inputDim, colDim);
  sp.setGlobalInhibition(true);
  ASSERT_TRUE(sp.getInhibitionRadius() == 57u);

  colDim.clear();
  inputDim.clear();
  // avgColumnsPerInput = 4
  // avgConnectedSpanForColumn = 3
  UInt numInputs = 3;
  inputDim.push_back(numInputs);
  UInt numCols = 12;
  colDim.push_back(numCols);
  sp.initialize(inputDim, colDim);
  sp.setGlobalInhibition(false);

  for (UInt i = 0; i < numCols; i++) {
    Real permArr[] = {1, 1, 1};
    sp.setPermanence(i, permArr);
  }
  UInt trueInhibitionRadius = 6;
  // ((3 * 4) - 1)/2 => round up
  sp.updateInhibitionRadius_();
  ASSERT_TRUE(trueInhibitionRadius == sp.getInhibitionRadius());

  colDim.clear();
  inputDim.clear();
  // avgColumnsPerInput = 1.2
  // avgConnectedSpanForColumn = 0.5
  numInputs = 5;
  inputDim.push_back(numInputs);
  numCols = 6;
  colDim.push_back(numCols);
  sp.initialize(inputDim, colDim);
  sp.setGlobalInhibition(false);

  for (UInt i = 0; i < numCols; i++) {
    Real permArr[] = {1, 0, 0, 0, 0};
    if (i % 2 == 0) {
      permArr[0] = 0;
    }
    sp.setPermanence(i, permArr);
  }
  trueInhibitionRadius = 1;
  sp.updateInhibitionRadius_();
  ASSERT_TRUE(trueInhibitionRadius == sp.getInhibitionRadius());

  colDim.clear();
  inputDim.clear();
  // avgColumnsPerInput = 2.4
  // avgConnectedSpanForColumn = 2
  numInputs = 5;
  inputDim.push_back(numInputs);
  numCols = 12;
  colDim.push_back(numCols);
  sp.initialize(inputDim, colDim);
  sp.setGlobalInhibition(false);

  for (UInt i = 0; i < numCols; i++) {
    Real permArr[] = {1, 1, 0, 0, 0};
    sp.setPermanence(i, permArr);
  }
  trueInhibitionRadius = 2;
  // ((2.4 * 2) - 1)/2 => round up
  sp.updateInhibitionRadius_();
  ASSERT_TRUE(trueInhibitionRadius == sp.getInhibitionRadius());
}

TEST(SpatialPoolerTest, testUpdateMinDutyCycles) {
  SpatialPooler sp;
  UInt numColumns = 10;
  UInt numInputs = 5;
  setup(sp, numInputs, numColumns);
  sp.setMinPctOverlapDutyCycles(0.01f);

  Real initOverlapDuty[10] = {0.01f,   0.001f, 0.02f,  0.3f,    0.012f,
                              0.0512f, 0.054f, 0.221f, 0.0873f, 0.309f};

  Real initActiveDuty[10] = {0.01f,   0.045f, 0.812f, 0.091f, 0.001f,
                             0.0003f, 0.433f, 0.136f, 0.211f, 0.129f};

  sp.setOverlapDutyCycles(initOverlapDuty);
  sp.setActiveDutyCycles(initActiveDuty);
  sp.setGlobalInhibition(true);
  sp.setInhibitionRadius(2);
  sp.updateMinDutyCycles_();
  Real resultMinOverlap[10];
  sp.getMinOverlapDutyCycles(resultMinOverlap);

  sp.updateMinDutyCyclesGlobal_();
  Real resultMinOverlapGlobal[10];
  sp.getMinOverlapDutyCycles(resultMinOverlapGlobal);

  sp.updateMinDutyCyclesLocal_();
  Real resultMinOverlapLocal[10];
  sp.getMinOverlapDutyCycles(resultMinOverlapLocal);

  ASSERT_TRUE(check_vector_eq(resultMinOverlap, resultMinOverlapGlobal, numColumns));

  sp.setGlobalInhibition(false);
  sp.updateMinDutyCycles_();
  sp.getMinOverlapDutyCycles(resultMinOverlap);

  ASSERT_TRUE(!check_vector_eq(resultMinOverlap, resultMinOverlapGlobal, numColumns));
}

TEST(SpatialPoolerTest, testUpdateMinDutyCyclesGlobal) {
  SpatialPooler sp;
  UInt numColumns = 5;
  UInt numInputs = 5;
  setup(sp, numInputs, numColumns);
  Real minPctOverlap;

  minPctOverlap = 0.01f;

  sp.setMinPctOverlapDutyCycles(minPctOverlap);

  Real overlapArr1[] = {0.06f, 1.0f, 3.0f, 6.0f, 0.5f};
  Real activeArr1[] = {0.6f, 0.07f, 0.5f, 0.4f, 0.3f};

  sp.setOverlapDutyCycles(overlapArr1);
  sp.setActiveDutyCycles(activeArr1);

  Real trueMinOverlap1 = 0.01f * 6.0f;

  sp.updateMinDutyCyclesGlobal_();
  Real resultOverlap1[5];
  sp.getMinOverlapDutyCycles(resultOverlap1);
  for (UInt i = 0; i < numColumns; i++) {
    ASSERT_TRUE(resultOverlap1[i] == trueMinOverlap1);
  }

  minPctOverlap = 0.015f;

  sp.setMinPctOverlapDutyCycles(minPctOverlap);

  Real overlapArr2[] = {0.86f, 2.4f, 0.03f, 1.6f, 1.5f};
  Real activeArr2[] = {0.16f, 0.007f, 0.15f, 0.54f, 0.13f};

  sp.setOverlapDutyCycles(overlapArr2);
  sp.setActiveDutyCycles(activeArr2);

  Real trueMinOverlap2 = 0.015f * 2.4f;

  sp.updateMinDutyCyclesGlobal_();
  Real resultOverlap2[5];
  sp.getMinOverlapDutyCycles(resultOverlap2);
  for (size_t i = 0; i < numColumns; i++) {
    ASSERT_TRUE(almost_eq(resultOverlap2[i], trueMinOverlap2));
  }

  minPctOverlap = 0.015f;

  sp.setMinPctOverlapDutyCycles(minPctOverlap);

  Real overlapArr3[] = {0, 0, 0, 0, 0};
  Real activeArr3[] = {0, 0, 0, 0, 0};

  sp.setOverlapDutyCycles(overlapArr3);
  sp.setActiveDutyCycles(activeArr3);

  Real trueMinOverlap3 = 0.0f;

  sp.updateMinDutyCyclesGlobal_();
  Real resultOverlap3[5];
  sp.getMinOverlapDutyCycles(resultOverlap3);
  for (UInt i = 0; i < numColumns; i++) {
    ASSERT_TRUE(almost_eq(resultOverlap3[i], trueMinOverlap3));
  }
}

TEST(SpatialPoolerTest, testUpdateMinDutyCyclesLocal) {
  // wrapAround=false
  {
    UInt numColumns = 8;
    SpatialPooler sp(
        /*inputDimensions*/ {5},
        /*columnDimensions*/ {numColumns},
        /*potentialRadius*/ 16,
        /*potentialPct*/ 0.5f,
        /*globalInhibition*/ false,
        /*localAreaDensity*/ -1.0f,
        /*numActiveColumnsPerInhArea*/ 3,
        /*stimulusThreshold*/ 1,
        /*synPermInactiveDec*/ 0.008f,
        /*synPermActiveInc*/ 0.05f,
        /*synPermConnected*/ 0.1f,
        /*minPctOverlapDutyCycles*/ 0.001f,
        /*dutyCyclePeriod*/ 1000,
        /*boostStrength*/ 0.0f,
        /*seed*/ 1,
        /*spVerbosity*/ 0,
        /*wrapAround*/ false);

    sp.setInhibitionRadius(1);

    Real activeDutyArr[] = {0.9f, 0.3f, 0.5f, 0.7f, 0.1f, 0.01f, 0.08f, 0.12f};
    sp.setActiveDutyCycles(activeDutyArr);

    Real overlapDutyArr[] = {0.7f, 0.1f, 0.5f, 0.01f, 0.78f, 0.55f, 0.1f, 0.001f};
    sp.setOverlapDutyCycles(overlapDutyArr);

    sp.setMinPctOverlapDutyCycles(0.2f);

    sp.updateMinDutyCyclesLocal_();

    Real trueOverlapArr[] = {  0.2f*0.7f,
                               0.2f*0.7f,
                               0.2f*0.5f,
                               0.2f*0.78f,
                               0.2f*0.78f,
                               0.2f*0.78f,
                               0.2f*0.55f,
                               0.2f*0.1f };
    Real resultMinOverlapArr[8];
    sp.getMinOverlapDutyCycles(resultMinOverlapArr);
    ASSERT_TRUE( check_vector_eq(resultMinOverlapArr, trueOverlapArr, numColumns));
  }

  // wrapAround=true
  {
    UInt numColumns = 8;
    SpatialPooler sp(
        /*inputDimensions*/ {5},
        /*columnDimensions*/ {numColumns},
        /*potentialRadius*/ 16,
        /*potentialPct*/ 0.5f,
        /*globalInhibition*/ false,
        /*localAreaDensity*/ -1.0f,
        /*numActiveColumnsPerInhArea*/ 3,
        /*stimulusThreshold*/ 1,
        /*synPermInactiveDec*/ 0.008f,
        /*synPermActiveInc*/ 0.05f,
        /*synPermConnected*/ 0.1f,
        /*minPctOverlapDutyCycles*/ 0.001f,
        /*dutyCyclePeriod*/ 1000,
        /*boostStrength*/ 10.0f,
        /*seed*/ 1,
        /*spVerbosity*/ 0,
        /*wrapAround*/ true);

    sp.setInhibitionRadius(1);

    Real activeDutyArr[] = {0.9f, 0.3f, 0.5f, 0.7f, 0.1f, 0.01f, 0.08f, 0.12f};
    sp.setActiveDutyCycles(activeDutyArr);

    Real overlapDutyArr[] = {0.7f, 0.1f, 0.5f, 0.01f, 0.78f, 0.55f, 0.1f, 0.001f};
    sp.setOverlapDutyCycles(overlapDutyArr);

    sp.setMinPctOverlapDutyCycles(0.2f);

    sp.updateMinDutyCyclesLocal_();

    Real trueOverlapArr[] =   {0.2f*0.7f,
                               0.2f*0.7f,
                               0.2f*0.5f,
                               0.2f*0.78f,
                               0.2f*0.78f,
                               0.2f*0.78f,
                               0.2f*0.55f,
                               0.2f*0.7f};
    Real resultMinOverlapArr[8];
    sp.getMinOverlapDutyCycles(resultMinOverlapArr);
    ASSERT_TRUE( check_vector_eq(resultMinOverlapArr, trueOverlapArr, numColumns));
  }
}

TEST(SpatialPoolerTest, testUpdateDutyCycles) {
  SpatialPooler sp;
  UInt numInputs = 5u;
  UInt numColumns = 5u;
  setup(sp, numInputs, numColumns);
  vector<UInt> overlaps;

  Real initOverlapArr1[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  sp.setOverlapDutyCycles(initOverlapArr1);
  UInt overlapNewVal1[] = {1u, 5u, 7u, 0u, 0u};
  overlaps.assign(overlapNewVal1, overlapNewVal1 + numColumns);
  UInt active[] = {0u, 0u, 0u, 0u, 0u};

  sp.setIterationNum(2);
  sp.updateDutyCycles_(overlaps, active);

  Real resultOverlapArr1[5];
  sp.getOverlapDutyCycles(resultOverlapArr1);

  Real trueOverlapArr1[] = {1.0f, 1.0f, 1.0f, 0.5f, 0.5f};
  ASSERT_TRUE(check_vector_eq(resultOverlapArr1, trueOverlapArr1, numColumns));

  sp.setOverlapDutyCycles(initOverlapArr1);
  sp.setIterationNum(2000);
  sp.setUpdatePeriod(1000);
  sp.updateDutyCycles_(overlaps, active);

  Real resultOverlapArr2[5];
  sp.getOverlapDutyCycles(resultOverlapArr2);
  Real trueOverlapArr2[] = {1.0f, 1.0f, 1.0f, 0.999f, 0.999f};

  ASSERT_TRUE(check_vector_eq(resultOverlapArr2, trueOverlapArr2, numColumns));
}

TEST(SpatialPoolerTest, testAvgColumnsPerInput) {
  SpatialPooler sp;
  vector<UInt> inputDim, colDim;
  inputDim.clear();
  colDim.clear();

  UInt colDim1[4] = {2u, 2u, 2u, 2u};
  UInt inputDim1[4] = {4u, 4u, 4u, 4u};
  Real trueAvgColumnPerInput1 = 0.5f;

  inputDim.assign(inputDim1, inputDim1 + 4);
  colDim.assign(colDim1, colDim1 + 4);
  sp.initialize(inputDim, colDim);
  Real result = sp.avgColumnsPerInput_();
  ASSERT_FLOAT_EQ(result, trueAvgColumnPerInput1);

  UInt colDim2[4] = {2u, 2u, 2u, 2u};
  UInt inputDim2[4] = {7u, 5u, 1u, 3u};
  Real trueAvgColumnPerInput2 = (2.0f / 7.0f + 2.0f / 5.0f + 2.0f / 1.0f + 2.0f / 3.0f) / 4.0f;

  inputDim.assign(inputDim2, inputDim2 + 4);
  colDim.assign(colDim2, colDim2 + 4);
  sp.initialize(inputDim, colDim);
  result = sp.avgColumnsPerInput_();
  ASSERT_FLOAT_EQ(result, trueAvgColumnPerInput2);

  UInt colDim3[2] = {3u, 3u};
  UInt inputDim3[2] = {3u, 3u};
  Real trueAvgColumnPerInput3 = 1.0f;

  inputDim.assign(inputDim3, inputDim3 + 2);
  colDim.assign(colDim3, colDim3 + 2);
  sp.initialize(inputDim, colDim);
  result = sp.avgColumnsPerInput_();
  ASSERT_FLOAT_EQ(result, trueAvgColumnPerInput3);

  UInt colDim4[1] = {25u};
  UInt inputDim4[1] = {5u};
  Real trueAvgColumnPerInput4 = 5.0f;

  inputDim.assign(inputDim4, inputDim4 + 1);
  colDim.assign(colDim4, colDim4 + 1);
  sp.initialize(inputDim, colDim);
  result = sp.avgColumnsPerInput_();
  ASSERT_FLOAT_EQ(result, trueAvgColumnPerInput4);

  UInt colDim5[7] = {3u, 5u, 6u};
  UInt inputDim5[7] = {3u, 5u, 6u};
  Real trueAvgColumnPerInput5 = 1.0f;

  inputDim.assign(inputDim5, inputDim5 + 3);
  colDim.assign(colDim5, colDim5 + 3);
  sp.initialize(inputDim, colDim);
  result = sp.avgColumnsPerInput_();
  ASSERT_FLOAT_EQ(result, trueAvgColumnPerInput5);

  UInt colDim6[4]   = {2u, 4u, 6u, 8u};
  UInt inputDim6[4] = {2u, 2u, 2u, 2u};
                   //  1   2   3   4
  Real trueAvgColumnPerInput6 = 2.5f;

  inputDim.assign(inputDim6, inputDim6 + 4);
  colDim.assign(colDim6, colDim6 + 4);
  sp.initialize(inputDim, colDim);
  result = sp.avgColumnsPerInput_();
  ASSERT_FLOAT_EQ(result, trueAvgColumnPerInput6);
}

TEST(SpatialPoolerTest, testAvgConnectedSpanForColumn1D) {

  SpatialPooler sp;
  UInt numColumns = 9;
  UInt numInputs = 8;
  setup(sp, numInputs, numColumns);

  Real permArr[9][8] = {{0, 1, 0, 1, 0, 1, 0, 1}, {0, 0, 0, 1, 0, 0, 0, 1},
                        {0, 0, 0, 0, 0, 0, 1, 0}, {0, 0, 1, 0, 0, 0, 1, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0, 0, 0},
                        {0, 0, 1, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 1, 0, 0, 0},
                        {1, 1, 1, 1, 1, 1, 1, 1}};

  UInt trueAvgConnectedSpan[9] = {7, 5, 1, 5, 0, 2, 3, 3, 8};

  for (UInt i = 0; i < numColumns; i++) {
    sp.setPermanence(i, permArr[i]);
    UInt result = static_cast<UInt>(sp.avgConnectedSpanForColumn1D_(i));
    ASSERT_TRUE(result == trueAvgConnectedSpan[i]);
  }
}

TEST(SpatialPoolerTest, testAvgConnectedSpanForColumn2D) {
  SpatialPooler sp;

  UInt numColumns = 7u;
  UInt numInputs = 20u;

  vector<UInt> colDim, inputDim;
  Real permArr1[7][20] =
    {{0, 1, 1, 1,
      0, 1, 1, 1,
      0, 1, 1, 1,
      0, 0, 0, 0,
      0, 0, 0, 0},
  // rowspan = 3, colspan = 3, avg = 3

     {1, 1, 1, 1,
      0, 0, 1, 1,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0},
  // rowspan = 2 colspan = 4, avg = 3

     {1, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 1},
  // row span = 5, colspan = 4, avg = 4.5

     {0, 1, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 1, 0, 0,
      0, 1, 0, 0},
  // rowspan = 5, colspan = 1, avg = 3

     {0, 0, 0, 0,
      1, 0, 0, 1,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0},
  // rowspan = 1, colspan = 4, avg = 2.5

     {0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1},
  // rowspan = 2, colspan = 2, avg = 2

     {0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0}
  // rowspan = 0, colspan = 0, avg = 0
    };

  inputDim.push_back(5);
  inputDim.push_back(4);
  colDim.push_back(10);
  colDim.push_back(1);
  sp.initialize(inputDim, colDim);

  UInt trueAvgConnectedSpan1[7] = {3, 3, 4, 3, 2, 2, 0};

  for (UInt i = 0; i < numColumns; i++) {
    sp.setPermanence(i, permArr1[i]);
    UInt result = static_cast<UInt>(sp.avgConnectedSpanForColumn2D_(i));
    ASSERT_TRUE(result == (trueAvgConnectedSpan1[i]));
  }

  // 1D tests repeated
  numColumns = 9u;
  numInputs = 8u;

  colDim.clear();
  inputDim.clear();
  inputDim.push_back(numInputs);
  inputDim.push_back(1);
  colDim.push_back(numColumns);
  colDim.push_back(1);

  sp.initialize(inputDim, colDim);

  Real permArr2[9][8] =
      {{0, 1, 0, 1, 0, 1, 0, 1},
       {0, 0, 0, 1, 0, 0, 0, 1},
       {0, 0, 0, 0, 0, 0, 1, 0},
       {0, 0, 1, 0, 0, 0, 1, 0},
       {0, 0, 0, 0, 0, 0, 0, 0},
       {0, 1, 1, 0, 0, 0, 0, 0},
       {0, 0, 1, 1, 1, 0, 0, 0},
       {0, 0, 1, 0, 1, 0, 0, 0},
       {1, 1, 1, 1, 1, 1, 1, 1}};

  UInt trueAvgConnectedSpan2[9] = {8, 5, 1, 5, 0, 2, 3, 3, 8};

  for (UInt i = 0; i < numColumns; i++) {
    sp.setPermanence(i, permArr2[i]);
    UInt result = static_cast<UInt>(sp.avgConnectedSpanForColumn2D_(i));
    ASSERT_TRUE(result == (trueAvgConnectedSpan2[i] + 1) / 2);
  }
}

TEST(SpatialPoolerTest, testAvgConnectedSpanForColumnND) {
  SpatialPooler sp;
  vector<UInt> inputDim, colDim;
  inputDim.push_back(4);
  inputDim.push_back(4);
  inputDim.push_back(2);
  inputDim.push_back(5);
  colDim.push_back(5);
  colDim.push_back(1);
  colDim.push_back(1);
  colDim.push_back(1);

  sp.initialize(inputDim, colDim);

  UInt numInputs = 160;
  UInt numColumns = 5;

  Real permArr0[4][4][2][5];
  Real permArr1[4][4][2][5];
  Real permArr2[4][4][2][5];
  Real permArr3[4][4][2][5];
  Real permArr4[4][4][2][5];

  for (UInt i = 0; i < numInputs; i++) {
    ((Real *)permArr0)[i] = 0;
    ((Real *)permArr1)[i] = 0;
    ((Real *)permArr2)[i] = 0;
    ((Real *)permArr3)[i] = 0;
    ((Real *)permArr4)[i] = 0;
  }

  permArr0[1][0][1][0] = 1;
  permArr0[1][0][1][1] = 1;
  permArr0[3][2][1][0] = 1;
  permArr0[3][0][1][0] = 1;
  permArr0[1][0][1][3] = 1;
  permArr0[2][2][1][0] = 1;

  permArr1[2][0][1][0] = 1;
  permArr1[2][0][0][0] = 1;
  permArr1[3][0][0][0] = 1;
  permArr1[3][0][1][0] = 1;

  permArr2[0][0][1][4] = 1;
  permArr2[0][0][0][3] = 1;
  permArr2[0][0][0][1] = 1;
  permArr2[1][0][0][2] = 1;
  permArr2[0][0][1][1] = 1;
  permArr2[3][3][1][1] = 1;

  permArr3[3][3][1][4] = 1;
  permArr3[0][0][0][0] = 1;

  sp.setPermanence(0, (Real *)permArr0);
  sp.setPermanence(1, (Real *)permArr1);
  sp.setPermanence(2, (Real *)permArr2);
  sp.setPermanence(3, (Real *)permArr3);
  sp.setPermanence(4, (Real *)permArr4);

  Real trueAvgConnectedSpan[5] = {11.0f / 4.0f, 6.0f / 4.0f, 14.0f / 4.0f, 15.0f / 4.0f, 0};

  for (UInt i = 0; i < numColumns; i++) {
    Real result = sp.avgConnectedSpanForColumnND_(i);
    ASSERT_TRUE(result == trueAvgConnectedSpan[i]);
  }
}

TEST(SpatialPoolerTest, testAdaptSynapses) {
  SpatialPooler sp;
  UInt numColumns = 4;
  UInt numInputs = 8;
  setup(sp, numInputs, numColumns);

  vector<UInt> activeColumns;
  vector<UInt> inputVector;

  UInt potentialArr1[4][8] =
      {{1, 1, 1, 1, 0, 0, 0, 0},
       {1, 0, 0, 0, 1, 1, 0, 1},
       {0, 0, 1, 0, 0, 0, 1, 0},
       {1, 0, 0, 0, 0, 0, 1, 0}};

  Real permanencesArr1[5][8] =
      {{0.200f, 0.120f, 0.090f, 0.060f, 0.000f, 0.000f, 0.000f, 0.000f},
       {0.150f, 0.000f, 0.000f, 0.000f, 0.180f, 0.120f, 0.000f, 0.450f},
       {0.000f, 0.000f, 0.014f, 0.000f, 0.000f, 0.000f, 0.110f, 0.000f},
       {0.070f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.178f, 0.000f}};

  Real truePermanences1[5][8] =
      {{ 0.300f, 0.110f, 0.080f, 0.160f, 0.000f, 0.000f, 0.000f, 0.000f},
      //   Inc     Dec   Dec     Inc       -       -       -       -
        {0.250f, 0.000f, 0.000f, 0.000f, 0.280f, 0.110f, 0.000f, 0.440f},
      //   Inc      -      -      -        Inc     Dec     -       Dec
        {0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.210f, 0.000f},
      //   -       -       Trim    -      -        -       Inc     -
        {0.070f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.178f, 0.000f}};
      //    -      -      -      -      -      -      -       -

  UInt inputArr1[8] = {1, 0, 0, 1, 1, 0, 1, 0};
  UInt activeColumnsArr1[3] = {0, 1, 2};

  for (UInt column = 0; column < numColumns; column++) {
    sp.setPotential(column, potentialArr1[column]);
    sp.setPermanence(column, permanencesArr1[column]);
  }

  activeColumns.assign(&activeColumnsArr1[0], &activeColumnsArr1[3]);

  sp.adaptSynapses_(inputArr1, activeColumns);
  cout << endl;
  for (UInt column = 0; column < numColumns; column++) {
    auto permArr = new Real[numInputs];
    sp.getPermanence(column, permArr);
    ASSERT_TRUE(check_vector_eq(truePermanences1[column], permArr, numInputs));
    delete[] permArr;
  }


  UInt potentialArr2[4][8] =
      {{1, 1, 1, 0, 0, 0, 0, 0},
       {0, 1, 1, 1, 0, 0, 0, 0},
       {0, 0, 1, 1, 1, 0, 0, 0},
       {1, 0, 0, 0, 0, 0, 1, 0}};

  Real permanencesArr2[4][8] =
      {{0.200f, 0.120f, 0.090f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f},
       {0.000f, 0.017f, 0.232f, 0.400f, 0.000f, 0.000f, 0.000f, 0.000f},
       {0.000f, 0.000f, 0.014f, 0.051f, 0.730f, 0.000f, 0.000f, 0.000f},
       {0.170f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.380f, 0.000f}};

  Real truePermanences2[4][8] =
      {{0.300f, 0.110f, 0.080f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f},
    // #  Inc     Dec     Dec     -       -       -       -       -
       {0.000f, 0.000f, 0.222f, 0.500f, 0.000f, 0.000f, 0.000f, 0.000f},
    // #  -       Trim    Dec     Inc     -       -       -       -
       {0.000f, 0.000f, 0.000f, 0.151f, 0.830f, 0.000f, 0.000f, 0.000f},
    // #  -       -       Trim    Inc     Inc     -       -       -
       {0.170f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.380f, 0.000f}};
    // #  -       -       -       -       -       -       -       -

  UInt inputArr2[8] = {1, 0, 0, 1, 1, 0, 1, 0};
  UInt activeColumnsArr2[3] = {0, 1, 2};

  for (UInt column = 0; column < numColumns; column++) {
    sp.setPotential(column, potentialArr2[column]);
    sp.setPermanence(column, permanencesArr2[column]);
  }

  activeColumns.assign(&activeColumnsArr2[0], &activeColumnsArr2[3]);

  sp.adaptSynapses_(inputArr2, activeColumns);
  cout << endl;
  for (UInt column = 0; column < numColumns; column++) {
    auto permArr = new Real[numInputs];
    sp.getPermanence(column, permArr);
    ASSERT_TRUE(check_vector_eq(truePermanences2[column], permArr, numInputs));
    delete[] permArr;
  }
}

TEST(SpatialPoolerTest, testBumpUpWeakColumns) {
  SpatialPooler sp;
  UInt numInputs = 8;
  UInt numColumns = 5;
  setup(sp, numInputs, numColumns);
  sp.setSynPermBelowStimulusInc(0.01f);
  sp.setSynPermTrimThreshold(0.05f);
  Real overlapDutyCyclesArr[] = {0.000f, 0.009f, 0.100f, 0.001f, 0.002f};
  sp.setOverlapDutyCycles(overlapDutyCyclesArr);
  Real minOverlapDutyCyclesArr[] = {0.01f, 0.01f, 0.01f, 0.01f, 0.01f};
  sp.setMinOverlapDutyCycles(minOverlapDutyCyclesArr);

  UInt potentialArr[5][8] =
      {{1, 1, 1, 1, 0, 0, 0, 0},
       {1, 0, 0, 0, 1, 1, 0, 1},
       {0, 0, 1, 0, 1, 1, 1, 0},
       {1, 1, 1, 0, 0, 0, 1, 0},
       {1, 1, 1, 1, 1, 1, 1, 1}};

  Real permArr[5][8] =
      {{0.200f, 0.120f, 0.090f, 0.040f, 0.000f, 0.000f, 0.000f, 0.000f},
       {0.150f, 0.000f, 0.000f, 0.000f, 0.180f, 0.120f, 0.000f, 0.450f},
       {0.000f, 0.000f, 0.074f, 0.000f, 0.062f, 0.054f, 0.110f, 0.000f},
       {0.051f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.178f, 0.000f},
       {0.100f, 0.738f, 0.085f, 0.002f, 0.052f, 0.008f, 0.208f, 0.034f}};

  Real truePermArr[5][8] =
      {{0.210f, 0.130f, 0.100f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f},
    //    Inc     Inc     Inc     Trim    -       -       -       -
       {0.160f, 0.000f, 0.000f, 0.000f, 0.190f, 0.130f, 0.000f, 0.460f},
    //    Inc     -       -       -       Inc     Inc     -       Inc
       {0.000f, 0.000f, 0.074f, 0.000f, 0.062f, 0.054f, 0.110f, 0.000f},  // unchanged
    //    -       -       -       -       -       -       -       -
       {0.061f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.188f, 0.000f},
    //    Inc     Trim    Trim    -       -       -       Inc     -
       {0.110f, 0.748f, 0.095f, 0.000f, 0.062f, 0.000f, 0.218f, 0.000f}};

  for (UInt i = 0; i < numColumns; i++) {
    sp.setPotential(i, potentialArr[i]);
    sp.setPermanence(i, permArr[i]);
    Real perm[8];
    sp.getPermanence(i, perm);
  }

  sp.bumpUpWeakColumns_();

  for (UInt i = 0; i < numColumns; i++) {
    Real perm[8];
    sp.getPermanence(i, perm);
    ASSERT_TRUE(check_vector_eq(truePermArr[i], perm, numInputs));
  }
}

TEST(SpatialPoolerTest, testUpdateDutyCyclesHelper) {
  SpatialPooler sp;
  vector<Real> dutyCycles;
  vector<UInt> newValues;
  UInt period;

  dutyCycles.clear();
  newValues.clear();
  Real dutyCyclesArr1[] = {1000.0, 1000.0, 1000.0, 1000.0, 1000.0};
  UInt newValues1[] = {0, 0, 0, 0, 0};
  period = 1000;
  Real trueDutyCycles1[] = {999.0, 999.0, 999.0, 999.0, 999.0};
  dutyCycles.assign(dutyCyclesArr1, dutyCyclesArr1 + 5);
  newValues.assign(newValues1, newValues1 + 5);
  sp.updateDutyCyclesHelper_(dutyCycles, newValues, period);
  ASSERT_TRUE(check_vector_eq(trueDutyCycles1, dutyCycles));

  dutyCycles.clear();
  newValues.clear();
  Real dutyCyclesArr2[] = {1000.0, 1000.0, 1000.0, 1000.0, 1000.0};
  UInt newValues2[] = {1000, 1000, 1000, 1000, 1000};
  period = 1000;
  Real trueDutyCycles2[] = {1000.0, 1000.0, 1000.0, 1000.0, 1000.0};
  dutyCycles.assign(dutyCyclesArr2, dutyCyclesArr2 + 5);
  newValues.assign(newValues2, newValues2 + 5);
  sp.updateDutyCyclesHelper_(dutyCycles, newValues, period);
  ASSERT_TRUE(check_vector_eq(trueDutyCycles2, dutyCycles));

  dutyCycles.clear();
  newValues.clear();
  Real dutyCyclesArr3[] = {1000.0, 1000.0, 1000.0, 1000.0, 1000.0};
  UInt newValues3[] = {2000, 4000, 5000, 6000, 7000};
  period = 1000;
  Real trueDutyCycles3[] = {1001.0, 1003.0, 1004.0, 1005.0, 1006.0};
  dutyCycles.assign(dutyCyclesArr3, dutyCyclesArr3 + 5);
  newValues.assign(newValues3, newValues3 + 5);
  sp.updateDutyCyclesHelper_(dutyCycles, newValues, period);
  ASSERT_TRUE(check_vector_eq(trueDutyCycles3, dutyCycles));

  dutyCycles.clear();
  newValues.clear();
  Real dutyCyclesArr4[] = {1000.0, 800.0, 600.0, 400.0, 2000.0};
  UInt newValues4[] = {0, 0, 0, 0, 0};
  period = 2;
  Real trueDutyCycles4[] = {500.0, 400.0, 300.0, 200.0, 1000.0};
  dutyCycles.assign(dutyCyclesArr4, dutyCyclesArr4 + 5);
  newValues.assign(newValues4, newValues4 + 5);
  sp.updateDutyCyclesHelper_(dutyCycles, newValues, period);
  ASSERT_TRUE(check_vector_eq(trueDutyCycles4, dutyCycles));
}

TEST(SpatialPoolerTest, testUpdateBoostFactors) {
  SpatialPooler sp;
  setup(sp, 5, 6);

  Real initActiveDutyCycles1[] = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
  Real initBoostFactors1[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
  vector<Real> trueBoostFactors1 = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  vector<Real> resultBoostFactors1(6, 0);
  sp.setGlobalInhibition(false);
  sp.setBoostStrength(10);
  sp.setBoostFactors(initBoostFactors1);
  sp.setActiveDutyCycles(initActiveDutyCycles1);
  sp.updateBoostFactors_();
  sp.getBoostFactors(resultBoostFactors1.data());
  ASSERT_TRUE(check_vector_eq(trueBoostFactors1, resultBoostFactors1));

  Real initActiveDutyCycles2[] = {0.1f, 0.3f, 0.02f, 0.04f, 0.7f, 0.12f};
  Real initBoostFactors2[] =  {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
  vector<Real> trueBoostFactors2 =
      {3.10599f, 0.42035f, 6.91251f, 5.65949f, 0.00769898f, 2.54297f};
  vector<Real> resultBoostFactors2(6, 0);
  sp.setGlobalInhibition(false);
  sp.setBoostStrength(10);
  sp.setBoostFactors(initBoostFactors2);
  sp.setActiveDutyCycles(initActiveDutyCycles2);
  sp.updateBoostFactors_();
  sp.getBoostFactors(resultBoostFactors2.data());

  ASSERT_TRUE(check_vector_eq(trueBoostFactors2, resultBoostFactors2));

  Real initActiveDutyCycles3[] = {0.1f, 0.3f, 0.02f, 0.04f, 0.7f, 0.12f};
  Real initBoostFactors3[] =  {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
  vector<Real> trueBoostFactors3 =
      { 1.25441f, 0.840857f, 1.47207f, 1.41435f, 0.377822f, 1.20523f };
  vector<Real> resultBoostFactors3(6, 0);
  sp.setWrapAround(true);
  sp.setGlobalInhibition(false);
  sp.setBoostStrength(2.0);
  sp.setInhibitionRadius(5);
  sp.setNumActiveColumnsPerInhArea(1);
  sp.setBoostFactors(initBoostFactors3);
  sp.setActiveDutyCycles(initActiveDutyCycles3);
  sp.updateBoostFactors_();
  sp.getBoostFactors(resultBoostFactors3.data());

  ASSERT_TRUE(check_vector_eq(trueBoostFactors3, resultBoostFactors3));

  Real initActiveDutyCycles4[] =  {0.1f, 0.3f, 0.02f, 0.04f, 0.7f, 0.12f};
  Real initBoostFactors4[] =      {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
  vector<Real> trueBoostFactors4 =
      { 1.94773f, 0.263597f, 4.33476f, 3.549f, 0.00482795f, 1.59467f };
  vector<Real> resultBoostFactors4(6, 0);
  sp.setGlobalInhibition(true);
  sp.setBoostStrength(10);
  sp.setNumActiveColumnsPerInhArea(1);
  sp.setInhibitionRadius(3);
  sp.setBoostFactors(initBoostFactors4);
  sp.setActiveDutyCycles(initActiveDutyCycles4);
  sp.updateBoostFactors_();
  sp.getBoostFactors(resultBoostFactors4.data());

  ASSERT_TRUE(check_vector_eq(trueBoostFactors3, resultBoostFactors3));
}

TEST(SpatialPoolerTest, testUpdateBookeepingVars) {
  SpatialPooler sp;
  sp.setIterationNum(5);
  sp.setIterationLearnNum(3);
  sp.updateBookeepingVars_(true);
  ASSERT_TRUE(6 == sp.getIterationNum());
  ASSERT_TRUE(4 == sp.getIterationLearnNum());

  sp.updateBookeepingVars_(false);
  ASSERT_TRUE(7 == sp.getIterationNum());
  ASSERT_TRUE(4 == sp.getIterationLearnNum());
}

TEST(SpatialPoolerTest, testCalculateOverlap) {
  SpatialPooler sp;
  UInt numInputs = 10;
  UInt numColumns = 5;
  UInt numTrials = 5;
  setup(sp, numInputs, numColumns);
  sp.setStimulusThreshold(0);

  Real permArr[5][10] = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                         {0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                         {0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
                         {0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
                         {0, 0, 0, 0, 0, 0, 0, 0, 1, 1}};

  UInt inputs[5][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                        {0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
                        {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1}};

  UInt trueOverlaps[5][5] = {{0, 0, 0, 0, 0},
                             {10, 8, 6, 4, 2},
                             {5, 4, 3, 2, 1},
                             {5, 3, 1, 0, 0},
                             {1, 1, 1, 1, 1}};

  for (UInt i = 0; i < numColumns; i++) {
    sp.setPermanence(i, permArr[i]);
  }

  for (UInt i = 0; i < numTrials; i++) {
    vector<UInt> overlaps;
    sp.calculateOverlap_(inputs[i], overlaps);
    ASSERT_TRUE(check_vector_eq(trueOverlaps[i], overlaps));
  }
}

TEST(SpatialPoolerTest, testCalculateOverlapPct) {
  SpatialPooler sp;
  UInt numInputs = 10;
  UInt numColumns = 5;
  UInt numTrials = 5;
  setup(sp, numInputs, numColumns);
  sp.setStimulusThreshold(0);

  Real permArr[5][10] = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                         {0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                         {0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
                         {0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
                         {0, 0, 0, 0, 0, 0, 0, 0, 1, 1}};

  UInt overlapsArr[5][10] = {{0, 0, 0, 0, 0},
                             {10, 8, 6, 4, 2},
                             {5, 4, 3, 2, 1},
                             {5, 3, 1, 0, 0},
                             {1, 1, 1, 1, 1}};

  Real trueOverlapsPct[5][5] =
      {{0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
       {1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
       {0.5f, 0.5f, 0.5f, 0.5f, 0.5f},
       {0.5f, 3.0f/8, 1.0f/6,  0.0f,  0.0f},
       { 1.0f/10,  1.0f/8,  1.0f/6,  1.0f/4,  1.0f/2}};

  for (UInt i = 0; i < numColumns; i++) {
    sp.setPermanence(i, permArr[i]);
  }

  for (UInt i = 0; i < numTrials; i++) {
    vector<Real> overlapsPct;
    vector<UInt> overlaps;
    overlaps.assign(&overlapsArr[i][0], &overlapsArr[i][numColumns]);
    sp.calculateOverlapPct_(overlaps, overlapsPct);
    ASSERT_TRUE(check_vector_eq(trueOverlapsPct[i], overlapsPct));
  }
}

TEST(SpatialPoolerTest, testInhibitColumns) {
  SpatialPooler sp;
  setup(sp, 10, 10);

  vector<Real> overlapsReal;
  vector<Real> overlaps;
  vector<UInt> activeColumns;
  vector<UInt> activeColumnsGlobal;
  vector<UInt> activeColumnsLocal;
  Real density;
  UInt inhibitionRadius;
  UInt numColumns;

  density = 0.3f;
  numColumns = 10;
  Real overlapsArray[10] = {10.0f,21.0f,34.0f,4.0f,18.0f,3.0f,12.0f,5.0f,7.0f,1.0f };

  overlapsReal.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumnsGlobal_(overlapsReal, density, activeColumnsGlobal);
  overlapsReal.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumnsLocal_(overlapsReal, density, activeColumnsLocal);

  sp.setInhibitionRadius(5);
  sp.setGlobalInhibition(true);
  sp.setLocalAreaDensity(density);

  overlaps.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumns_(overlaps, activeColumns);

  ASSERT_TRUE(check_vector_eq(activeColumns, activeColumnsGlobal));
  ASSERT_TRUE(!check_vector_eq(activeColumns, activeColumnsLocal));

  sp.setGlobalInhibition(false);
  sp.setInhibitionRadius(numColumns + 1);

  overlaps.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumns_(overlaps, activeColumns);

  ASSERT_TRUE(check_vector_eq(activeColumns, activeColumnsGlobal));
  ASSERT_TRUE(!check_vector_eq(activeColumns, activeColumnsLocal));

  inhibitionRadius = 2;
  density = 2.0f / 5;

  sp.setInhibitionRadius(inhibitionRadius);
  sp.setNumActiveColumnsPerInhArea(2);

  overlapsReal.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumnsGlobal_(overlapsReal, density, activeColumnsGlobal);
  overlapsReal.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumnsLocal_(overlapsReal, density, activeColumnsLocal);

  overlaps.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumns_(overlaps, activeColumns);

  ASSERT_TRUE(!check_vector_eq(activeColumns, activeColumnsGlobal));
  ASSERT_TRUE(check_vector_eq(activeColumns, activeColumnsLocal));
}

TEST(SpatialPoolerTest, testInhibitColumnsGlobal) {
  SpatialPooler sp;
  UInt numInputs = 10;
  UInt numColumns = 10;
  setup(sp, numInputs, numColumns);
  vector<Real> overlaps;
  vector<UInt> activeColumns;
  vector<UInt> trueActive;
  vector<UInt> active;
  Real density;

  density = 0.3f;
  Real overlapsArray[10] = {1.0f,2.0f,1.0f,4.0f,8.0f,3.0f,12.0f,5.0f,4.0f,1.0f };
  overlaps.assign(&overlapsArray[0], &overlapsArray[numColumns]);
  sp.inhibitColumnsGlobal_(overlaps, density, activeColumns);
  UInt trueActiveArray1[3] = {4, 6, 7};

  trueActive.assign(numColumns, 0);
  active.assign(numColumns, 0);

  for (auto &elem : trueActiveArray1) {
    trueActive[elem] = 1;
  }

  for (auto &activeColumn : activeColumns) {
    active[activeColumn] = 1;
  }

  ASSERT_TRUE(check_vector_eq(trueActive, active));

  density = 0.5f;
  Real overlapsArray2[10] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                             6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
  overlaps.assign(&overlapsArray2[0], &overlapsArray2[numColumns]);
  sp.inhibitColumnsGlobal_(overlaps, density, activeColumns);
  UInt trueActiveArray2[5] = {5, 6, 7, 8, 9};

  for (auto &elem : trueActiveArray2) {
    trueActive[elem] = 1;
  }

  for (auto &activeColumn : activeColumns) {
    active[activeColumn] = 1;
  }

  ASSERT_TRUE(check_vector_eq(trueActive, active));
}

TEST(SpatialPoolerTest, testValidateGlobalInhibitionParameters) {
  // With 10 columns the minimum sparsity for global inhibition is 10%
  // Setting sparsity to 2% should throw an exception
  SpatialPooler sp;
  setup(sp, 10, 10);
  sp.setGlobalInhibition(true);
  sp.setLocalAreaDensity(0.02f);
  vector<UInt> input(sp.getNumInputs(), 1);
  vector<UInt> out1(sp.getNumColumns(), 0);
  EXPECT_THROW(sp.compute(input.data(), false, out1.data()),
               nupic::LoggingException);
}


TEST(SpatialPoolerTest, testFewColumnsGlobalInhibitionCrash) {
  /** this test exposes bug where too small (few columns) SP crashes with global inhibition  */
  SpatialPooler sp{std::vector<UInt>{1000} /* input*/, std::vector<UInt>{200}/* SP output cols XXX sensitive*/ };
  sp.setBoostStrength(0.0);
  sp.setPotentialRadius(20);
  sp.setPotentialPct(0.5);
  sp.setGlobalInhibition(true);
  sp.setLocalAreaDensity(0.02f);

  vector<UInt> input(sp.getNumInputs(), 1);
  vector<UInt> out1(sp.getNumColumns(), 0);

  EXPECT_NO_THROW(sp.compute(input.data(), false, out1.data()));
}


TEST(SpatialPoolerTest, testInhibitColumnsLocal) {
  // wrapAround = false
  {
    SpatialPooler sp(
        /*inputDimensions*/ {10},
        /*columnDimensions*/ {10},
        /*potentialRadius*/ 16,
        /*potentialPct*/ 0.5f,
        /*globalInhibition*/ false,
        /*localAreaDensity*/ -1.0f,
        /*numActiveColumnsPerInhArea*/ 3,
        /*stimulusThreshold*/ 1,
        /*synPermInactiveDec*/ 0.008f,
        /*synPermActiveInc*/ 0.05f,
        /*synPermConnected*/ 0.1f,
        /*minPctOverlapDutyCycles*/ 0.001f,
        /*dutyCyclePeriod*/ 1000,
        /*boostStrength*/ 10.0f,
        /*seed*/ 1,
        /*spVerbosity*/ 0,
        /*wrapAround*/ false);

    Real density;
    UInt inhibitionRadius;

    vector<Real> overlaps;
    vector<UInt> active;

    Real overlapsArray1[10] = { 1.0f, 2.0f, 7.0f, 0.0f, 3.0f, 4.0f, 16.0f, 1.0f, 1.5f, 1.7f};
                              //  L     W     W     L     L     W     W      L     L     W

    inhibitionRadius = 2;
    density = 0.5f;
    overlaps.assign(&overlapsArray1[0], &overlapsArray1[10]);
    UInt trueActive[5] = {1, 2, 5, 6, 9};
    sp.setInhibitionRadius(inhibitionRadius);
    sp.inhibitColumnsLocal_(overlaps, density, active);
    ASSERT_EQ(5u, active.size());
    ASSERT_TRUE(check_vector_eq(trueActive, active));

    Real overlapsArray2[10] = {1.0f, 2.0f, 7.0f, 0.0f, 3.0f, 4.0f, 16.0f, 1.0f, 1.5f, 1.7f};
                            //   L     W     W     L     L     W     W      L     L     W
    overlaps.assign(&overlapsArray2[0], &overlapsArray2[10]);
    UInt trueActive2[6] = {1, 2, 4, 5, 6, 9};
    inhibitionRadius = 3;
    density = 0.5f;
    sp.setInhibitionRadius(inhibitionRadius);
    sp.inhibitColumnsLocal_(overlaps, density, active);
    ASSERT_TRUE(active.size() == 6);
    ASSERT_TRUE(check_vector_eq(trueActive2, active));

    // Test arbitration

    Real overlapsArray3[10] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
                              // W     L     W     L     W     L     W     L     L     L
    overlaps.assign(&overlapsArray3[0], &overlapsArray3[10]);
    UInt trueActive3[4] = {0, 2, 4, 6};
    inhibitionRadius = 3;
    density = 0.25f;
    sp.setInhibitionRadius(inhibitionRadius);
    sp.inhibitColumnsLocal_(overlaps, density, active);

    ASSERT_TRUE(active.size() == 4u);
    ASSERT_TRUE(check_vector_eq(trueActive3, active));
  }

  // wrapAround = true
  {
    SpatialPooler sp(
        /*inputDimensions*/ {10},
        /*columnDimensions*/ {10},
        /*potentialRadius*/ 16,
        /*potentialPct*/ 0.5f,
        /*globalInhibition*/ false,
        /*localAreaDensity*/ -1.0f,
        /*numActiveColumnsPerInhArea*/ 3,
        /*stimulusThreshold*/ 1,
        /*synPermInactiveDec*/ 0.008f,
        /*synPermActiveInc*/ 0.05f,
        /*synPermConnected*/ 0.1f,
        /*minPctOverlapDutyCycles*/ 0.001f,
        /*dutyCyclePeriod*/ 1000,
        /*boostStrength*/ 10.0f,
        /*seed*/ 1,
        /*spVerbosity*/ 0,
        /*wrapAround*/ true);

    Real density;
    UInt inhibitionRadius;

    vector<Real> overlaps;
    vector<UInt> active;

    Real overlapsArray1[10] = { 1.0f, 2.0f, 7.0f, 0.0f, 3.0f, 4.0f, 16.0f, 1.0f, 1.5f, 1.7f};
                              //  L     W     W     L     L     W     W      L     W     W

    inhibitionRadius = 2;
    density = 0.5f;
    overlaps.assign(&overlapsArray1[0], &overlapsArray1[10]);
    UInt trueActive[6] = {1, 2, 5, 6, 8, 9};
    sp.setInhibitionRadius(inhibitionRadius);
    sp.inhibitColumnsLocal_(overlaps, density, active);
    ASSERT_EQ(6u, active.size());
    ASSERT_TRUE(check_vector_eq(trueActive, active));

    Real overlapsArray2[10] = {1.0f, 2.0f, 7.0f, 0.0f, 3.0f, 4.0f, 16.0f, 1.0f, 1.5f, 1.7f};
                            //   L     W     W     L     W     W     W      L     L     W
    overlaps.assign(&overlapsArray2[0], &overlapsArray2[10]);
    UInt trueActive2[6] = {1, 2, 4, 5, 6, 9};
    inhibitionRadius = 3;
    density = 0.5f;
    sp.setInhibitionRadius(inhibitionRadius);
    sp.inhibitColumnsLocal_(overlaps, density, active);
    ASSERT_TRUE(active.size() == 6u);
    ASSERT_TRUE(check_vector_eq(trueActive2, active));

    // Test arbitration

    Real overlapsArray3[10] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
                              // W     W     L     L     W     W     L     L     L     W
    overlaps.assign(&overlapsArray3[0], &overlapsArray3[10]);
    UInt trueActive3[4] = {0, 1, 4, 5};
    inhibitionRadius = 3;
    density = 0.25f;
    sp.setInhibitionRadius(inhibitionRadius);
    sp.inhibitColumnsLocal_(overlaps, density, active);

    ASSERT_TRUE(active.size() == 4u);
    ASSERT_TRUE(check_vector_eq(trueActive3, active));
  }
}

TEST(SpatialPoolerTest, testIsUpdateRound) {
  SpatialPooler sp;
  sp.setUpdatePeriod(50);
  sp.setIterationNum(1);
  ASSERT_TRUE(!sp.isUpdateRound_());
  sp.setIterationNum(39);
  ASSERT_TRUE(!sp.isUpdateRound_());
  sp.setIterationNum(50);
  ASSERT_TRUE(sp.isUpdateRound_());
  sp.setIterationNum(1009);
  ASSERT_TRUE(!sp.isUpdateRound_());
  sp.setIterationNum(1250);
  ASSERT_TRUE(sp.isUpdateRound_());

  sp.setUpdatePeriod(125);
  sp.setIterationNum(0);
  ASSERT_TRUE(sp.isUpdateRound_());
  sp.setIterationNum(200);
  ASSERT_TRUE(!sp.isUpdateRound_());
  sp.setIterationNum(249);
  ASSERT_TRUE(!sp.isUpdateRound_());
  sp.setIterationNum(1330);
  ASSERT_TRUE(!sp.isUpdateRound_());
  sp.setIterationNum(1249);
  ASSERT_TRUE(!sp.isUpdateRound_());
  sp.setIterationNum(1375);
  ASSERT_TRUE(sp.isUpdateRound_());
}

TEST(SpatialPoolerTest, testRaisePermanencesToThreshold) {
  SpatialPooler sp;
  UInt stimulusThreshold = 3;
  Real synPermConnected = 0.1f;
  Real synPermBelowStimulusInc = 0.01f;
  UInt numInputs = 5;
  UInt numColumns = 7;
  setup(sp, numInputs, numColumns);
  sp.setStimulusThreshold(stimulusThreshold);
  sp.setSynPermConnected(synPermConnected);
  sp.setSynPermBelowStimulusInc(synPermBelowStimulusInc);

  UInt potentialArr[7][5] =
      {{ 1, 1, 1, 1, 1 },
       { 1, 1, 1, 1, 1 },
       { 1, 1, 1, 1, 1 },
       { 1, 1, 1, 1, 1 },
       { 1, 1, 1, 1, 1 },
       { 1, 1, 0, 0, 1 },
       { 0, 1, 1, 1, 0 }};


  Real permArr[7][5] =
      {{ 0.00f,  0.11f,   0.095f, 0.092f, 0.01f  },
       { 0.12f,  0.15f,   0.02f,  0.120f, 0.09f  },
       { 0.51f,  0.081f,  0.025f, 0.089f, 0.31f  },
       { 0.18f,  0.0601f, 0.11f,  0.011f, 0.03f  },
       { 0.011f, 0.011f,  0.011f, 0.011f, 0.011f },
       { 0.12f,  0.056f,  0.000f, 0.000f, 0.078f },
       { 0.00f,  0.061f,  0.070f, 0.140f, 0.000f }};

  Real truePerm[7][5] =
      {{  0.01f,  0.12f,   0.105f, 0.102f, 0.02f  },  // incremented once
       {  0.12f,  0.15f,   0.02f,  0.12f,  0.09f  },  // no change
       {  0.53f,  0.101f,  0.045f, 0.109f, 0.33f  },  // increment twice
       {  0.22f,  0.1001f, 0.15f,  0.051f, 0.07f  },  // increment four times
       {  0.101f, 0.101f,  0.101f, 0.101f, 0.101f },  // increment 9 times
       {  0.17f,  0.106f,  0.000f, 0.000f, 0.128f },  // increment 5 times
       {  0.00f,  0.101f,  0.11f,  0.18f,  0.000f }}; // increment 4 times

  UInt trueConnectedCount[7] = {3, 3, 4, 3, 5, 3, 3};

  for (UInt i = 0; i < numColumns; i++) {
    vector<Real> perm;
    vector<UInt> potential;
    perm.assign(&permArr[i][0], &permArr[i][numInputs]);
    for (UInt j = 0; j < numInputs; j++) {
      if (potentialArr[i][j] > 0) {
        potential.push_back(j);
      }
    }
    UInt connected = sp.raisePermanencesToThreshold_(perm, potential);
    ASSERT_TRUE(check_vector_eq(truePerm[i], perm));
    ASSERT_TRUE(connected == trueConnectedCount[i]);
  }
}

TEST(SpatialPoolerTest, testUpdatePermanencesForColumn) {
  vector<UInt> inputDim;
  vector<UInt> columnDim;

  UInt numInputs = 5;
  UInt numColumns = 5;
  SpatialPooler sp;
  setup(sp, numInputs, numColumns);
  Real synPermTrimThreshold = 0.05f;
  sp.setSynPermTrimThreshold(synPermTrimThreshold);

  Real permArr[5][5] =
      {{ -0.10f, 0.500f, 0.400f, 0.010f, 0.020f },
       { 0.300f, 0.010f, 0.020f, 0.120f, 0.090f },
       { 0.070f, 0.050f, 1.030f, 0.190f, 0.060f },
       { 0.180f, 0.090f, 0.110f, 0.010f, 0.030f },
       { 0.200f, 0.101f, 0.050f, -0.09f, 1.100f }};

  Real truePerm[5][5] =
       {{ 0.000f, 0.500f, 0.400f, 0.000f, 0.000f},
        // Clip     -       -       Trim    Trim
        {0.300f, 0.000f, 0.000f, 0.120f, 0.090f},
         // -      Trim    Trim    -       -
        {0.070f, 0.050f, 1.000f, 0.190f, 0.060f},
        // -       -     Clip      -       -
        {0.180f, 0.090f, 0.110f, 0.000f, 0.000f},
        // -       -       -       Trim    Trim
        {0.200f, 0.101f, 0.050f, 0.000f, 1.000f}};
        // -       -       -       Clip    Clip

  UInt trueConnectedSynapses[5][5] =
      {{0, 1, 1, 0, 0},
       {1, 0, 0, 1, 0},
       {0, 0, 1, 1, 0},
       {1, 0, 1, 0, 0},
       {1, 1, 0, 0, 1 }};

  UInt trueConnectedCount[5] = {2, 2, 2, 2, 3};

  for (UInt i = 0; i < 5; i++) {
    vector<Real> perm(&permArr[i][0], &permArr[i][5]);
    sp.updatePermanencesForColumn_(perm, i, false);
    auto permArr = new Real[numInputs];
    auto connectedArr = new UInt[numInputs];
    auto connectedCountsArr = new UInt[numColumns];
    sp.getPermanence(i, permArr);
    sp.getConnectedSynapses(i, connectedArr);
    sp.getConnectedCounts(connectedCountsArr);
    ASSERT_TRUE(check_vector_eq(truePerm[i], permArr, numInputs));
    ASSERT_TRUE(check_vector_eq(trueConnectedSynapses[i], connectedArr, numInputs));
    ASSERT_TRUE(trueConnectedCount[i] == connectedCountsArr[i]);
    delete[] permArr;
    delete[] connectedArr;
    delete[] connectedCountsArr;
  }
}

TEST(SpatialPoolerTest, testInitPermanence) {
  vector<UInt> inputDim;
  vector<UInt> columnDim;
  inputDim.push_back(8);
  columnDim.push_back(2);

  SpatialPooler sp;
  Real synPermConnected = 0.2f;
  Real synPermTrimThreshold = 0.1f;
  Real synPermActiveInc = 0.05f;
  sp.initialize(inputDim, columnDim);
  sp.setSynPermConnected(synPermConnected);
  sp.setSynPermTrimThreshold(synPermTrimThreshold);
  sp.setSynPermActiveInc(synPermActiveInc);

  UInt arr[8] = {0, 1, 1, 0, 0, 1, 0, 1};
  vector<UInt> potential(&arr[0], &arr[8]);
  vector<Real> perm = sp.initPermanence_(potential, 1.0);
  for (UInt i = 0; i < 8; i++)
    if (potential[i])
      ASSERT_TRUE(perm[i] >= synPermConnected);
    else
      ASSERT_TRUE(perm[i] < 1e-5);

  perm = sp.initPermanence_(potential, 0);
  for (UInt i = 0; i < 8; i++)
    if (potential[i])
      ASSERT_LE(perm[i], synPermConnected);
    else
      ASSERT_LT(perm[i], 1e-5f);

  inputDim[0] = 100;
  sp.initialize(inputDim, columnDim);
  sp.setSynPermConnected(synPermConnected);
  sp.setSynPermTrimThreshold(synPermTrimThreshold);
  sp.setSynPermActiveInc(synPermActiveInc);
  potential.clear();

  for (UInt i = 0; i < 100; i++)
    potential.push_back(1);

  perm = sp.initPermanence_(potential, 0.5);
  int count = 0;
  for (UInt i = 0; i < 100; i++) {
    ASSERT_TRUE(perm[i] < 1e-5f || perm[i] >= synPermTrimThreshold);
    if (perm[i] >= synPermConnected)
      count++;
  }
  ASSERT_TRUE(count > 5 && count < 95);
}

TEST(SpatialPoolerTest, testInitPermConnected) {
  SpatialPooler sp;
  Real synPermConnected = 0.2f;
  Real synPermMax = 1.0f;

  sp.setSynPermConnected(synPermConnected);
  sp.setSynPermMax(synPermMax);

  for (UInt i = 0; i < 100; i++) {
    Real permVal = sp.initPermConnected_();
    ASSERT_GE(permVal, synPermConnected);
    ASSERT_LE(permVal, synPermMax);
  }
}

TEST(SpatialPoolerTest, testInitPermNonConnected) {
  SpatialPooler sp;
  Real synPermConnected = 0.2f;
  sp.setSynPermConnected(synPermConnected);
  for (UInt i = 0; i < 100; i++) {
    Real permVal = sp.initPermNonConnected_();
    ASSERT_GE(permVal, 0);
    ASSERT_LE(permVal, synPermConnected);
  }
}

TEST(SpatialPoolerTest, testMapColumn) {
  {
    // Test 1D.
    SpatialPooler sp(
        /*inputDimensions*/ {12},
        /*columnDimensions*/ {4});

    EXPECT_EQ(1u, sp.mapColumn_(0));
    EXPECT_EQ(4u, sp.mapColumn_(1));
    EXPECT_EQ(7u, sp.mapColumn_(2));
    EXPECT_EQ(10u, sp.mapColumn_(3));
  }

  {
    // Test 1D with same dimensions of columns and inputs.
    SpatialPooler sp(
        /*inputDimensions*/ {4},
        /*columnDimensions*/ {4});

    EXPECT_EQ(0u, sp.mapColumn_(0));
    EXPECT_EQ(1u, sp.mapColumn_(1));
    EXPECT_EQ(2u, sp.mapColumn_(2));
    EXPECT_EQ(3u, sp.mapColumn_(3));
  }

  {
    // Test 1D with dimensions of length 1.
    SpatialPooler sp(
        /*inputDimensions*/ {1},
        /*columnDimensions*/ {1});

    EXPECT_EQ(0u, sp.mapColumn_(0));
  }

  {
    // Test 2D.
    SpatialPooler sp(
        /*inputDimensions*/ {36, 12},
        /*columnDimensions*/ {12, 4});

    EXPECT_EQ(13u, sp.mapColumn_(0));
    EXPECT_EQ(49u, sp.mapColumn_(4));
    EXPECT_EQ(52u, sp.mapColumn_(5));
    EXPECT_EQ(58u, sp.mapColumn_(7));
    EXPECT_EQ(418u, sp.mapColumn_(47));
  }

  {
    // Test 2D, some input dimensions smaller than column dimensions.
    SpatialPooler sp(
        /*inputDimensions*/ {3, 5},
        /*columnDimensions*/ {4, 4});

    EXPECT_EQ(0u, sp.mapColumn_(0));
    EXPECT_EQ(4u, sp.mapColumn_(3));
    EXPECT_EQ(14u, sp.mapColumn_(15));
  }
}

TEST(SpatialPoolerTest, testMapPotential1D) {
  vector<UInt> inputDim, columnDim;
  inputDim.push_back(12);
  columnDim.push_back(4);
  UInt potentialRadius = 2;

  SpatialPooler sp;
  sp.initialize(inputDim, columnDim);
  sp.setPotentialRadius(potentialRadius);

  vector<UInt> mask;

  // Test without wrapAround and potentialPct = 1
  sp.setPotentialPct(1.0);

  UInt expectedMask1[12] = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
  mask = sp.mapPotential_(0, false);
  ASSERT_TRUE(check_vector_eq(expectedMask1, mask));

  UInt expectedMask2[12] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0};
  mask = sp.mapPotential_(2, false);
  ASSERT_TRUE(check_vector_eq(expectedMask2, mask));

  // Test with wrapAround and potentialPct = 1
  sp.setPotentialPct(1.0);

  UInt expectedMask3[12] = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1};
  mask = sp.mapPotential_(0, true);
  ASSERT_TRUE(check_vector_eq(expectedMask3, mask));

  UInt expectedMask4[12] = {1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1};
  mask = sp.mapPotential_(3, true);
  ASSERT_TRUE(check_vector_eq(expectedMask4, mask));

  // Test with potentialPct < 1
  sp.setPotentialPct(0.5);
  UInt supersetMask1[12] = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1};
  mask = sp.mapPotential_(0, true);
  ASSERT_TRUE(sum(mask) == 3);

  UInt unionMask1[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (UInt i = 0; i < 12; i++) {
    unionMask1[i] = supersetMask1[i] | mask.at(i);
  }

  ASSERT_TRUE(check_vector_eq(unionMask1, supersetMask1, 12));
}

TEST(SpatialPoolerTest, testMapPotential2D) {
  vector<UInt> inputDim, columnDim;
  inputDim.push_back(6);
  inputDim.push_back(12);
  columnDim.push_back(2);
  columnDim.push_back(4);
  UInt potentialRadius = 1;
  Real potentialPct = 1.0;

  SpatialPooler sp;
  sp.initialize(inputDim, columnDim);
  sp.setPotentialRadius(potentialRadius);
  sp.setPotentialPct(potentialPct);

  vector<UInt> mask;

  // Test without wrapAround
  UInt expectedMask1[72] = {
      1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
  mask = sp.mapPotential_(0, false);
  ASSERT_TRUE(check_vector_eq(expectedMask1, mask));

  UInt expectedMask2[72] = {
      0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
  mask = sp.mapPotential_(2, false);
  ASSERT_TRUE(check_vector_eq(expectedMask2, mask));

  // Test with wrapAround
  potentialRadius = 2;
  sp.setPotentialRadius(potentialRadius);
  UInt expectedMask3[72] = {
      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1
    };
  mask = sp.mapPotential_(0, true);
  ASSERT_TRUE(check_vector_eq(expectedMask3, mask));

  UInt expectedMask4[72] = {
      1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1
    };
  mask = sp.mapPotential_(3, true);
  ASSERT_TRUE(check_vector_eq(expectedMask4, mask));
}

TEST(SpatialPoolerTest, testStripUnlearnedColumns) {
  SpatialPooler sp;
  vector<UInt> inputDim, columnDim;
  inputDim.push_back(5);
  columnDim.push_back(3);
  sp.initialize(inputDim, columnDim);

  // None learned, none active
  {
    Real activeDutyCycles[3] = {0.0f, 0.0f, 0.0f };
    UInt activeArray[3] = {0, 0, 0};
    UInt expected[3] = {0, 0, 0};

    sp.setActiveDutyCycles(activeDutyCycles);
    sp.stripUnlearnedColumns(activeArray);

    ASSERT_TRUE(check_vector_eq(activeArray, expected, 3));
  }

  // None learned, some active
  {
    Real activeDutyCycles[3] = {0.0f, 0.0f, 0.0f };
    UInt activeArray[3] = {1, 0, 1};
    UInt expected[3] = {0, 0, 0};

    sp.setActiveDutyCycles(activeDutyCycles);
    sp.stripUnlearnedColumns(activeArray);

    ASSERT_TRUE(check_vector_eq(activeArray, expected, 3));
  }

  // Some learned, none active
  {
    Real activeDutyCycles[3] = {1.0f, 1.0f, 0.0f };
    UInt activeArray[3] = {0, 0, 0};
    UInt expected[3] = {0, 0, 0};

    sp.setActiveDutyCycles(activeDutyCycles);
    sp.stripUnlearnedColumns(activeArray);

    ASSERT_TRUE(check_vector_eq(activeArray, expected, 3));
  }

  // Some learned, some active
  {
    Real activeDutyCycles[3] = {1.0f, 1.0f, 0.0f };
    UInt activeArray[3] = {1, 0, 1};
    UInt expected[3] = {1, 0, 0};

    sp.setActiveDutyCycles(activeDutyCycles);
    sp.stripUnlearnedColumns(activeArray);

    ASSERT_TRUE(check_vector_eq(activeArray, expected, 3));
  }
}

TEST(SpatialPoolerTest, getOverlaps) {
  SpatialPooler sp;
  const vector<UInt> inputDim = {5};
  const vector<UInt> columnDim = {3};
  sp.initialize(inputDim, columnDim);

  UInt potential[5] = {1, 1, 1, 1, 1};
  sp.setPotential(0, potential);
  sp.setPotential(1, potential);
  sp.setPotential(2, potential);

  Real permanence0[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  sp.setPermanence(0, permanence0);
  Real permanence1[5] = {1.0f, 1.0f, 1.0f, 0.0f, 0.0f};
  sp.setPermanence(1, permanence1);
  Real permanence2[5] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  sp.setPermanence(2, permanence2);

  vector<Real> boostFactors = {1.0f, 2.0f, 3.0f};
  sp.setBoostFactors(boostFactors.data());

  vector<UInt> input = {1, 1, 1, 1, 1};
  vector<UInt> activeColumns = {0, 0, 0};
  sp.compute(input.data(), true, activeColumns.data());

  const vector<UInt> &overlaps = sp.getOverlaps();
  const vector<UInt> expectedOverlaps = {0, 3, 5};
  EXPECT_EQ(expectedOverlaps, overlaps);

  const vector<Real> &boostedOverlaps = sp.getBoostedOverlaps();
  const vector<Real> expectedBoostedOverlaps = {0.0f, 6.0f, 15.0f};
  EXPECT_EQ(expectedBoostedOverlaps, boostedOverlaps);
}

TEST(SpatialPoolerTest, ZeroOverlap_NoStimulusThreshold_GlobalInhibition) {
  const UInt inputSize = 10;
  const UInt nColumns = 20;

  SpatialPooler sp( {inputSize},
  					{nColumns},
                   /*potentialRadius*/ 10,
                   /*potentialPct*/ 0.5f,
                   /*globalInhibition*/ true,
                   /*localAreaDensity*/ -1.0f,
                   /*numActiveColumnsPerInhArea*/ 3,
                   /*stimulusThreshold*/ 0,
                   /*synPermInactiveDec*/ 0.008f,
                   /*synPermActiveInc*/ 0.05f,
                   /*synPermConnected*/ 0.1f,
                   /*minPctOverlapDutyCycles*/ 0.001f,
                   /*dutyCyclePeriod*/ 1000,
                   /*boostStrength*/ 10.0f,
                   /*seed*/ 1,
                   /*spVerbosity*/ 0,
                   /*wrapAround*/ true);

  vector<UInt> input(inputSize, 0);
  vector<UInt> activeColumns(nColumns, 0);
  sp.compute(input.data(), true, activeColumns.data());

  EXPECT_EQ(3u, countNonzero(activeColumns));
}

TEST(SpatialPoolerTest, ZeroOverlap_StimulusThreshold_GlobalInhibition) {
  const UInt inputSize = 10;
  const UInt nColumns = 20;

  SpatialPooler sp( {inputSize},
  					{nColumns},
                   /*potentialRadius*/ 5,
                   /*potentialPct*/ 0.5f,
                   /*globalInhibition*/ true,
                   /*localAreaDensity*/ -1.0f,
                   /*numActiveColumnsPerInhArea*/ 1,
                   /*stimulusThreshold*/ 1,
                   /*synPermInactiveDec*/ 0.008f,
                   /*synPermActiveInc*/ 0.05f,
                   /*synPermConnected*/ 0.1f,
                   /*minPctOverlapDutyCycles*/ 0.001f,
                   /*dutyCyclePeriod*/ 1000,
                   /*boostStrength*/ 10.0f,
                   /*seed*/ 1,
                   /*spVerbosity*/ 0,
                   /*wrapAround*/ true);

  vector<UInt> input(inputSize, 0);
  vector<UInt> activeColumns(nColumns, 0);
  sp.compute(input.data(), true, activeColumns.data());

  EXPECT_EQ(0u, countNonzero(activeColumns));
}

TEST(SpatialPoolerTest, ZeroOverlap_NoStimulusThreshold_LocalInhibition) {
  const UInt inputSize = 10;
  const UInt nColumns = 20;

  SpatialPooler sp( {inputSize},
  					{nColumns},
                   /*potentialRadius*/ 5,
                   /*potentialPct*/ 0.5f,
                   /*globalInhibition*/ false,
                   /*localAreaDensity*/ -1.0f,
                   /*numActiveColumnsPerInhArea*/ 1,
                   /*stimulusThreshold*/ 0,
                   /*synPermInactiveDec*/ 0.008f,
                   /*synPermActiveInc*/ 0.05f,
                   /*synPermConnected*/ 0.1f,
                   /*minPctOverlapDutyCycles*/ 0.001f,
                   /*dutyCyclePeriod*/ 1000,
                   /*boostStrength*/ 10.0f,
                   /*seed*/ 1,
                   /*spVerbosity*/ 0,
                   /*wrapAround*/ true);

  vector<UInt> input(inputSize, 0);
  vector<UInt> activeColumns(nColumns, 0);
  sp.compute(input.data(), true, activeColumns.data());

  // This exact number of active columns is determined by the inhibition
  // radius, which changes based on the random synapses (i.e. weird math).
  EXPECT_GT(countNonzero(activeColumns), 2u);
  EXPECT_LT(countNonzero(activeColumns), 10u);
}

TEST(SpatialPoolerTest, ZeroOverlap_StimulusThreshold_LocalInhibition) {
  const UInt inputSize = 10;
  const UInt nColumns = 20;

  SpatialPooler sp( {inputSize},
  					{nColumns},
                   /*potentialRadius*/ 10,
                   /*potentialPct*/ 0.5f,
                   /*globalInhibition*/ false,
                   /*localAreaDensity*/ -1.0f,
                   /*numActiveColumnsPerInhArea*/ 3,
                   /*stimulusThreshold*/ 1,
                   /*synPermInactiveDec*/ 0.008f,
                   /*synPermActiveInc*/ 0.05f,
                   /*synPermConnected*/ 0.1f,
                   /*minPctOverlapDutyCycles*/ 0.001f,
                   /*dutyCyclePeriod*/ 1000,
                   /*boostStrength*/ 10.0f,
                   /*seed*/ 1,
                   /*spVerbosity*/ 0,
                   /*wrapAround*/ true);

  vector<UInt> input(inputSize, 0);
  vector<UInt> activeColumns(nColumns, 0);
  sp.compute(input.data(), true, activeColumns.data());

  EXPECT_EQ(0u, countNonzero(activeColumns));
}

TEST(SpatialPoolerTest, testSaveLoad) {
  Directory::create("TestOutputDir", false, true);
  const char *filename = "TestOutputDir/SpatialPoolerSerialization.save";
  SpatialPooler sp1, sp2;
  UInt numInputs = 6;
  UInt numColumns = 12;
  setup(sp1, numInputs, numColumns);

  ofstream outfile;
  outfile.open(filename, ios::binary);
  sp1.save(outfile);
  outfile.close();

  ifstream infile (filename, ios::binary);
  sp2.load(infile);
  infile.close();

  ASSERT_NO_FATAL_FAILURE(check_spatial_eq(sp1, sp2));

  Directory::removeTree("TestOutputDir");
}



TEST(SpatialPoolerTest, testConstructorVsInitialize) {
  // Initialize SP using the constructor
  SpatialPooler sp1(
      /*inputDimensions*/ {100},
      /*columnDimensions*/ {100},
      /*potentialRadius*/ 16,
      /*potentialPct*/ 0.5f,
      /*globalInhibition*/ true,
      /*localAreaDensity*/ -1.0f,
      /*numActiveColumnsPerInhArea*/ 10,
      /*stimulusThreshold*/ 0,
      /*synPermInactiveDec*/ 0.008f,
      /*synPermActiveInc*/ 0.05f,
      /*synPermConnected*/ 0.1f,
      /*minPctOverlapDutyCycles*/ 0.001f,
      /*dutyCyclePeriod*/ 1000,
      /*boostStrength*/ 0.0f,
      /*seed*/ 1,
      /*spVerbosity*/ 0,
      /*wrapAround*/ true);

  // Initialize SP using the "initialize" method
  SpatialPooler sp2;
  sp2.initialize(
      /*inputDimensions*/ {100},
      /*columnDimensions*/ {100},
      /*potentialRadius*/ 16,
      /*potentialPct*/ 0.5f,
      /*globalInhibition*/ true,
      /*localAreaDensity*/ -1.0f,
      /*numActiveColumnsPerInhArea*/ 10,
      /*stimulusThreshold*/ 0,
      /*synPermInactiveDec*/ 0.008f,
      /*synPermActiveInc*/ 0.05f,
      /*synPermConnected*/ 0.1f,
      /*minPctOverlapDutyCycles*/ 0.001f,
      /*dutyCyclePeriod*/ 1000,
      /*boostStrength*/ 0.0f,
      /*seed*/ 1,
      /*spVerbosity*/ 0,
      /*wrapAround*/ true);

  // The two SP should be the same
  check_spatial_eq(sp1, sp2);
}

} // end anonymous namespace
