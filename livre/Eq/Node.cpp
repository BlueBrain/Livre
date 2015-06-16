
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
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

#include <livre/Eq/Node.h>

#include <livre/Eq/Config.h>
#include <livre/Eq/Error.h>
#include <livre/Eq/FrameData.h>
#include <livre/Eq/Pipe.h>
#include <livre/Eq/Event.h>

#include <livre/Eq/Settings/VolumeSettings.h>
#include <livre/Lib/Cache/RawDataCache.h>
#include <livre/Lib/Cache/TextureDataCache.h>
#include <livre/Lib/Configuration/VolumeRendererParameters.h>
#include <livre/Lib/Uploaders/DataUploadProcessor.h>
#include <livre/core/Dash/DashTree.h>
#include <livre/core/DashPipeline/DashProcessorOutput.h>
#include <livre/core/Data/VolumeDataSource.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{

namespace detail
{

class Node
{
public:
    Node( livre::Node* node )
        : _config( static_cast< livre::Config* >( node->getConfig( )))
    {}

    void initializeCaches()
    {
        _rawDataCachePtr.reset( new livre::RawDataCache( ));
        _textureDataCachePtr .reset( new livre::TextureDataCache( GL_UNSIGNED_BYTE ));
        _rawDataCachePtr->setDataSource( _dataSourcePtr );

        ConstVolumeRendererParametersPtr vrRenderParametersPtr =
                _config->getFrameData().getVRParameters();
        _rawDataCachePtr->setMaximumMemory(
                        vrRenderParametersPtr->maxDataMemoryMB );
        _textureDataCachePtr->setMaximumMemory(
                    vrRenderParametersPtr->maxTextureDataMemoryMB );
    }

    bool initializeVolume()
    {
        try
        {
            VolumeSettingsPtr volumeSettingsPtr =
                    _config->getFrameData().getVolumeSettings();
            const lunchbox::URI& uri = lunchbox::URI( volumeSettingsPtr->getURI( ));
            _dataSourcePtr.reset( new livre::VolumeDataSource( uri ));
            _dashTreePtr.reset( new livre::DashTree( _dataSourcePtr ));

            // Inform application of real-world size for camera manipulations
            const livre::VolumeInformation& info =
                    _dataSourcePtr->getVolumeInformation();
            _config->sendEvent( VOLUME_BOUNDING_BOX ) << info.boundingBox;
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

        initializeCaches();
        return true;
    }

    void configExit()
    {
        releaseCaches();
        releaseVolume();
    }

    void releaseVolume()
    {
        _dataSourcePtr.reset();
        _dashTreePtr.reset();
    }

    void releaseCaches()
    {
        _textureDataCachePtr.reset();
        _rawDataCachePtr.reset();
    }

    livre::Config* const _config;
    RawDataCachePtr _rawDataCachePtr;
    TextureDataCachePtr _textureDataCachePtr;
    VolumeDataSourcePtr _dataSourcePtr;
    DashTreePtr _dashTreePtr;
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

RawDataCache& Node::getRawDataCache()
{
    return *_impl->_rawDataCachePtr;
}

TextureDataCache& Node::getTextureDataCache()
{
    return *_impl->_textureDataCachePtr;
}

DashTreePtr Node::getDashTree()
{
    return _impl->_dashTreePtr;
}

void Node::frameStart( const eq::uint128_t &frameId, const uint32_t frameNumber)
{
    if( !isApplicationNode() )
    {
        Config *config = static_cast< Config *>( getConfig( ));
        config->getFrameData().sync( frameId );
    }

    eq::Node::frameStart( frameId, frameNumber );
}

}