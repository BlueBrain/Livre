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

#ifndef _DataSource_h_
#define _DataSource_h_

#include <lunchbox/clock.h>

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/mathTypes.h>
#include <livre/core/defines.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/data/LODNode.h>

namespace livre
{

class DataSource
{
public:
    /**
     * DataSource constructor.
     * @param uri Initialization URI. The volume data source is generated accordingly
     * @param accessMode The access mode.
     */
    LIVRECORE_API DataSource( const lunchbox::URI& uri,
                              const AccessMode accessMode = MODE_READ );

    LIVRECORE_API ~DataSource();

    /** Load all plugin DSOs. */
    LIVRECORE_API static void loadPlugins();

    /** Unload all plugin DSOs. */
    LIVRECORE_API static void unloadPlugins();

    /**
     * @return The volume information.
     */
    LIVRECORE_API const VolumeInformation& getVolumeInfo() const;

    /**
     * @return The volume information.
     */
    LIVRECORE_API static VolumeInformation getVolumeInfo( const lunchbox::URI& uri );

    /** Initializes the GL specific functions. */
    LIVRECORE_API bool initializeGL();

    /**
     * Read the data for a given node.
     * @param nodeId NodeId to be read.
     * @return The memory block containing the data for the node.
     */
    LIVRECORE_API MemoryUnitPtr getData( const NodeId& nodeId );

    /** @copydoc getData( const NodeId& nodeId ) */
    LIVRECORE_API ConstMemoryUnitPtr getData( const NodeId& nodeId ) const;

    /**
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID or an invalid node if not found.
     */
    LIVRECORE_API LODNode getNode( const NodeId& nodeId ) const;

    /** @copydoc DataSourcePlugin::update() */
    LIVRECORE_API bool update();

private:

    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif // _DataSource_h_
