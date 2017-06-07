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

#ifndef _DataSource_h_
#define _DataSource_h_

#include <livre/data/LODNode.h>
#include <livre/data/MemoryUnit.h>
#include <livre/data/VolumeInformation.h>
#include <livre/data/api.h>

namespace livre
{
class DataSource
{
public:
    /**
     * DataSource constructor.
     * @param uri Initialization URI. The volume data source is generated
     * accordingly
     * @param accessMode The access mode.
     */
    LIVREDATA_API DataSource(const servus::URI& uri,
                             const AccessMode accessMode = MODE_READ);

    LIVREDATA_API ~DataSource();

    /** Load all plugin DSOs. */
    LIVREDATA_API static void loadPlugins();

    /** @return information on all loaded plugins. */
    LIVREDATA_API static std::string getDescriptions();

    /** @return true if a datasource plugin for the given URI is available. */
    LIVREDATA_API static bool handles(const servus::URI& uri);

    /**
     * @return The volume information.
     */
    LIVREDATA_API const VolumeInformation& getVolumeInfo() const;

    /**
     * @return The volume information.
     */
    LIVREDATA_API static VolumeInformation getVolumeInfo(
        const servus::URI& uri);

    /** Initializes the GL specific functions. */
    LIVREDATA_API bool initializeGL();

    /** @copydoc DataSourcePlugin::finishGL() */
    LIVREDATA_API void finishGL();

    /**
     * Read the data for a given node.
     * @param nodeId NodeId to be read.
     * @return The memory block containing the data for the node.
     */
    LIVREDATA_API MemoryUnitPtr getData(const NodeId& nodeId);

    /** @copydoc getData( const NodeId& nodeId ) */
    LIVREDATA_API ConstMemoryUnitPtr getData(const NodeId& nodeId) const;

    /**
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID or an invalid node if not found.
     */
    LIVREDATA_API LODNode getNode(const NodeId& nodeId) const;

    /** @copydoc DataSourcePlugin::update() */
    LIVREDATA_API bool update();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}

#endif // _DataSource_h_
