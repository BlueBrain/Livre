/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include "GLRaycastRenderer.h"
#include <livre/core/render/RenderInputs.h>
#include <livre/core/settings/ApplicationSettings.h>
#include <livre/core/settings/RenderSettings.h>

#include <livre/core/configuration/RendererParameters.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/render/GLSLShaders.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/TextureState.h>
#include <livre/core/data/LODNode.h>


#include <lexis/render/ColorMap.h>

#include <lunchbox/pluginRegisterer.h>
#include <GL/glew.h>

namespace livre
{
namespace
{
lunchbox::PluginRegisterer< GLRaycastRenderer > registerer;
}

// Sort helper function for sorting the textures with their distances to viewpoint
struct DistanceOperator
{
    explicit DistanceOperator( const DataSource& dataSource, const Frustum& frustum )
        : _frustum( frustum )
        , _dataSource( dataSource )
    { }

    bool operator()( const ConstCacheObjectPtr& rb1, const ConstCacheObjectPtr& rb2 )
    {
        const LODNode& lodNode1 = _dataSource.getNode( NodeId( rb1->getId( )));
        const LODNode& lodNode2 = _dataSource.getNode( NodeId( rb2->getId( )));

        const float distance1 = ( _frustum.getMVMatrix() *
                                  lodNode1.getWorldBox().getCenter() ).length();
        const float distance2 = ( _frustum.getMVMatrix() *
                                  lodNode2.getWorldBox().getCenter() ).length();
        return  distance1 < distance2;
    }
    const Frustum& _frustum;
    const DataSource& _dataSource;
};

namespace
{
const std::string vertRayCastFile = "shaders/vertRaycast.glsl";
const std::string fragRayCastFile = "shaders/fragRaycast.glsl";
const std::string vertTexCopyFile = "shaders/vertTexCopy.glsl";
const std::string fragTexCopyFile = "shaders/fragTexCopy.glsl";

const uint32_t maxSamplesPerRay = 32;
const uint32_t minSamplesPerRay = 512;
const size_t nVerticesRenderBrick = 36;
const GLfloat fullScreenQuad[] = { -1.0f, -1.0f, 0.0f,
                                    1.0f, -1.0f, 0.0f,
                                   -1.0f,  1.0f, 0.0f,
                                   -1.0f,  1.0f, 0.0f,
                                    1.0f, -1.0f, 0.0f,
                                    1.0f,  1.0f, 0.0f };
const uint32_t SH_UINT = 0u;
const uint32_t SH_INT = 1u;
const uint32_t SH_FLOAT = 2u;
}

struct RenderTexture
{
    RenderTexture()
        : texture( -1u )
        , width( 0 )
        , height( 0 )
        , target(GL_TEXTURE_RECTANGLE_ARB )
        , internalFormat( GL_RGBA32F )
        , format( GL_RGBA )
        , type( GL_FLOAT )
    {
    }

    ~RenderTexture()
    {
        if( texture != -1u )
            glDeleteTextures( 1, &texture );
    }

    void resize( const size_t width_, const size_t height_ )
    {
        if( width_ == width && height_ == height )
            return;

        width = width_;
        height = height_;

        if( texture != -1u )
            glDeleteTextures( 1, &texture );

        glGenTextures( 1, &texture );
        glBindTexture( target, texture );
        glTexImage2D( target, 0, internalFormat, width, height, 0, format, type, 0 );

        const Floats emptyBuffer( width * height * 4, 0.0 );
        glTexSubImage2D( target, 0, 0, 0, width, height,
                         format, type, emptyBuffer.data( ));


        const int ret = glGetError();
        if( ret != GL_NO_ERROR )
            LBTHROW( std::runtime_error( "Error resizing render texture" ));
    }

    GLuint texture;
    size_t width;
    size_t height;
    const GLenum target;
    GLuint internalFormat;
    GLuint format;
    GLuint type;

};

struct GLRaycastRenderer::Impl
{
    Impl()
        : _colorMapTexture( 0 )
    {
        // Create QUAD VBO
        glGenBuffers( 1, &_quadVBO );
        glBindBuffer( GL_ARRAY_BUFFER, _quadVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof( fullScreenQuad ), fullScreenQuad, GL_STATIC_DRAW );

        initColorMap( lexis::render::ColorMap::getDefaultColorMap( 0.0f, 256.0f ));
    }

