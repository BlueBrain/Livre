/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *
 * This file is part of Livre <https://github.com/BlueBrain/Livre>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _coreTypes_h_
#define _coreTypes_h_

#include <lunchbox/debug.h>
#include <lunchbox/log.h>
#include <lunchbox/uri.h>
#include <servus/uint128_t.h>

#include <boost/foreach.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>

#include <stdint.h>
#include <set>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>
#include <utility>
#include <typeindex>
#include <memory>

namespace livre
{

class AllocMemoryUnit;
class Cache;
class CacheObject;
class CacheObjectObserver;
class CachePolicy;
class CacheStatistics;
class Configuration;
class DataSourceFactory;
class EventHandler;
class EventHandlerFactory;
class EventInfo;
class EventMapper;
class Executor;
class Executable;
class Filter;
class FilterInput;
class FilterOutput;
class PortData;
class PortDataFuture;
class PortDataPromise;
class Frustum;
class GLContext;
class GLSLShaders;
class GLWidget;
class InputPort;
class OutputPort;
class LODNode;
class LODEvaluator;
class MemoryUnit;
class NodeId;
class Parameter;
class Pipeline;
class PipeFilter;
class Plane;
class PortData;
class PortInfo;
class Processor;
class ProcessorInput;
class ProcessorOutput;
class RenderBrick;
class Renderer;
class RootNode;
class TexturePool;
class TexturePoolFactory;
class View;
class VisitState;
class VolumeDataSource;
class VolumeDataSourcePlugin;
class VolumeDataSourcePluginData;
class WorkInput;
class Workers;
class WorkQueue;

struct FrameInfo;
struct TextureState;
struct VolumeInformation;

using servus::uint128_t;
using lunchbox::Strings;

typedef uint64_t Identifier;
typedef Identifier CacheId;
typedef std::array< float, 2 > Range;

/**
 * SmartPtr definitions
 */
typedef boost::scoped_ptr< GLSLShaders > GLSLShadersPtr;
typedef boost::scoped_ptr< CacheStatistics > CacheStatisticsPtr;

typedef boost::shared_ptr< AllocMemoryUnit > AllocMemoryUnitPtr;
typedef boost::shared_ptr< RenderBrick > RenderBrickPtr;
typedef boost::shared_ptr< LODNode > LODNodePtr;
typedef boost::shared_ptr< const LODNode > ConstLODNodePtr;
typedef boost::shared_ptr< Processor > ProcessorPtr;
typedef boost::shared_ptr< ProcessorInput > ProcessorInputPtr;
typedef boost::shared_ptr< ProcessorOutput > ProcessorOutputPtr;
typedef boost::shared_ptr< Renderer > RendererPtr;
typedef boost::shared_ptr< const View > ConstViewPtr;
typedef boost::shared_ptr< View > ViewPtr;
typedef boost::shared_ptr< GLContext > GLContextPtr;
typedef boost::shared_ptr< const GLContext > ConstGLContextPtr;
typedef boost::shared_ptr< GLWidget > GLWidgetPtr;
typedef boost::shared_ptr< VolumeInformation > VolumeInformationPtr;
typedef boost::shared_ptr< const VolumeInformation > ConstVolumeInformationPtr;
typedef boost::shared_ptr< TextureState > TextureStatePtr;
typedef boost::shared_ptr< const TextureState > ConstTextureStatePtr;
typedef boost::shared_ptr< VolumeDataSource > VolumeDataSourcePtr;
typedef boost::shared_ptr< const VolumeDataSource > ConstVolumeDataSourcePtr;
typedef boost::shared_ptr< TexturePool > TexturePoolPtr;
typedef boost::shared_ptr< EventHandler > EventHandlerPtr;
typedef boost::shared_ptr< EventHandlerFactory > EventHandlerFactoryPtr;
typedef boost::shared_ptr< EventMapper > EventMapperPtr;
typedef boost::shared_ptr< DataSourceFactory > DataSourceFactoryPtr;
typedef boost::shared_ptr< MemoryUnit > MemoryUnitPtr;
typedef boost::shared_ptr< const MemoryUnit > ConstMemoryUnitPtr;
typedef boost::shared_ptr< WorkQueue > WorkQueuePtr;
typedef boost::shared_ptr< PortData > PortDataPtr;
typedef boost::shared_ptr< Filter > FilterPtr;
typedef boost::shared_ptr< const Filter > ConstFilterPtr;
typedef boost::shared_ptr< PortData > PortDataPtr;
typedef boost::shared_ptr< const PortData > ConstPortDataPtr;
typedef boost::shared_ptr< Pipeline > PipelinePtr;
typedef boost::shared_ptr< const Pipeline > ConstPipelinePtr;
typedef boost::shared_ptr< PipeFilter > PipeFilterPtr;
typedef boost::shared_ptr< const PipeFilter > ConstPipeFilterPtr;
typedef boost::shared_ptr< Workers > WorkersPtr;
typedef boost::shared_ptr< Executor > ExecutorPtr;
typedef boost::shared_ptr< InputPort > InputPortPtr;
typedef boost::shared_ptr< OutputPort > OutputPortPtr;
typedef boost::shared_ptr< const InputPort > ConstInputPortPtr;
typedef boost::shared_ptr< const OutputPort > ConstOutputPortPtr;
typedef boost::shared_ptr< LODEvaluator > LODEvaluatorPtr;
typedef boost::shared_ptr< const LODEvaluator > ConstLODEvaluatorPtr;
typedef boost::shared_ptr< Cache > CachePtr;
typedef boost::shared_ptr< const Cache > ConstCachePtr;
typedef boost::shared_ptr< Executable > ExecutablePtr;
typedef boost::shared_ptr< const Executable > ConstExecutablePtr;

typedef boost::intrusive_ptr< CacheObject > CacheObjectPtr;
typedef boost::intrusive_ptr< const CacheObject > ConstCacheObjectPtr;

/**
  * Pair definitions
  */
typedef std::pair< FilterPtr, FilterPtr > FilterPair;
typedef std::pair< NodeId, FilterPtr > WorkInputFilterPair;
typedef std::pair< std::string, std::type_index > NameTypePair;

/**
 * Helper classes for shared_ptr objects
 */
template<typename T>
struct DeleteArray
{
  void operator()(const T* t) const { delete[] t; }
};

template<typename T>
struct DeleteObject
{
  void operator()(const T* t) const { delete t; }
};

template<typename T>
struct DontDeleteObject
{
  void operator()(const T*) const { }
};

/**
 * Vector definitions basic types
 */
typedef std::vector< float > FloatVector;
typedef std::vector< double > DoubleVector;

typedef std::vector< int8_t > Int8Vector;
typedef std::vector< uint8_t > UInt8Vector;

typedef std::vector< int16_t > Int16Vector;
typedef std::vector< uint16_t > UInt16Vector;

typedef std::vector< int32_t > Int32Vector;
typedef std::vector< uint32_t > UInt32Vector;

typedef std::vector< int64_t > Int64Vector;
typedef std::vector< uint64_t > UInt64Vector;

typedef std::vector< bool > BoolVector;
typedef std::vector< NodeId > NodeIds;

/**
 * Vector definitions for complex types
 */
typedef std::vector< CacheObjectPtr > CacheObjects;
typedef std::vector< ConstCacheObjectPtr > ConstCacheObjects;
typedef std::vector< RenderBrickPtr > RenderBricks;
typedef std::vector< TexturePoolPtr > TexturePools;
typedef std::vector< FilterPtr > FilterPtrs;
typedef std::vector< ConstFilterPtr > ConstFilterPtrs;
typedef std::vector< FilterPair > ConstFilterPtrPairs;
typedef std::vector< ConstPortDataPtr > ConstPortDataPtrs;
typedef std::vector< WorkInputFilterPair > WorkInputFilterPairs;
typedef std::vector< WorkInput > WorkInputs;
typedef std::vector< PipelinePtr > PipelinePtrs;
typedef std::vector< PipeFilterPtr > PipeFilterPtrs;
typedef std::vector< OutputPortPtr > OutputPortPtrs;
typedef std::vector< InputPortPtr > InputPortPtrs;
typedef std::vector< ConstOutputPortPtr > ConstOutputPortPtrs;
typedef std::vector< ConstInputPortPtr > ConstInputPortPtrs;
typedef std::vector< NameTypePair > NameTypePairs;
typedef std::vector< PortInfo > PortInfos;

typedef std::list< ExecutablePtr > ExecutablePtrs;

/**
 * Map definitions
 */
typedef boost::unordered_map< NodeId, LODNode > NodeIDLODNodeMap;
typedef boost::unordered_map< CacheId, CacheObjectPtr > CacheMap;
typedef boost::unordered_map< CacheId, ConstCacheObjectPtr > ConstCacheMap;
typedef boost::unordered_map< uint32_t, EventHandlerPtr > EventHandlerPtrMap;

/**
 * Set definitions
 */
typedef std::set< CacheId > CacheIdSet;
typedef std::set< NodeId > NodeIdSet;
typedef std::set< CacheObjectObserver *> CacheObjectObserverSet;

/**
 * Locking object definitions
 */
typedef boost::shared_mutex ReadWriteMutex;
typedef boost::shared_lock< ReadWriteMutex > ReadLock;
typedef boost::unique_lock< ReadWriteMutex > WriteLock;
typedef boost::unique_lock< boost::mutex > ScopedLock;

// Enums

enum AccessMode
{
    MODE_READ = 0u,
    MODE_WRITE = 1u
};

// Constants
const uint32_t INVALID_TEXTURE_ID = -1; //!< Invalid OpenGL texture id.
const Identifier INVALID_CACHE_ID = -1; //!< Invalid cache id.
const Identifier INVALID_NODE_ID = -1; //!< Invalid node ID.

const uint32_t MAX_CHILDREN_BITS = 4; //!< Maximum number of children is 16
const uint32_t NODEID_LEVEL_BITS = 4; //>! @see NodeId
const uint32_t NODEID_BLOCK_BITS = 14; //>! @see NodeId
const uint32_t NODEID_FRAME_BITS = 18; //>! @see NodeId

const uint32_t INVALID_POSITION = ( 1u << NODEID_BLOCK_BITS ) - 1; //!< Invalid node ID.
const uint32_t INVALID_LEVEL = ( 1u << NODEID_LEVEL_BITS ) - 1; //!< Invalid tree level.4 bits is on
const uint32_t INVALID_FRAME = ( 1u << NODEID_FRAME_BITS ) - 1; //!< Invalid tree level.4 bits is on

// functions

typedef boost::function< void( PipeFilter& )> FilterFunc;


#define HIDDEN_PROGRAMDESCRIPTION_STR "_HIDDEN_"

}

#endif // _coreTypes_h_
