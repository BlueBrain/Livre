
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

#include <livre/eq/settings/VolumeSettings.h>
#include <livre/lib/cache/DataCache.h>
#include <livre/lib/cache/HistogramCache.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/core/data/DataSource.h>

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
        const VolumeRendererParameters& vrRenderParameters =
                _config->getFrameData().getVRParameters();

        const size_t maxMemBytes = vrRenderParameters.getMaxCPUCacheMemoryMB() * LB_1MB;

        _dataCache.reset( new DataCache( maxMemBytes, *_dataSource.get( )));
        const size_t histCacheSize =
                32 * LB_1MB; // Histogram cache is 32 MB. Can hold approx 16k hists
        _histogramCache.reset( new HistogramCache( histCacheSize, *_dataCache ));
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

        initializeCache();
        return true;
    }

    void frameStart( const eq::uint128_t &frameId )
    {
        if( !_node->isApplicationNode( ))
            _config->getFrameData().sync( frameId );
    }

    void updateAndSendFrameRange()
    {
        _dataSource->update();

        const livre::VolumeInformation& info = _dataSource->getVolumeInfo();
        _config->sendEvent( VOLUME_FRAME_RANGE ) << info.frameRange;
    }

    livre::Node* const _node;
    livre::Config* const _config;
    std::unique_ptr< DataSource > _dataSource;
    std::unique_ptr< DataCache > _dataCache;
    std::unique_ptr< HistogramCache > _histogramCache;
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

    livre::Client* client = static_cast<livre::Client*>( getClient( ).get());
    client->setIdleFunction( std::bind( &Impl::updateAndSendFrameRange,
                                        _impl.get()));

    if( !isApplicationNode( ))
    {
        Config *config = static_cast< Config *>( getConfig() );
        config->mapFrameData( initId );
    }

    return _impl->configInit();
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

DataCache& Node::getDataCache()
{
    return *_impl->_dataCache;
}


livre::HistogramCache& livre::Node::getHistogramCache()
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
