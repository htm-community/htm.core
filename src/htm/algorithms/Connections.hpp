/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2014-2016, Numenta, Inc.
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
 * ---------------------------------------------------------------------- */

/** @file
 * Definitions for the Connections class in C++
 */

#ifndef NTA_CONNECTIONS_HPP
#define NTA_CONNECTIONS_HPP

#include <limits>
#include <map>
#include <unordered_map>
#include <set>
#include <utility>
#include <vector>
#include <deque>

#include <htm/types/Types.hpp>
#include <htm/types/Serializable.hpp>
#include <htm/types/Sdr.hpp>

namespace htm {


//TODO instead of typedefs, use templates for proper type-checking?
using CellIdx   = htm::ElemSparse; // CellIdx must match with ElemSparse, defined in Sdr.hpp
using SegmentIdx= UInt16; /** Index of segment in cell. */
using SynapseIdx= UInt16; /** Index of synapse in segment. */
using Segment   = UInt32;    /** Index of segment's data. */
using Synapse   = UInt32;    /** Index of synapse's data. */
using Permanence= Real32;
constexpr const Permanence minPermanence = static_cast<Permanence>(0.0);
constexpr const Permanence maxPermanence = static_cast<Permanence>(1.0);

 /**
  * Epsilon is defined for the whole math and algorithms of the Numenta
  * Platform, independently of the concrete type chosen to handle floating point
  * numbers.
  *     numeric_limits<float>::epsilon()  == 1.19209e-7
  *     numeric_limits<double>::epsilon() == 2.22045e-16
  */
static constexpr const Permanence Epsilon = static_cast<Permanence>(1e-6);



/**
 * SynapseData class used in Connections.
 *
 * @b Description
 * The SynapseData contains the underlying data for a synapse.
 *
 * @param presynapticCellIdx
 * Cell that this synapse gets input from.
 *
 * @param permanence
 * Permanence of synapse.
 */
struct SynapseData: public Serializable {
  CellIdx presynapticCell;
  Permanence permanence;
  Segment segment;
  Synapse presynapticMapIndex_;

  SynapseData() {}

  //Serialization
  CerealAdapter;
  template<class Archive>
  void save_ar(Archive & ar) const {
    ar(CEREAL_NVP(permanence),
       CEREAL_NVP(presynapticCell),
       CEREAL_NVP(segment),
       CEREAL_NVP(presynapticMapIndex_)
    );
  }
  template<class Archive>
  void load_ar(Archive & ar) {
    ar( permanence, presynapticCell, segment, presynapticMapIndex_);
  }

  //operator==
  bool operator==(const SynapseData& o) const {
    try {
    NTA_CHECK(presynapticCell == o.presynapticCell ) << "SynapseData equals: presynapticCell";
    NTA_CHECK(permanence == o.permanence ) << "SynapseData equals: permanence";
    NTA_CHECK(segment == o.segment ) << "SynapseData equals: segment";
    NTA_CHECK(presynapticMapIndex_ == o.presynapticMapIndex_ ) << "SynapseData equals: presynapticMapIndex_";
    } catch(const htm::Exception& ex) {
      UNUSED(ex);    // this avoids the warning if ex is not used.
      //NTA_WARN << "SynapseData equals: " << ex.what(); //Note: uncomment for debug, tells you 
      //where the diff is. It's perfectly OK for the "exception" to occur, as it just denotes
      //that the data is NOT equal.
      return false;
    }
    return true;
  }
  inline bool operator!=(const SynapseData& o) const { return !operator==(o); }

};

/**
 * SegmentData class used in Connections.
 *
 * @b Description
 * The SegmentData contains the underlying data for a Segment.
 *
 * @param synapses
 * Synapses on this segment.
 *
 * @param cell
 * The cell that this segment is on.
 */
struct SegmentData: public Serializable {
  SegmentData(const CellIdx cell) : cell(cell), numConnected(0) {} //default constructor

  std::vector<Synapse> synapses;
  CellIdx cell; //mother cell that this segment originates from
  SynapseIdx numConnected; //number of permanences from `synapses` that are >= synPermConnected, ie connected synapses

