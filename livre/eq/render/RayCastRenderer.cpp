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

#include <livre/core/data/DataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/render/GLSLShaders.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/TransferFunction1D.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/render/GLContext.h>

#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/DataCache.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/eq/FrameData.h>
#include <livre/eq/render/RayCastRenderer.h>
#include <livre/eq/render/shaders/fragRayCast.glsl.h>
#include <livre/eq/render/shaders/vertRayCast.glsl.h>
#include <livre/eq/render/shaders/fragTexCopy.glsl.h>
#include <livre/eq/render/shaders/vertTexCopy.glsl.h>
#include <livre/eq/settings/RenderSettings.h>

#include <eq/eq.h>
#include <eq/gl.h>

#define SH_UINT 0
#define SH_INT 1
#define SH_FLOAT 2

namespace livre
{

// Sort helper function for sorting the textures with their distances to viewpoint
struct DistanceOperator
{
    explicit DistanceOperator( const DataSource& dataSource, const Frustum& frustum )
        : _frustum( frustum )
        , _dataSource( dataSource )
    { }

    bool operator()( const NodeId& rb1,
                     const NodeId& rb2 )
    {
        const LODNode& lodNode1 = _dataSource.getNode( rb1 );
        const LODNode& lodNode2 = _dataSource.getNode( rb2 );

        const float distance1 = ( _frustum.getMVMatrix() *
                                  lodNode1.getWorldBox().getCenter() ).length();
        const float distance2 = ( _frustum.getMVMatrix() *
                                  lodNode2.getWorldBox().getCenter() ).length();
        return  distance1 < distance2;
    }
    const Frustum& _frustum;
    const DataSource& _dataSource;
};


#define glewGetContext() GLContext::getCurrent()->glewGetContext()

namespace
{
std::string where( const char* file, const int line )
{
    return std::string( " in " ) + std::string( file ) + ":" +
           boost::lexical_cast< std::string >( line );
}

const uint32_t maxSamplesPerRay = 32;
const uint32_t minSamplesPerRay = 512;

const GLfloat fullScreenQuad[] = { -1.0f, -1.0f, 0.0f,
                                    1.0f, -1.0f, 0.0f,
                                   -1.0f,  1.0f, 0.0f,
                                   -1.0f,  1.0f, 0.0f,
                                    1.0f, -1.0f, 0.0f,
                                    1.0f,  1.0f, 0.0f };

}

struct RayCastRenderer::Impl
{
    Impl( const TextureCache& textureCache,
          const uint32_t samplesPerRay,
          const uint32_t samplesPerPixel )
        : _renderTexture( GL_TEXTURE_RECTANGLE_ARB, glewGetContext( ))
        , _nSamplesPerRay( samplesPerRay )
        , _nSamplesPerPixel( samplesPerPixel )
        , _computedSamplesPerRay( samplesPerRay )
        , _transferFunctionTexture( 0 )
        , _textureCache( textureCache )
        , _dataSource( textureCache.getDataCache().getDataSource( ))
        , _volInfo( _dataSource.getVolumeInfo( ))
        , _posVBO( 0 )
    {
        TransferFunction1D transferFunction;
        initTransferFunction( transferFunction );

        int error = _rayCastShaders.loadShaders( ShaderData( vertRayCast_glsl,
                                                             fragRayCast_glsl ));

        if( error != GL_NO_ERROR )
            LBTHROW( std::runtime_error( "Can't load glsl shaders: " +
                                         eq::glError( error ) +
                                         where( __FILE__, __LINE__ )));

        error = _texCopyShaders.loadShaders( ShaderData( vertTexCopy_glsl,
                                                         fragTexCopy_glsl ));

        if( error != GL_NO_ERROR )
            LBTHROW( std::runtime_error( "Can't load glsl shaders: " +
                                         eq::glError( error ) +
                                         where( __FILE__, __LINE__ )));

        // Create QUAD VBO
        glGenBuffers( 1, &_quadVBO );
        glBindBuffer( GL_ARRAY_BUFFER, _quadVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof( fullScreenQuad ), fullScreenQuad, GL_STATIC_DRAW );
    }

    ~Impl()
    {
        _renderTexture.flush();
        glDeleteBuffers( 1, &_quadVBO );
    }

    NodeIds order( const NodeIds& bricks,
                   const Frustum& frustum ) const
    {
        NodeIds rbs = bricks;
        DistanceOperator distanceOp( _dataSource, frustum );
        std::sort( rbs.begin(), rbs.end(), distanceOp );
        return rbs;
    }

