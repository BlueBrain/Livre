/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/Render/GLSLShaders.h>
#include <livre/core/Render/RenderBrick.h>
#include <livre/core/Render/Frustum.h>
#include <livre/core/Render/TransferFunction1D.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Maths/Maths.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/core/Render/Viewport.h>
#include <livre/core/Render/View.h>
#include <livre/core/Render/gl.h>

#include <livre/Eq/Render/Shaders/vertRayCast.glsl.h>
#include <livre/Eq/Render/Shaders/fragRayCast.glsl.h>
#include <livre/Eq/Render/RayCastRenderer.h>

#include <eq/eq.h>

namespace livre
{
namespace
{
std::string where( const char* file, const int line )
{
    return std::string( " in " ) + std::string( file ) + ":" +
           boost::lexical_cast< std::string >( line );
}
}

RayCastRenderer::RayCastRenderer( const GLEWContext* glewContext,
                                  const uint32_t samples,
                                  const uint32_t componentCount,
                                  const GLenum gpuDataType,
                                  const GLint internalFormat )
    : Renderer( componentCount, gpuDataType, internalFormat )
    , nSamples_( samples )
    , transferFunctionTexture_( 0 )
{
    if( !glewContext )
        LBTHROW( std::runtime_error( "No GLEW context given" +
                                     where( __FILE__, __LINE__ )));

    TransferFunction1D< unsigned char > transferFunction;
    initTransferFunction( transferFunction );

    shadersPtr_.reset( new GLSLShaders );

    // TODO: Add the shaders from resource directory
    const int error = shadersPtr_->loadShaders( ShaderData( vertRayCast_glsl,
                                                            fragRayCast_glsl ));
    if( error != GL_NO_ERROR )
        LBTHROW( std::runtime_error( "Can't load glsl shaders: " +
                                     eq::glError( error ) +
                                     where( __FILE__, __LINE__ )));

    framebufferTempTexturePtr_.reset(
        new eq::util::Texture( GL_TEXTURE_RECTANGLE_ARB, glewContext ));
}

RayCastRenderer::~RayCastRenderer()
{
    framebufferTempTexturePtr_->flush();
}

// TODO : a templated initTF function for different kinds of tf ( float, short, etc )
void RayCastRenderer::initTransferFunction( const TransferFunction1D< uint8_t >& transferFunction )
{
    const UInt8Vector& transferFunctionData = transferFunction.getData();

    assert( transferFunction.getNumChannels() == 4u );

    if( transferFunctionTexture_ == 0 )
    {
        GLuint tfTexture = 0;
        glGenTextures( 1, &tfTexture );
        glBindTexture( GL_TEXTURE_1D, tfTexture );

        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glBindTexture( GL_TEXTURE_1D, tfTexture);
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA, transferFunctionData.size( )/4u, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, &transferFunctionData[ 0 ] );

        transferFunctionTexture_  = tfTexture;
    }
    else
    {
        glBindTexture( GL_TEXTURE_1D, transferFunctionTexture_ );
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA, transferFunctionData.size( )/4u, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, &transferFunctionData[ 0 ] );
    }
}

bool RayCastRenderer::readFromFrameBuffer_( const GLWidget& glWidget,
                                            const View& view,
                                            const Frustum &frustum,
                                            const RenderBrick& renderBrick,
                                            Vector2i& screenPos )
{
    Viewporti pixViewport;
    glWidget.setViewport( &view, pixViewport );

    Vector2i minPos;
    Vector2i maxPos;

    renderBrick.getScreenCoordinates( frustum, pixViewport, minPos, maxPos );

    const Vector2i texSize = maxPos - minPos;

    const eq::fabric::PixelViewport pixelViewPort( minPos[ 0 ], minPos[ 1 ],
                                                   texSize[ 0 ],  texSize[ 1 ] );
    if( !pixelViewPort.hasArea( ) )
        return false;

    framebufferTempTexturePtr_->copyFromFrameBuffer( GL_RGBA, pixelViewPort );

    screenPos = minPos;

    return true;
}

