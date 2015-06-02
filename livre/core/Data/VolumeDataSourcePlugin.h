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

#include <livre/core/Data/NodeId.h>
#include <livre/core/Data/VolumeInformation.h>

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
     * @param internalNode Internal node.
     * @param lodNode Destination lod node.
     */
    virtual void internalNodeToLODNode( const NodeId internalNode,
                                        LODNode& lodNode ) const;

    /**
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID or 0 if not found.
     */
    ConstLODNodePtr getNode( const NodeId nodeId ) const;

protected:
    /**
     * will be generated and mapped, where it can be modified with the derived
     * class.
     * @param nodeId The nodeId to get the node for.
     * @return The LODNode for the ID. If not found, an invalid node is returned.
     */
    LODNodePtr _getNodeFromNodeID( uint32_t nodeId );

    mutable NodeIDLODNodePtrMap _lodNodeMap;
    VolumeInformation _volumeInfo;
};

/**
 * A helper function to fill up a regular volume tree.
 * @return False if it fails to initialize volume
 */
bool fillRegularVolumeInfo( VolumeInformation& info );

}

namespace boost
{
template<> inline
std::string lexical_cast( const livre::VolumeDataSourcePluginData& data )
{
    return lexical_cast< std::string >( data.getURI( ));
}
}

#endif // _VolumeDataSourcePlugin_h_
