
/* Copyright (c) 2012-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
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

#include <eq/gl.h>

#define CONNECTION_ID 0u

const uint32_t maxQueueSize = 65536;

namespace livre
{

namespace detail
{

class Pipe
{
public:
    Pipe( livre::Pipe* pipe )
        : _pipe( pipe )
        , _dashProcessorPtr( new DashProcessor( ))
        , _frameDataPtr( new FrameData( ))
    {}

    void startUploadProcessors()
    {
        if( !_textureUploadProcessorPtr->isRunning( ))
            _textureUploadProcessorPtr->start();

        if( !_dataUploadProcessorPtr->isRunning( ))
            _dataUploadProcessorPtr->start();
    }

    void stopUploadProcessors()
    {
        // TO_EXIT was set as ThreadOp in Channel::configExit()
        _dashProcessorPtr->getProcessorOutput_()->commit( 0 );
        _textureUploadProcessorPtr->join();
        _dataUploadProcessorPtr->join();
    }

    bool mapFrameData( const eq::uint128_t& initId )
    {
        livre::Config* config = static_cast< livre::Config* >( _pipe->getConfig( ));
        _frameDataPtr->initialize( config );
        if( !_frameDataPtr->map( config, initId ))
            return false;
        _frameDataPtr->mapObjects();
        return true;
    }

    void unmapFrameData()
    {
        _frameDataPtr->unmapObjects();
        _frameDataPtr->unmap( static_cast< livre::Config* >( _pipe->getConfig( )));
    }

    void initializeCaches()
    {
        _textureCachePtr->setMaximumMemory(
                    _frameDataPtr->getVRParameters()->maxTextureMemoryMB );
    }

    void releaseCaches()
    {
        // has to be called from PipeThread to release textures properly
        _textureCachePtr.reset();
    }

    void initializePipelineProcessors()
    {
        livre::Node* node = static_cast< livre::Node* >( _pipe->getNode( ));

        _dashProcessorPtr->setDashContext( node->getDashTree()->createContext( ));
        _dataUploadProcessorPtr.reset( new DataUploadProcessor( node->getDashTree(),
                                                                node->getRawDataCache(),
                                                                node->getTextureDataCache( )));

        _textureCachePtr.reset( new TextureCache( GL_LUMINANCE8 ) );
        _textureUploadProcessorPtr.reset( new TextureUploadProcessor( node->getDashTree(),
                                                                      *_textureCachePtr ));

        startUploadProcessors();
    }

    void releasePipelineProcessors()
    {
        stopUploadProcessors();
        _textureUploadProcessorPtr->setDashContext( DashContextPtr( ));
        _dataUploadProcessorPtr->setDashContext( DashContextPtr( ));
        _textureUploadProcessorPtr.reset();
        _dataUploadProcessorPtr.reset();
        _dashProcessorPtr->setDashContext( DashContextPtr( ));
    }

    void initializePipelineConnections()
    {
        // Connects data uploader to texture uploader
        DashConnectionPtr dataOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _dataUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >( )
                ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

        _textureUploadProcessorPtr->getProcessorInput_< DashProcessorInput >( )
                ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

        // Connects texture uploader to pipe
        DashConnectionPtr texOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _textureUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >()
                ->addConnection( CONNECTION_ID, texOutputConnectionPtr );
        _dashProcessorPtr->getProcessorInput_< DashProcessorInput >()
                ->addConnection( CONNECTION_ID, texOutputConnectionPtr );
        // Connects pipe to data uploader
        DashConnectionPtr pipeOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _dataUploadProcessorPtr->getProcessorInput_< DashProcessorInput >()
                ->addConnection( CONNECTION_ID, pipeOutputConnectionPtr );
        _dashProcessorPtr->getProcessorOutput_< DashProcessorOutput >()
                ->addConnection( CONNECTION_ID, pipeOutputConnectionPtr );
    }

    void releasePipelineConnections( )
    {
        _dashProcessorPtr->getProcessorOutput_< DashProcessorOutput >()
                ->removeConnection( CONNECTION_ID );
        _dataUploadProcessorPtr->getProcessorInput_< DashProcessorInput >()
                ->removeConnection( CONNECTION_ID );
        _dashProcessorPtr->getProcessorInput_< DashProcessorInput >( )
                ->removeConnection( CONNECTION_ID );
        _textureUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >( )
                ->removeConnection( CONNECTION_ID );
        _textureUploadProcessorPtr->getProcessorInput_< DashProcessorInput >( )
                ->removeConnection( CONNECTION_ID );
        _dataUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >( )
                ->removeConnection( CONNECTION_ID );
    }

    void frameStart( const eq::uint128_t& frameId )
    {
        _frameDataPtr->sync( frameId );

        _dashProcessorPtr->getDashContext()->setCurrent();

        if( _dashProcessorPtr->getProcessorInput_()->dataWaitingOnInput( CONNECTION_ID ))
            _dashProcessorPtr->getProcessorInput_()->applyAll( CONNECTION_ID );
    }

    bool configInit( const eq::uint128_t& initId )
    {
        if( !mapFrameData( initId ))
            return false;
        initializePipelineProcessors();
        initializePipelineConnections();
        initializeCaches();
        return true;
    }

    void configExit()
    {
        if( !_dashProcessorPtr->getDashContext( ))
            return;
        releaseCaches( );
        releasePipelineConnections( );
        releasePipelineProcessors( );
        unmapFrameData();
    }

    livre::Pipe* const _pipe;
    TextureUploadProcessorPtr _textureUploadProcessorPtr;
    DataUploadProcessorPtr _dataUploadProcessorPtr;
    TextureCachePtr _textureCachePtr;
    DashProcessorPtr _dashProcessorPtr;
    FrameDataPtr _frameDataPtr;
};

}

Pipe::Pipe( eq::Node* parent )
    : eq::Pipe( parent )
    , _impl( new detail::Pipe( this ))
{
}

Pipe::~Pipe( )
{
    delete _impl;
}

TextureUploadProcessorPtr Pipe::getTextureUploadProcessor()
{
    return _impl->_textureUploadProcessorPtr;
}

ConstTextureUploadProcessorPtr Pipe::getTextureUploadProcessor() const
{
    return _impl->_textureUploadProcessorPtr;
}

DataUploadProcessorPtr Pipe::getDataUploadProcessor()
{
    return _impl->_dataUploadProcessorPtr;
}

ConstDataUploadProcessorPtr Pipe::getDataUploadProcessor() const
{
    return _impl->_dataUploadProcessorPtr;
}

DashProcessorPtr Pipe::getProcessor()
{
    return _impl->_dashProcessorPtr;
}

ConstDashProcessorPtr Pipe::getProcessor() const
{
    return _impl->_dashProcessorPtr;
}

void Pipe::frameStart( const eq::uint128_t& frameId, const uint32_t frameNumber )
{
    _impl->frameStart( frameId );
    eq::Pipe::frameStart( frameId, frameNumber );
}

bool Pipe::configInit( const eq::uint128_t& initId )
{
    if( !eq::Pipe::configInit( initId ))
        return false;

    return _impl->configInit( initId );
}

bool Pipe::configExit()
{
    _impl->configExit();
    return eq::Pipe::configExit();
}

ConstFrameDataPtr Pipe::getFrameData() const
{
    return _impl->_frameDataPtr;
}

}
