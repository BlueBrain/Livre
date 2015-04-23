
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *               2012,      David Steiner   <steiner@ifi.uzh.ch>
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
#include <livre/Eq/Pipe.h>
#include <livre/Eq/Error.h>
#include <livre/Eq/Config.h>
#include <livre/Eq/FrameData.h>
#include <livre/Eq/Render/EqContext.h>

#include <livre/Lib/Cache/TextureCache.h>
#include <livre/Lib/Configuration/VolumeRendererParameters.h>
#include <livre/Lib/Uploaders/DataUploadProcessor.h>
#include <livre/Lib/Uploaders/TextureUploadProcessor.h>
#include <livre/core/Dash/DashTree.h>
#include <livre/core/DashPipeline/DashConnection.h>
#include <livre/core/DashPipeline/DashProcessor.h>
#include <livre/core/DashPipeline/DashProcessorInput.h>
#include <livre/core/DashPipeline/DashProcessorOutput.h>

#include <eq/client/gl.h>

#define CONNECTION_ID 0u

namespace livre
{

Pipe::Pipe( eq::Node* parent )
    : eq::Pipe( parent ),
      dashProcessorPtr_( new DashProcessor( ) ),
      frameDataPtr_( new FrameData( ) )
{
    LBASSERT( parent );
}

Pipe::~Pipe( )
{
}

void Pipe::startUploadProcessors_( )
{
    if( !textureUploadProcessorPtr_->isRunning( ) )
        textureUploadProcessorPtr_->start( );

    if( !dataUploadProcessorPtr_->isRunning( ) )
        dataUploadProcessorPtr_->start( );
}

void Pipe::stopUploadProcessors_( )
{
    // TO_EXIT was set as ThreadOp in Channel::configExit()
    getProcessor( )->getProcessorOutput_( )->commit( 0 );

    textureUploadProcessorPtr_->join();
    dataUploadProcessorPtr_->join();
}

ConstFrameDataPtr Pipe::getFrameData( ) const
{
    return frameDataPtr_;
}

TextureUploadProcessorPtr Pipe::getTextureUploadProcessor( )
{
    return textureUploadProcessorPtr_;
}

ConstTextureUploadProcessorPtr Pipe::getTextureUploadProcessor( ) const
{
    return textureUploadProcessorPtr_;
}

DataUploadProcessorPtr Pipe::getDataUploadProcessor( )
{
    return dataUploadProcessorPtr_;
}

ConstDataUploadProcessorPtr Pipe::getDataUploadProcessor( ) const
{
    return dataUploadProcessorPtr_;
}

DashProcessorPtr Pipe::getProcessor( )
{
    return dashProcessorPtr_;
}

ConstDashProcessorPtr Pipe::getProcessor( ) const
{
    return dashProcessorPtr_;
}

void Pipe::initializeCaches_( )
{
    textureCachePtr_->setMaximumMemory( frameDataPtr_->getVRParameters( )->maxTextureMemoryMB );
}

void Pipe::releaseCaches_( )
{
    // has to be called from PipeThread to release textures properly
    textureCachePtr_.reset();
}

void Pipe::frameStart( const eq::uint128_t& frameId, const uint32_t frameNumber )
{
    frameDataPtr_->sync( frameId );

    dashProcessorPtr_->getDashContext( )->setCurrent( );

    if( dashProcessorPtr_->getProcessorInput_( )->dataWaitingOnInput( CONNECTION_ID ) )
        dashProcessorPtr_->getProcessorInput_( )->applyAll( CONNECTION_ID );

    eq::Pipe::frameStart( frameId, frameNumber );
}

void Pipe::initializePipelineProcessors_( )
{
    DashContextPtr localContext( new dash::Context( ) );
    dashProcessorPtr_->setDashContext( localContext );

    Node *node = static_cast< Node *>( getNode( ) );
    dataUploadProcessorPtr_.reset( new DataUploadProcessor( node->getRawDataCache( ),
                                                            node->getTextureDataCache( ) ) );

    textureCachePtr_.reset( new TextureCache( GL_LUMINANCE8 ) );
    textureUploadProcessorPtr_.reset( new TextureUploadProcessor( *textureCachePtr_ ) );

    DashContextPtr dataLoaderContext( new dash::Context( ) );
    dataUploadProcessorPtr_->setDashContext( dataLoaderContext );

    DashContextPtr textureLoaderContext( new dash::Context( ) );
    textureUploadProcessorPtr_->setDashContext( textureLoaderContext );
}


void Pipe::releasePipelineProcessors_( )
{
    textureUploadProcessorPtr_->setDashContext( DashContextPtr() );
    dataUploadProcessorPtr_->setDashContext( DashContextPtr() );
    textureUploadProcessorPtr_.reset( );
    dataUploadProcessorPtr_.reset( );
    dashProcessorPtr_->setDashContext( DashContextPtr() );

}

void Pipe::initalizeMapping_( )
{
    Node *node = static_cast< Node *>( getNode( ) );

    dataUploadProcessorPtr_->setDashTree( node->getDashTree( )->getRootNode( ) );
    textureUploadProcessorPtr_->setDashTree( node->getDashTree( )->getRootNode( ) );

    node->getDashContext( )->map( node->getDashTree( )->getRootNode( ), *dataUploadProcessorPtr_->getDashContext( ) );
    node->getDashContext( )->map( node->getDashTree( )->getRootNode( ), *textureUploadProcessorPtr_->getDashContext( ) );
    node->getDashContext( )->map( node->getDashTree( )->getRootNode( ), *dashProcessorPtr_->getDashContext( ) );

    dashProcessorPtr_->getDashContext( )->setCurrent( );
}

void Pipe::releaseMapping_( )
{
    Node *node = static_cast< Node *>( getNode( ) );

    dashProcessorPtr_->getDashContext( )->unmap( node->getDashTree( )->getRootNode( ) );
    textureUploadProcessorPtr_->getDashContext( )->unmap( node->getDashTree( )->getRootNode( ) );
    dataUploadProcessorPtr_->getDashContext( )->unmap( node->getDashTree( )->getRootNode( ) );

    textureUploadProcessorPtr_->setDashTree( dash::NodePtr() );
    dataUploadProcessorPtr_->setDashTree( dash::NodePtr() );
}


void Pipe::initializePipelineConnections_( )
{
    // Connects data uploader to texture uploader
    const uint32_t maxQueueSize = 65536;

    DashConnectionPtr dataOutputConnectionPtr( new DashConnection( maxQueueSize ) );
    dataUploadProcessorPtr_->getProcessorOutput_< DashProcessorOutput >( )
            ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

    textureUploadProcessorPtr_->getProcessorInput_< DashProcessorInput >( )
            ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

    // Connects texture uploader to pipe
    DashConnectionPtr texOutputConnectionPtr( new DashConnection( maxQueueSize ) );
    textureUploadProcessorPtr_->getProcessorOutput_< DashProcessorOutput >( )
            ->addConnection( CONNECTION_ID, texOutputConnectionPtr );
    dashProcessorPtr_->getProcessorInput_< DashProcessorInput >( )->addConnection( CONNECTION_ID,
                                                                                  texOutputConnectionPtr );
    // Connects pipe to data uploader
    DashConnectionPtr pipeOutputConnectionPtr( new DashConnection( maxQueueSize ) );
    dataUploadProcessorPtr_->getProcessorInput_< DashProcessorInput >( )
            ->addConnection( CONNECTION_ID, pipeOutputConnectionPtr );

    dashProcessorPtr_->getProcessorOutput_< DashProcessorOutput >( )->addConnection( CONNECTION_ID,
                                                                                    pipeOutputConnectionPtr );
}

void Pipe::releasePipelineConnections_( )
{
    dashProcessorPtr_->getProcessorOutput_< DashProcessorOutput >( )->removeConnection( CONNECTION_ID );
    dataUploadProcessorPtr_->getProcessorInput_< DashProcessorInput >( )->removeConnection( CONNECTION_ID );
    dashProcessorPtr_->getProcessorInput_< DashProcessorInput >( )->removeConnection( CONNECTION_ID );
    textureUploadProcessorPtr_->getProcessorOutput_< DashProcessorOutput >( )->removeConnection( CONNECTION_ID );
    textureUploadProcessorPtr_->getProcessorInput_< DashProcessorInput >( )->removeConnection( CONNECTION_ID );
    dataUploadProcessorPtr_->getProcessorOutput_< DashProcessorOutput >( )->removeConnection( CONNECTION_ID );
}


void Pipe::mapFrameData_(const eq::uint128_t& initId)
{
    frameDataPtr_->initialize( getConfig( ) );
    frameDataPtr_->map( getConfig( ), initId );
    frameDataPtr_->mapObjects( );
}

void Pipe::unmapFrameData_( )
{
    frameDataPtr_->unmapObjects( );
    frameDataPtr_->unmap( getConfig( ) );
}

bool Pipe::configInit( const eq::uint128_t& initId )
{
    if( !eq::Pipe::configInit( initId ))
        return false;

    mapFrameData_( initId );

    initializePipelineProcessors_( );
    initializePipelineConnections_( );
    initalizeMapping_( );
    initializeCaches_( );

    return true;
}

bool Pipe::configExit( )
{
    // If pipe is not configured, nothing to clean up.
    if( !dashProcessorPtr_->getDashContext( ) )
         return eq::Pipe::configExit( );

    releaseCaches_( );
    releaseMapping_( );
    releasePipelineConnections_( );
    releasePipelineProcessors_( );
    unmapFrameData_( );

    return eq::Pipe::configExit( );
}

}