void RayCastRenderer::onFrameStart_( const GLWidget& glWidget LB_UNUSED,
                                     const View& view LB_UNUSED,
                                     const Frustum &frustum,
                                     const RenderBricks& )
{
#ifdef LIVRE_DEBUG_RENDERING
    if( usedTextures_[0] != usedTextures_[1] )
    {
        std::cout << "Render ";
        std::copy( usedTextures_[1].begin(), usedTextures_[1].end(),
                   std::ostream_iterator< uint32_t >( std::cout, " " ));
        std::cout << std::endl;
    }
    usedTextures_[0].swap( usedTextures_[1] );
    usedTextures_[1].clear();
#endif

    glDisable( GL_LIGHTING );
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );

    GLSLShaders::Handle shader = shadersPtr_->getProgram( );
    LBASSERT( shader );

    // Enable shaders
    glUseProgramObjectARB( shader );
    GLint tParamNameGL;

    tParamNameGL = glGetUniformLocationARB( shader, "invProjectionMatrix" );
    glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvProjectionMatrix( ).array );

    tParamNameGL = glGetUniformLocationARB( shader, "invModelViewMatrix" );
    glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvModelViewMatrix( ).array );

    Vector4i viewport;
    glGetIntegerv( GL_VIEWPORT, viewport.array );
    tParamNameGL = glGetUniformLocationARB( shader, "viewport" );
    glUniform4ivARB( tParamNameGL, 1, viewport.array );

    tParamNameGL = glGetUniformLocationARB( shader, "depthRange" );

    Vector2f depthRange;
    glGetFloatv( GL_DEPTH_RANGE, depthRange.array );
    glUniform2fvARB( tParamNameGL, 1, depthRange.array );

    tParamNameGL = glGetUniformLocationARB( shader, "worldEyePosition" );
    glUniform3fvARB( tParamNameGL, 1, frustum.getEyeCoords( ).array );

    // Disable shader
    glUseProgramObjectARB( 0 );
}


void RayCastRenderer::renderBrick_( const GLWidget& glWidget,
                                    const View& view,
                                    const Frustum &frustum,
                                    const RenderBrick& renderBrick )
{
    GLhandleARB shader = shadersPtr_->getProgram( );
    LBASSERT( shader );

    // Enable shaders
    glUseProgramObjectARB( shader );

    if( renderBrick.getTextureState( )->textureId == INVALID_TEXTURE_ID )
    {
        LBERROR << "Invalid texture for node : " << renderBrick.getLODNode( )->getNodeId( ) << std::endl;
        return;
    }

    GLint tParamNameGL = glGetUniformLocationARB( shader, "aabbMin" );
    const ConstLODNodePtr& lodNodePtr = renderBrick.getLODNode( );
    glUniform3fvARB( tParamNameGL, 1, lodNodePtr->getWorldBox( ).getMin( ).array );

    tParamNameGL = glGetUniformLocationARB( shader, "aabbMax" );
    glUniform3fvARB( tParamNameGL, 1, lodNodePtr->getWorldBox( ).getMax( ).array );

    tParamNameGL = glGetUniformLocationARB( shader, "textureMin" );
    ConstTextureStatePtr texState = renderBrick.getTextureState( );
    glUniform3fvARB( tParamNameGL, 1, texState->textureCoordsMin.array );

    tParamNameGL = glGetUniformLocationARB( shader, "textureMax" );
    glUniform3fvARB( tParamNameGL, 1, texState->textureCoordsMax.array );

    const Vector3f voxSize = renderBrick.getTextureState( )->textureSize / lodNodePtr->getWorldBox( ).getDimension( );
    tParamNameGL = glGetUniformLocationARB( shader, "voxelSpacePerWorldSpace" );
    glUniform3fvARB( tParamNameGL, 1, voxSize.array );

    // Read from buffer and getthe position screen
    Vector2i screenPos;
    if( !readFromFrameBuffer_( glWidget, view, frustum, renderBrick, screenPos ))
    {
        glUseProgramObjectARB( 0 );
        return;
    }

    // Put data to the shader
    tParamNameGL = glGetUniformLocationARB( shader, "screenPos" );
    glUniform2ivARB( tParamNameGL, 1, screenPos.array );

    glActiveTextureARB( GL_TEXTURE2 );
    framebufferTempTexturePtr_->bind( );
    framebufferTempTexturePtr_->applyZoomFilter( eq::FILTER_LINEAR );
    framebufferTempTexturePtr_->applyWrap( );

    tParamNameGL = glGetUniformLocationARB( shader, "frameBufferTex" );
    glUniform1iARB( tParamNameGL, 2 );

    glActiveTextureARB( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_1D, transferFunctionTexture_ ); //preintegrated values
    tParamNameGL = glGetUniformLocationARB( shader, "transferFnTex" );
    glUniform1iARB( tParamNameGL, 1 ); //f-shader

    glActiveTextureARB( GL_TEXTURE0 );
    texState->bind( );
    tParamNameGL = glGetUniformLocationARB( shader, "volumeTex" );
    glUniform1iARB( tParamNameGL, 0 ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader, "nSamples" );
    glUniform1iARB( tParamNameGL, nSamples_ );

    const uint32_t refLevel = renderBrick.getLODNode( )->getRefLevel( );

    tParamNameGL = glGetUniformLocationARB(  shader, "refLevel" );
    glUniform1iARB( tParamNameGL, refLevel );

#ifdef LIVRE_DEBUG_RENDERING
    usedTextures_[1].push_back( texState->textureId );
#endif
    renderBrick.drawBrick( true /* draw front */, false /* cull back */ );

    glUseProgramObjectARB( 0 );
}

}
