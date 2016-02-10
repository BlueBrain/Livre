
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
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/uploaders/DataUploadProcessor.h>
#include <livre/core/dash/DashRenderStatus.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/dashpipeline/DashProcessorOutput.h>
#include <livre/core/data/VolumeDataSource.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{

namespace detail
{

class Node
{
public:
    explicit Node( livre::Node* node )
       : _node( node )
       , _config( static_cast< livre::Config* >( node->getConfig( )))
    {}

    void initializeCache()
    {
        const VolumeRendererParameters& vrRenderParameters =
                _config->getFrameData().getVRParameters();

        const size_t maxMemBytes =
                vrRenderParameters.getMaxCPUCacheMemoryMB() * LB_1MB;

        _textureDataCache.reset( new livre::TextureDataCache( maxMemBytes,
                                                              _dataSource,
                                                              GL_UNSIGNED_BYTE ));
    }

    bool initializeVolume()
    {
        try
        {
            const VolumeSettings& volumeSettings =
                    _config->getFrameData().getVolumeSettings();
            const lunchbox::URI& uri = lunchbox::URI( volumeSettings.getURI( ));
            dash::Context::getMain(); // Create the main context
            _dataSource.reset( new livre::VolumeDataSource( uri ));
            _dashTree.reset( new livre::DashTree( _dataSource ));
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

    void configExit()
    {
        releaseCache();
        releaseVolume();
    }

    void frameStart( const eq::uint128_t &frameId )
    {
        if( !_node->isApplicationNode( ))
            _config->getFrameData().sync( frameId );
    }

    void releaseVolume()
    {
        _dataSource.reset();
        _dashTree.reset();
    }

    void releaseCache()
    {
        _textureDataCache.reset();
    }

    void updateAndSendFrameRange()
    {
        _dataSource->update();

        const livre::VolumeInformation& info =
                _dataSource->getVolumeInformation();

        _config->sendEvent( VOLUME_FRAME_RANGE ) << info.frameRange;
    }

    livre::Node* const _node;
    livre::Config* const _config;
    TextureDataCachePtr _textureDataCache;
    VolumeDataSourcePtr _dataSource;
    DashTreePtr _dashTree;
};

}

Node::Node( eq::Config* parent )
    : eq::Node( parent )
    , _impl( new detail::Node( this ))
{
}

Node::~Node()
{
    delete _impl;
}

bool Node::configInit( const eq::uint128_t& initId )
{
    // All render data is static or multi-buffered, we can run asynchronously
    if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
        setIAttribute( IATTR_THREAD_MODEL, eq::ASYNC );

    if( !eq::Node::configInit( initId ))
        return false;

    livre::Client* client = static_cast<livre::Client*>( getClient( ).get());
    client->setIdleFunction( std::bind( &detail::Node::updateAndSendFrameRange,
                                        _impl));

    if( !isApplicationNode( ))
    {
        Config *config = static_cast< Config *>( getConfig() );
        config->mapFrameData( initId );
    }

    return _impl->configInit();
}

bool Node::configExit()
{
    _impl->configExit();

    if( !isApplicationNode( ))
    {
        Config *config = static_cast< Config *>( getConfig() );
        config->unmapFrameData();
    }

    return eq::Node::configExit();
}

TextureDataCache& Node::getTextureDataCache()
{
    return *_impl->_textureDataCache;
}

DashTreePtr Node::getDashTree()
{
    return _impl->_dashTree;
}

ConstDashTreePtr Node::getDashTree() const
{
    return _impl->_dashTree;
}

void Node::frameStart( const eq::uint128_t &frameId,
                       const uint32_t frameNumber)
{
    _impl->frameStart( frameId );
    eq::Node::frameStart( frameId, frameNumber );
}

}