  //Serialize
  SegmentData() {}; //empty constructor for serialization, do not use
  CerealAdapter;
  template<class Archive>
  void save_ar(Archive & ar) const {
    ar(CEREAL_NVP(synapses),
       CEREAL_NVP(cell),
       CEREAL_NVP(numConnected)
    );
  }
  template<class Archive>
  void load_ar(Archive & ar) {
    ar( synapses, cell, numConnected);
  }

  //equals op==
  bool operator==(const SegmentData& o) const {
    try {
      NTA_CHECK(synapses == o.synapses) << "SegmentData equals: synapses";
      NTA_CHECK(cell == o.cell) << "SegmentData equals: cell";
      NTA_CHECK(numConnected == o.numConnected) << "SegmentData equals: numConnected";

    } catch(const htm::Exception& ex) {
      UNUSED(ex);    // this avoids the warning if ex is not used.
      //NTA_WARN << "SegmentData equals: " << ex.what();
      return false;
    }
    return true;
  }
  inline bool operator!=(const SegmentData& o) const { return !operator==(o); }
};

/**
 * CellData class used in Connections.
 * A cell consists of segments and in Connections is indexed by CellIdx.
 *
 * @b Description
 * The CellData contains the underlying data for a Cell.
 *
 * @param segments
 * Segments on this cell.
 *
 */
struct CellData : public Serializable {
  std::vector<Segment> segments;

  //Serialization
  CerealAdapter;
  template<class Archive>
  void save_ar(Archive & ar) const {
    ar(CEREAL_NVP(segments)
    );
  }
  template<class Archive>
  void load_ar(Archive & ar) {
    ar( segments);
  }

  //operator==
  bool operator==(const CellData& o) const {
    try {
      NTA_CHECK( segments == o.segments ) << "CellData equals: segments";
    } catch(const htm::Exception& ex) {
      UNUSED(ex);    // this avoids the warning if ex is not used.
      //NTA_WARN << "CellData equals: " << ex.what();
      return false;
    }
    return true;
  }
  inline bool operator!=(const CellData& o) const { return !operator==(o); }
};


/**
 * A base class for Connections event handlers.
 *
 * @b Description
 * This acts as a plug-in point for logging / visualizations.
 */
class ConnectionsEventHandler {
public:
  virtual ~ConnectionsEventHandler() {}

  /**
   * Called after a segment is created.
   */
  virtual void onCreateSegment(Segment segment) {}

  /**
   * Called before a segment is destroyed.
   */
  virtual void onDestroySegment(Segment segment) {}

  /**
   * Called after a synapse is created.
   */
  virtual void onCreateSynapse(Synapse synapse) {}

  /**
   * Called before a synapse is destroyed.
   */
  virtual void onDestroySynapse(Synapse synapse) {}

  /**
   * Called after a synapse's permanence crosses the connected threshold.
   */
  virtual void onUpdateSynapsePermanence(Synapse synapse,
                                         Permanence permanence) {}
};

/**
 * Connections implementation in C++.
 *
 * @b Description
 * The Connections class is a data structure that represents the
 * connections of a collection of cells. It is used in the HTM
 * learning algorithms to store and access data related to the
 * connectivity of cells.
 *
 * Its main utility is to provide a common, optimized data structure
 * that all HTM learning algorithms can use. It is flexible enough to
 * support any learning algorithm that operates on a collection of cells.
 *
 * Each type of connection (proximal, distal basal, apical) should be
 * represented by a different instantiation of this class. This class
 * will help compute the activity along those connections due to active
 * input cells. The responsibility for what effect that activity has on
 * the cells and connections lies in the user of this class.
 *
 * This class is optimized to store connections between cells, and
 * compute the activity of cells due to input over the connections.
 *
 * This class assigns each segment a unique "flatIdx" so that it's
 * possible to use a simple vector to associate segments with values.
 * Create a vector of length `connections.segmentFlatListLength()`,
 * iterate over segments and update the vector at index `segment`.
 *
 */
class Connections : public Serializable
 {
public:
  static const UInt16 VERSION = 2;

  /**
   * Connections empty constructor.
   * (Does not call `initialize`.)
   */
  Connections(){};

