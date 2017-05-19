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

#pragma once

#include <array>
#include <memory>
#include <stdint.h>
#include <vector>

#include <vmmlib/aabb.hpp>
#include <vmmlib/matrix.hpp>
#include <vmmlib/vector.hpp>

#include <lexis/render/ClipPlanes.h>

namespace livre
{
class AllocMemoryUnit;
class LODNode;
class MemoryUnit;
class NodeId;
class NodeVisitor;
class RootNode;
class VisitState;
class DataSource;
class DataSourcePlugin;
class DataSourcePluginData;

struct VolumeInformation;

typedef uint64_t Identifier;
typedef std::array<float, 2> Range;

using vmml::Vector2ui;
using vmml::Vector3ui;
using vmml::Vector3f;
using vmml::Vector4f;
using vmml::Matrix4f;
using vmml::Frustumf;
using vmml::FrustumCullerf;
typedef vmml::AABBf Boxf;
typedef vmml::AABB<int32_t> Boxi;
typedef vmml::AABB<uint32_t> Boxui;
typedef Vector4f Plane;

using ::lexis::render::ClipPlanes;

typedef std::vector<NodeId> NodeIds;

enum AccessMode
{
    MODE_READ = 0u,
    MODE_WRITE = 1u
};

/** SmartPtr definitions */
typedef std::shared_ptr<AllocMemoryUnit> AllocMemoryUnitPtr;
typedef std::shared_ptr<MemoryUnit> MemoryUnitPtr;
typedef std::shared_ptr<const MemoryUnit> ConstMemoryUnitPtr;

// Constants
const Identifier INVALID_NODE_ID = -1; //!< Invalid node ID.

const uint32_t MAX_CHILDREN_BITS = 4;     //!< Maximum number of children is 16
const uint32_t NODEID_LEVEL_BITS = 4;     //>! see NodeId
const uint32_t NODEID_BLOCK_BITS = 14;    //>! see NodeId
const uint32_t NODEID_TIMESTEP_BITS = 18; //>! see NodeId

const uint32_t INVALID_LEVEL =
    (1u << NODEID_LEVEL_BITS) - 1; //!< Invalid tree level.4 bits is on
const uint32_t INVALID_TIMESTEP =
    (1u << NODEID_TIMESTEP_BITS) - 1; //!< Invalid time step. 18 bits is on

const Vector2ui INVALID_FRAME_RANGE(INVALID_TIMESTEP);
const Vector2ui FULL_FRAME_RANGE(0, INVALID_TIMESTEP);
}
