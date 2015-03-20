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

#include <livre/core/mathTypes.h>
#include <livre/core/defines.h>
#include <livre/core/Data/MemoryUnit.h>
#include <livre/core/Data/VolumeInformation.h>

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
    VolumeDataSource( const lunchbox::URI& uri,
                      const AccessMode accessMode = MODE_READ );

    ~VolumeDataSource();

    /** Load all plugin DSOs. */
    static void loadPlugins();

    /** Unload all plugin DSOs. */
    static void unloadPlugins();

    /**
     * @return The volume information.
     */
    const VolumeInformation& getVolumeInformation() const;

    /** Initializes the GL specific functions. */
    bool initializeGL();

    /**
     * Read the data for a given node.
     * @param node LODNode to be read.
     * @return The memory block containing the data for the node.
     */
    MemoryUnitPtr getData( const LODNode& node );

    /**
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID or 0 if not found.
     */
    ConstLODNodePtr getNodeFromNodeID( const NodeId nodeId ) const;

private:

    detail::VolumeDataSource *_impl;

};

}

#endif // _VolumeDataSource_h_