    ~Impl()
    {
        glDeleteBuffers( 1, &_quadVBO );
        glDeleteTextures( 1, &_colorMapTexture );
    }

    void initShaders( const RenderInputs& renderInputs )
    {
        if( _rayCastShaders )
            return;

        const auto& resourceFolders = renderInputs.appSettings.getResourceFolders();
        _rayCastShaders.reset( new GLSLShaders(
                      ShaderFiles( resourceFolders, vertRayCastFile, fragRayCastFile, "" )));
        _texCopyShaders.reset( new GLSLShaders(
                      ShaderFiles( resourceFolders, vertTexCopyFile, fragTexCopyFile, "" )));
    }

    void initColorMap( const lexis::render::ColorMap colorMap )
    {
        if( _colorMapTexture == 0 )
        {
            glGenTextures( 1, &_colorMapTexture );
            glBindTexture( GL_TEXTURE_1D, _colorMapTexture );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        }

        glBindTexture( GL_TEXTURE_1D, _colorMapTexture );

        _colors = colorMap.sampleColors< uint8_t >( 256, 0.0f, 256.0f, 0 );
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA,
                       GLsizei( _colors.size( )), 0,
                       GL_RGBA, GL_UNSIGNED_BYTE,
                       reinterpret_cast< uint8_t* >( _colors.data( )));
    }

    void resizeRenderTexture( const Viewport& viewport )
    {
        const size_t width = viewport[ 2 ] - viewport[ 0 ];
        const size_t height = viewport[ 3 ] - viewport[ 1 ];
        _renderTexture.resize( width, height );
    }

    uint32_t getShaderDataType( const VolumeInformation& volInfo ) const
    {
        switch( volInfo.dataType )
        {
            case DT_UINT8:
            case DT_UINT16:
            case DT_UINT32:
                return SH_UINT;
            case DT_FLOAT:
                return SH_FLOAT;
            case DT_INT8:
            case DT_INT16:
            case DT_INT32:
                return SH_INT;
            case DT_UNDEFINED:
            default:
                LBTHROW( std::runtime_error( "Unsupported type in the shader." ));
        }
    }

    void preRender( const RenderInputs& renderInputs, const ConstCacheObjects& renderData )
    {
        initColorMap( renderInputs.renderSettings.getColorMap( ));
        initShaders( renderInputs );
        _computedSamplesPerRay = renderInputs.vrParameters.getSamplesPerRay();

        const DataSource& dataSource = renderInputs.dataSource;
        const VolumeInformation& volInfo = dataSource.getVolumeInfo();
        const Frustum& frustum = renderInputs.frameInfo.frustum;

        if( renderInputs.vrParameters.getSamplesPerRay() == 0 ) // Find sampling rate
        {
            uint32_t maxLOD = 0;
            for( const ConstCacheObjectPtr& rd : renderData )
            {
                const LODNode& lodNode = dataSource.getNode( NodeId( rd->getId( )));
                const uint32_t level = lodNode.getRefLevel();
                if( level > maxLOD )
                    maxLOD = level;
            }

            const float maxVoxelDim = volInfo.voxels.find_max();
            const float maxVoxelsAtLOD = maxVoxelDim /
                    (float)( 1u << ( volInfo.rootNode.getDepth() - maxLOD - 1 ));
            // Nyquist limited nb of samples according to voxel size
            _computedSamplesPerRay = std::max( maxVoxelsAtLOD, (float)minSamplesPerRay );
        }

        glDisable( GL_LIGHTING );
        glEnable( GL_CULL_FACE );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_BLEND );
        glGetIntegerv( GL_DRAW_BUFFER, &_drawBuffer );
        glDrawBuffer( GL_NONE );

        const GLuint program = _rayCastShaders->getProgram();

        // Enable shaders
        glUseProgram( _rayCastShaders->getProgram( ));
        GLint tParamNameGL;

