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

#ifndef _VolumeDataSource_h_
#define _VolumeDataSource_h_

#include <lunchbox/clock.h>

#include <livre/core/api.h>
#include <livre/core/mathTypes.h>
#include <livre/core/defines.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/VolumeInformation.h>

namespace livre
{

namespace detail
{
    class VolumeDataSource;
}

class VolumeDataSource : boost::noncopyable
{
public:
    /**
     * VolumeDataSource constructor.
     * @param uri Initialization URI. The volume data source is generated accordingly
     * @param accessMode The access mode.
     */
    LIVRECORE_API VolumeDataSource( const lunchbox::URI& uri,
                                    const AccessMode accessMode = MODE_READ );

    LIVRECORE_API ~VolumeDataSource();

    /** Load all plugin DSOs. */
    LIVRECORE_API static void loadPlugins();

    /** Unload all plugin DSOs. */
    LIVRECORE_API static void unloadPlugins();

    /**
     * @return The volume information.
     */
    LIVRECORE_API const VolumeInformation& getVolumeInformation() const;

    /** Initializes the GL specific functions. */
    LIVRECORE_API bool initializeGL();

    /**
     * Read the data for a given node.
     * @param node NodeId to be read.
     * @return The memory block containing the data for the node.
     */
    LIVRECORE_API MemoryUnitPtr getData( const NodeId& nodeId );

    /** @copydoc getData( const NodeId& node ) */
    LIVRECORE_API ConstMemoryUnitPtr getData( const NodeId& nodeId ) const;

    /**
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID or an invalid node if not found.
     */
    LIVRECORE_API LODNode getNode( const NodeId& nodeId ) const;

    /** @copydoc VolumeDataSourcePlugin::update() */
    LIVRECORE_API void update();

private:
    detail::VolumeDataSource* _impl;
};

}

#endif // _VolumeDataSource_h_
