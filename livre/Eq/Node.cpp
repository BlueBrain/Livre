
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
#include <livre/core/DashPipeline/DashConnection.h>
#include <livre/core/DashPipeline/DashProcessorOutput.h>
#include <livre/core/Data/VolumeDataSource.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{

Node::Node( eq::Config* parent )
    : eq::Node( parent )
{
    dash::Context::getMain();
    setDashContext( DashContextPtr( new dash::Context( ) ) );
}

RawDataCache& Node::getRawDataCache()
{
    return *rawDataCachePtr_;
}

TextureDataCache& Node::getTextureDataCache()
{
    return *textureDataCachePtr_;
}

ConstVolumeDataSourcePtr Node::getVolumeDataSource() const
{
    return dataSourcePtr_;
}

DashTreePtr Node::getDashTree()
{
    return dashTreePtr_;
}

bool Node::initializeVolume_()
{
    getDashContext()->setCurrent();
    try
    {
        dataSourcePtr_.reset( new VolumeDataSource( lunchbox::URI( volumeSettingsPtr_->getURI( ))));
        dashTreePtr_.reset( new DashTree );
        dataSourcePtr_->initializeDashTree( dashTreePtr_->getRootNode() );

        // Inform application of real-world size for camera manipulations
        const VolumeInformation& info = dataSourcePtr_->getVolumeInformation();
        eq::Config* config = getConfig();
        config->sendEvent( VOLUME_BOUNDING_BOX ) << info.boundingBox;
    }
    catch( const std::runtime_error& err )
    {
        LBWARN << "Data source initialization failed: "
               << err.what() << std::endl;
        return false;
    }

    return true;
}

void Node::releaseVolume_()
{
    dataSourcePtr_.reset( );
    dashTreePtr_.reset( );
}

void Node::initializeCaches_()
{
    rawDataCachePtr_.reset( new RawDataCache( ) );
    textureDataCachePtr_ .reset( new TextureDataCache( GL_UNSIGNED_BYTE ) );
    rawDataCachePtr_->setDataSource( dataSourcePtr_ );

    rawDataCachePtr_->setMaximumMemory( vrRenderParametersPtr_->maxDataMemoryMB );
    textureDataCachePtr_->setMaximumMemory( vrRenderParametersPtr_->maxTextureDataMemoryMB );
}

void Node::releaseCaches_()
{
    textureDataCachePtr_ .reset( );
    rawDataCachePtr_.reset( );
}

FrameData& Node::getFrameData_()
{
     Config *config = static_cast< Config *>( getConfig() );
     return config->getFrameData();
}

bool Node::configInit( const eq::uint128_t& initId )
{
    // All render data is static or multi-buffered, we can run asynchronously
    if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
        setIAttribute( IATTR_THREAD_MODEL, eq::ASYNC );

    if( !eq::Node::configInit( initId ))
        return false;

    Config *config = static_cast< Config *>( getConfig() );
    if( !isApplicationNode( ))
        config->mapFrameData( initId );

    volumeSettingsPtr_ = getFrameData_().getVolumeSettings();
    vrRenderParametersPtr_ = getFrameData_().getVRParameters();

    if( !initializeVolume_( ))
        return false;

    initializeCaches_( );
    return true;
}

void Node::frameStart( const eq::uint128_t &frameId, const uint32_t frameNumber)
{
    if( !isApplicationNode() )
        getFrameData_().sync( frameId );

    eq::Node::frameStart( frameId, frameNumber );
}

bool Node::configExit()
{
    releaseCaches_( );
    releaseVolume_( );

    if( !isApplicationNode() )
    {
        Config *config = static_cast< Config *>( getConfig() );
        config->unmapFrameData();
    }

    return eq::Node::configExit();
}

}