    void update( const FrameData& frameData )
    {
        initTransferFunction( frameData.getRenderSettings().getTransferFunction( ));
        _nSamplesPerRay = frameData.getVRParameters().getSamplesPerRay();
        _computedSamplesPerRay = _nSamplesPerRay;
        _nSamplesPerPixel = frameData.getVRParameters().getSamplesPerPixel();
    }

    void initTransferFunction( const TransferFunction1D& transferFunction )
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

        const uint8_t* transferFunctionData = transferFunction.getLut();
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA,
                       GLsizei( transferFunction.getLutSize() / 4u ), 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, transferFunctionData );
    }

    void createAndInitializeRenderTexture( const Viewport& viewport )
    {
        const int width = viewport[ 2 ] - viewport[ 0 ];
        const int height = viewport[ 3 ] - viewport[ 1 ];
        if( _renderTexture.getWidth() != width || _renderTexture.getHeight() != height )
        {
            _renderTexture.flush();
            _renderTexture.init( GL_RGBA32F, width, height );
            const Floats emptyBuffer( _renderTexture.getWidth() * _renderTexture.getHeight() * 4,
                                      0.0 );
            _renderTexture.upload( _renderTexture.getWidth(),
                                   _renderTexture.getHeight(),
                                   emptyBuffer.data( ));
        }
    }

    void onFrameStart( const Frustum& frustum,
                       const ClipPlanes& planes,
                       const NodeIds& renderBricks )
    {
        if( _nSamplesPerRay == 0 ) // Find sampling rate
        {
            uint32_t maxLOD = 0;
            for( const NodeId& rb : renderBricks )
            {
                const LODNode& lodNode = _dataSource.getNode( rb );
                const uint32_t level = lodNode.getRefLevel();
                if( level > maxLOD )
                    maxLOD = level;
            }

            const float maxVoxelDim = _volInfo.voxels.find_max();
            const float maxVoxelsAtLOD = maxVoxelDim /
                    (float)( 1u << ( _volInfo.rootNode.getDepth() - maxLOD - 1 ));
            // Nyquist limited nb of samples according to voxel size
            _computedSamplesPerRay = std::max( maxVoxelsAtLOD, (float)minSamplesPerRay );
        }

        glDisable( GL_LIGHTING );
        glEnable( GL_CULL_FACE );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_BLEND );
        glGetIntegerv( GL_DRAW_BUFFER, &_drawBuffer );
        glDrawBuffer( GL_NONE );


        GLSLShaders::Handle program = _rayCastShaders.getProgram( );
        LBASSERT( program );

        // Enable shaders
        glUseProgram( program );
        GLint tParamNameGL;

        tParamNameGL = glGetUniformLocation( program, "invProjectionMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvProjMatrix( ).array );

        tParamNameGL = glGetUniformLocation( program, "invModelViewMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvMVMatrix( ).array );

        tParamNameGL = glGetUniformLocation( program, "modelViewProjectionMatrix" );
        glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getMVPMatrix( ).array );

        // Because the volume is centered to the origin we can compute the volume AABB by using
        // the volume total size.
        const Vector3f halfWorldSize = _volInfo.worldSize / 2.0;

        tParamNameGL = glGetUniformLocation( program, "globalAABBMin" );
        glUniform3fv( tParamNameGL, 1, ( -halfWorldSize ).array );

        tParamNameGL = glGetUniformLocation( program, "globalAABBMax" );
        glUniform3fv( tParamNameGL, 1, ( halfWorldSize ).array );

        Vector4i viewport;
        glGetIntegerv( GL_VIEWPORT, viewport.array );
        tParamNameGL = glGetUniformLocation( program, "viewport" );
        glUniform4iv( tParamNameGL, 1, viewport.array );

        tParamNameGL = glGetUniformLocation( program, "depthRange" );

        Vector2f depthRange;
        glGetFloatv( GL_DEPTH_RANGE, depthRange.array );
        glUniform2fv( tParamNameGL, 1, depthRange.array );

        tParamNameGL = glGetUniformLocation( program, "worldEyePosition" );
        glUniform3fv( tParamNameGL, 1, frustum.getEyePos( ).array );

        tParamNameGL = glGetUniformLocation( program, "nSamplesPerRay" );
        glUniform1i( tParamNameGL, _computedSamplesPerRay );

        tParamNameGL = glGetUniformLocation( program, "maxSamplesPerRay" );
        glUniform1i( tParamNameGL, maxSamplesPerRay );

        tParamNameGL = glGetUniformLocation( program, "nSamplesPerPixel" );
        glUniform1i( tParamNameGL, _nSamplesPerPixel );

        tParamNameGL = glGetUniformLocation( program, "nearPlaneDist" );
        glUniform1f( tParamNameGL, frustum.nearPlane( ));

        const auto& clipPlanes = planes.getPlanes();
        const size_t nPlanes = clipPlanes.size();
        tParamNameGL = glGetUniformLocation( program, "nClipPlanes" );
        glUniform1i( tParamNameGL, nPlanes );

        switch( _dataSource.getVolumeInfo().dataType )
        {
            case DT_UINT8:
            case DT_UINT16:
            case DT_UINT32:
                tParamNameGL = glGetUniformLocation( program, "datatype" );
                glUniform1ui( tParamNameGL, SH_UINT );
                break;
            case DT_FLOAT:
                tParamNameGL = glGetUniformLocation( program, "datatype" );
                glUniform1ui( tParamNameGL, SH_FLOAT );
                break;
            case DT_INT8:
            case DT_INT16:
            case DT_INT32:
                tParamNameGL = glGetUniformLocation( program, "datatype" );
                glUniform1ui( tParamNameGL, SH_INT );
                break;
            case DT_UNDEFINED:
            default:
                LBTHROW( std::runtime_error( "Unsupported type in the shader." ));
                break;
        }

        // This is temporary. In the future it will be given by the gui.
        Vector2f dataSourceRange( 0.0f, 255.0f );
        tParamNameGL = glGetUniformLocation( program, "dataSourceRange" );
        glUniform2fv( tParamNameGL, 1, dataSourceRange.array );

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

        createAndFillVertexBuffer( renderBricks );
        createAndInitializeRenderTexture( viewport );

        glBindImageTexture( 0, _renderTexture.getName(),
                            0, GL_FALSE, 0, GL_READ_WRITE,
                            _renderTexture.getInternalFormat( ));

        tParamNameGL = glGetUniformLocation( program, "renderTexture" );
        glUniform1i( tParamNameGL, 0 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_1D, _transferFunctionTexture );
        tParamNameGL = glGetUniformLocation( program, "transferFnTex" );
        glUniform1i( tParamNameGL, 1 );

        // Disable shader
        glUseProgram( 0 );
    }

    void createAndFillVertexBuffer( const NodeIds& renderBricks )
    {
        Vector3fs positions;
        positions.reserve( 36 * renderBricks.size( ));
        for( const NodeId& rb: renderBricks )
        {
            const LODNode& lodNode = _dataSource.getNode( rb );
            createBrick( lodNode, positions );
        }

        glGenBuffers( 1, &_posVBO );
        glBindBuffer( GL_ARRAY_BUFFER, _posVBO );
        glBufferData( GL_ARRAY_BUFFER,
                      positions.size() * 3 * sizeof( float ),
                      positions.data(), GL_STATIC_DRAW );
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

    void onFrameRender( const NodeIds& bricks )
    {
        size_t index = 0;
        for( const NodeId& brick: bricks )
            renderBrick( brick, index++ );
    }

    void renderBrickVBO( const size_t index, bool front, bool back )
    {
        if( !front && !back )
            return;

        else if( front && !back )
            glCullFace( GL_BACK );

        else if( !front && back )
            glCullFace( GL_FRONT );

        glBindBuffer( GL_ARRAY_BUFFER, _posVBO );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
        glDrawArrays( GL_TRIANGLES, index * 36, 36 );
    }

    void renderBrick( const NodeId& rb,
                      const size_t index )
    {
        GLSLShaders::Handle program = _rayCastShaders.getProgram( );
        LBASSERT( program );

        // Enable shaders
        glUseProgram( program );

        const ConstTextureObjectPtr textureObj =
                std::static_pointer_cast< const TextureObject >( _textureCache.get( rb.getId( )));
        const ConstTextureStatePtr& texState = textureObj->getTextureState();
        const LODNode& lodNode = _dataSource.getNode( rb );

        if( texState->textureId == INVALID_TEXTURE_ID )
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
        glUniform3fv( tParamNameGL, 1, texState->textureCoordsMin.array );

        tParamNameGL = glGetUniformLocation( program, "textureMax" );
        glUniform3fv( tParamNameGL, 1, texState->textureCoordsMax.array );

        const Vector3f& voxSize = texState->textureSize / lodNode.getWorldBox().getSize();
        tParamNameGL = glGetUniformLocation( program, "voxelSpacePerWorldSpace" );
        glUniform3fv( tParamNameGL, 1, voxSize.array );

        glActiveTexture( GL_TEXTURE0 );
        texState->bind();
        tParamNameGL = glGetUniformLocation( program, "volumeTexUint8" );
        glUniform1i( tParamNameGL, 0 );

        tParamNameGL = glGetUniformLocation( program, "volumeTexFloat" );
        glUniform1i( tParamNameGL, 0 );

        const uint32_t refLevel = lodNode.getRefLevel();

        tParamNameGL = glGetUniformLocation( program, "refLevel" );
        glUniform1i( tParamNameGL, refLevel );

        _usedTextures[1].push_back( texState->textureId );

        renderBrickVBO( index, false /* draw front */, true /* cull back */ );
        glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

        glUseProgram( 0 );
    }

    void copyTexToFrameBufAndClear()
    {
        GLSLShaders::Handle program = _texCopyShaders.getProgram( );
        LBASSERT( program );

        glUseProgram( program );
        glBindImageTexture( 0, _renderTexture.getName(),
                            0, GL_FALSE, 0, GL_READ_WRITE,
                            _renderTexture.getInternalFormat( ));
        GLint tParamNameGL = glGetUniformLocation( program, "renderTexture" );
        glUniform1i( tParamNameGL, 0 );

        glBindBuffer( GL_ARRAY_BUFFER, _quadVBO );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

        glDisable( GL_CULL_FACE );
        glDrawArrays( GL_TRIANGLES, 0, 6 );

        glUseProgram( 0 );
    }

    void onFrameEnd()
    {
        std::sort( _usedTextures[1].begin(), _usedTextures[1].end( ));
#ifdef LIVRE_DEBUG_RENDERING
        if( _usedTextures[0] != _usedTextures[1] )
        {
            std::cout << "Rendered ";
            std::copy( _usedTextures[1].begin(), _usedTextures[1].end(),
                       std::ostream_iterator< uint32_t >( std::cout, " " ));
            std::cout << " in " << (void*)this << std::endl;
        }
#endif
        _usedTextures[0].swap( _usedTextures[1] );
        _usedTextures[1].clear();

        glDeleteBuffers( 1, &_posVBO );
        glDrawBuffer( _drawBuffer );
        copyTexToFrameBufAndClear();
    }

    eq::util::Texture _renderTexture;
    GLSLShaders _rayCastShaders;
    GLSLShaders _texCopyShaders;
    uint32_t _nSamplesPerRay;
    uint32_t _nSamplesPerPixel;
    uint32_t _computedSamplesPerRay;
    uint32_t _transferFunctionTexture;
    std::vector< uint32_t > _usedTextures[2]; // last, current frame
    const TextureCache& _textureCache;
    const DataSource& _dataSource;
    const VolumeInformation& _volInfo;
    GLuint _posVBO;
    GLuint _quadVBO;
    GLint _drawBuffer;
};

