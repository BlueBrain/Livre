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

#include <livre/core/data/VolumeInformation.h>
#include <livre/core/render/GLSLShaders.h>
#include <livre/core/render/RenderBrick.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/TransferFunction1D.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/maths/maths.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/render/GLWidget.h>
#include <livre/core/render/View.h>

#include <livre/eq/render/shaders/vertRayCast.glsl.h>
#include <livre/eq/render/shaders/fragRayCast.glsl.h>
#include <livre/eq/render/RayCastRenderer.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{

#define glewGetContext() GLContext::glewGetContext()

namespace
{
std::string where( const char* file, const int line )
{
    return std::string( " in " ) + std::string( file ) + ":" +
           boost::lexical_cast< std::string >( line );
}
}

struct RayCastRenderer::Impl
{
    Impl( uint32_t samplesPerRay,
          uint32_t samplesPerPixel,
          const VolumeInformation& volInfo )
        :  _framebufferTexture(
            new eq::util::Texture( GL_TEXTURE_RECTANGLE_ARB, glewGetContext( )))
        , _shaders( new GLSLShaders )
        , _nSamplesPerRay( samplesPerRay )
        , _nSamplesPerPixel( samplesPerPixel )
        , _computedSamplePerRay( samplesPerRay )
        , _volInfo( volInfo )
        , _transferFunctionTexture( 0 )
    {
        TransferFunction1D< unsigned char > transferFunction;
        initTransferFunction( transferFunction );

        // TODO: Add the shaders from resource directory
        const int error = _shaders->loadShaders( ShaderData( vertRayCast_glsl,
                                                             fragRayCast_glsl ));
        if( error != GL_NO_ERROR )
            LBTHROW( std::runtime_error( "Can't load glsl shaders: " +
                                         eq::glError( error ) +
                                         where( __FILE__, __LINE__ )));
    }

    ~Impl()
    {
        _framebufferTexture->flush();
    }