  /**
   * Connections constructor.
   *
   * @param numCells           Number of cells.
   * @param connectedThreshold Permanence threshold for synapses connecting or
   *                           disconnecting.
   *
   * @params timeseries - Optional, default false.  If true AdaptSegment will not
   * apply the same learning update to a synapse on consequetive cycles, because
   * then staring at the same object for too long will mess up the synapses.
   * IE Highly correlated inputs will cause the synapse permanences to saturate.
   * This change allows it to work with timeseries data which moves very slowly,
   * instead of the usual HTM inputs which reliably change every cycle.  See
   * also (Kropff & Treves, 2007. http://dx.doi.org/10.2976/1.2793335).
   */
  Connections(const CellIdx numCells, 
	      const Permanence connectedThreshold = static_cast<Permanence>(0.5),
              const bool timeseries = false);

  virtual ~Connections() {} 

  /**
   * Initialize connections.
   *
   * @param numCells           Number of cells.
   * @param connectedThreshold Permanence threshold for synapses connecting or
   *                           disconnecting.
   * @param timeseries         See constructor.
   */
  void initialize(const CellIdx numCells, 
		  const Permanence connectedThreshold = static_cast<Permanence>(0.5),
                  const bool timeseries = false);

  /**
   * Creates a segment on the specified cell.
   *
   * @param cell Cell to create segment on.
   *
   * @param maxSegmentsPerCell Optional. Enforce limit on maximum number of segments that can be
   * created on a Cell. If the limit is exceeded, call `destroySegment` to remove least useful segments 
   * (as determined by a heuristic). Default value is numeric_limits::max() of the data-type, 
   * so effectively disabled.
   *
   * @retval Unique ID of the created segment `seg`. Use `dataForSegment(seg)` to obtain the segment's data. 
   * Use  `idxOfSegmentOnCell()` to get SegmentIdx of `seg` on this `cell`. 
   *
   */
  Segment createSegment(const CellIdx cell, 
		        const SegmentIdx maxSegmentsPerCell = std::numeric_limits<SegmentIdx>::max());

  /**
   * Creates a synapse on the specified segment that connects to the presynaptic cell.
   *
   * Note 1: If attemping to connect to an already synapsed presynaptic cell, we don't create
   *   a duplicit synapse, and just return early with the existing synapse. 
   *   This has an effect that `connections.synapsesForSegment()` is not ensured to grow (by +1)
   *   after calling `createSynapse()` is the method conditionally skips. Users can query this by
   *   `connections.numSynapses(segment)`.
   *
   *   Explanation:
   *     Biological motivation (?):
   *     There are structural constraints on the shapes of axons & synapses
   *     which prevent a large number duplicate of connections.
   *
   *     It's important to prevent cells from growing duplicate synapses onto a segment,
   *     because otherwise a strong input would be sampled many times and grow many synapses.
   *     That would give such input a stronger connection.
   *     Synapses are supposed to have binary effects (0 or 1) but duplicate synapses give
   *     them (synapses 0/1) varying levels of strength.
   *
   * @param segment         Segment to create synapse on.
   * @param presynapticCell Cell to synapse on.
   * @param permanence      Initial permanence of new synapse. If calling "create" on an existing synapse (same segment, presynapticCell),
   *   then we either keep the old one, or update the old one to have higher permanence (from the new call).
   *
   * @return Created synapse - index to the newly created synapse. Use `dataForSynapse(returnedValue)` to work with it.
   */
  Synapse createSynapse(const Segment segment,
                        const CellIdx presynapticCell,
                        Permanence permanence);



  /**
   * Grow new synapses for disconnected inputs/candidates.
   *
   * For each specified segments, grow synapses to all specified inputs that
   * aren't already connected to the segment. 
   *
   * @param segment - Segment: The segment to modify 
   * @param growthCandidates - vector<Synapse>&: The inputs to connect to 
   * @param initialPermanence - Permanence: The permanence for each added synapse
   * @param maxNew - (optional) size_t, default = 0/unused. If set to [1, growthCandidates.size())
   *   then we subsample the disconnected inputs to be connected. This is used to limit "bursts" when
   *   a huge number of cells/inputs would be newly connected at once. Which would disturb the HTM.
   * @param rng - Random&, used to sample if `maxNew` is used. 
   * @param maxSynapsesPerSegment - (optional) size_t, default=0/off. If >0: enforce limit on max
   *   number of synapses on a segment. If reached, weak synapses will be purged to make space.
   *
   **/
  void growSynapses(const Segment segment, 
		                    const std::vector<Synapse>& growthCandidates, 
				    const Permanence initialPermanence,
				    Random& rng,
				    const size_t maxNew = 0,
				    const size_t maxSynapsesPerSegment = 0
				    );