RayCastRenderer::RayCastRenderer( const TextureCache& textureCache,
                                  const uint32_t samplesPerRay,
                                  const uint32_t samplesPerPixel )
    : _impl( new RayCastRenderer::Impl( textureCache, samplesPerRay, samplesPerPixel ))
{}

RayCastRenderer::~RayCastRenderer()
{}

void RayCastRenderer::update( const FrameData& frameData )
{
    _impl->update( frameData );
}


NodeIds RayCastRenderer::_order( const NodeIds& bricks,
                                 const Frustum& frustum ) const
{
    return _impl->order( bricks, frustum );
}

void RayCastRenderer::_onFrameStart( const Frustum& frustum,
                                     const ClipPlanes& planes,
                                     const PixelViewport&,
                                     const NodeIds& renderBricks )
{
    _impl->onFrameStart( frustum, planes, renderBricks );
}

void RayCastRenderer::_onFrameRender( const Frustum&,
                                      const ClipPlanes&,
                                      const PixelViewport&,
                                      const NodeIds& orderedBricks )
{
    _impl->onFrameRender( orderedBricks );
}

void RayCastRenderer::_onFrameEnd( const Frustum&,
                                   const ClipPlanes&,
                                   const PixelViewport&,
                                   const NodeIds& )
{
    _impl->onFrameEnd();
}

size_t RayCastRenderer::getNumBricksUsed() const
{
    return _impl->_usedTextures[0].size();
}

}
