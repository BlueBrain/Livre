/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#ifndef _DataSourceVisitor_h_
#define _DataSourceVisitor_h_

#include <livre/data/DataSource.h>
#include <livre/data/NodeVisitor.h>
#include <livre/data/api.h>
#include <livre/data/types.h>

namespace livre
{
/**
 * The DataSourceVisitor is for data source traversal.
 */
class DataSourceVisitor : public NodeVisitor
{
public:
    LIVREDATA_API DataSourceVisitor(const DataSource& dataSource);
    LIVREDATA_API ~DataSourceVisitor();

    /**
     * @copydoc NodeVisitor::visit
     */
    virtual bool visit(const LODNode& node) = 0;

    /**
     * @return Returns the data source
     */
    LIVREDATA_API const DataSource& getDataSource() const;

private:
    LIVREDATA_API bool visit(const NodeId& nodeId) final;

    class Impl;
    std::unique_ptr<Impl> _impl;
};
}

#endif // _DataSourceVisitor_h_
