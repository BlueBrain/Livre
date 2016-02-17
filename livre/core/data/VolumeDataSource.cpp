/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <livre/core/defines.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeDataSourcePlugin.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/version.h>

#include <lunchbox/pluginFactory.h>

namespace livre
{
namespace
{
    lunchbox::DSOs _plugins;
}
namespace detail
{

class VolumeDataSource
{
public:
    typedef lunchbox::PluginFactory< VolumeDataSourcePlugin,
                                     VolumeDataSourcePluginData >
                PluginFactory;

    VolumeDataSource( const lunchbox::URI& uri,
                      const AccessMode accessMode )
        : plugin( PluginFactory::getInstance().create(
                      VolumeDataSourcePluginData( uri, accessMode )))
    {}

    LODNode getNode( const NodeId& nodeId ) const
    {
        return plugin->getNode( nodeId );
    }

    MemoryUnitPtr getData( const LODNode& node )
    {
        return plugin->getData( node );
    }

    ConstMemoryUnitPtr getData( const LODNode& node ) const
    {
        return plugin->getData( node );
    }

    boost::scoped_ptr< VolumeDataSourcePlugin > plugin;
};

}

VolumeDataSource::VolumeDataSource( const lunchbox::URI& uri,
                                    const AccessMode accessMode )
    : _impl( new detail::VolumeDataSource( uri, accessMode ) )
{
}

void VolumeDataSource::loadPlugins()
{
    unloadPlugins();
    _plugins = detail::VolumeDataSource::PluginFactory::getInstance().load(
        LIVRECORE_VERSION_ABI, lunchbox::getLibraryPaths(), "Livre.*Source" );
}

void VolumeDataSource::unloadPlugins()
{
    for( lunchbox::DSO* plugin: _plugins )
        detail::VolumeDataSource::PluginFactory::getInstance().unload( plugin );
}

LODNode VolumeDataSource::getNode( const NodeId& nodeId ) const
{
    return _impl->getNode( nodeId );
}

void VolumeDataSource::update()
{
    _impl->plugin->update();
}

const VolumeInformation& VolumeDataSource::getVolumeInformation() const
{
    return _impl->plugin->getVolumeInformation();
}

bool VolumeDataSource::initializeGL()
{
    return _impl->plugin->initializeGL();
}

MemoryUnitPtr VolumeDataSource::getData( const NodeId& nodeId )
{
    if( !nodeId.isValid( ))
        return MemoryUnitPtr();

    const LODNode& lodNode = getNode( nodeId );
    return _impl->plugin->getData( lodNode );
}

ConstMemoryUnitPtr VolumeDataSource::getData( const NodeId& nodeId ) const
{
    if( !nodeId.isValid( ))
        return ConstMemoryUnitPtr();

    const LODNode& lodNode = getNode( nodeId );
    return _impl->plugin->getData( lodNode );
}

VolumeDataSource::~VolumeDataSource()
{}

}
