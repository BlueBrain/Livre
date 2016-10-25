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
#include <livre/core/data/DataSource.h>
#include <livre/core/data/DataSourcePlugin.h>
#include <livre/core/version.h>

#include <lunchbox/pluginFactory.h>

namespace livre
{

namespace
{
    lunchbox::DSOs _plugins;
}

struct DataSource::Impl
{
public:
    typedef lunchbox::PluginFactory< DataSourcePlugin, DataSourcePluginData > PluginFactory;

    Impl( const lunchbox::URI& uri, const AccessMode accessMode )
        : plugin( PluginFactory::getInstance().create( DataSourcePluginData( uri, accessMode )))
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

    std::unique_ptr< DataSourcePlugin > plugin;
};

DataSource::DataSource( const lunchbox::URI& uri, const AccessMode accessMode )
    : _impl( new Impl( uri, accessMode ) )
{
}

void DataSource::loadPlugins()
{
    unloadPlugins();
    _plugins = DataSource::Impl::PluginFactory::getInstance().load(
        LIVRECORE_VERSION_ABI, lunchbox::getLibraryPaths(), "Livre.*Source" );
}

void DataSource::unloadPlugins()
{
    for( lunchbox::DSO* plugin: _plugins )
        DataSource::Impl::PluginFactory::getInstance().unload( plugin );
}

LODNode DataSource::getNode( const NodeId& nodeId ) const
{
    return _impl->getNode( nodeId );
}

bool DataSource::update()
{
    return _impl->plugin->update();
}

const VolumeInformation& DataSource::getVolumeInfo() const
{
    return _impl->plugin->getVolumeInfo();
}

bool DataSource::initializeGL()
{
    return _impl->plugin->initializeGL();
}

MemoryUnitPtr DataSource::getData( const NodeId& nodeId )
{
    if( !nodeId.isValid( ))
        return MemoryUnitPtr();

    const LODNode& lodNode = getNode( nodeId );
    if( !lodNode.isValid( ))
        return MemoryUnitPtr();

    return _impl->plugin->getData( lodNode );
}

ConstMemoryUnitPtr DataSource::getData( const NodeId& nodeId ) const
{
    if( !nodeId.isValid( ))
        return ConstMemoryUnitPtr();

    const LODNode& lodNode = getNode( nodeId );
    if( !lodNode.isValid( ))
        return ConstMemoryUnitPtr();

    return _impl->plugin->getData( lodNode );
}

VolumeInformation DataSource::getVolumeInfo( const lunchbox::URI& uri )
{
    const DataSource source( uri );
    return source.getVolumeInfo();
}

DataSource::~DataSource()
{}

}