  /**
   * Destroys segment.
   *
   * @param segment Segment to destroy.
   */
  void destroySegment(const Segment segment);

  /**
   * Destroys synapse.
   *
   * @param synapse Synapse to destroy.
   * @throws if synapse does not exist (ie already removed)
   */
  void destroySynapse(const Synapse synapse);

  /**
   * Updates a synapse's permanence.
   *
   * @param synapse    Synapse to update.
   * @param permanence New permanence.
   */
  void updateSynapsePermanence(const Synapse synapse, 
		               Permanence permanence);

  /**
   * Gets the segments for a cell.
   *
   * @param cell Cell to get segments for.
   *
   * @retval Segments on cell.
   */
  const std::vector<Segment> &segmentsForCell(const CellIdx cell) const {
    return cells_[cell].segments;
  }

  /**
   * Gets the synapses for a segment.
   *
   * @param segment Segment to get synapses for.
   *
   * @retval Synapses on segment.
   */
  const std::vector<Synapse> &synapsesForSegment(const Segment segment) const {
    NTA_ASSERT(segment < segments_.size()) << "Segment out of bounds! " << segment;
    return segments_[segment].synapses;
  }

  /**
   * Gets the cell that this segment is on.
   *
   * @param segment Segment to get the cell for.
   *
   * @retval Cell that this segment is on.
   */
  CellIdx cellForSegment(const Segment segment) const {
    return segments_[segment].cell;
  }

  /**
   *  Collect all presynaptic cells for segments (inputs)
   *  @param 
   *
   **/
  std::vector<CellIdx> presynapticCellsForSegment(const Segment segment) const;

  /**
   * Gets the index of this segment on its respective cell.
   *
   * @param segment Segment to get the idx for.
   *
   * @retval Index of the segment.
   */
  SegmentIdx idxOnCellForSegment(const Segment segment) const;

  /**
   * Gets the segment that this synapse is on.
   *
   * @param synapse Synapse to get Segment for.
   *
   * @retval Segment that this synapse is on.
   */
  Segment segmentForSynapse(const Synapse synapse) const {
    return synapses_[synapse].segment;
  }

  /**
   * Gets the data for a segment.
   *
   * @param segment Segment to get data for.
   *
   * @retval Segment data.
   */
  const SegmentData &dataForSegment(const Segment segment) const {
    return segments_[segment];
  }
  SegmentData& dataForSegment(const Segment segment) { //editable access, needed by SP
    return segments_[segment];
  }

  /**
   * Gets the data for a synapse.
   *
   * @param synapse Synapse to get data for.
   *
   * @retval Synapse data.
   */
  inline const SynapseData& dataForSynapse(const Synapse synapse) const {
    NTA_CHECK(synapseExists_(synapse, true));
    return synapses_[synapse];
  }

  /**
   * Get the segment at the specified cell and offset.
   *
   * @param cell The cell that the segment is on.
   * @param idx The index of the segment on the cell.
   *
   * @retval Segment
   */
  inline Segment getSegment(const CellIdx cell, const SegmentIdx idx) const {
    return cells_[cell].segments[idx];
  }

  /**
   * Get the vector length needed to use segments as indices.
   *
   * @retval A vector length
   */
  inline size_t segmentFlatListLength() const noexcept { return segments_.size(); };

  /**
   * Compare two segments. Returns true if a < b.
   *
   * Segments are ordered first by cell, then by their order on the cell.
   *
   * @param a Left segment to compare
   * @param b Right segment to compare
   *
   * @retval true if a < b, false otherwise.
   */
  bool compareSegments(const Segment a, const Segment b) const;

  /**
   * Returns the synapses for the source cell that they synapse on.
   *
   * @param presynapticCell(int) Source cell index
   *
   * @return Synapse indices
   */
  std::vector<Synapse> synapsesForPresynapticCell(const CellIdx presynapticCell) const;

