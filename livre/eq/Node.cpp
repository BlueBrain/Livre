
/* Copyright (c) 2011-2015, Maxim Makhinya <maxmah@gmail.com>
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
        ConstVolumeRendererParametersPtr vrRenderParametersPtr =
                _config->getFrameData().getVRParameters();
        _textureDataCachePtr.reset(
               new livre::TextureDataCache( vrRenderParametersPtr->maxCPUCacheMemoryMB * LB_1MB,
                                            _dataSourcePtr,
                                            GL_UNSIGNED_BYTE ));
    }

    bool initializeVolume()
    {
        try
        {
            VolumeSettingsPtr volumeSettingsPtr =
                    _config->getFrameData().getVolumeSettings();
            const lunchbox::URI& uri = lunchbox::URI( volumeSettingsPtr->getURI( ));
            _dataSourcePtr.reset( new livre::VolumeDataSource( uri ));

            // Inform application of real-world size for camera manipulations
            const livre::VolumeInformation& info =
                    _dataSourcePtr->getVolumeInformation();
            _config->sendEvent( VOLUME_BOUNDING_BOX ) << info.boundingBox;
            _config->sendEvent( VOLUME_FRAME_RANGE ) << info.getFrameRange();

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
        _dataSourcePtr.reset();
    }

    void releaseCache()
    {
        _textureDataCachePtr.reset();
    }

    void updateAndSendFrameRange()
    {
        _dataSourcePtr->update();

        const livre::VolumeInformation& info =
                _dataSourcePtr->getVolumeInformation();

        _config->sendEvent( VOLUME_FRAME_RANGE ) << info.frameRange;
    }

    livre::Node* const _node;
    livre::Config* const _config;
    CachePtr _textureDataCachePtr;
    VolumeDataSourcePtr _dataSourcePtr;
};

}

Node::Node( eq::Config* parent )
    : eq::Node( parent )
    , _impl( new detail::Node( this ))
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

CachePtr Node::getTextureDataCache()
{
    return _impl->_textureDataCachePtr;
}

VolumeDataSourcePtr Node::getDataSource()
{
    return _impl->_dataSourcePtr;
}

ConstVolumeDataSourcePtr Node::getDataSource() const
{
    return _impl->_dataSourcePtr;
}

void Node::frameStart( const eq::uint128_t &frameId,
                       const uint32_t frameNumber)
{
    _impl->frameStart( frameId );
    eq::Node::frameStart( frameId, frameNumber );
}

}