    void initTransferFunction(
        const TransferFunction1Dc& transferFunction )
    {
        assert( transferFunction.getNumChannels() == 4u );

        if( _transferFunctionTexture == 0 )
        {
            GLuint tfTexture = 0;
            glGenTextures( 1, &tfTexture );
            glBindTexture( GL_TEXTURE_1D, tfTexture );

            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            _transferFunctionTexture  = tfTexture;
        }
        glBindTexture( GL_TEXTURE_1D, _transferFunctionTexture );

        const UInt8Vector& transferFunctionData = transferFunction.getData();
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA, GLsizei(transferFunctionData.size()/4u), 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, &transferFunctionData[ 0 ] );
    }

    void readFromFrameBuffer( const GLWidget& glWidget,
                              const View& view )
    {
        const Viewport& viewport = glWidget.getViewport( view );
        const eq::PixelViewport pvp( 0, 0, viewport[2], viewport[3] );
        _framebufferTexture->copyFromFrameBuffer( GL_RGBA, pvp );
    }

    void onFrameStart( const GLWidget& glWidget LB_UNUSED,
                       const View& view LB_UNUSED,
                       const Frustum& frustum,
                       const RenderBricks& renderBricks )
    {
    #ifdef LIVRE_DEBUG_RENDERING
        if( _usedTextures[0] != _usedTextures[1] )
        {
            std::cout << "Render ";
            std::copy( _usedTextures[1].begin(), _usedTextures[1].end(),
                       std::ostream_iterator< uint32_t >( std::cout, " " ));
            std::cout << std::endl;
        }
        _usedTextures[0].swap( _usedTextures[1] );
        _usedTextures[1].clear();
    #endif

        if( _nSamplesPerRay == 0 ) // Find sampling rate
        {
            uint32_t maxLOD = 0;
            for( const RenderBrickPtr& rb : renderBricks )
            {
                const uint32_t level = rb->getLODNode()->getRefLevel();
                if( level > maxLOD )
                    maxLOD = level;
            }

            const float maxVoxelDim = _volInfo.voxels.find_max();
            const float maxVoxelsAtLOD = maxVoxelDim /
                    (float)( 1u << ( _volInfo.rootNode.getDepth() - maxLOD ));
            // Nyquist limited nb of samples according to voxel size
            _computedSamplePerRay = 2.0f * maxVoxelsAtLOD;
        }

        glDisable( GL_LIGHTING );
        glEnable( GL_CULL_FACE );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_BLEND );

        GLSLShaders::Handle program = _shaders->getProgram( );
        LBASSERT( program );

        // Enable shaders
        glUseProgram( program );
        GLint tParamNameGL;

        tParamNameGL = glGetUniformLocation( program, "invProjectionMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvProjectionMatrix( ).array );

        tParamNameGL = glGetUniformLocation( program, "invModelViewMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvModelViewMatrix( ).array );

        Vector4i viewport;
        glGetIntegerv( GL_VIEWPORT, viewport.array );
        tParamNameGL = glGetUniformLocation( program, "viewport" );
        glUniform4iv( tParamNameGL, 1, viewport.array );

        tParamNameGL = glGetUniformLocation( program, "depthRange" );

        Vector2f depthRange;
        glGetFloatv( GL_DEPTH_RANGE, depthRange.array );
        glUniform2fv( tParamNameGL, 1, depthRange.array );

        tParamNameGL = glGetUniformLocation( program, "worldEyePosition" );
        glUniform3fv( tParamNameGL, 1, frustum.getEyeCoords( ).array );

        tParamNameGL = glGetUniformLocation( program, "nSamplesPerRay" );
        glUniform1i( tParamNameGL, _computedSamplePerRay );

        tParamNameGL = glGetUniformLocation( program, "nSamplesPerPixel" );
        glUniform1i( tParamNameGL, _nSamplesPerPixel );

        tParamNameGL = glGetUniformLocation( program, "nearPlaneDist" );
        glUniform1f( tParamNameGL, frustum.getFrustumLimits( PL_NEAR ));

        // Disable shader
        glUseProgram( 0 );
    }


    void renderBrick( const GLWidget& glWidget,
                      const View& view,
                      const RenderBrick& rb )
    {
        GLSLShaders::Handle program = _shaders->getProgram( );
        LBASSERT( program );

        // Enable shaders
        glUseProgram( program );

        if( rb.getTextureState( )->textureId == INVALID_TEXTURE_ID )
        {
            LBERROR << "Invalid texture for node : " << rb.getLODNode( )->getNodeId( ) << std::endl;
            return;
        }

        GLint tParamNameGL = glGetUniformLocation( program, "aabbMin" );
        const ConstLODNodePtr& lodNodePtr = rb.getLODNode( );
        glUniform3fv( tParamNameGL, 1, lodNodePtr->getWorldBox( ).getMin( ).array );

        tParamNameGL = glGetUniformLocation( program, "aabbMax" );
        glUniform3fv( tParamNameGL, 1, lodNodePtr->getWorldBox( ).getMax( ).array );

        tParamNameGL = glGetUniformLocation( program, "textureMin" );
        ConstTextureStatePtr texState = rb.getTextureState( );
        glUniform3fv( tParamNameGL, 1, texState->textureCoordsMin.array );

        tParamNameGL = glGetUniformLocation( program, "textureMax" );
        glUniform3fv( tParamNameGL, 1, texState->textureCoordsMax.array );

        const Vector3f& voxSize =
                rb.getTextureState( )->textureSize / lodNodePtr->getWorldBox( ).getDimension( );
        tParamNameGL = glGetUniformLocation( program, "voxelSpacePerWorldSpace" );
        glUniform3fv( tParamNameGL, 1, voxSize.array );

        readFromFrameBuffer( glWidget, view );

        glActiveTexture( GL_TEXTURE2 );
        _framebufferTexture->bind( );
        _framebufferTexture->applyZoomFilter( eq::FILTER_LINEAR );
        _framebufferTexture->applyWrap( );

        tParamNameGL = glGetUniformLocation( program, "frameBufferTex" );
        glUniform1i( tParamNameGL, 2 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_1D, _transferFunctionTexture ); //preintegrated values
        tParamNameGL = glGetUniformLocation( program, "transferFnTex" );
        glUniform1i( tParamNameGL, 1 ); //f-shader

        glActiveTexture( GL_TEXTURE0 );
        texState->bind( );
        tParamNameGL = glGetUniformLocation( program, "volumeTex" );
        glUniform1i( tParamNameGL, 0 ); //f-shader

        const uint32_t refLevel = lodNodePtr->getRefLevel();

        tParamNameGL = glGetUniformLocation( program, "refLevel" );
        glUniform1i( tParamNameGL, refLevel );

    #ifdef LIVRE_DEBUG_RENDERING
        _usedTextures[1].push_back( texState->textureId );
    #endif
        rb.drawBrick( false /* draw front */, true /* cull back */ );

        glUseProgram( 0 );
    }

    EqTexturePtr _framebufferTexture;
    GLSLShadersPtr _shaders;
    const uint32_t _nSamplesPerRay;
    const uint32_t _nSamplesPerPixel;
    uint32_t _computedSamplePerRay;
    const VolumeInformation& _volInfo;
    uint32_t _transferFunctionTexture;
    std::vector< uint32_t > _usedTextures[2]; // last, current frame

};

RayCastRenderer::RayCastRenderer( uint32_t samplesPerRay,
                                  uint32_t samplesPerPixel,
                                  const VolumeInformation& volInfo,
                                  uint32_t gpuDataType,
                                  int32_t internalFormat )
    : Renderer( volInfo.compCount, gpuDataType, internalFormat ),
      _impl( new RayCastRenderer::Impl( samplesPerRay,
                                        samplesPerPixel,
                                        volInfo ))
{}

RayCastRenderer::~RayCastRenderer()
{}

void RayCastRenderer::initTransferFunction(
        const TransferFunction1Dc& transferFunction )
{
    _impl->initTransferFunction( transferFunction );
}

void RayCastRenderer::onFrameStart_( const GLWidget& glWidget,
                                     const View& view,
                                     const Frustum& frustum,
                                     const RenderBricks& renderBricks )
{
    _impl->onFrameStart( glWidget, view, frustum, renderBricks );
}


void RayCastRenderer::renderBrick_( const GLWidget& glWidget,
                                    const View& view,
                                    const Frustum&,
                                    const RenderBrick& renderBrick )
{
    _impl->renderBrick( glWidget, view, renderBrick );
}

}
