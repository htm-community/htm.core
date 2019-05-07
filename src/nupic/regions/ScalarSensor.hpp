/* ---------------------------------------------------------------------
 * Numenta Platform for Intelligent Computing (NuPIC)
 * Copyright (C) 2016, Numenta, Inc.  Unless you have an agreement
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
 * Defines the ScalarSensor
 */

#ifndef NTA_SCALAR_SENSOR_HPP
#define NTA_SCALAR_SENSOR_HPP

#include <string>
#include <vector>

#include <nupic/engine/RegionImpl.hpp>
#include <nupic/ntypes/Value.hpp>
#include <nupic/types/Serializable.hpp>
#include <nupic/encoders/ScalarEncoder.hpp>

namespace nupic {
/**
 * A network region that encapsulates the ScalarEncoder.
 *
 * @b Description
 * A ScalarSensor encapsulates ScalarEncoders, connecting them to the Network
 * API. As a network runs, the client will specify new encoder inputs by
 * setting the "sensedValue" parameter. On each compute, the ScalarSensor will
 * encode its "sensedValue" to output.
 */
class ScalarSensor : public RegionImpl, Serializable {
public:
  ScalarSensor(const ValueMap &params, Region *region);
  ScalarSensor(BundleIO &bundle, Region *region);  // TODO:cereal Remove
  ScalarSensor(ArWrapper& wrapper, Region *region);

  virtual ~ScalarSensor() override;

  static Spec *createSpec();

  virtual Real64 getParameterReal64(const std::string &name, Int64 index = -1) override;
  virtual UInt32 getParameterUInt32(const std::string &name, Int64 index = -1) override;
  virtual void setParameterReal64(const std::string &name, Int64 index, Real64 value) override;
  virtual void initialize() override;

  virtual void serialize(BundleIO &bundle) override;
  virtual void deserialize(BundleIO &bundle) override;

  void compute() override;
  virtual std::string executeCommand(const std::vector<std::string> &args,
                                     Int64 index) override;

  virtual size_t
  getNodeOutputElementCount(const std::string &outputName) const override;

  CerealAdapter;  // see Serializable.hpp
  // FOR Cereal Serialization
  template<class Archive>
  void save_ar(Archive& ar) const {
    ar(CEREAL_NVP(sensedValue_));
    ar(cereal::make_nvp("minimum", params_.minimum),
       cereal::make_nvp("maximum", params_.maximum),
       cereal::make_nvp("clipInput", params_.clipInput),
       cereal::make_nvp("periodic", params_.periodic),
       cereal::make_nvp("activeBits", params_.activeBits),
       cereal::make_nvp("sparsity", params_.sparsity),
       cereal::make_nvp("size", params_.size),
       cereal::make_nvp("radius", params_.radius),
       cereal::make_nvp("resolution", params_.resolution));
    // TODO:cereal   Also serialize the outputs
  }
  // FOR Cereal Deserialization
  // NOTE: the Region Implementation must have been allocated
  //       using the RegionImplFactory so that it is connected
  //       to the Network and Region objects. This will populate
  //       the region_ field in the Base class.
  template<class Archive>
  void load_ar(Archive& ar) {
    ar(CEREAL_NVP(sensedValue_));
    ar(cereal::make_nvp("minimum", params_.minimum),
       cereal::make_nvp("maximum", params_.maximum),
       cereal::make_nvp("clipInput", params_.clipInput),
       cereal::make_nvp("periodic", params_.periodic),
       cereal::make_nvp("activeBits", params_.activeBits),
       cereal::make_nvp("sparsity", params_.sparsity),
       cereal::make_nvp("size", params_.size),
       cereal::make_nvp("radius", params_.radius),
       cereal::make_nvp("resolution", params_.resolution));
    // TODO:cereal   Also serialize the outputs
  }


private:
  Real64 sensedValue_;
  encoders::ScalarEncoderParameters params_;

  encoders::ScalarEncoder *encoder_;
  Output *encodedOutput_;
};
} // namespace nupic

#endif // NTA_SCALAR_SENSOR_HPP
