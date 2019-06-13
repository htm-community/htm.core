/* ---------------------------------------------------------------------
 * Numenta Platform for Intelligent Computing (NuPIC)
 * Copyright (C) 2018, Numenta, Inc.  Unless you have an agreement
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


 * Author: David Keeney, April, 2018
 * ---------------------------------------------------------------------
 */

/*---------------------------------------------------------------------
 * This is a test of the TMRegion module.  It does not check the
 *BacktrackingTMCpp itself but rather just the plug-in mechanisom to call the
 *BacktrackingTMCpp.
 *
 * For those not familiar with GTest:
 *     ASSERT_TRUE(value)   -- Fatal assertion that the value is true.  Test
 *terminates if false. ASSERT_FALSE(value)   -- Fatal assertion that the value
 *is false. Test terminates if true. ASSERT_STREQ(str1, str2)   -- Fatal
 *assertion that the strings are equal. Test terminates if false.
 *
 *     EXPECT_TRUE(value)   -- Nonfatal assertion that the value is true.  Test
 *continues if false. EXPECT_FALSE(value)   -- Nonfatal assertion that the value
 *is false. Test continues if true. EXPECT_STREQ(str1, str2)   -- Nonfatal
 *assertion that the strings are equal. Test continues if false.
 *
 *     EXPECT_THROW(statement, exception_type) -- nonfatal exception, cought and
 *continues.
 *---------------------------------------------------------------------
 */

#include <nupic/engine/Input.hpp>
#include <nupic/engine/Link.hpp>
#include <nupic/engine/Network.hpp>
#include <nupic/engine/NuPIC.hpp>
#include <nupic/engine/Output.hpp>
#include <nupic/engine/Region.hpp>
#include <nupic/engine/RegisteredRegionImplCpp.hpp>
#include <nupic/engine/Spec.hpp>
#include <nupic/engine/YAMLUtils.hpp>
#include <nupic/ntypes/Array.hpp>
#include <nupic/os/Directory.hpp>
#include <nupic/os/Env.hpp>
#include <nupic/os/Path.hpp>
#include <nupic/os/Timer.hpp>
#include <nupic/regions/TMRegion.hpp>
#include <nupic/types/Exception.hpp>

#include <cmath>   // fabs/abs
#include <cstdlib> // exit
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <vector>

#include "RegionTestUtilities.hpp"
#include "yaml-cpp/yaml.h"
#include "gtest/gtest.h"