        tParamNameGL = glGetUniformLocation( program, "invProjectionMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvProjMatrix().array );

        tParamNameGL = glGetUniformLocation( program, "invModelViewMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvMVMatrix().array );

        tParamNameGL = glGetUniformLocation( program, "modelViewProjectionMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getMVPMatrix().array );

          // Because the volume is centered to the origin we can compute the volume AABB by using
        // the volume total size.
        const Vector3f halfWorldSize = volInfo.worldSize / 2.0;

        tParamNameGL = glGetUniformLocation( program, "globalAABBMin" );
        glUniform3fv( tParamNameGL, 1, ( -halfWorldSize ).array );

        tParamNameGL = glGetUniformLocation( program, "globalAABBMax" );
        glUniform3fv( tParamNameGL, 1, ( halfWorldSize ).array );

        Vector4i viewport;
        glGetIntegerv( GL_VIEWPORT, viewport.array );
        tParamNameGL = glGetUniformLocation( program, "viewport" );
        glUniform4iv( tParamNameGL, 1, viewport.array );

        tParamNameGL = glGetUniformLocation( program, "worldEyePosition" );
        glUniform3fv( tParamNameGL, 1, frustum.getEyePos( ).array );

        tParamNameGL = glGetUniformLocation( program, "nSamplesPerRay" );
        glUniform1i( tParamNameGL, _computedSamplesPerRay );

        tParamNameGL = glGetUniformLocation( program, "maxSamplesPerRay" );
        glUniform1i( tParamNameGL, maxSamplesPerRay );

        tParamNameGL = glGetUniformLocation( program, "nSamplesPerPixel" );
        glUniform1i( tParamNameGL, renderInputs.vrParameters.getSamplesPerPixel( ));

        tParamNameGL = glGetUniformLocation( program, "nearPlaneDist" );
        glUniform1f( tParamNameGL, frustum.nearPlane( ));

        const auto& clipPlanes = renderInputs.renderSettings.getClipPlanes().getPlanes();
        const size_t nPlanes = clipPlanes.size();
        tParamNameGL = glGetUniformLocation( program, "nClipPlanes" );
        glUniform1i( tParamNameGL, nPlanes );

        tParamNameGL = glGetUniformLocation( program, "datatype" );
        glUniform1ui( tParamNameGL, getShaderDataType( volInfo ));

        // This is temporary. In the future it will be given by the gui.
        tParamNameGL = glGetUniformLocation( program, "dataSourceRange" );
        glUniform2fv( tParamNameGL, 1, renderInputs.dataSourceRange.array );

        if( nPlanes > 0 )
        {
            Floats planesData;
            planesData.reserve( 4 * nPlanes );
            for( size_t i = 0; i < nPlanes; ++i )
            {
                const ::lexis::render::Plane& plane = clipPlanes[ i ];
                const float* normal = plane.getNormal();
                planesData.push_back( normal[ 0 ]);
                planesData.push_back( normal[ 1 ]);
                planesData.push_back( normal[ 2 ]);
                planesData.push_back( plane.getD( ));
            }

            tParamNameGL = glGetUniformLocation( program, "clipPlanes" );
            glUniform4fv( tParamNameGL, nPlanes, planesData.data( ));
        }

        resizeRenderTexture( viewport );

        glBindImageTexture( 0, _renderTexture.texture,
                            0, GL_FALSE, 0, GL_READ_WRITE,
                            _renderTexture.internalFormat );

        tParamNameGL = glGetUniformLocation( program, "renderTexture" );
        glUniform1i( tParamNameGL, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_1D, _colorMapTexture );
        tParamNameGL = glGetUniformLocation( program, "transferFnTex" );
        glUniform1i( tParamNameGL, 1 );

        // Disable shader
        glUseProgram( 0 );
    }

    GLuint createAndFillVertexBuffer( const DataSource& dataSource,
                                      const ConstCacheObjects& renderBricks ) const
    {
        Vector3fs positions;
        positions.reserve( nVerticesRenderBrick * renderBricks.size( ));
        for( const auto& rb: renderBricks )
        {
            const LODNode& lodNode = dataSource.getNode( NodeId( rb->getId( )));
            createBrick( lodNode, positions );
        }

        GLuint posVBO;
        glGenBuffers( 1, &posVBO );
        glBindBuffer( GL_ARRAY_BUFFER, posVBO );
        glBufferData( GL_ARRAY_BUFFER,
                      positions.size() * 3 * sizeof( float ),
                      positions.data(), GL_STATIC_DRAW );
        return posVBO;
    }

    void createBrick( const LODNode& lodNode, Vector3fs& positions ) const
    {
        const Boxf& worldBox = lodNode.getWorldBox();
        const Vector3f& minPos = worldBox.getMin();
        const Vector3f& maxPos = worldBox.getMax();

        // BACK
        positions.emplace_back( maxPos[0], minPos[1], minPos[2] );
        positions.emplace_back( minPos[0], minPos[1], minPos[2] );
        positions.emplace_back( minPos[0], maxPos[1], minPos[2] );

        positions.emplace_back( minPos[0], maxPos[1], minPos[2] );
        positions.emplace_back( maxPos[0], maxPos[1], minPos[2] );
        positions.emplace_back( maxPos[0], minPos[1], minPos[2] );

        // FRONT
        positions.emplace_back( maxPos[0], maxPos[1], maxPos[2] );
        positions.emplace_back( minPos[0], maxPos[1], maxPos[2] );
        positions.emplace_back( minPos[0], minPos[1], maxPos[2] );

        positions.emplace_back( minPos[0], minPos[1], maxPos[2] );
        positions.emplace_back( maxPos[0], minPos[1], maxPos[2] );
        positions.emplace_back( maxPos[0], maxPos[1], maxPos[2] );

        // LEFT
        positions.emplace_back( minPos[0], maxPos[1], minPos[2] );
        positions.emplace_back( minPos[0], minPos[1], minPos[2] );
        positions.emplace_back( minPos[0], minPos[1], maxPos[2] );

        positions.emplace_back( minPos[0], minPos[1], maxPos[2] );
        positions.emplace_back( minPos[0], maxPos[1], maxPos[2] );
        positions.emplace_back( minPos[0], maxPos[1], minPos[2] );

        // RIGHT
        positions.emplace_back( maxPos[0], maxPos[1], maxPos[2] );
        positions.emplace_back( maxPos[0], minPos[1], maxPos[2] );
        positions.emplace_back( maxPos[0], minPos[1], minPos[2] );

        positions.emplace_back( maxPos[0], minPos[1], minPos[2] );
        positions.emplace_back( maxPos[0], maxPos[1], minPos[2] );
        positions.emplace_back( maxPos[0], maxPos[1], maxPos[2] );

        // BOTTOM
        positions.emplace_back( maxPos[0], minPos[1], maxPos[2] );
        positions.emplace_back( minPos[0], minPos[1], maxPos[2] );
        positions.emplace_back( minPos[0], minPos[1], minPos[2] );

        positions.emplace_back( minPos[0], minPos[1], minPos[2] );
        positions.emplace_back( maxPos[0], minPos[1], minPos[2] );
        positions.emplace_back( maxPos[0], minPos[1], maxPos[2] );

        // TOP
        positions.emplace_back( maxPos[0], maxPos[1], minPos[2] );
        positions.emplace_back( minPos[0], maxPos[1], minPos[2] );
        positions.emplace_back( minPos[0], maxPos[1], maxPos[2] );

        positions.emplace_back( minPos[0], maxPos[1], maxPos[2] );
        positions.emplace_back( maxPos[0], maxPos[1], maxPos[2] );
        positions.emplace_back( maxPos[0], maxPos[1], minPos[2] );
    }

    void render( const RenderInputs& renderInputs, const ConstCacheObjects& renderData )
    {
        auto renderDataCopy = renderData;
        DistanceOperator distanceOp( renderInputs.dataSource, renderInputs.frameInfo.frustum );
        std::sort( renderDataCopy.begin(), renderDataCopy.end(), distanceOp );

        const GLuint posVBO = createAndFillVertexBuffer( renderInputs.dataSource, renderDataCopy );

        size_t index = 0;
        for( const auto& brick: renderDataCopy )
            renderBrick( renderInputs.dataSource, brick, index++, posVBO );

        glDeleteBuffers( 1, &posVBO );

        // The flush is needed because the textures are loaded asynchronously by a thread pool.
        glFlush();
    }

    void renderBrickVBO( const size_t index, const GLuint posVBO, bool front, bool back )
    {
        if( !front && !back )
            return;
        else if( front && !back )
            glCullFace( GL_BACK );
        else if( !front && back )
            glCullFace( GL_FRONT );

        glBindBuffer( GL_ARRAY_BUFFER, posVBO );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );


        glDrawArrays( GL_TRIANGLES, index * nVerticesRenderBrick, nVerticesRenderBrick );
    }

    void renderBrick( const DataSource& dataSource,
                      const ConstCacheObjectPtr& rb,
                      const size_t index,
                      const GLuint posVBO )
    {
        const GLuint program = _rayCastShaders->getProgram( );

        // Enable shaders
        glUseProgram( program );

        const ConstTextureObjectPtr textureObj =
                std::static_pointer_cast< const TextureObject >( rb );
        const TextureState& texState = textureObj->getTextureState();
        const LODNode& lodNode = dataSource.getNode( NodeId( rb ->getId( )));

        if( texState.textureId == INVALID_TEXTURE_ID )
        {
            LBERROR << "Invalid texture for node : "
                    << lodNode.getNodeId() << std::endl;
            return;
        }

        GLint tParamNameGL = glGetUniformLocation( program, "aabbMin" );
        glUniform3fv( tParamNameGL, 1, lodNode.getWorldBox().getMin().array );

        tParamNameGL = glGetUniformLocation( program, "aabbMax" );
        glUniform3fv( tParamNameGL, 1, lodNode.getWorldBox().getMax().array );

        tParamNameGL = glGetUniformLocation( program, "textureMin" );
        glUniform3fv( tParamNameGL, 1, texState.textureCoordsMin.array );

        tParamNameGL = glGetUniformLocation( program, "textureMax" );
        glUniform3fv( tParamNameGL, 1, texState.textureCoordsMax.array );

        glActiveTexture( GL_TEXTURE0 );
        texState.bind();
        tParamNameGL = glGetUniformLocation( program, "volumeTexUint8" );
        glUniform1i( tParamNameGL, 0 );

        tParamNameGL = glGetUniformLocation( program, "volumeTexFloat" );
        glUniform1i( tParamNameGL, 0 );

        const uint32_t refLevel = lodNode.getRefLevel();

        tParamNameGL = glGetUniformLocation( program, "refLevel" );
        glUniform1i( tParamNameGL, refLevel );

        renderBrickVBO( index, posVBO, false /* draw front */, true /* cull back */ );
        glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

        glUseProgram( 0 );
    }

    void copyTexToFrameBufAndClear()
    {
        const GLuint program = _texCopyShaders->getProgram();

        glUseProgram( program );
        glBindImageTexture( 0, _renderTexture.texture,
                            0, GL_FALSE, 0, GL_READ_WRITE,
                            _renderTexture.internalFormat );
        GLint tParamNameGL = glGetUniformLocation( program, "renderTexture" );
        glUniform1i( tParamNameGL, 0 );

        glBindBuffer( GL_ARRAY_BUFFER, _quadVBO );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

        glDisable( GL_CULL_FACE );
        glDrawArrays( GL_TRIANGLES, 0, 6 );

        glUseProgram( 0 );
    }

    void postRender()
    {
        glDrawBuffer( _drawBuffer );
        copyTexToFrameBufAndClear();
    }

    RenderTexture _renderTexture;
    uint32_t _computedSamplesPerRay;
    uint32_t _colorMapTexture;
    GLuint _quadVBO;
    GLint _drawBuffer;
    lexis::render::Colors< uint8_t > _colors;
    std::unique_ptr< GLSLShaders > _rayCastShaders;
    std::unique_ptr< GLSLShaders > _texCopyShaders;

};

GLRaycastRenderer::GLRaycastRenderer( const std::string& name )
    : RendererPlugin( name )
    , _impl( new GLRaycastRenderer::Impl( ))
{}

GLRaycastRenderer::~GLRaycastRenderer()
{}

void GLRaycastRenderer::preRender( const RenderInputs& renderInputs,
                                   const ConstCacheObjects& renderData )
{
    _impl->preRender( renderInputs, renderData );
}

void GLRaycastRenderer::render( const RenderInputs& renderInputs,
                                const ConstCacheObjects& renderData )
{
    _impl->render( renderInputs, renderData );
}

void GLRaycastRenderer::postRender( const RenderInputs&,
                                    const ConstCacheObjects& )
{
    _impl->postRender();
}

}
