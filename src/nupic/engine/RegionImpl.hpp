/* ---------------------------------------------------------------------
 * Numenta Platform for Intelligent Computing (NuPIC)
 * Copyright (C) 2013-2015, Numenta, Inc.  Unless you have an agreement
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
 * Definition of the RegionImpl API
 *
 * A RegionImpl is a node "plugin" that provides most of the
 * implementation of a Region, including algorithms.
 *
 * The RegionImpl class is expected to be subclassed for particular
 * node types (e.g. FDRNode, PyNode, etc) and RegionImpls are created
 * by the RegionImplFactory
 */

#ifndef NTA_REGION_IMPL_HPP
#define NTA_REGION_IMPL_HPP

#include <iostream>
#include <string>
#include <vector>
#include <nupic/engine/Output.hpp>
#include <nupic/engine/Input.hpp>
#include <nupic/engine/Region.hpp>
#include <nupic/ntypes/Dimensions.hpp>
#include <nupic/ntypes/BundleIO.hpp>
#include <nupic/types/Serializable.hpp>

namespace nupic {

class Spec;
class Region;
class Dimensions;
class Input;
class Output;
class Array;
class NodeSet;

class RegionImpl
{
public:
  // All subclasses must call this constructor from their regular constructor
  RegionImpl(Region *region);

  virtual ~RegionImpl();

  /* ------- Convenience methods  that access region data -------- */

  std::string getType() const;

  std::string getName() const;


  /* ------- Parameter support in the base class. ---------*/
  // The default implementation of all of these methods goes through
  // set/getParameterFromBuffer, which is compatible with NuPIC 1.
  // RegionImpl subclasses may override for higher performance.

  virtual Int32 getParameterInt32(const std::string &name, Int64 index);
  virtual UInt32 getParameterUInt32(const std::string &name, Int64 index);
  virtual Int64 getParameterInt64(const std::string &name, Int64 index);
  virtual UInt64 getParameterUInt64(const std::string &name, Int64 index);
  virtual Real32 getParameterReal32(const std::string &name, Int64 index);
  virtual Real64 getParameterReal64(const std::string &name, Int64 index);
  virtual bool getParameterBool(const std::string &name, Int64 index);

  virtual void setParameterInt32(const std::string &name, Int64 index,
                                 Int32 value);
  virtual void setParameterUInt32(const std::string &name, Int64 index,
                                  UInt32 value);
  virtual void setParameterInt64(const std::string &name, Int64 index,
                                 Int64 value);
  virtual void setParameterUInt64(const std::string &name, Int64 index,
                                  UInt64 value);
  virtual void setParameterReal32(const std::string &name, Int64 index,
                                  Real32 value);
  virtual void setParameterReal64(const std::string &name, Int64 index,
                                  Real64 value);
  virtual void setParameterBool(const std::string &name, Int64 index,
                                bool value);

  virtual void getParameterArray(const std::string &name, Int64 index,
                                 Array &array);
  virtual void setParameterArray(const std::string &name, Int64 index,
                                 const Array &array);

  virtual void setParameterString(const std::string &name, Int64 index,
                                  const std::string &s);
  virtual std::string getParameterString(const std::string &name, Int64 index);

  /* -------- Methods that must be implemented by subclasses -------- */

  /**
   * Region implimentations must implement createSpec().
   * Can't declare a static method in an interface. But RegionFactory
   * expects to find this method. Caller gets ownership of Spec pointer.
   * The returned spec pointer is cached by RegionImplFactory in regionSpecMap
   * which is a map of shared_ptr's.
   */
  // static Spec* createSpec();

  // Serialize/Deserialize state.
  virtual void serialize(BundleIO &bundle) = 0;
  virtual void deserialize(BundleIO &bundle) = 0;

  virtual void cereal_adapter_save(ArWrapper& a) const {};
  virtual void cereal_adapter_load(ArWrapper& a) {};

  /**
    * Inputs/Outputs are made available in initialize()
    * It is always called after the constructor (or load from serialized state)
    */
  virtual void initialize() = 0;

