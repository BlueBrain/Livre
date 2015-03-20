/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#ifndef _DashTree_h_
#define _DashTree_h_

#include <boost/noncopyable.hpp>
#include <livre/core/types.h>
#include <livre/core/dashTypes.h>

namespace livre
{

namespace detail
{
    class DashTree;
}

/**
 * The DashTree class keeps the hierarcy of the dash nodes representing the LOD nodes.
 */
class DashTree : public boost::noncopyable
{
public:

    DashTree( ConstVolumeDataSourcePtr dataSource );
    ~DashTree();

    /**
     * @return Returns the data source
     */
    ConstVolumeDataSourcePtr getDataSource( ) const;

    /**
     * Creates a new context, registers it and maps its data to already registered contexts.
     * @return Returns a new dash context
     */
    DashContextPtr createContext();

    /**
     * @return The render status of the dash tree.
     */
    const DashRenderStatus& getRenderStatus() const;

    /**
     * @return The render status of the dash tree.
     */
    DashRenderStatus& getRenderStatus();

    /**
     * @return parent of the render node. If there is no parent empty NodePtr is returned.
     */
    const dash::NodePtr getParentNode( const NodeId& nodeId );

    /**
     * @return a node by its nodeId, creates a new one if it does not exist.
     */
    dash::NodePtr getDashNode( const NodeId& nodeId );

private:

    detail::DashTree* _impl;


};

}

#endif // _DashTree_h_