namespace testing {

#define VERBOSE if (verbose) std::cerr << "[          ] "
static bool verbose = false; // turn this on to print extra stuff for debugging the test.

// The following string should contain a valid expected Spec - manually
// verified.
#define EXPECTED_SPEC_COUNT 18 // The number of parameters expected in the TMRegion Spec

using namespace nupic;


// Verify that all parameters are working.
// Assumes that the default value in the Spec is the same as the default when
// creating a region with default constructor.
TEST(TMRegionTest, testSpecAndParameters) {
  Network net;

  // Turn on runtime Debug logging.
 //if (verbose)  LogItem::setLogLevel(LogLevel::LogLevel_Verbose);

  // create a TM region with default parameters
  std::set<std::string> excluded;
  std::shared_ptr<Region> region1 = net.addRegion("region1", "TMRegion", "");
  checkGetSetAgainstSpec(region1, EXPECTED_SPEC_COUNT, excluded, verbose);
  checkInputOutputsAgainstSpec(region1, verbose);
}

TEST(TMRegionTest, checkTMRegionImpl) {
  Network net;

  size_t regionCntBefore = net.getRegions().size();

  VERBOSE << "Adding a built-in TMRegion region..." << std::endl;
  std::shared_ptr<Region> region1 = net.addRegion("region1", "TMRegion", "");
  size_t regionCntAfter = net.getRegions().size();
  ASSERT_TRUE(regionCntBefore + 1 == regionCntAfter)
      << " Expected number of regions to increase by one.  ";
  ASSERT_TRUE(region1->getType() == "TMRegion")
      << " Expected type for region1 to be \"TMRegion\" but type is: "
      << region1->getType();

  EXPECT_THROW(region1->getOutputData("doesnotexist"), std::exception);
  EXPECT_THROW(region1->getInputData("doesnotexist"), std::exception);

  // run and compute() should fail because network has not been initialized
  EXPECT_THROW(net.run(1), std::exception);
  EXPECT_THROW(region1->compute(), std::exception);
}

TEST(TMRegionTest, initialization_with_custom_impl) {
  VERBOSE << "Creating network..." << std::endl;
  Network net;

  size_t regionCntBefore = net.getRegions().size();

  // make sure the custom region registration works for CPP.
  // We will just use the same TMRegion class but it could be a subclass or some
  // different custom class. While we are at it, let's make sure we can initialize the
  // parameters from here too. The parameter names and data types
  // must match those of the spec. Explicit parameters are Yaml format...but
  // since YAML is a superset of JSON, you can use JSON format as well.
  // Here we set a unique value for every parameter we can set (per the spec).
  std::string nodeParams =
      "{numberOfCols: 100, cellsPerColumn: 20, "
      "activationThreshold: 12, initialPermanence: 0.22, "
      "connectedPermanence: 0.4, minThreshold: 7, "
      "maxNewSynapseCount: 21, permanenceIncrement: 0.2, "
      "permanenceDecrement: 0.2, predictedSegmentDecrement: 0.0004, "
      "maxSynapsesPerSegment: 254, seed: 43, learningMode: false}";

  VERBOSE << "Adding a custom-built TMRegion region..." << std::endl;
  net.registerRegion("TMRegionCustom", new RegisteredRegionImplCpp<TMRegion>());
  std::shared_ptr<Region> region2 = net.addRegion("region2", "TMRegionCustom", nodeParams);
  size_t regionCntAfter = net.getRegions().size();
  ASSERT_TRUE(regionCntBefore + 1 == regionCntAfter)
      << "  Expected number of regions to increase by one.  ";
  ASSERT_TRUE(region2->getType() == "TMRegionCustom")
      << " Expected type for region2 to be \"TMRegionCustom\" but type is: "
      << region2->getType();

  // Check that all of the node parameters have been correctly parsed and available.
  EXPECT_EQ(region2->getParameterUInt32("numberOfCols"), 100u);
  EXPECT_EQ(region2->getParameterUInt32("cellsPerColumn"), 20u);
  EXPECT_EQ(region2->getParameterUInt32("activationThreshold"), 12u);
  EXPECT_FLOAT_EQ(region2->getParameterReal32("initialPermanence"), 0.22f);
  EXPECT_FLOAT_EQ(region2->getParameterReal32("connectedPermanence"), 0.4f);
  EXPECT_EQ(region2->getParameterUInt32("minThreshold"), 7u);
  EXPECT_EQ(region2->getParameterUInt32("maxNewSynapseCount"), 21u);
  EXPECT_FLOAT_EQ(region2->getParameterReal32("permanenceIncrement"), 0.2f);
  EXPECT_FLOAT_EQ(region2->getParameterReal32("permanenceDecrement"), 0.2f);
  EXPECT_FLOAT_EQ(region2->getParameterReal32("predictedSegmentDecrement"), 0.0004f);
  EXPECT_EQ(region2->getParameterUInt32("maxSynapsesPerSegment"), 254u);
  EXPECT_EQ(region2->getParameterInt32("seed"), 43);
  EXPECT_EQ(region2->getParameterBool("learningMode"), false);

  // compute() should fail because network has not been initialized
  EXPECT_THROW(net.run(1), std::exception);
  EXPECT_THROW(region2->compute(), std::exception);

  EXPECT_THROW(net.initialize(), std::exception)
      << "Exception should state: region2 has unspecified dimensions. ";
}

TEST(TMRegionTest, testLinking) {
  // This is a minimal end-to-end test containing an TMRegion region.
  // To make sure we can feed data from some other region to our TMRegion
  // this test will hook up the VectorFileSensor to an SPRegion to our
  // TMRegion and then connect our TMRegion to a VectorFileEffector to
  // capture the results.
  //
  std::string test_input_file = "TestOutputDir/TMRegionTestInput.csv";
  std::string test_output_file = "TestOutputDir/TMRegionTestOutput.csv";

  // make a place to put test data.
  if (!Directory::exists("TestOutputDir"))
    Directory::create("TestOutputDir", false, true);
  if (Path::exists(test_input_file))
    Path::remove(test_input_file);
  if (Path::exists(test_output_file))
    Path::remove(test_output_file);

  // Create a csv file to use as input.
  // The SDR data we will feed it will be a matrix with 1's on the diagonal
  // and we will feed it one row at a time, for 10 rows.
  size_t dataWidth = 20u;
  size_t dataRows = 10u;
  std::ofstream f(test_input_file.c_str());
  for (size_t i = 0; i < 10u; i++) {
    for (size_t j = 0u; j < dataWidth; j++) {
      if ((j % dataRows) == i)
        f << "1.0,";
      else
        f << "0.0,";
    }
    f << std::endl;
  }
  f.close();

  VERBOSE << "Setup Network; add 4 regions and 3 links." << std::endl;
  Network net;

  // Explicit parameters:  (Yaml format...but since YAML is a superset of JSON,
  // you can use JSON format as well)
  std::string parameters = "{activeOutputCount: " + std::to_string(dataWidth) + "}";
  std::shared_ptr<Region> region1 = net.addRegion("region1", "VectorFileSensor",parameters);
  std::shared_ptr<Region> region2 = net.addRegion("region2", "SPRegion", "{dim: [2,10]}");
  std::shared_ptr<Region> region3 = net.addRegion("region3", "TMRegion",
                                        "{activationThreshold: 9, cellsPerColumn: 5}");
  std::shared_ptr<Region> region4 = net.addRegion("region4", "VectorFileEffector",
                                        "{outputFile: '" + test_output_file + "'}");

  net.link("region1", "region2", "", "", "dataOut", "bottomUpIn");
  net.link("region2", "region3", "", "", "bottomUpOut", "bottomUpIn");
  net.link("region3", "region4", "", "", "bottomUpOut", "dataIn");

  VERBOSE << "Load Data." << std::endl;
  region1->executeCommand({"loadFile", test_input_file});

  VERBOSE << "Initialize." << std::endl;
  net.initialize();

  VERBOSE << "Dimensions: \n";
  VERBOSE << " VectorFileSensor  - " << region1->getOutputDimensions("dataOut")    <<"\n";
  VERBOSE << " SPRegion in       - " << region2->getInputDimensions("bottomUpIn")  <<"\n";
  VERBOSE << " SPRegion out      - " << region2->getOutputDimensions("bottomUpOut")<<"\n";
  VERBOSE << " TMRegion in       - " << region3->getInputDimensions("bottomUpIn")  <<"\n";
  VERBOSE << " TMRegion out      - " << region3->getOutputDimensions("bottomUpOut")<<"\n";
  VERBOSE << " VectorFileEffector- " << region4->getInputDimensions("dataIn")      <<"\n";

  // check actual dimensions
  ASSERT_EQ(region3->getParameterUInt32("numberOfCols"), dataWidth);
  ASSERT_EQ(region3->getParameterUInt32("inputWidth"), (UInt32)dataWidth);

  VERBOSE << "Execute once." << std::endl;
  net.run(1);

  VERBOSE << "Checking data after first iteration..." << std::endl;
  VERBOSE << "  VectorFileSensor Output" << std::endl;
  Array r1OutputArray = region1->getOutputData("dataOut");
  VERBOSE << "    " << r1OutputArray << "\n";
  EXPECT_EQ(r1OutputArray.getCount(), dataWidth);
  EXPECT_TRUE(r1OutputArray.getType() == NTA_BasicType_Real32);

  // check anomaly
  EXPECT_FLOAT_EQ(region3->getParameterReal32("anomaly"), 1.0f);
  const Real32 *anomalyBuffer = reinterpret_cast<const Real32*>(region3->getOutputData("anomaly").getBuffer());
  EXPECT_FLOAT_EQ(anomalyBuffer[0], 0.0f); // Note: it is zero because no links are connected to this output.


  VERBOSE << "  SPRegion Output " << std::endl;
  Array r2OutputArray = region2->getOutputData("bottomUpOut");
  VERBOSE << "    " << r2OutputArray << "\n";

  VERBOSE << "  TMRegion input "
          << region3->getInputDimensions("bottomUpIn") << "\n";
  Array r3InputArray = region3->getInputData("bottomUpIn");
  ASSERT_TRUE(r2OutputArray.getCount() == r3InputArray.getCount())
      << "Buffer length different. Output from SP is "
      << r2OutputArray.getCount() << ", input to TPRegion is "
      << r3InputArray.getCount();
  EXPECT_TRUE(r3InputArray.getType() == NTA_BasicType_SDR);
  VERBOSE << "   " << r3InputArray << "\n";
  SDR expectedR3Input({(UInt32)r3InputArray.getCount()});
  expectedR3Input.setSparse(SDR_sparse_t{ 1, 2, 3, 5, 6, 8, 11, 13, 17, 19 });
  EXPECT_EQ(r3InputArray.getSDR(), expectedR3Input);

  VERBOSE << "  TMRegion output "
          << region3->getOutputDimensions("bottomUpOut") << "\n";
  Array r3OutputArray = region3->getOutputData("bottomUpOut");
  UInt32 numberOfCols = region3->getParameterUInt32("numberOfCols");
  UInt32 cellsPerColumn = region3->getParameterUInt32("cellsPerColumn");
  size_t expectedWidth = r2OutputArray.getCount() * cellsPerColumn;
  ASSERT_TRUE(expectedWidth == r3OutputArray.getCount())
      << "Buffer length different. Output from SP is "
      << r2OutputArray.getCount() << ", input to TPRegion is "
      << r3InputArray.getCount();
  EXPECT_TRUE(r3OutputArray.getType() == NTA_BasicType_SDR);
  VERBOSE << "   " << r3OutputArray << "\n";
  SDR expectedR3Out({(UInt32)r3OutputArray.getCount()});
  expectedR3Out.setSparse( SDR_sparse_t{ 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u, 17u, 18u, 19u,
             25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u, 33u, 34u, 40u, 41u, 42u, 43u,
             44u, 55u, 56u, 57u, 58u, 59u, 65u, 66u, 67u, 68u, 69u, 85u, 86u, 87u,
             88u, 89u, 95u, 96u, 97u, 98u, 99u });
  EXPECT_EQ(r3OutputArray.getSDR(), expectedR3Out);
  EXPECT_EQ(r3OutputArray.getSDR().getSum(), 50u);

  // execute TMRegion several more times and check that it has output.
  VERBOSE << "Execute 9 times." << std::endl;
  net.run(9);

  VERBOSE << "Checking Output Data." << std::endl;
  VERBOSE << "  TMRegion output" << std::endl;
  r3OutputArray = region3->getOutputData("bottomUpOut");
  ASSERT_TRUE(r3OutputArray.getCount() == numberOfCols * cellsPerColumn)
      << "Buffer length different. Output from TMRegion is "
      << r3OutputArray.getCount() << ", should be ("
      << numberOfCols << " * " << cellsPerColumn;
  VERBOSE << "   " << r3OutputArray << ")\n";
  expectedR3Out.setSparse(
            SDR_sparse_t{20u, 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 35u, 36u, 37u, 38u,
             39u, 45u, 46u, 47u, 48u, 49u, 50u, 51u, 52u, 53u, 54u, 60u, 61u, 62u,
             63u, 64u, 70u, 71u, 72u, 73u, 74u, 80u, 81u, 82u, 83u, 84u, 90u, 91u,
             92u, 93u, 94u, 95u, 96u, 97u, 98u, 99u});
  EXPECT_EQ(r3OutputArray, expectedR3Out.getDense());


  VERBOSE << "   Input to VectorFileEffector "
          << region4->getInputDimensions("dataIn") << "\n";
  Array r4InputArray = region4->getInputData("dataIn");
  EXPECT_TRUE(r4InputArray.getType() == NTA_BasicType_Real32);
  VERBOSE << "   " << r4InputArray << "\n";
  EXPECT_TRUE(r4InputArray == expectedR3Out.getDense());

  // cleanup
  region3->executeCommand({"closeFile"});
}

TEST(TMRegionTest, testSerialization) {
  // use default parameters the first time
  Network *net1 = new Network();
  Network *net2 = nullptr;
  Network *net3 = nullptr;

  try {

    VERBOSE << "Setup first network and save it" << std::endl;
    std::shared_ptr<Region> n1region1 = net1->addRegion( "region1", "ScalarSensor",
                                             "{n: 48,w: 10,minValue: 0.05,maxValue: 10}");
    std::shared_ptr<Region> n1region2 =  net1->addRegion("region2", "TMRegion", "{numberOfCols: 48}");

    net1->link("region1", "region2", "", "", "encoded", "bottomUpIn");
    n1region1->setParameterReal64("sensedValue", 5.0);

    net1->run(1);

    // take a snapshot of everything in TMRegion at this point
    // save to a bundle.
    std::map<std::string, std::string> parameterMap;
    EXPECT_TRUE(captureParameters(n1region2, parameterMap))
        << "Capturing parameters before save.";

    VERBOSE << "saveToFile" << std::endl;
    Directory::removeTree("TestOutputDir", true);
    net1->saveToFile("TestOutputDir/tmRegionTest.stream");

    VERBOSE << "Restore from bundle into a second network and compare." << std::endl;
    net2 = new Network();
    net2->loadFromFile("TestOutputDir/tmRegionTest.stream");


    VERBOSE << "checked restored network" << std::endl;
    std::shared_ptr<Region> n2region2 = net2->getRegion("region2");
    ASSERT_TRUE(n2region2->getType() == "TMRegion")
        << " Restored TMRegion region does not have the right type.  Expected "
           "TMRegion, found "
        << n2region2->getType();

    EXPECT_FLOAT_EQ(n2region2->getParameterReal32("anomaly"), 1.0f);
    EXPECT_TRUE(compareParameters(n2region2, parameterMap))
        << "Conflict when comparing TMRegion parameters after restore with "
           "before save.";

    VERBOSE << "continue with execution." << std::endl;
    // can we continue with execution?  See if we get any exceptions.
    n1region1->setParameterReal64("sensedValue", 0.12);
    net1->run(1);
    net2->run(1);

    // Change a parameter and see if it is retained after a restore.
    n2region2->setParameterReal32("permanenceDecrement", 0.099f);

    parameterMap.clear();
    EXPECT_TRUE(captureParameters(n2region2, parameterMap))
        << "Capturing parameters before second save.";
    // serialize using a stream to a single file
    VERBOSE << "save second network." << std::endl;
    net2->saveToFile("TestOutputDir/tmRegionTest.stream");

    VERBOSE << "Restore into a third network and compare changed parameters." << std::endl;
    net3 = new Network();
    net3->loadFromFile("TestOutputDir/tmRegionTest.stream");
    std::shared_ptr<Region> n3region2 = net3->getRegion("region2");
    EXPECT_TRUE(n3region2->getType() == "TMRegion")
        << "Failure: Restored region does not have the right type. "
           " Expected \"TMRegion\", found \""
        << n3region2->getType() << "\".";

    Real32 p = n3region2->getParameterReal32("permanenceDecrement");
    EXPECT_FLOAT_EQ(p, 0.099f);

    EXPECT_TRUE(compareParameters(n3region2, parameterMap))
        << "Comparing parameters after second restore with before save.";
  } catch (nupic::Exception &ex) {
    FAIL() << "Failure: Exception: " << ex.getFilename() << "("
           << ex.getLineNumber() << ") " << ex.getMessage() << "" << std::endl;
  } catch (std::exception &e) {
    FAIL() << "Failure: Exception: " << e.what() << "" << std::endl;
  }

  VERBOSE << "Cleanup" << std::endl;
  // cleanup
  if (net1 != nullptr) {
    delete net1;
  }
  if (net2 != nullptr) {
    delete net2;
  }
  if (net3 != nullptr) {
    delete net3;
  }
  Directory::removeTree("TestOutputDir", true);
}


} // namespace testing
