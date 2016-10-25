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

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <stdint.h>
#include <set>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>
#include <utility>
#include <memory>
#include <unordered_map>
#include <list>

#include <functional>
#include <typeindex>

namespace livre
{

class AllocMemoryUnit;
class ApplicationSettings;
class Cache;
class CacheObject;
class CacheStatistics;
class ClipPlanes;
class Configuration;
class DataSource;
class DataSourcePlugin;
class DataSourcePluginData;
class EventHandler;
class EventHandlerFactory;
class EventInfo;
class EventMapper;
class Frustum;
class GLContext;
class GLSLShaders;
class Histogram;
class LODNode;
class MemoryUnit;
class NodeId;
class NodeVisitor;
class Parameter;
class Renderer;
class RendererPlugin;
class RenderPipeline;
class RenderSettings;
class RootNode;
class TexturePool;
class VisitState;
class RendererParameters;


/**
 * Pipeline
 */
class AsyncData;
class Executor;
class Executable;
class Filter;
class Future;
class FutureMap;
class FutureMap;
class InputPort;
class PortData;
class Promise;
class PromiseMap;
class OutputPort;
class OutFutures;
class UniqueFutureMap;
class Pipeline;
class PipeFilter;
class Workers;

struct FrameInfo;
struct RenderStatistics;
struct RenderInputs;
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
typedef std::shared_ptr< GLContext > GLContextPtr;
typedef std::shared_ptr< const GLContext > ConstGLContextPtr;
typedef std::shared_ptr< TextureState > TextureStatePtr;
typedef std::shared_ptr< const TextureState > ConstTextureStatePtr;
typedef std::shared_ptr< DataSource > DataSourcePtr;
typedef std::shared_ptr< const DataSource > ConstDataSourcePtr;
typedef std::shared_ptr< EventHandler > EventHandlerPtr;
typedef std::shared_ptr< EventHandlerFactory > EventHandlerFactoryPtr;
typedef std::shared_ptr< MemoryUnit > MemoryUnitPtr;
typedef std::shared_ptr< const MemoryUnit > ConstMemoryUnitPtr;
typedef std::shared_ptr< CacheObject > CacheObjectPtr;
typedef std::shared_ptr< const CacheObject > ConstCacheObjectPtr;
typedef std::shared_ptr< CacheObject > CacheObjectPtr;
typedef std::shared_ptr< const CacheObject > ConstCacheObjectPtr;
typedef std::shared_ptr< PortData > PortDataPtr;
typedef std::shared_ptr< Executable > ExecutablePtr;

typedef std::unique_ptr< Filter > FilterPtr;

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
typedef std::vector< float > Floats;
typedef std::vector< double > Doubles;

typedef std::vector< int8_t > Int8s;
typedef std::vector< uint8_t > UInt8s;

typedef std::vector< int16_t > Int16s;
typedef std::vector< uint16_t > UInt16s;

typedef std::vector< int32_t > Int32s;
typedef std::vector< uint32_t > UInt32s;

typedef std::vector< int64_t > Int64s;
typedef std::vector< uint64_t > UInt64s;

typedef std::vector< NodeId > NodeIds;
typedef std::vector< CacheId > CacheIds;

/**
 * Vector definitions for complex types
 */
typedef std::vector< CacheObjectPtr > CacheObjects;
typedef std::vector< ConstCacheObjectPtr > ConstCacheObjects;


/**
 * List definitions for complex types
 */
typedef std::list< Executable* > Executables;
typedef std::list< Future > Futures;
typedef std::list< Promise > Promises;

/**
 * Map definitions
 */
typedef std::unordered_map< CacheId, CacheObjectPtr > CacheMap;
typedef std::unordered_map< CacheId, ConstCacheObjectPtr > ConstCacheMap;
typedef std::unordered_map< uint32_t, EventHandlerPtr > EventHandlerMap;

template < class T >
inline std::type_index getType()
{
    typedef typename std::remove_const<T>::type UnconstT;
    return std::type_index( typeid( UnconstT ));
}

typedef std::map< std::string, std::type_index > DataInfos;
typedef std::map< std::string, PipeFilter > PipeFilterMap;
typedef DataInfos::value_type DataInfo;

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
const uint32_t INVALID_TEXTURE_ID = -1u; //!< Invalid OpenGL texture id.
const Identifier INVALID_CACHE_ID = -1u; //!< Invalid cache id.
const Identifier INVALID_NODE_ID = -1u; //!< Invalid node ID.

const uint32_t MAX_CHILDREN_BITS = 4; //!< Maximum number of children is 16
const uint32_t NODEID_LEVEL_BITS = 4; //>! see NodeId
const uint32_t NODEID_BLOCK_BITS = 14; //>! see NodeId

const uint32_t INVALID_POSITION = ( 1u << NODEID_BLOCK_BITS ) - 1u; //!< Invalid node ID.
const uint32_t INVALID_LEVEL = ( 1u << NODEID_LEVEL_BITS ) - 1u; //!< Invalid tree level.4 bits is on
const uint32_t INVALID_FRAMEID = -1u;
const uint32_t LATEST_FRAME = INT_MAX; //!< Maximum frame number


// Program Options
typedef boost::program_options::variables_map ProgramOptionsMap;
typedef boost::program_options::options_description ProgramOptionsDescription;
typedef std::map< std::string,
                  ProgramOptionsDescription > ProgramOptionsDescriptionMap;

// Const definitions
static const std::string HIDDEN_PROGRAMDESCRIPTION_STR("_HIDDEN_");
static const std::string NO_PREFIX = "";

}

#endif // _coreTypes_h_
