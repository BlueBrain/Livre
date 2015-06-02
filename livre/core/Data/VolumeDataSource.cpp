
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

#include <dash/dash.h>

#include <livre/core/defines.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/VolumeDataSourcePlugin.h>
#include <livre/core/Data/LODNode.h>
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
    {
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
        LIVRE_VERSION_ABI, lunchbox::getLibraryPaths(), "Livre.*Source" );
}

void VolumeDataSource::unloadPlugins()
{
    BOOST_FOREACH( lunchbox::DSO* plugin, _plugins )
        detail::VolumeDataSource::PluginFactory::getInstance().unload( plugin );
}

ConstLODNodePtr VolumeDataSource::getNode( const NodeId nodeId ) const
{
    return _impl->plugin->getNode( nodeId );
}

const VolumeInformation& VolumeDataSource::getVolumeInformation() const
{
    return _impl->plugin->getVolumeInformation();
}

bool VolumeDataSource::initializeGL()
{
    return _impl->plugin->initializeGL();
}

MemoryUnitPtr VolumeDataSource::getData( const LODNode& node )
{
    return _impl->plugin->getData( node );
}

livre::VolumeDataSource::~VolumeDataSource()
{
    delete _impl;
}

}
