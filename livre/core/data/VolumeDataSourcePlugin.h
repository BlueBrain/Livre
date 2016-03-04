/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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


#ifndef _VolumeDataSourcePlugin_h_
#define _VolumeDataSourcePlugin_h_

#include <livre/core/api.h>
#include <livre/core/data/NodeId.h>
#include <livre/core/data/VolumeInformation.h>

#include <lunchbox/plugin.h>

namespace livre
{

class VolumeDataSourcePluginData
{
public:
    explicit VolumeDataSourcePluginData( const lunchbox::URI& uri,
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
class VolumeDataSourcePlugin : public boost::noncopyable
{
public:

    LIVRECORE_API VolumeDataSourcePlugin();

    /** Needed by the PluginRegisterer. */
    typedef VolumeDataSourcePlugin PluginT;

    /** Needed by the PluginRegisterer. */
    typedef VolumeDataSourcePluginData InitDataT;

    virtual ~VolumeDataSourcePlugin() {}

    /**
     * @return The volume information.
     */
    const VolumeInformation& getVolumeInformation() const;

    /**
     * Initializes the GL specific functions.
     */
    virtual bool initializeGL() { return true; }

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
     */
    LIVRECORE_API virtual void update() {}

    /**
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID or 0 if not found.
     */
    LODNode getNode( const NodeId& nodeId ) const;

protected:

    typedef boost::unordered_map< NodeId, LODNode > NodeIDLODNodeMap;

    mutable NodeIDLODNodeMap _lodNodeMap;
    VolumeInformation _volumeInfo;
    mutable ReadWriteMutex _mutex;

};

/**
 * A helper function to fill up a regular volume tree.
 * @return False if it fails to initialize volume
 */
LIVRECORE_API bool fillRegularVolumeInfo( VolumeInformation& info );

}

#include <lunchbox/pluginFactory.h>

namespace boost
{
template<> inline
std::string lexical_cast( const livre::VolumeDataSourcePluginData& data )
{
    return lexical_cast< std::string >( data.getURI( ));
}
}

// http://stackoverflow.com/questions/1566963/singleton-in-a-dll
#ifdef _MSC_VER
   template class LIVRECORE_API
   lunchbox::PluginFactory< livre::VolumeDataSourcePlugin,
                            livre::VolumeDataSourcePluginData >;
#endif

#endif // _VolumeDataSourcePlugin_h_
