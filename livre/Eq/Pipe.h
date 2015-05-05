
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 *                    2012, David Steiner   <steiner@ifi.uzh.ch>
 *                    2013, Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#ifndef _Pipe_h_
#define _Pipe_h_

#include <eq/client/pipe.h>
#include <livre/Eq/types.h>

namespace livre
{

/**
 * The Pipe class is a standard EQ abstraction for OpenGL contexts.
 */
class Pipe : public eq::Pipe
{
public:

    /**
     * @param parent Parent node that owns the pipe.
     */
    Pipe( eq::Node* parent );

    /**
     * @return The texture upload processor.
     */
    TextureUploadProcessorPtr getTextureUploadProcessor( );

    /**
     * @return The const texture upload processor.
     */
    ConstTextureUploadProcessorPtr getTextureUploadProcessor( ) const;

    /**
     * @return The data upload processor.
     */
    DataUploadProcessorPtr getDataUploadProcessor( );

    /**
     * @return The const data upload processor.
     */
    ConstDataUploadProcessorPtr getDataUploadProcessor( ) const;

    /**
     * @return The dash processor that is shared between channels.
     */
    DashProcessorPtr getProcessor();

    /**
     * @return The const dash processor that is shared between channels.
     */
    ConstDashProcessorPtr getProcessor() const;

    virtual ~Pipe();

    /**
     * Starts the upload processors.
     */
    void startUploadProcessors_( );

    /**
     * Stops the upload processors.
     */
    void stopUploadProcessors_( );

    /**
     * @return The frame data.
     */
    ConstFrameDataPtr getFrameData( ) const;

private:

    void initalizeMapping_();
    void releaseMapping_();

    void initializeCaches_();
    void releaseCaches_();

    void initializePipelineProcessors_();
    void releasePipelineProcessors_();

    void initializePipelineConnections_();
    void releasePipelineConnections_();

    void mapFrameData_( const eq::uint128_t& initId );
    void unmapFrameData_( );

    void frameStart( const eq::uint128_t& frameId,
                     const uint32_t frameNumber ) final;
    bool configInit( const eq::uint128_t& initId ) final;
    bool configExit() final;

    TextureUploadProcessorPtr textureUploadProcessorPtr_;
    DataUploadProcessorPtr dataUploadProcessorPtr_;
    TextureCachePtr textureCachePtr_;
    DashProcessorPtr dashProcessorPtr_;
    FrameDataPtr frameDataPtr_;
};

}

#endif //_Pipe_h_
