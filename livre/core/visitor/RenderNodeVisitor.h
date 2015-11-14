/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _RenderNodeVisitor_h_
#define _RenderNodeVisitor_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/visitor/NodeVisitor.h>

namespace livre
{

/**
 * The RenderNodeVisitor class is class for invoking traversing events on LODNodes.
 */
class RenderNodeVisitor : public NodeVisitor
{
public:
    LIVRECORE_API RenderNodeVisitor( const VolumeDataSource& volumeDataSource );
    LIVRECORE_API ~RenderNodeVisitor();

    /**

     * @see NodeVisitor::visit
     */
    virtual void visit( const LODNode& node,
                        VisitState& state ) = 0;

    /**
     * @return Returns the data source
     */
    LIVRECORE_API const VolumeDataSource& getDataSource() const;

private:

    LIVRECORE_API void visit( const NodeId& nodeId,
                              VisitState& state ) final;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _RenderNodeVisitor_h_
