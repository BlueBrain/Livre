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


#ifndef _DataSourcePlugin_h_
#define _DataSourcePlugin_h_

#include <livre/core/api.h>
#include <livre/core/data/NodeId.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/data/VolumeInformation.h>

#include <lunchbox/plugin.h>

namespace livre
{

class DataSourcePluginData
{
public:
    explicit DataSourcePluginData( const lunchbox::URI& uri,
                                   const AccessMode accessMode = MODE_READ )
        : _uri( uri ),
          _accessMode( accessMode )
    {}

    const lunchbox::URI& getURI( ) const { return _uri; }
    AccessMode getAccessMode( ) const { return _accessMode; }

private:

    const lunchbox::URI& _uri;
    const AccessMode _accessMode;
};

/**
 * Interface for volume data sources.
 *
 * Implementations are responsible for filling the VolumeInformation.
 */
class DataSourcePlugin
{
public:

    LIVRECORE_API DataSourcePlugin();

    /** Needed by the PluginRegisterer. */
    typedef DataSourcePlugin InterfaceT;

    /** Needed by the PluginRegisterer. */
    typedef DataSourcePluginData InitDataT;

    virtual ~DataSourcePlugin() {}

    /**
     * @return The volume information.
     */
    const VolumeInformation& getVolumeInfo() const;

    /** Initializes the GL specific functions. */
    virtual bool initializeGL() { return true; }

    /** Last call with a valid and active GL context to clear GL objects. */
    virtual void finishGL() {}

    /**
     * Read the data for a given node.
     * @param node LODNode to be read.
     * @return The memory block containing the data for the node.
     */
    virtual MemoryUnitPtr getData( const LODNode& node ) = 0;

    /**
     * Converts internal node to lod node.
     * @param nodeId Internal node.
     * @returns lodNode for the node id ( world space definition, voxel size etc )
     */
    LIVRECORE_API virtual LODNode internalNodeToLODNode( const NodeId& nodeId ) const;

    /**
     * Updates the data source. For example, data sources may update their
     * temporal range based on newly available data.
     * @return true if the datasource has changed since the last update().
     */
    LIVRECORE_API virtual bool update() { return false; }

    /**
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID or 0 if not found.
     */
    LODNode getNode( const NodeId& nodeId ) const;

protected:

    DataSourcePlugin( const DataSourcePlugin& ) = delete;
    DataSourcePlugin& operator=( const DataSourcePlugin& ) = delete;

    typedef std::unordered_map< Identifier, LODNode > IdLODNodeMap;

    mutable IdLODNodeMap _lodNodeMap;
    VolumeInformation _volumeInfo;
    mutable ReadWriteMutex _mutex;

};

/**
 * A helper function to fill up a regular volume tree.
 * @return False if it fails to initialize volume
 */
LIVRECORE_API bool fillRegularVolumeInfo( VolumeInformation& info );

}


namespace std
{
inline string to_string( const livre::DataSourcePluginData& data )
{
    return to_string( data.getURI( ));
}
}

// http://stackoverflow.com/questions/1566963/singleton-in-a-dll
#ifdef _MSC_VER
#  include <lunchbox/pluginFactory.h>
    template class LIVRECORE_API
    lunchbox::PluginFactory< livre::DataSourcePlugin,
                             livre::DataSourcePluginData >;
#endif

#endif // _DataSourcePlugin_h_
