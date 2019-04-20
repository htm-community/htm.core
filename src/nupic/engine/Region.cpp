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
Implementation of the Region class

Methods related to parameters are in Region_parameters.cpp
Methods related to inputs and outputs are in Region_io.cpp

*/

#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

#include <nupic/engine/Input.hpp>
#include <nupic/engine/Output.hpp>
#include <nupic/engine/Link.hpp>
#include <nupic/engine/Region.hpp>
#include <nupic/engine/RegionImpl.hpp>
#include <nupic/engine/RegionImplFactory.hpp>
#include <nupic/engine/Spec.hpp>
#include <nupic/utils/Log.hpp>
#include <nupic/ntypes/BundleIO.hpp>
#include <nupic/ntypes/Array.hpp>
#include <nupic/ntypes/BasicType.hpp>


namespace nupic {

class GenericRegisteredRegionImpl;

// Create region from parameter spec
Region::Region(std::string name, const std::string &nodeType,
               const std::string &nodeParams, Network *network)
    : name_(std::move(name)), type_(nodeType), initialized_(false),
      network_(network), profilingEnabled_(false) {
  // Set region info before creating the RegionImpl so that the
  // Impl has access to the region info in its constructor.
  RegionImplFactory &factory = RegionImplFactory::getInstance();
  spec_ = factory.getSpec(nodeType);
  impl_.reset(factory.createRegionImpl(nodeType, nodeParams, this));
}

Region::Region(Network *net) {
      network_ = net;
      initialized_ = false;
      profilingEnabled_ = false;
    } // for deserialization of region.


Network *Region::getNetwork() { return network_; }

void Region::createInputsAndOutputs_() {
  // This is called when a Region is added.
  // Create all the outputs for this region from the Spec.
  // By default outputs are zero size, no dimensions.
  for (size_t i = 0; i < spec_->outputs.getCount(); ++i) {
    const std::pair<std::string, OutputSpec> &p = spec_->outputs.getByIndex(i);
    const std::string& outputName = p.first;
    const OutputSpec &os = p.second;
    auto output = new Output(this, outputName, os.dataType);
    outputs_[outputName] = output;
  }

  // Create all the inputs for this node type.
  for (size_t i = 0; i < spec_->inputs.getCount(); ++i) {
    const std::pair<std::string, InputSpec> &p = spec_->inputs.getByIndex(i);
    const std::string& inputName = p.first;
    const InputSpec &is = p.second;

    Input* input = new Input(this, inputName, is.dataType);
    inputs_[inputName] = input;
  }
}

bool Region::hasOutgoingLinks() const {
  for (const auto &elem : outputs_) {
    if (elem.second->hasOutgoingLinks()) {
      return true;
    }
  }
  return false;
}

Region::~Region() {
  if (initialized_)
    uninitialize();

  // If there are any links connected to our outputs, this should fail.
  // We catch this error in the Network class and give the
  // user a good error message (regions may be removed either in
  // Network::removeRegion or Network::~Network())
  for (auto &elem : outputs_) {
    delete elem.second;
    elem.second = nullptr;
  }
  outputs_.clear();

  clearInputs(); // just in case there are some still around.

  // Note: the impl will be deleted when the region goes out of scope.
}

void Region::clearInputs() {
  for (auto &input : inputs_) {
    auto &links = input.second->getLinks();
    for (auto &link : links) {
      	link->getSrc().removeLink(link); // remove it from the Output object.
    }
	links.clear();
    delete input.second; // This is an Input object. Its destructor deletes the links.
    input.second = nullptr;
  }
  inputs_.clear();
}

void Region::initialize() {

  if (initialized_)
    return;

  // Make sure all unconnected outputs have a buffer.
  for(auto out: outputs_) {
    if (!out.second->getData().has_buffer()) {
      out.second->determineDimensions();
      out.second->initialize();
    }
  }

  impl_->initialize();
  initialized_ = true;
}


const std::shared_ptr<Spec>& Region::getSpecFromType(const std::string &nodeType) {
  RegionImplFactory &factory = RegionImplFactory::getInstance();
  return factory.getSpec(nodeType);
}


std::string Region::executeCommand(const std::vector<std::string> &args) {
  std::string retVal;
  if (args.size() < 1) {
    NTA_THROW << "Invalid empty command specified";
  }

  if (profilingEnabled_)
    executeTimer_.start();

  retVal = impl_->executeCommand(args, (UInt64)(-1));

  if (profilingEnabled_)
    executeTimer_.stop();

  return retVal;
}

void Region::compute() {
  if (!initialized_)
    NTA_THROW << "Region " << getName()
              << " unable to compute because not initialized";

  if (profilingEnabled_)
    computeTimer_.start();

  impl_->compute();

  if (profilingEnabled_)
    computeTimer_.stop();

  return;
}

/**
 * These internal methods are called by Network as
 * part of initialization.
 */

void Region::evaluateLinks() {
  for (auto &elem : inputs_) {
    (elem.second)->initialize();
  }
}

size_t Region::getNodeInputElementCount(const std::string &name) {
  size_t count = impl_->getNodeInputElementCount(name);
  return count;
}
size_t Region::getNodeOutputElementCount(const std::string &name) {
  size_t count = impl_->getNodeOutputElementCount(name);
  return count;
}

// Ask the implementation how dimensions should be set
Dimensions Region::askImplForInputDimensions(const std::string &name) const {
  Dimensions dim;
  try {
    dim = impl_->askImplForInputDimensions(name);
  } catch (Exception &e) {
      NTA_THROW << "Internal error -- the dimensions for the input " << name
                << "is unknown. : " << e.what();
  }
  return dim;
}
Dimensions Region::askImplForOutputDimensions(const std::string &name) const {
  Dimensions dim;
  try {
    dim = impl_->askImplForOutputDimensions(name);
  } catch (Exception &e) {
      NTA_THROW << "Internal error -- the dimensions for the input " << name
                << "is unknown. : " << e.what();
  }
  return dim;
}

Dimensions Region::getInputDimensions(std::string name) const {
  if (name.empty()) {
    name = spec_->getDefaultOutputName();
  }
  Input* in = getInput(name);
  NTA_CHECK(in != nullptr)
    << "Unknown input (" << name << ") requested on " << name_;
  return in->getDimensions();
}
Dimensions Region::getOutputDimensions(std::string name) const {
  if (name.empty()) {
    name = spec_->getDefaultOutputName();
  }
  Output* out = getOutput(name);
  NTA_CHECK(out != nullptr)
    << "Unknown output (" << name << ") requested on " << name_;
  return out->getDimensions();
}

void Region::setInputDimensions(std::string name, const Dimensions& dim) {
  if (name.empty()) {
    name = spec_->getDefaultOutputName();
  }
  Input* in = getInput(name);
  NTA_CHECK(in != nullptr)
    << "Unknown input (" << name << ") requested on " << name_;
  return in->setDimensions(dim);
}
void Region::setOutputDimensions(std::string name, const Dimensions& dim) {
  if (name.empty()) {
    name = spec_->getDefaultOutputName();
  }
  Output* out = getOutput(name);
  NTA_CHECK(out != nullptr)
    << "Unknown output (" << name << ") requested on " << name_;
  return out->setDimensions(dim);
}


// This is for backward compatability with API
// Normally Output dimensions are set by setting parameters known to the implementation.
// This set a global dimension.
void Region::setDimensions(Dimensions dim) {
  NTA_CHECK(!initialized_) << "Cannot set region dimensions after initialization.";
  impl_->setDimensions(dim);
}
Dimensions Region::getDimensions() const {
  return impl_->getDimensions();
}



void Region::removeAllIncomingLinks() {
  InputMap::const_iterator i = inputs_.begin();
  for (; i != inputs_.end(); i++) {
    auto &links = i->second->getLinks();
    while (links.size() > 0) {
      i->second->removeLink(links[0]);
    }
  }
}

void Region::uninitialize() { initialized_ = false; }

void Region::setPhases(std::set<UInt32> &phases) { phases_ = phases; }

std::set<UInt32> &Region::getPhases() { return phases_; }


void Region::save(std::ostream &f) const {
  f << "{\n";
  f << "name: " << name_ << "\n";
  f << "nodeType: " << type_ << "\n";
  f << "phases: [ " << phases_.size() << "\n";
  for (const auto &phases_phase : phases_) {
      f << phases_phase << " ";
  }
  f << "]\n";
  f << "outputs: [\n";
  for(auto out: outputs_) {
    f << out.first << " " << out.second->getDimensions() << "\n";
  }
  f << "]\n";
  f << "inputs: [\n";
  for(auto in: inputs_) {
    f << in.first << " " << in.second->getDimensions() << "\n";
  }
  f << "]\n";
  f << "RegionImpl:\n";
  // Now serialize the RegionImpl plugin.
  BundleIO bundle(&f);
  impl_->serialize(bundle);

  f << "}\n";
}

void Region::load(std::istream &f) {
  char bigbuffer[5000];
  std::string tag;
  Size count;
  Dimensions d;

  // Each region is a map -- extract the 5 values in the map
  f >> tag;
  NTA_CHECK(tag == "{") << "bad region entry (not a map)";

  // 1. name
  f >> tag;
  NTA_CHECK(tag == "name:");
  f.ignore(1);
  f.getline(bigbuffer, sizeof(bigbuffer));
  name_ = bigbuffer;

  // 2. nodeType
  f >> tag;
  NTA_CHECK(tag == "nodeType:");
  f.ignore(1);
  f.getline(bigbuffer, sizeof(bigbuffer));
  type_ = bigbuffer;

  // 3. phases
  f >> tag;
  NTA_CHECK(tag == "phases:");
  f >> tag;
  NTA_CHECK(tag == "[") << "Expecting a sequence of phases.";
  f >> count;
  phases_.clear();
  for (Size i = 0; i < count; i++)
  {
    UInt32 val;
    f >> val;
    phases_.insert(val);
  }
  f >> tag;
  NTA_CHECK(tag == "]") << "Expected end of sequence of phases.";

  // create inputs and outputs
  RegionImplFactory &factory = RegionImplFactory::getInstance();
  spec_ = factory.getSpec(type_);
  createInputsAndOutputs_();

  // 4. Restore dimensions on outputs
  {
    f >> tag;
    NTA_CHECK(tag == "outputs:");
    f >> tag;
    NTA_CHECK(tag == "[") << "Expecting a sequence of outputs.";
    f >> tag;
    while(!f.eof() && tag != "]") {
      f >> d;
      auto itr = outputs_.find(tag);
      if (itr != outputs_.end())
        itr->second->setDimensions(d);
      f >> tag;
    }
    NTA_CHECK(tag == "]") << "Expected end of sequence of outputs.";
  }

  // 5. Restore dimensions on inputs
  {
    f >> tag;
    NTA_CHECK(tag == "inputs:");
    f >> tag;
    NTA_CHECK(tag == "[") << "Expecting a sequence of inputs.";
    f >> tag;
    while(!f.eof() && tag != "]") {
      f >> d;
      auto itr = inputs_.find(tag);
      if (itr != inputs_.end())
        itr->second->setDimensions(d);
      f >> tag;
    }
    NTA_CHECK(tag == "]") << "Expected end of sequence of inputs.";
  }

  // 6. impl
  f >> tag;
  NTA_CHECK(tag == "RegionImpl:") << "Expected beginning of RegionImpl.";
  f.ignore(1);

  BundleIO bundle(&f);
  impl_.reset(factory.deserializeRegionImpl(type_, bundle, this));

  f >> tag;
  NTA_CHECK(tag == "}") << "Expected end of region. Found '" << tag << "'.";
}



void Region::enableProfiling() { profilingEnabled_ = true; }

void Region::disableProfiling() { profilingEnabled_ = false; }

void Region::resetProfiling() {
  computeTimer_.reset();
  executeTimer_.reset();
}

const Timer &Region::getComputeTimer() const { return computeTimer_; }

const Timer &Region::getExecuteTimer() const { return executeTimer_; }

bool Region::operator==(const Region &o) const {

  if (initialized_ != o.initialized_ || outputs_.size() != o.outputs_.size() ||
      inputs_.size() != o.inputs_.size()) {
    return false;
  }

  if (name_ != o.name_ || type_ != o.type_ ||
      spec_ != o.spec_ || phases_ != o.phases_ ) {
    return false;
  }
  if (getDimensions() != o.getDimensions()) {
    return false;
  }

  // Compare Regions's Input (checking only input buffer names and type)
  static auto compareInput = [](decltype(*inputs_.begin()) a, decltype(*inputs_.begin()) b) {
    if (a.first != b.first) {
      return false;
    }
    auto input_a = a.second;
    auto input_b = b.second;
    if (input_a->getDimensions() != input_b->getDimensions()) return false;
    if (input_a->isInitialized() != input_b->isInitialized()) return false;
    if (input_a->isInitialized()) {
      if (input_a->getData().getType() != input_b->getData().getType() ||
          input_a->getData().getCount() != input_b->getData().getCount())
        return false;
    }
    auto links_a = input_a->getLinks();
    auto links_b = input_b->getLinks();
    if (links_a.size() != links_b.size()) {
      return false;
    }
    for (size_t i = 0; i < links_a.size(); i++) {
      if (*(links_a[i]) != *(links_b[i])) {
        return false;
      }
    }
    return true;
  };
  if (!std::equal(inputs_.begin(), inputs_.end(), o.inputs_.begin(),
                  compareInput)) {
    return false;
  }
  // Compare Regions's Output (checking only output buffer names and type.)
  static auto compareOutput = [](decltype(*outputs_.begin()) a, decltype(*outputs_.begin()) b) {
    if (a.first != b.first ) {
      return false;
    }
    auto output_a = a.second;
    auto output_b = b.second;
    if (output_a->getDimensions() != output_b->getDimensions()) return false;
    if (output_a->getData().getType() != output_b->getData().getType() ||
        output_a->getData().getCount() != output_b->getData().getCount())
        return false;
    return true;
  };
  if (!std::equal(outputs_.begin(), outputs_.end(), o.outputs_.begin(),
                  compareOutput)) {
    return false;
  }

  return true;
}





// Internal methods called by RegionImpl.

Output *Region::getOutput(const std::string &name) const {
  auto o = outputs_.find(name);
  if (o == outputs_.end())
    return nullptr;
  return o->second;
}

Input *Region::getInput(const std::string &name) const {
  auto i = inputs_.find(name);
  if (i == inputs_.end())
    return nullptr;
  return i->second;
}

// Called by Network during serialization
const std::map<std::string, Input *> &Region::getInputs() const {
  return inputs_;
}

const std::map<std::string, Output *> &Region::getOutputs() const {
  return outputs_;
}


const Array& Region::getOutputData(const std::string &outputName) const {
  auto oi = outputs_.find(outputName);
  if (oi == outputs_.end())
    NTA_THROW << "getOutputData -- unknown output '" << outputName
              << "' on region " << getName();

  const Array& data = oi->second->getData();
  return data;
}

const Array& Region::getInputData(const std::string &inputName) const {
  auto ii = inputs_.find(inputName);
  if (ii == inputs_.end())
    NTA_THROW << "getInput -- unknown input '" << inputName << "' on region "
              << getName();

  const Array & data = ii->second->getData();
  return data;
}

void Region::prepareInputs() {
  // Ask each input to prepare itself
  for (InputMap::const_iterator i = inputs_.begin(); i != inputs_.end(); i++) {
    i->second->prepare();
  }
}


// setParameter

void Region::setParameterInt32(const std::string &name, Int32 value) {
  impl_->setParameterInt32(name, (Int64)-1, value);
}

void Region::setParameterUInt32(const std::string &name, UInt32 value) {
  impl_->setParameterUInt32(name, (Int64)-1, value);
}

void Region::setParameterInt64(const std::string &name, Int64 value) {
  impl_->setParameterInt64(name, (Int64)-1, value);
}

void Region::setParameterUInt64(const std::string &name, UInt64 value) {
  impl_->setParameterUInt64(name, (Int64)-1, value);
}

void Region::setParameterReal32(const std::string &name, Real32 value) {
  impl_->setParameterReal32(name, (Int64)-1, value);
}

void Region::setParameterReal64(const std::string &name, Real64 value) {
  impl_->setParameterReal64(name, (Int64)-1, value);
}

void Region::setParameterBool(const std::string &name, bool value) {
  impl_->setParameterBool(name, (Int64)-1, value);
}

// getParameter

Int32 Region::getParameterInt32(const std::string &name) const {
  return impl_->getParameterInt32(name, (Int64)-1);
}

Int64 Region::getParameterInt64(const std::string &name) const {
  return impl_->getParameterInt64(name, (Int64)-1);
}

UInt32 Region::getParameterUInt32(const std::string &name) const {
  return impl_->getParameterUInt32(name, (Int64)-1);
}

UInt64 Region::getParameterUInt64(const std::string &name) const {
  return impl_->getParameterUInt64(name, (Int64)-1);
}

Real32 Region::getParameterReal32(const std::string &name) const {
  return impl_->getParameterReal32(name, (Int64)-1);
}

Real64 Region::getParameterReal64(const std::string &name) const {
  return impl_->getParameterReal64(name, (Int64)-1);
}

bool Region::getParameterBool(const std::string &name) const {
  return impl_->getParameterBool(name, (Int64)-1);
}

// array parameters

void Region::getParameterArray(const std::string &name, Array &array) const {
  impl_->getParameterArray(name, (Int64)-1, array);
}

void Region::setParameterArray(const std::string &name, const Array &array) {
  impl_->setParameterArray(name, (Int64)-1, array);
}

void Region::setParameterString(const std::string &name, const std::string &s) {
  impl_->setParameterString(name, (Int64)-1, s);
}

std::string Region::getParameterString(const std::string &name) {
  return impl_->getParameterString(name, (Int64)-1);
}

bool Region::isParameter(const std::string &name) const {
  return (spec_->parameters.contains(name));
}

// Some functions used to prevent symbles from being in Region.hpp
void Region::saveDims(std::map<std::string,Dimensions>& outDims,
                      std::map<std::string,Dimensions>& inDims) const {
  for(auto out: outputs_) {
    Dimensions& dim = out.second->getDimensions();
    outDims[out.first] = dim;
  }
  for(auto in: inputs_) {
    Dimensions& dim = in.second->getDimensions();
    inDims[in.first] = dim;
  }
}
void Region::loadDims(std::map<std::string,Dimensions>& outDims,
                     std::map<std::string,Dimensions>& inDims) const {
  for(auto out: outDims) {
      auto itr = outputs_.find(out.first);
      if (itr != outputs_.end()) {
        itr->second->setDimensions(out.second);
      }
  }
  for(auto in: inDims) {
      auto itr = inputs_.find(in.first);
      if (itr != inputs_.end()) {
        itr->second->setDimensions(in.second);
      }
  }
}

void Region::serializeImpl(ArWrapper& arw) const{
    impl_->cereal_adapter_save(arw);
}
void Region::deserializeImpl(ArWrapper& arw) {
    RegionImplFactory &factory = RegionImplFactory::getInstance();
    spec_ = factory.getSpec(type_);
    createInputsAndOutputs_();

    impl_.reset(factory.deserializeRegionImpl(type_, arw, this));
}



} // namespace nupic