  /**
   * For use with time-series datasets.
   */
  void reset() noexcept;

  /**
   * Compute the segment excitations for a vector of active presynaptic
   * cells.
   *
   * The output vectors aren't grown or cleared. They must be
   * preinitialized with the length returned by
   * getSegmentFlatVectorLength().
   *
   * @param (optional) numActivePotentialSynapsesForSegment
   * An output vector for active potential synapse counts per segment.
   *
   * @param activePresynapticCells
   * Active cells in the input.
   *
   * @param bool learn : enable learning updates (default true)
   *
   * @return numActiveConnectedSynapsesForSegment
   * An output vector for active connected synapse counts per segment.
   *
   */
  std::vector<SynapseIdx> computeActivity(
                       std::vector<SynapseIdx> &numActivePotentialSynapsesForSegment,
                       const std::vector<CellIdx> &activePresynapticCells,
		       const bool learn = true);

  std::vector<SynapseIdx> computeActivity(const std::vector<CellIdx> &activePresynapticCells, 
		                          const bool learn = true);

  /**
   * The primary method in charge of learning.   Adapts the permanence values of
   * the synapses based on the input SDR.  Learning is applied to a single
   * segment.  Permanence values are increased for synapses connected to input
   * bits that are turned on, and decreased for synapses connected to inputs
   * bits that are turned off.
   *
   * @param segment  Index of segment to apply learning to.  Is returned by 
   *        method getSegment.
   * @param inputVector  An SDR
   * @param increment  Change in permanence for synapses with active presynapses.
   * @param decrement  Change in permanence for synapses with inactive presynapses.
   * @param pruneZeroSynapses (default false) If set, synapses that reach minPermanence(aka. "zero")
   *        are removed. This is used in TemporalMemory and SpatialPooler.  
   * @param segmentThreshold (optional) (default 0) Minimum number of connected synapses for a segment
   *        to be considered active. @see raisePermenencesToThreshold(). Equivalent to `SP.stimulusThreshold`.
   *        If `pruneZeroSynapses` is used and synapses are removed, if the amount of synapses drops below 
   *        `segmentThreshold`, we'll remove the segment as it can never become active again. See `destroySegment`.
   */
  void adaptSegment(const Segment segment,
                    const SDR &inputs,
                    const Permanence increment,
                    const Permanence decrement,
		    const bool pruneZeroSynapses = false,
		    const UInt segmentThreshold = 0);

  /**
   * Ensures a minimum number of connected synapses.  This raises permance
   * values until the desired number of synapses have permanences above the
   * connectedThreshold.  This is applied to a single segment.
   *
   * @param segment  Index of segment in connections. Is returned by method getSegment.
   * @param segmentThreshold  Desired number of connected synapses.
   */
  void raisePermanencesToThreshold(const Segment    segment,
                                   const UInt       segmentThreshold);


  /**
   *  iteration: ever increasing step count. 
   *  Increases each main call to "compute". Since connections has more
   *  methods that are called instead of compute (adaptSegment, computeActivity,..)
   *  this counter is increased in @ref `computeActivity` as it is called by both
   *  SP & TM. 
   */
//!  const UInt32& iteration = iteration_; //FIXME cannot construct iteration like this?
  UInt32 iteration() const noexcept { return iteration_; }


  /**
   * Ensures that the number of connected synapses is sane.  This method
   * controls the sparsity of the synaptic connections, which is important for
   * the segment to detect things.  If there are too few connections then the
   * segment will not detect anything, and if there are too many connections
   * then the segment will detect everything.
   *
   * See file: docs/synapse_competition.docx
   *
   * This method connects and disconnects synapses by uniformly changing the
   * permanences of all synapses on the segment.
   *
   * @param segment  Index of segment in connections. Is returned by method getSegment.
   * @param minimumSynapses Minimum number of connected synapses allowed on this segment (inclusive).
   * @param maximumSynapses Maximum number of connected synapses allowed on this segment (inclusive).
   */
  void synapseCompetition(  const Segment    segment,
                            const SynapseIdx minimumSynapses,
                            const SynapseIdx maximumSynapses); 


  /**
   * Modify all permanence on the given segment, uniformly.
   *
   * @param segment  Index of segment on cell. Is returned by method getSegment.
   * @param delta  Change in permanence value
   */
  void bumpSegment(const Segment segment, const Permanence delta);

