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
 * Implementation of the ArrayBase class
 */

#include <iostream> // for ostream
#include <sstream>  // for stringstream
#include <cstring>  // for memcpy, memcmp
#include <cstdlib> // for size_t
#include <vector>

#include <nupic/ntypes/ArrayBase.hpp>

#include <nupic/utils/Log.hpp>

namespace nupic {

/**
 * This makes a deep copy of the buffer so this class will own the buffer.
 */
ArrayBase::ArrayBase(NTA_BasicType type, void *buffer, size_t count) {
  if (!BasicType::isValid(type)) {
    NTA_THROW << "Invalid NTA_BasicType " << type
              << " used in array constructor";
  }
  type_ = type;
  allocateBuffer(count);
  if (has_buffer()) {
    std::memcpy(reinterpret_cast<char *>(getBuffer()), reinterpret_cast<char *>(buffer),
                count * BasicType::getSize(type));
  }
}

/**
 * constructor for Array object containing an SDR.
 * The SDR is copied. Array is the owner of the copy.
 */
ArrayBase::ArrayBase(const sdr::SDR &sdr) {
  type_ = NTA_BasicType_SDR;
  auto dim = sdr.dimensions;
  allocateBuffer(dim);
  if (has_buffer()) {
    std::memcpy(reinterpret_cast<char *>(getBuffer()), reinterpret_cast<char *>(sdr.getDense().data()), count_);
  }
  // sdr.setDenseInplace();
}

/**
 * Caller does not provide a buffer --
 * Nupic will either provide a buffer via setBuffer or
 * ask the ArrayBase to allocate a buffer via allocateBuffer.
 */
ArrayBase::ArrayBase(NTA_BasicType type) {
  if (!BasicType::isValid(type)) {
    NTA_THROW << "Invalid NTA_BasicType " << type
              << " used in array constructor";
  }
  type_ = type;
  releaseBuffer();
}

/**
 * The destructor will result in the shared_ptr being deleted.
 * If this is the last reference to the pointer, and this class owns the buffer,
 * the pointer will be deleted...making sure it will not leak.
 */
ArrayBase::~ArrayBase() {}

/**
 * Ask ArrayBase to allocate its buffer.  This class owns the buffer.
 * If there was already a buffer allocated, it will be released.
 * The buffer will be deleted when the last copy of this class has been deleted.
 */
char *ArrayBase::allocateBuffer(size_t count) {
  // Note that you can allocate a buffer of size zero.
  // The C++ spec (5.3.4/7) requires such a new request to return
  // a non-NULL value which is safe to delete.  This allows us to
  // disambiguate uninitialized ArrayBases and ArrayBases initialized with
  // size zero.
  count_ = count;
  if (type_ == NTA_BasicType_SDR) {
    std::vector<UInt> dimension;
    dimension.push_back((UInt)count);
    allocateBuffer(dimension);
  } else {
    std::shared_ptr<char> sp(new char[count_ * BasicType::getSize(type_)],
                             std::default_delete<char[]>());
    buffer_ = sp;
  }
  return buffer_.get();
}

char *ArrayBase::allocateBuffer( const std::vector<UInt>& dimensions) { // only for SDR
  NTA_CHECK(type_ == NTA_BasicType_SDR) << "Dimensions can only be set on the SDR payload";
  sdr::SDR *sdr = new sdr::SDR(dimensions);
  std::shared_ptr<char> sp(reinterpret_cast<char *>(sdr));
  buffer_ = sp;
  count_ = sdr->size;
  return buffer_.get();
}

/**
 * Will fill the buffer with 0's.
 */
void ArrayBase::zeroBuffer() {
  if (has_buffer()) {
    if (type_ == NTA_BasicType_SDR) {
        getSDR().zero();
    } else
      std::memset(buffer_.get(), 0, count_ * BasicType::getSize(type_));
  }
}

/**
 * Internal function
 * Use the given pointer as the buffer.
 * The caller is responsible to delete the buffer.
 * This class will NOT own the buffer so when this class and all copies
 * of this class are deleted the buffer will NOT be deleted.
 * NOTE: A crash condition WILL exists if this class is used
 *       after the object pointed to has gone out of scope. No protections.
 * This allows external buffers to be carried in the Array structure.
 */
void ArrayBase::setBuffer(void *buffer, size_t count) {
  NTA_CHECK(type_ != NTA_BasicType_SDR);
  count_ = count;
  buffer_ = std::shared_ptr<char>(reinterpret_cast<char *>(buffer), nonDeleter());
}
void ArrayBase::setBuffer(sdr::SDR &sdr) {
  type_ = NTA_BasicType_SDR;
  buffer_ = std::shared_ptr<char>(reinterpret_cast<char *>(&sdr), nonDeleter());
  count_ = sdr.size;
}



void ArrayBase::releaseBuffer() {
  buffer_.reset();
  count_ = 0;
}

void *ArrayBase::getBuffer() {
  if (has_buffer()) {
    if (type_ == NTA_BasicType_SDR) {
      return getSDR().getDense().data();
    }
    return buffer_.get();
  }
  return nullptr;
}

const void *ArrayBase::getBuffer() const {
  if (has_buffer()) {
    if (buffer_ != nullptr && type_ == NTA_BasicType_SDR) {
      return getSDR().getDense().data();
    }
    return buffer_.get();
  }
  return nullptr;
}

sdr::SDR& ArrayBase::getSDR() {
  NTA_CHECK(type_ == NTA_BasicType_SDR) << "Does not contain an SDR object";
  if (buffer_ == nullptr) {
    std::vector<UInt> zeroDim;
    zeroDim.push_back(0u);
    allocateBuffer(zeroDim);  // Create an empty SDR object.
  }
  sdr::SDR& sdr = *(reinterpret_cast<sdr::SDR *>(buffer_.get()));
  sdr.setDense(sdr.getDense()); // cleanup cache
  return sdr;
}
const sdr::SDR& ArrayBase::getSDR() const {
  NTA_CHECK(type_ == NTA_BasicType_SDR) << "Does not contain an SDR object";
  if (buffer_ == nullptr)
    // this is const, cannot create an empty SDR.
    NTA_THROW << "getSDR: SDR pointer is null";
  sdr::SDR& sdr = *(reinterpret_cast<sdr::SDR *>(buffer_.get()));
  sdr.setDense(sdr.getDense()); // cleanup cache
  return sdr;
}


/**
 * number of elements of the given type in the buffer.
 */
size_t ArrayBase::getCount() const {
  if (has_buffer() && type_ == NTA_BasicType_SDR) {
    return (reinterpret_cast<sdr::SDR *>(buffer_.get()))->size;
  }
  return count_;
};




/**
 * Return the NTA_BasicType of the current contents.
 */
NTA_BasicType ArrayBase::getType() const { return type_; };

/**
 * Return true if a buffer has been allocated.
 */
bool ArrayBase::has_buffer() const { return (buffer_.get() != nullptr); }

/**
 * Convert the buffer contents of the current ArrayBase into
 * the type of the incoming ArrayBase 'a'. Applying an offset if specified.
 * This may be called multiple times to set values of different offsets.
 * If there is not enough room in the destination buffer a new one is created.
 * After allocating the buffer, zero it to clear zero values (if converting
 * from Sparse to Dense).
 *
 * For Fan-In condition, be sure there is enough space in the buffer before
 * the first conversion to avoid loosing data during re-allocation. Then do
 * them in order so that the largest index is last.
 *
 * Be careful when using this with destination of SDR...it will remove
 * dimensions if buffer is  not big enough.
 *
 * args:
 *    a         - Destination buffer
 *    offset    - Index used as starting index. (defaults to 0)
 *    maxsize   - Total size of destination buffer (if 0, use source size)
 *                This is used to allocate destination buffer size (in counts).
 */
void ArrayBase::convertInto(ArrayBase &a, size_t offset, size_t maxsize) const {
  if (maxsize == 0)
    maxsize = getCount() + offset;
  if (maxsize > a.getCount()) {
    a.allocateBuffer(maxsize);
    a.zeroBuffer();
  }
	// TODO:  Comment this out until we are sure that it is not needed.
  //if (offset == 0) {
  //  // This could be the first buffer of a Fan-In set.
  //  if (a.getCount() != maxsize)
  //    a.setCount(maxsize);
  //}
  NTA_CHECK(getCount() + offset <= maxsize);
  char *toPtr =  reinterpret_cast<char *>(a.getBuffer()); // char* so it has size
  if (offset)
    toPtr += (offset * BasicType::getSize(a.getType()));
  const void *fromPtr = getBuffer();
  BasicType::convertArray(toPtr, a.type_, fromPtr, type_, getCount());
}

bool ArrayBase::isInstance(const ArrayBase &a) const {
  if (a.buffer_ == nullptr || buffer_ == nullptr)
    return false;
  return (buffer_.get() == a.buffer_.get());
}


///////////////////////////////////////////////////////////////////////////////
//    Compare operators
///////////////////////////////////////////////////////////////////////////////
// Compare contents of two ArrayBase objects
// Note: An Array and an ArrayRef could be the same if type, count, and buffer
// contents are the same.
bool operator==(const ArrayBase &lhs, const ArrayBase &rhs) {
  if (lhs.getType() != rhs.getType() || lhs.getCount() != rhs.getCount())
    return false;
  if (lhs.getCount() == 0u)
    return true;
  if (lhs.getType() == NTA_BasicType_SDR) {
    return (lhs.getSDR() == rhs.getSDR());
  }
  return (std::memcmp(lhs.getBuffer(), rhs.getBuffer(),
                      lhs.getCount() * BasicType::getSize(lhs.getType())) == 0);
}

template<typename T>
static bool compare_array_0_and_non0s_helper_(T ptr, const Byte *v, size_t size) {
    for (size_t i = 0; i < size; i++) {
      if (((v[i]!=0) && (((T)ptr)[i] == 0)) 
       || ((v[i]==0) && (((T)ptr)[i] != 0)))
        return false;
    }
    return true;
}

// Compare contents of a ArrayBase object and a vector of type Byte.  Actually
// we are only interested in 0 and non-zero values in this compare.
static bool compare_array_0_and_non0s_(const ArrayBase &a_side, const std::vector<nupic::Byte> &v_side) {
  
  if (a_side.getCount() != v_side.size()) return false;
  size_t ele_size = BasicType::getSize(a_side.getType());
  size_t size = a_side.getCount();
  const void *a_ptr = a_side.getBuffer();
  const Byte *v_ptr = &v_side[0];
  switch(ele_size) { 
  default:
  case 1: return compare_array_0_and_non0s_helper_(reinterpret_cast<const Byte*  >(a_ptr), v_ptr, size);
  case 2: return compare_array_0_and_non0s_helper_(reinterpret_cast<const UInt16*>(a_ptr), v_ptr, size);
  case 4: return compare_array_0_and_non0s_helper_(reinterpret_cast<const UInt32*>(a_ptr), v_ptr, size);
  case 8: return compare_array_0_and_non0s_helper_(reinterpret_cast<const UInt64*>(a_ptr), v_ptr, size);
  }
  return true;
}
bool operator==(const ArrayBase &lhs, const std::vector<nupic::Byte> &rhs) {
  return compare_array_0_and_non0s_(lhs, rhs);
}
bool operator==(const std::vector<nupic::Byte> &lhs, const ArrayBase &rhs) {
  return compare_array_0_and_non0s_(rhs, lhs);
}

////////////////////////////////////////////////////////////////////////////////
//         Stream Serialization (as binary)
////////////////////////////////////////////////////////////////////////////////
void ArrayBase::save(std::ostream &outStream) const {
  outStream << "[ " << count_ << " " << BasicType::getName(type_) << " ";
  if (has_buffer() && type_ == NTA_BasicType_SDR) {
    const sdr::SDR& sdr = getSDR();
    sdr.save(outStream);
  } else {

    if (count_ > 0) {
      Size size = count_ * BasicType::getSize(type_);
      outStream.write(reinterpret_cast<const char *>(buffer_.get()), size);
    }
  }
  outStream << "]" << std::endl;
}
void ArrayBase::load(std::istream &inStream) {
  std::string tag;
  size_t count;

  NTA_CHECK(inStream.get() == '[')
      << "Binary load of Array, expected starting '['.";
  inStream >> count;
  inStream >> tag;
  type_ = BasicType::parse(tag);
  if (count > 0 && type_ == NTA_BasicType_SDR) {
    sdr::SDR *sdr = new sdr::SDR();
    sdr->load(inStream);
    std::shared_ptr<char> sp(reinterpret_cast<char *>(sdr));
    buffer_ = sp;
    count_ = sdr->size;
  } else {
    allocateBuffer(count);
    inStream.ignore(1);
    inStream.read(buffer_.get(), count_ * BasicType::getSize(type_));
  }
  NTA_CHECK(inStream.get() == ']')
      << "Binary load of Array, expected ending ']'.";
  inStream.ignore(1); // skip over the endl
}

////////////////////////////////////////////////////////////////////////////////
//         Stream Serialization  (as Ascii text character strings)
//              [ type count ( item item item ...) ... ]
////////////////////////////////////////////////////////////////////////////////

template <typename T>
static void _templatedStreamBuffer(std::ostream &outStream, const void *inbuf,
                                   size_t numElements) {
  outStream << "( ";

  // Stream the elements
  auto it = reinterpret_cast<const T *>(inbuf);
  auto const end = it + numElements;
  if (it < end) {
    for (; it < end; ++it) {
      outStream << 0 + (*it) << " ";
    }
    // note: Adding 0 to value so Byte displays as numeric.
  }
  outStream << ") ";
}

std::string ArrayBase::toString() const {
  std::stringstream outStream;

  auto const inbuf = getBuffer();
  auto const numElements = getCount();
  auto const elementType = getType();
  if (elementType == NTA_BasicType_SDR) {
    if (!has_buffer())
      outStream << "[ SDR(0) nullptr ]";
    else
      outStream << "[ " << getSDR() << " ]";
  }
  else {
    outStream << "[ " << BasicType::getName(elementType) << " " << numElements
              << " ";

    switch (elementType) {
    case NTA_BasicType_Byte:
      _templatedStreamBuffer<Byte>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int16:
      _templatedStreamBuffer<Int16>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt16:
      _templatedStreamBuffer<UInt16>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int32:
      _templatedStreamBuffer<Int32>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt32:
      _templatedStreamBuffer<UInt32>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int64:
      _templatedStreamBuffer<Int64>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt64:
      _templatedStreamBuffer<UInt64>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real32:
      _templatedStreamBuffer<Real32>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real64:
      _templatedStreamBuffer<Real64>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Bool:
      _templatedStreamBuffer<bool>(outStream, inbuf, numElements);
      break;
    default:
      NTA_THROW << "Unexpected Element Type: " << elementType;
      break;
    }
    outStream << " ] ";
  }
  return outStream.str();
}

std::ostream &operator<<(std::ostream &outStream, const ArrayBase &a) {
	outStream << a.toString(); 
	return outStream;
}


template <typename T>
static void _templatedStreamBuffer(std::istream &inStream, void *buf,
                                   size_t numElements) {
  std::string v;
  inStream >> v;
  NTA_CHECK(v == "(")
      << "deserialize Array buffer...expected an opening '(' but not found.";

  // Stream the elements
  auto it = reinterpret_cast<T *>(buf);
  auto const end = it + numElements;
  if (it < end) {
    for (; it < end; ++it) {
      inStream >> *it;
    }
  }
  inStream >> v;
  NTA_CHECK(v == ")")
      << "deserialize Array buffer...expected a closing ')' but not found.";
}

std::istream &operator>>(std::istream &inStream, ArrayBase &a) {
  std::string v;
  size_t numElements;

  inStream >> v;
  NTA_CHECK(v == "[")
      << "deserialize Array object...expected an opening '[' but not found.";

  inStream >> v;
  a.type_ = BasicType::parse(v);
  inStream >> numElements;
  if (numElements > 0 && a.type_ == NTA_BasicType_SDR) {
    sdr::SDR *sdr = new sdr::SDR();
    sdr->load(inStream);
    std::shared_ptr<char> sp(reinterpret_cast<char *>(sdr));
    a.buffer_ = sp;
  } else {
    a.allocateBuffer(numElements);
  }

  if (a.has_buffer()) {
    auto inbuf = a.getBuffer();
    switch (a.type_) {
    case NTA_BasicType_Byte:
      _templatedStreamBuffer<Byte>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int16:
      _templatedStreamBuffer<Int16>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt16:
      _templatedStreamBuffer<UInt16>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int32:
      _templatedStreamBuffer<Int32>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt32:
      _templatedStreamBuffer<UInt32>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int64:
      _templatedStreamBuffer<Int64>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt64:
      _templatedStreamBuffer<UInt64>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real32:
      _templatedStreamBuffer<Real32>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real64:
      _templatedStreamBuffer<Real64>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Bool:
      _templatedStreamBuffer<bool>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_SDR:
      _templatedStreamBuffer<Byte>(inStream, inbuf, numElements);
      break;
    default:
      NTA_THROW << "Unexpected Element Type: " << a.type_;
      break;
    }
  }
  inStream >> v;
  NTA_CHECK(v == "]")
      << "deserialize Array buffer...expected a closing ']' but not found.";
  inStream.ignore(1);

  return inStream;
}

} // namespace nupic
