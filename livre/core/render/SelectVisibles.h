/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#ifndef _SelectVisibles_h_
#define _SelectVisibles_h_

#include <livre/core/render/Frustum.h>
#include <livre/core/types.h>
#include <livre/core/visitor/DataSourceVisitor.h>

namespace livre
{
/** Selects all visible rendering nodes */
class SelectVisibles : public DataSourceVisitor
{
public:
    /**
     * Construcutor
     * @param dataSource data source
     * @param frustum frustum
     * @param windowHeight height of window in pixels
     * @param screenSpaceError number of voxels per pixel
     * @param minLOD minimum level of detail
     * @param maxLOD maximum level of detail
     * @param range range of the data
     * @param ClipPlanes clip planes
     */
    SelectVisibles(const DataSource& dataSource, const Frustum& frustum,
                   const uint32_t windowHeight, const float screenSpaceError,
                   const uint32_t minLOD, const uint32_t maxLOD,
                   const Range& range, const ClipPlanes& clipPlanes);

    ~SelectVisibles();

    /**
     * @return the list of visibles
     */
    const NodeIds& getVisibles() const;

protected:
    void visitPre() final;
    void visit(const LODNode& lodNode, VisitState& state) final;
    void visitPost() final;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}
#endif //_SelectVisibles_h_