  /**
   * Destroy the synapses with the lowest permanence values.  This method is
   * useful for making room for more synapses on a segment which is already
   * full.
   *
   * @param segment - Index of segment in Connections, to be modified.
   * @param nDestroy - Must be greater than or equal to zero!
   * @param excludeCells - Presynaptic cells which will NOT have any synapses destroyed.
   */
  void destroyMinPermanenceSynapses(const Segment segment, 
		                    const size_t nDestroy,
                                    const SDR_sparse_t &excludeCells = {});

  /**
   * Print diagnostic info
   */
  friend std::ostream& operator<< (std::ostream& stream, const Connections& self);


  // Serialization
  CerealAdapter;
  template<class Archive>
  void save_ar(Archive & ar) const {
    ar(CEREAL_NVP(connectedThreshold_));
    ar(CEREAL_NVP(iteration_));
    ar(CEREAL_NVP(cells_));
    ar(CEREAL_NVP(segments_));
    ar(CEREAL_NVP(synapses_));

    ar(CEREAL_NVP(destroyedSynapses_));
    ar(CEREAL_NVP(destroyedSegments_));

    ar(CEREAL_NVP(potentialSynapsesForPresynapticCell_));
    ar(CEREAL_NVP(connectedSynapsesForPresynapticCell_));
    ar(CEREAL_NVP(potentialSegmentsForPresynapticCell_));
    ar(CEREAL_NVP(connectedSegmentsForPresynapticCell_));

    ar(CEREAL_NVP(timeseries_));
    ar(CEREAL_NVP(previousUpdates_));
    ar(CEREAL_NVP(currentUpdates_));

    ar(CEREAL_NVP(prunedSyns_));
    ar(CEREAL_NVP(prunedSegs_));
  }

  template<class Archive>
  void load_ar(Archive & ar) {
    ar(CEREAL_NVP(connectedThreshold_));
    ar(CEREAL_NVP(iteration_));
    //!initialize(numCells, connectedThreshold_); //initialize Connections //Note: we actually don't call Connections
    //initialize() as all the members are de/serialized. 
    ar(CEREAL_NVP(cells_));
    ar(CEREAL_NVP(segments_));
    ar(CEREAL_NVP(synapses_));

    ar(CEREAL_NVP(destroyedSynapses_));
    ar(CEREAL_NVP(destroyedSegments_));

    ar(CEREAL_NVP(potentialSynapsesForPresynapticCell_));
    ar(CEREAL_NVP(connectedSynapsesForPresynapticCell_));
    ar(CEREAL_NVP(potentialSegmentsForPresynapticCell_));
    ar(CEREAL_NVP(connectedSegmentsForPresynapticCell_));

    ar(CEREAL_NVP(timeseries_));
    ar(CEREAL_NVP(previousUpdates_));
    ar(CEREAL_NVP(currentUpdates_));

    ar(CEREAL_NVP(prunedSyns_));
    ar(CEREAL_NVP(prunedSegs_));
  }

  /**
   * Gets the number of cells.
   *
   * @retval Number of cells.
   */
  size_t numCells() const noexcept { return cells_.size(); }

  constexpr Permanence getConnectedThreshold() const noexcept { return connectedThreshold_; }

  /**
   * Gets the number of segments.
   *
   * @retval Number of segments.
   */
  size_t numSegments() const { 
	  return segments_.size() - destroyedSegments_.size(); }

  /**
   * Gets the number of segments on a cell.
   *
   * @retval Number of segments.
   */
  size_t numSegments(const CellIdx cell) const { 
	  return cells_[cell].segments.size(); 
  }

  /**
   * Gets the number of synapses.
   *
   * @retval Number of synapses.
   */
  size_t numSynapses() const {
    NTA_ASSERT(synapses_.size() >= destroyedSynapses_.size());
    return synapses_.size() - destroyedSynapses_.size();
  }

  /**
   * Gets the number of synapses on a segment.
   *
   * @retval Number of synapses.
   */
  size_t numSynapses(const Segment segment) const { 
	  return segments_[segment].synapses.size(); 
  }