  // Compute outputs from inputs and internal state
  virtual void compute() = 0;

  /* -------- Methods that may be overridden by subclasses -------- */

  // Execute a command
  virtual std::string executeCommand(const std::vector<std::string> &args,
                                     Int64 index);


  // Buffer size (in elements) of the given input/output.
  // It is the total element count.
  // This method is called only for buffers whose size is not
  // specified in the Spec.  This is used to allocate
  // buffers during initialization.  New implementations should instead
  // override askImplForOutputDimensions() or askImplForInputDimensions()
  // and return a full dimension.
  // Return 0 for outputs that are not used or size does not matter.
  virtual size_t getNodeInputElementCount(const std::string &outputName) const {
    return Dimensions::DONTCARE;
  }
  virtual size_t getNodeOutputElementCount(const std::string &outputName) const {
    return Dimensions::DONTCARE;
  }


  // The dimensions for the specified input or output.  This is called by
  // Link when it allocates buffers during initialization.
  // If this region sets topology (an SP for example) and will be
  // setting the dimensions (i.e. from parameters) then
  // return the dimensions that should be placed in its Array buffer.
  // Return an isDontCare() Dimension if this region should inherit
  // dimensions from elsewhere.
  //
  // If this is not overridden, the default implementation will call
  // getNodeOutputElementCount() or getNodeInputElementCount() to obtain
  // a 1D dimension for this input/output.
  virtual Dimensions askImplForInputDimensions(const std::string &name);
  virtual Dimensions askImplForOutputDimensions(const std::string &name);


  /**
   * Array-valued parameters may have a size determined at runtime.
   * This method returns the number of elements in the named parameter.
   * If parameter is not an array type, may throw an exception or return 1.
   *
   * Must be implemented only if the node has one or more array
   * parameters with a dynamically-determined length.
   */
  virtual size_t getParameterArrayCount(const std::string &name, Int64 index);

  /**
   * Set Global dimensions on a region.
   * Normally a Region Impl will use this to set the dimensions on the default output.
   * This cannot be used to override a fixed buffer setting in the Spec.
   * Args: dim   - The dimensions to set
   */
  virtual void setDimensions(Dimensions dim) { dim_ = dim; }
  virtual Dimensions getDimensions() const { return dim_; }


protected:
  // A pointer to the Region object. This is the portion visible
	// to the applications.  This class and it's subclasses are the
	// hidden implementations behind the Region class.
	// Note: this cannot be a shared_ptr. Its pointer is passed via
	//       the API so it must be a bare pointer so we don't have
	//       a copy of the shared_ptr held by the Collection in Network.
	//       This pointer must NOT be deleted.
  Region* region_;

  /* -------- Methods provided by the base class for use by subclasses --------
   */

  // Region level dimensions.  This is set by the parameter "{dim: [2,3]}"
  // or by region->setDimensions(d);
  // A region implementation may use this for whatever it wants but it is normally
  // applied to the default output buffer.
  Dimensions dim_;

  // ---
  /// Callback for subclasses to get an output stream during serialize()
  /// (for output) and the deserializing constructor (for input)
  /// It is invalid to call this method except inside serialize() in a subclass.
  ///
  /// Only one serialization stream may be open at a time. Calling
  /// getSerializationXStream a second time automatically closes the
  /// first stream. Any open stream is closed when serialize() returns.
  // ---
  //std::ostream &getSerializationOutputStream(const std::string &name);
  //std::istream &getSerializationInputStream(const std::string &name);
  //std::string getSerializationPath(const std::string &name);

  // These methods provide access to inputs and outputs
  // They raise an exception if the named input or output is
  // not found.
  Input *getInput(const std::string &name) const;
  Output *getOutput(const std::string &name) const;
  Dimensions getInputDimensions(const std::string &name="") const;
  Dimensions getOutputDimensions(const std::string &name="") const;

};

} // namespace nupic

#endif // NTA_REGION_IMPL_HPP
