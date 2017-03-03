/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          bbp-open-source@googlegroups.com
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
#include <vmmlib/aabb.hpp>
#include <vmmlib/matrix.hpp>
#include <vmmlib/vector.hpp>

#include <co/distributable.h>
#include <lexis/render/ClipPlanes.h>
#include <lexis/render/Histogram.h>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdint.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include <functional>
#include <typeindex>

namespace livre
{
class AllocMemoryUnit;
class Cache;
class CacheObject;
class CacheStatistics;
using ClipPlanes = co::Distributable<::lexis::render::ClipPlanes>;
class Configuration;
class Frustum;
class GLContext;
class GLSLShaders;
using Histogram = co::Distributable<::lexis::render::Histogram>;
class LODNode;
class MemoryUnit;
class NodeId;
class NodeVisitor;
class Parameter;
class Renderer;
class RootNode;
class TexturePool;
class VisitState;
class DataSource;
class DataSourcePlugin;
class DataSourcePluginData;

/** Pipeline */
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
struct NodeAvailability;
struct TextureState;
struct VolumeInformation;

using servus::uint128_t;
using lunchbox::Strings;

typedef uint64_t Identifier;
typedef Identifier CacheId;
typedef std::array<float, 2> Range;

/** SmartPtr definitions */
typedef std::shared_ptr<AllocMemoryUnit> AllocMemoryUnitPtr;
typedef std::shared_ptr<GLContext> GLContextPtr;
typedef std::shared_ptr<const GLContext> ConstGLContextPtr;
typedef std::shared_ptr<TextureState> TextureStatePtr;
typedef std::shared_ptr<const TextureState> ConstTextureStatePtr;
typedef std::shared_ptr<DataSource> DataSourcePtr;
typedef std::shared_ptr<const DataSource> ConstDataSourcePtr;
typedef std::shared_ptr<MemoryUnit> MemoryUnitPtr;
typedef std::shared_ptr<const MemoryUnit> ConstMemoryUnitPtr;
typedef std::shared_ptr<CacheObject> CacheObjectPtr;
typedef std::shared_ptr<const CacheObject> ConstCacheObjectPtr;
typedef std::shared_ptr<CacheObject> CacheObjectPtr;
typedef std::shared_ptr<const CacheObject> ConstCacheObjectPtr;
typedef std::shared_ptr<PortData> PortDataPtr;
typedef std::shared_ptr<Executable> ExecutablePtr;

typedef std::unique_ptr<Filter> FilterPtr;

/** Helper classes for shared_ptr objects */
template <typename T>
struct DeleteArray
{
    void operator()(const T* t) const { delete[] t; }
};

template <typename T>
struct DeleteObject
{
    void operator()(const T* t) const { delete t; }
};

template <typename T>
struct DontDeleteObject
{
    void operator()(const T*) const {}
};

/** Vector definitions basic types */
typedef std::vector<float> Floats;
typedef std::vector<uint8_t> UInt8s;
typedef std::vector<uint32_t> UInt32s;

typedef std::vector<NodeId> NodeIds;
typedef std::vector<CacheId> CacheIds;

/** Vector definitions for complex types */
typedef std::vector<CacheObjectPtr> CacheObjects;
typedef std::vector<ConstCacheObjectPtr> ConstCacheObjects;

/** List definitions for complex types */
typedef std::list<Executable*> Executables;
typedef std::list<Future> Futures;
typedef std::list<Promise> Promises;

/** Map definitions */
typedef std::unordered_map<CacheId, CacheObjectPtr> CacheMap;
typedef std::unordered_map<CacheId, ConstCacheObjectPtr> ConstCacheMap;
typedef std::unordered_map<uint32_t, bool> BoolMap;

template <class T>
inline std::type_index getType()
{
    typedef typename std::remove_const<T>::type UnconstT;
    return std::type_index(typeid(UnconstT));
}

typedef std::map<std::string, std::type_index> DataInfos;
typedef DataInfos::value_type DataInfo;

/** Set definitions */
typedef std::set<CacheId> CacheIdSet;
typedef std::set<NodeId> NodeIdSet;

/** Locking object definitions */
typedef boost::shared_mutex ReadWriteMutex;
typedef boost::shared_lock<ReadWriteMutex> ReadLock;
typedef boost::unique_lock<ReadWriteMutex> WriteLock;
typedef boost::unique_lock<boost::mutex> ScopedLock;

// Enums

enum AccessMode
{
    MODE_READ = 0u,
    MODE_WRITE = 1u
};

// Constants
const uint32_t INVALID_TEXTURE_ID = -1; //!< Invalid OpenGL texture id.
const Identifier INVALID_CACHE_ID = -1; //!< Invalid cache id.
const Identifier INVALID_NODE_ID = -1;  //!< Invalid node ID.

const uint32_t MAX_CHILDREN_BITS = 4;     //!< Maximum number of children is 16
const uint32_t NODEID_LEVEL_BITS = 4;     //>! see NodeId
const uint32_t NODEID_BLOCK_BITS = 14;    //>! see NodeId
const uint32_t NODEID_TIMESTEP_BITS = 18; //>! see NodeId

const uint32_t INVALID_POSITION =
    (1u << NODEID_BLOCK_BITS) - 1; //!< Invalid node ID.
const uint32_t INVALID_LEVEL =
    (1u << NODEID_LEVEL_BITS) - 1; //!< Invalid tree level.4 bits is on
const uint32_t INVALID_TIMESTEP =
    (1u << NODEID_TIMESTEP_BITS) - 1; //!< Invalid time step. 18 bits is on
const uint32_t INVALID_FRAMEID = -1;
const uint32_t LATEST_FRAME = INT_MAX; //!< Maximum frame number

// Program Options
typedef boost::program_options::variables_map ProgramOptionsMap;
typedef boost::program_options::options_description ProgramOptionsDescription;
typedef std::map<std::string, ProgramOptionsDescription>
    ProgramOptionsDescriptionMap;

// Const definitions
static const std::string NO_PREFIX = "";

/** AABB definitions */
typedef vmml::AABBf Boxf;           //!< float AABB box.
typedef vmml::AABB<int32_t> Boxi;   //!< integer AABB box.
typedef vmml::AABB<uint32_t> Boxui; //!< integer AABB box.

/** Matrix definitions */
using vmml::Matrix3f; //!< A 3x3 float matrix.
using vmml::Matrix4f; //!< A 4x4 float matrix.

using vmml::Matrix3d; //!< A 3x3 double matrix.
using vmml::Matrix4d; //!< A 4x4 double matrix.

/** Vector definitions */
using vmml::Vector2i; //!< A two-component integer vector.
using vmml::Vector3i; //!< A three-component integer vector.
using vmml::Vector4i; //!< A four-component integer vector.

using vmml::Vector2ui; //!< A two-component integer vector.
using vmml::Vector3ui; //!< A three-component integer vector.
using vmml::Vector4ui; //!< A four-component integer vector.

using vmml::Vector2f;       //!< A two-component float vector.
using vmml::Vector3f;       //!< A three-component float vector.
using vmml::Vector3d;       //!< A three-component double vector.
using vmml::Vector4d;       //!< A four-component double vector.
using vmml::Vector4f;       //!< A four-component float vector.
using vmml::Vector4ub;      //!< A four-component byte vector
using vmml::Frustumf;       //!< Float frustum.
using vmml::FrustumCullerf; //!< Float frustum culler.
typedef Vector4f Plane;

/** Quaternion definitions */
using vmml::Quaternionf; //!< Float quaternion.

/** Viewport definitions */
typedef vmml::Vector4i PixelViewport;
typedef vmml::Vector4f Viewport;

/** Definitions */
const Vector2ui INVALID_FRAME_RANGE(INVALID_TIMESTEP);
const Vector2ui FULL_FRAME_RANGE(0, INVALID_TIMESTEP);

/** Vector definitions */
typedef std::vector<Vector3f> Vector3fs;
}

#endif // _coreTypes_h_
