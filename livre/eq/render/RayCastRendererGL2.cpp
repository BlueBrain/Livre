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

#include <livre/eq/FrameData.h>
#include <livre/eq/render/RayCastRenderer.h>
#include <livre/eq/render/shaders/fragRayCastGL2.glsl.h>
#include <livre/eq/render/shaders/vertRayCast.glsl.h>
#include <livre/eq/settings/RenderSettings.h>

#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/render/GLSLShaders.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/TransferFunction1D.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/render/GLContext.h>

#include <eq/eq.h>
#include <eq/gl.h>

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

}

struct RayCastRenderer::Impl
{
    Impl( const DataSource& dataSource,
          const Cache& textureCache,
          const uint32_t samplesPerRay,
          const uint32_t samplesPerPixel )
        : _framebufferTexture( GL_TEXTURE_RECTANGLE_ARB, glewGetContext( ))
        , _nSamplesPerRay( samplesPerRay )
        , _nSamplesPerPixel( samplesPerPixel )
        , _computedSamplesPerRay( samplesPerRay )
        , _transferFunctionTexture( 0 )
        , _textureCache( textureCache )
        , _dataSource( dataSource )
        , _volInfo( _dataSource.getVolumeInfo( ))
        , _posVBO( 0 )
    {
        TransferFunction1D transferFunction;
        initTransferFunction( transferFunction );

        // TODO: Add the shaders from resource directory
        const int error = _shaders.loadShaders( ShaderData( vertRayCast_glsl,
                                                            fragRayCastGL2_glsl ));
        if( error != GL_NO_ERROR )
            LBTHROW( std::runtime_error( "Can't load glsl shaders: " +
                                         eq::glError( error ) +
                                         where( __FILE__, __LINE__ )));
    }

    ~Impl()
    {
        _framebufferTexture.flush();
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

    void readFromFrameBuffer( const PixelViewport& viewport )
    {
        const eq::PixelViewport pvp( 0, 0, viewport[2], viewport[3] );
        _framebufferTexture.copyFromFrameBuffer( GL_RGBA, pvp );
    }

    void onFrameStart( const Frustum& frustum,
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

        GLSLShaders::Handle program = _shaders.getProgram( );
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

        // Disable shader
        glUseProgram( 0 );

        createAndFillVertexBuffer( renderBricks );
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

    void createBrick( const LODNode& lodNode, Vector3fs& positions  ) const
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

    void onFrameRender( const PixelViewport& view,
                        const NodeIds& bricks )
    {
        size_t index = 0;
        for( const NodeId& brick: bricks )
            renderBrick( view, brick, index++ );
    }

    void renderVBO( const size_t index, bool front, bool back )
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

    void renderBrick( const PixelViewport& viewport,
                      const NodeId& rb,
                      const size_t index )
    {
        GLSLShaders::Handle program = _shaders.getProgram( );
        LBASSERT( program );

        // Enable shaders
        glUseProgram( program );

        const ConstTextureObjectPtr textureObj =
                std::static_pointer_cast< const TextureObject >( _textureCache.get( rb.getId( )));
        const TextureState& texState = textureObj->getTextureState();
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
        glUniform3fv( tParamNameGL, 1, texState.textureCoordsMin.array );

        tParamNameGL = glGetUniformLocation( program, "textureMax" );
        glUniform3fv( tParamNameGL, 1, texState.textureCoordsMax.array );

        const Vector3f& voxSize = texState.textureSize / lodNode.getWorldBox().getSize();
        tParamNameGL = glGetUniformLocation( program, "voxelSpacePerWorldSpace" );
        glUniform3fv( tParamNameGL, 1, voxSize.array );

        readFromFrameBuffer( viewport );

        glActiveTexture( GL_TEXTURE2 );
        _framebufferTexture.bind( );
        _framebufferTexture.applyZoomFilter( eq::FILTER_LINEAR );
        _framebufferTexture.applyWrap( );

        tParamNameGL = glGetUniformLocation( program, "frameBufferTex" );
        glUniform1i( tParamNameGL, 2 );

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_1D, _transferFunctionTexture ); //preintegrated values
        tParamNameGL = glGetUniformLocation( program, "transferFnTex" );
        glUniform1i( tParamNameGL, 1 ); //f-shader

        glActiveTexture( GL_TEXTURE0 );
        texState.bind( );
        tParamNameGL = glGetUniformLocation( program, "volumeTex" );
        glUniform1i( tParamNameGL, 0 ); //f-shader

        const uint32_t refLevel = lodNode.getRefLevel();

        tParamNameGL = glGetUniformLocation( program, "refLevel" );
        glUniform1i( tParamNameGL, refLevel );

        _usedTextures[1].push_back( texState->textureId );

        renderVBO( index, false /* draw front */, true /* cull back */ );

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
    }

    eq::util::Texture _framebufferTexture;
    GLSLShaders _shaders;
    uint32_t _nSamplesPerRay;
    uint32_t _nSamplesPerPixel;
    uint32_t _computedSamplesPerRay;
    uint32_t _transferFunctionTexture;
    std::vector< uint32_t > _usedTextures[2]; // last, current frame
    const Cache& _textureCache;
    const DataSource& _dataSource;
    const VolumeInformation& _volInfo;
    GLuint _posVBO;
};

RayCastRenderer::RayCastRenderer( const DataSource& dataSource,
                                  const Cache& textureCache,
                                  const uint32_t samplesPerRay,
                                  const uint32_t samplesPerPixel )
    : _impl( new RayCastRenderer::Impl( dataSource,
                                        textureCache,
                                        samplesPerRay,
                                        samplesPerPixel ))
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

void RayCastRenderer::_onFrameStart(  const Frustum& frustum,
                                      const ClipPlanes&,
                                      const PixelViewport&,
                                      const NodeIds& renderBricks )
{
    _impl->onFrameStart( frustum, renderBricks );
}

void RayCastRenderer::_onFrameRender( const Frustum&,
                                      const ClipPlanes&,
                                      const PixelViewport& view,
                                      const NodeIds& orderedBricks )
{
    _impl->onFrameRender( view, orderedBricks );
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
