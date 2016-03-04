/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#include <dash/types.h>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>

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
class AsyncData;
class Cache;
class CacheObject;
class CacheStatistics;
class Configuration;
class DashConnection;
class DashProcessor;
class DashProcessorInput;
class DashProcessorOutput;
class DashRenderNode;
class DashRenderStatus;
class DashTree;
class DataSourceFactory;
class EventHandler;
class EventHandlerFactory;
class EventInfo;
class EventMapper;
class Executor;
class Executable;
class Filter;
class Future;
class PromiseMap;
class PortData;
class Promise;
class Frustum;
class GLContext;
class GLSLShaders;
class GLWidget;
class InFutureMap;
class InputPort;
class LODNode;
class LODEvaluator;
class MemoryUnit;
class NodeId;
class OutputPort;
class OutFutures;
class Parameter;
class Pipeline;
class PipeFilter;
class PortData;
class PortInfo;
class Processor;
class ProcessorInput;
class ProcessorOutput;
class Promise;
class RenderBrick;
class Renderer;
class RootNode;
class TexturePool;
class View;
class VisitState;
class DataSource;
class DataSourcePlugin;
class DataSourcePluginData;
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

typedef std::shared_ptr< AllocMemoryUnit > AllocMemoryUnitPtr;
typedef std::shared_ptr< RenderBrick > RenderBrickPtr;
typedef std::shared_ptr< DashConnection > DashConnectionPtr;
typedef std::shared_ptr< Processor > ProcessorPtr;
typedef std::shared_ptr< DashProcessor > DashProcessorPtr;
typedef std::shared_ptr< ProcessorInput > ProcessorInputPtr;
typedef std::shared_ptr< ProcessorOutput > ProcessorOutputPtr;
typedef std::shared_ptr< Renderer > RendererPtr;
typedef std::shared_ptr< View > ViewPtr;
typedef std::shared_ptr< GLContext > GLContextPtr;
typedef std::shared_ptr< const GLContext > ConstGLContextPtr;
typedef std::shared_ptr< GLWidget > GLWidgetPtr;
typedef std::shared_ptr< TextureState > TextureStatePtr;
typedef std::shared_ptr< const TextureState > ConstTextureStatePtr;
typedef std::shared_ptr< DataSource > DataSourcePtr;
typedef std::shared_ptr< const DataSource > ConstDataSourcePtr;
typedef std::shared_ptr< EventHandler > EventHandlerPtr;
typedef std::shared_ptr< EventHandlerFactory > EventHandlerFactoryPtr;
typedef std::shared_ptr< MemoryUnit > MemoryUnitPtr;
typedef std::shared_ptr< const MemoryUnit > ConstMemoryUnitPtr;
typedef std::shared_ptr< PortData > PortDataPtr;
typedef std::shared_ptr< Filter > FilterPtr;
typedef std::shared_ptr< const Filter > ConstFilterPtr;
typedef std::shared_ptr< PortData > PortDataPtr;
typedef std::shared_ptr< const PortData > ConstPortDataPtr;
typedef std::shared_ptr< Pipeline > PipelinePtr;
typedef std::shared_ptr< const Pipeline > ConstPipelinePtr;
typedef std::shared_ptr< PipeFilter > PipeFilterPtr;
typedef std::shared_ptr< const PipeFilter > ConstPipeFilterPtr;
typedef std::shared_ptr< Workers > WorkersPtr;
typedef std::shared_ptr< Executor > ExecutorPtr;
typedef std::shared_ptr< InputPort > InputPortPtr;
typedef std::shared_ptr< OutputPort > OutputPortPtr;
typedef std::shared_ptr< const InputPort > ConstInputPortPtr;
typedef std::shared_ptr< const OutputPort > ConstOutputPortPtr;
typedef std::shared_ptr< LODEvaluator > LODEvaluatorPtr;
typedef std::shared_ptr< const LODEvaluator > ConstLODEvaluatorPtr;
typedef std::shared_ptr< Cache > CachePtr;
typedef std::shared_ptr< const Cache > ConstCachePtr;
typedef std::shared_ptr< Executable > ExecutablePtr;
typedef std::shared_ptr< const Executable > ConstExecutablePtr;
typedef std::shared_ptr< CacheObject > CacheObjectPtr;
typedef std::shared_ptr< const CacheObject > ConstCacheObjectPtr;
typedef std::shared_ptr< Future > FuturePtr;
typedef std::shared_ptr< const Future > ConstFuturePtr;
typedef std::shared_ptr< Promise > PromisePtr;
typedef std::shared_ptr< const Promise > ConstPromisePtr;
typedef std::shared_ptr< CacheObject > CacheObjectPtr;
typedef std::shared_ptr< const CacheObject > ConstCacheObjectPtr;

/**
  * Pair definitions
  */
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

typedef std::vector< NodeId > NodeIds;
typedef std::vector< CacheId > CacheIds;

/**
 * Vector definitions for complex types
 */
typedef std::vector< CacheObjectPtr > CacheObjects;
typedef std::vector< ConstCacheObjectPtr > ConstCacheObjects;
typedef std::vector< RenderBrickPtr > RenderBricks;
typedef std::vector< FilterPtr > Filters;
typedef std::vector< ConstFilterPtr > ConstFilters;
typedef std::vector< ConstPortDataPtr > ConstPortDataPtrs;
typedef std::vector< WorkInput > WorkInputs;
typedef std::vector< PipeFilterPtr > PipeFilters;
typedef std::vector< OutputPortPtr > OutputPorts;
typedef std::vector< InputPortPtr > InputPorts;
typedef std::vector< ConstOutputPortPtr > ConstOutputPorts;
typedef std::vector< ConstInputPortPtr > ConstInputPorts;
typedef std::vector< NameTypePair > NameTypePairs;
typedef std::vector< PortInfo > PortInfos;
typedef std::vector< PromisePtr > Promises;

typedef std::list< Future > Futures;
typedef std::list< ExecutablePtr > Executables;

template <class T>
using ResultsT = std::vector< T >;

/**
 * Map definitions
 */
typedef boost::unordered_map< NodeId, dash::NodePtr > NodeIDDashNodePtrMap;
typedef boost::unordered_map< CacheId, CacheObjectPtr > CacheMap;
typedef boost::unordered_map< CacheId, ConstCacheObjectPtr > ConstCacheMap;
typedef boost::unordered_map< uint32_t, bool > BoolMap;
typedef boost::unordered_map< uint32_t, EventHandlerPtr > EventHandlerPtrMap;
typedef boost::unordered_map< uint32_t, DashConnectionPtr > DashConnectionPtrMap;

/**
 * Set definitions
 */
typedef std::set< CacheId > CacheIdSet;
typedef std::set< NodeId > NodeIdSet;

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

const uint32_t LATEST_FRAME = INT_MAX; //!< Maximum frame number


// Program Options
typedef boost::program_options::variables_map ProgramOptionsMap;
typedef boost::program_options::options_description ProgramOptionsDescription;
typedef std::map< std::string,
                  ProgramOptionsDescription > ProgramOptionsDescriptionMap;

// functions

typedef boost::function< void( const InFutureMap&, PromiseMap& )> FilterFunc;

static const std::string HIDDEN_PROGRAMDESCRIPTION_STR("_HIDDEN_");
static const std::string ALL_PORTS = "";
static const std::string NO_PREFIX = "";
}

#endif // _coreTypes_h_
