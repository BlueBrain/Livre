
/* Copyright (c) 2011-2016, Maxim Makhinya <maxmah@gmail.com>
 *                          David Steiner <steiner@ifi.uzh.ch>
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

#include <livre/eq/Node.h>

#include <livre/eq/Config.h>
#include <livre/eq/Client.h>
#include <livre/eq/Error.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/Event.h>
#include <livre/eq/serialization.h>

#include <livre/eq/settings/EqVolumeSettings.h>
#include <livre/core/configuration/RendererParameters.h>
#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/HistogramObject.h>

#include <livre/core/data/DataSource.h>
#include <livre/core/cache/Cache.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{
struct Node::Impl
{
public:
    explicit Impl( livre::Node* node )
       : _node( node )
       , _config( static_cast< livre::Config* >( node->getConfig( )))
    {}

    void initializeCache()
    {
        const RendererParameters& vrRenderParameters =
                _config->getFrameData().getVRParameters();

        const size_t maxMemBytes = vrRenderParameters.getMaxCPUCacheMemoryMB() * LB_1MB;
        _dataCache.reset( new CacheT< DataObject >( "DataCache", maxMemBytes ));

        const size_t histCacheSize =
                32 * LB_1MB; // Histogram cache is 32 MB. Can hold approx 16k hists
        _histogramCache.reset( new CacheT< HistogramObject >( "HistogramCache", histCacheSize ));
    }

    bool initializeVolume()
    {
        try
        {
            const VolumeSettings& volumeSettings = _config->getFrameData().getVolumeSettings();
            const lunchbox::URI& uri = lunchbox::URI( volumeSettings.getURI( ));
            _dataSource.reset( new livre::DataSource( uri ));
        }
        catch( const std::runtime_error& err )
        {
            LBWARN << "Data source initialization failed: "
                   << err.what() << std::endl;
            return false;
        }

        return true;
    }

    bool configInit()
    {
        if( !initializeVolume( ))
            return false;

        auto event = _config->sendEvent( VOLUME_INFO );
        event << _dataSource->getVolumeInfo();
        initializeCache();
        return true;
    }

    void frameStart( const eq::uint128_t &frameId )
    {
        if( !_node->isApplicationNode( ))
            _config->getFrameData().sync( frameId );
    }

    void updateDataSource()
    {
        if( !_dataSource->update( ))
            return;

        auto event = _config->sendEvent( VOLUME_INFO );
        event << _dataSource->getVolumeInfo();
    }

    livre::Node* const _node;
    livre::Config* const _config;
    std::unique_ptr< DataSource > _dataSource;
    std::unique_ptr< Cache > _dataCache;
    std::unique_ptr< Cache > _histogramCache;
};

Node::Node( eq::Config* parent )
    : eq::Node( parent )
    , _impl( new Impl( this ))
{
}

Node::~Node()
{
}

bool Node::configInit( const eq::uint128_t& initId )
{
    // All render data is static or multi-buffered, we can run asynchronously
    if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
        setIAttribute( IATTR_THREAD_MODEL, eq::ASYNC );

    if( !eq::Node::configInit( initId ))
        return false;

    if( !_impl->configInit( ))
        return false;

    livre::Client* client = static_cast<livre::Client*>( getClient( ).get());
    client->setIdleFunction( std::bind( &Impl::updateDataSource, _impl.get( )));

    if( !isApplicationNode( ))
    {
        Config *config = static_cast< Config *>( getConfig() );
        config->mapFrameData( initId );
    }

    return true;
}

bool Node::configExit()
{
    if( !isApplicationNode( ))
    {
        Config *config = static_cast< Config *>( getConfig() );
        config->unmapFrameData();
    }

    return eq::Node::configExit();
}

DataSource& Node::getDataSource()
{
    return *_impl->_dataSource;
}

const DataSource& Node::getDataSource() const
{
    return *_impl->_dataSource;
}

Cache& Node::getDataCache()
{
    return *_impl->_dataCache;
}

livre::Cache& livre::Node::getHistogramCache()
{
    return *_impl->_histogramCache;
}

void Node::frameStart( const eq::uint128_t &frameId,
                       const uint32_t frameNumber)
{
    _impl->frameStart( frameId );
    eq::Node::frameStart( frameId, frameNumber );
}

}
