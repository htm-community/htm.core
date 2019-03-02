# Breaking changes to the nupic API

We try to keep API as much as compatible with original [numenta/nupic.core repo](https://github.com/numenta/nupic.core). 
The API is specified in the [API Docs](http://nupic.docs.numenta.org/prerelease/api/index.html) 

## Motivation

We try to remain compatible where possible, to make it easy for the users and programmers to switch/use 
our implementation. And for developers to be easily able to navigate within the (known) codebase. 
Despite of this, sometimes changes need to happen - be it for optimization, removal/replacement of some 
features or implemenation detail, etc. 


## API breaking changes in this repo

* CapnProto serialization is replaced with binary streams in PR #62.  
Calls to `read()` and `write()` are no longer available. Use `save()` and `load()`. 
`Network(path)` is no longer used to deserialize a path. Use `Network net; net.load(stream);` to deserialize.  
Helpers `SaveToFile(path)` and `LoadFromFile(path)` are used to stream to and from a file using save() 
and load().

* The function `Network::newRegionFromBundle() was replaced with `newRegion(stream, name)` where the stream 
is an input stream reading a file created by region->save(steam)  or region->saveToFile(path).  PR#62

* Removed methods `SpatialPooler::setSynPermTrimThreshold` and  `SpatialPooler::getSynPermTrimThreshold`.
Synapse trimming was an optimization which is no longer possible because of an implementation change. PR #153

* Removed method `SpatialPooler::setSynPermMax` as the maximum permanence is now defined (hardcoded) as
`nupic::algorithms::connections::maxPermancence = 1.0f;` PR #153

* Changed callback `ConnectionsEventHandler::onUpdateSynapsePermanence()`.  Instead of being called
every time a synapses permanence changes, it is now called when a synapse changes connected state,
IE: it is called when a synapses permanence crosses the connected threshold. PR #153

* Removed (private) method `SpatialPooler::updatePermanencesForColumn_(vector<Real> &perm, UInt column,
bool raisePerm = true)`  due to SP implementation now using Connections. PR #153

* SpatialPooler now always applies boosting, even when `learn=false`. PR #206

* Removed methods `SpatialPooler::setSynPermConnected` and `TemporalMemory::setConnectedPermanence`. The connected synapse permanence threshold should instead be given to the constructor or the initialize method. PR #221

* When building with MS Visual Studio 2017, it will build bindings only for Python 3.4 and above.  
(i.e. No Python 2.7 under Windows)

* Setting dimensions on a region is now optional.  If given, it overrides any region parameters that set 
the width of the default output buffer.

* The splitter maps (and the LinkPolicy that creates them) were removed.  These were used as a way to 
re-arrange bits in the input buffer based on user defined patterns. However, thinking about how this 
works in biology, the neurons make connections to the synapse of other neurons. There is no order or 
pattern involved and in fact it needs to be fairly random as to how the neurons connect. It is the 
job of the Spatial Pooler to simulate that set of random connections. So in effect, any re-mapping 
of bits by a splitter map prior to being presented to the Spatial Pooler should have no affect on 
functionality. This is probably why this feature was not used anyplace except in the unit tests.
As a side-effect of this change, the LinkType and LinkParam parameters in the Network.Link( ) call 
are ignored.  PR #271

* Removed all matrix libraries.  Use the `Connections` class instead.  PR #169

* Removed `void SpatialPooler::stripUnlearnedColumns()` as unused and not useful (did not effectively remove any columns). PR #286 