  /**
   * Comparison operator.
   */
  bool operator==(const Connections &other) const;
  inline bool operator!=(const Connections &other) const { return !operator==(other); }

  /**
   * Add a connections events handler.
   *
   * The Connections instance takes ownership of the eventHandlers
   * object. Don't delete it. When calling from Python, call
   * eventHandlers.__disown__() to avoid garbage-collecting the object
   * while this instance is still using it. It will be deleted on
   * `unsubscribe`.
   *
   * @param handler
   * An object implementing the ConnectionsEventHandler interface
   *
   * @retval Unsubscribe token
   */
  UInt32 subscribe(ConnectionsEventHandler *handler);

  /**
   * Remove an event handler.
   *
   * @param token
   * The return value of `subscribe`.
   */
  void unsubscribe(UInt32 token);

protected:
  /**
   * Check whether this synapse still exists "in Connections" ( on its segment).
   * After calling `synapseCreate()` this should be True, after `synapseDestroy()` 
   * its False.
   *
   * Note: 
   *
   * @param Synapse
   * @param fast - bool, default false. If false, run the slow, proper check that is always correct. 
   *   If true, we use a "hack" for speed, where destroySynapse sets synapseData.permanence=-1,
   *   so we can check and compare alter, if ==-1 then synapse is "removed". 
   *   The problem is that synapseData are never truly removed. 
   *   #TODO instead of vector<SynapseData> synapses_, try map<Synapse, SynapseData>, that way, we can properly remove (and check).
   *
   * @retval True if synapse is valid (not removed, it's still in its segment's synapse list)
   */
  bool synapseExists_(const Synapse synapse, bool fast = false) const;

  /**
   * Remove a synapse from presynaptic maps.
   *
   * @param Synapse Index of synapse in presynaptic vector.
   *
   * @param vector<Synapse> ynapsesForPresynapticCell must a vector from be
   * either potentialSynapsesForPresynapticCell_ or
   * connectedSynapsesForPresynapticCell_, depending on whether the synapse is
   * connected or not.
   *
   * @param vector<Synapse> segmentsForPresynapticCell must be a vector from
   * either potentialSegmentsForPresynapticCell_ or
   * connectedSegmentsForPresynapticCell_, depending on whether the synapse is
   * connected or not.
   */
  void removeSynapseFromPresynapticMap_(const Synapse index,
                              std::vector<Synapse> &synapsesForPresynapticCell,
                              std::vector<Segment> &segmentsForPresynapticCell);

  /** 
   *  Remove least useful Segment from cell. 
   */
  void pruneSegment_(const CellIdx& cell);

private:
  std::vector<CellData>    cells_;
  std::vector<SegmentData> segments_;
  std::vector<Segment>     destroyedSegments_;
  std::vector<SynapseData> synapses_;
  std::vector<Synapse>     destroyedSynapses_;
  Permanence               connectedThreshold_; //TODO make const
  UInt32 iteration_ = 0;

  // Extra bookkeeping for faster computing of segment activity.
 
  struct identity { constexpr size_t operator()( const CellIdx t ) const noexcept { return t; };   };	//TODO in c++20 use std::identity 

  std::unordered_map<CellIdx, std::vector<Synapse>, identity> potentialSynapsesForPresynapticCell_;
  std::unordered_map<CellIdx, std::vector<Synapse>, identity> connectedSynapsesForPresynapticCell_;
  std::unordered_map<CellIdx, std::vector<Segment>, identity> potentialSegmentsForPresynapticCell_;
  std::unordered_map<CellIdx, std::vector<Segment>, identity> connectedSegmentsForPresynapticCell_;

  // These three members should be used when working with highly correlated
  // data. The vectors store the permanence changes made by adaptSegment.
  bool timeseries_;
  std::vector<Permanence> previousUpdates_;
  std::vector<Permanence> currentUpdates_;

  //for prune statistics
  Synapse prunedSyns_ = 0; //how many synapses have been removed?
  Segment prunedSegs_ = 0;

  //for listeners //TODO listeners are not serialized, nor included in equals ==
  UInt32 nextEventToken_;
  std::map<UInt32, ConnectionsEventHandler *> eventHandlers_;
}; // end class Connections

} // end namespace htm

#endif // NTA_CONNECTIONS_HPP
