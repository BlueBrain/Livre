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

#ifndef _DataSourceVisitor_h_
#define _DataSourceVisitor_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/visitor/NodeVisitor.h>
#include <livre/core/data/DataSource.h>

namespace livre
{

/**
 * The DataSourceVisitor is for data source traversal.
 */
class DataSourceVisitor : public NodeVisitor
{
public:
    LIVRECORE_API DataSourceVisitor( const DataSource& dataSource );
    LIVRECORE_API ~DataSourceVisitor();

    /**
     * @copydoc NodeVisitor::visit
     */
    virtual void visit( const LODNode& node,
                        VisitState& state ) = 0;

    /**
     * @return Returns the data source
     */
    LIVRECORE_API const DataSource& getDataSource() const;

private:

    LIVRECORE_API void visit( const NodeId& nodeId,
                              VisitState& state ) final;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _DataSourceVisitor_h_
