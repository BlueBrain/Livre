/* Copyright (c) 2016, EPFL/Blue Brain Project
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include <livre/lib/data/BoundingAxis.h>

#include <livre/core/data/VolumeInformation.h>

#include <eq/gl.h>

namespace livre
{

enum LineType
{
    BBOX = 0,
    AXIS
};

BoundingAxis::BoundingAxis( const VolumeInformation& volInfo )
  : _nVertices( 0 )
{
    BoundingAxisData bbAxisData;
    bbAxisData.volInfo = volInfo;
    const Vector4f color( 0.5, 1.0, 0.69, 1.0 );

    const float x = volInfo.worldSize.x() * 0.5f;
    const float y = volInfo.worldSize.y() * 0.5f;
    const float z = volInfo.worldSize.z() * 0.5f;

    const float axisSizes[ 3 ] = { x, y, z };
    std::vector< Vector3f > vertices;
    std::vector< Vector3f > normals;

    for( int32_t i = -1; i < 2; i += 2 )
    {
        for( int32_t j = 0; j < 3; ++j )
        {
            for( int32_t k = 0; k < 4; ++k )
            {
                int32_t sign1 = (int32_t)( k / 2 ) * 2 - 1;
                int32_t sign2 = ((int32_t)(( k + 1 ) / 2 ) % 2 ) * 2 - 1;

                Vector3f vertex;
                Vector3f normal;
                int32_t axis = j;
                vertex[ axis ] = -i * axisSizes[ axis ];
                normal[ axis ] = i;

                axis = (j + 1) % 3;
                vertex[ axis ] = sign1 * axisSizes[ axis ];
                normal[ axis ] = 0.0;

                axis = (j + 2) % 3;
                vertex[ axis ] = sign2 * axisSizes[ axis ];
                normal[ axis ] = 0.0;

                vertices.push_back( vertex );
                normals.push_back( normal );

                if( k != 0 )
                {
                    vertices.push_back( vertex );
                    normals.push_back( normal );
                }

                if( k == 3 )
                {
                    vertices.push_back( vertices[ vertices.size() - 7 ]);
                    normals.push_back( normals[ normals.size() - 7 ]);
                }
            }
        }
    }

    // There is a possible optimization here, it is possible to append directly
    // in the code lines [ 50 - 89 ].
    for( size_t i = 0; i < vertices.size(); ++i )
    {
        _append3dVector( bbAxisData.vertices, vertices[i] );
        _append3dVector( bbAxisData.normals, normals[i] );
        _append3dVector( bbAxisData.normals2, normals[i] );
        _append4dVector( bbAxisData.colors, color );
        bbAxisData.types.push_back( BBOX );

        ++_nVertices;
    }

    _createTicks( bbAxisData );
    _sendDataToGPU( bbAxisData );
}

BoundingAxis::~BoundingAxis()
{
    glDeleteBuffers( 1, &_axisBufferId );
    glDeleteBuffers( 1, &_typeBufferId );
}

void BoundingAxis::_sendDataToGPU( const BoundingAxisData& bbAxisData )
{
    glGenBuffers( 1, &_axisBufferId );
    glBindBuffer( GL_ARRAY_BUFFER, _axisBufferId );

    size_t totalSize = sizeof(float) * _nVertices * 13;
    glBufferData( GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW );

    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(float) * bbAxisData.vertices.size(),
                     &bbAxisData.vertices[0] );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(float) * ( 3 * _nVertices ),
                     sizeof(float) * bbAxisData.normals.size(), &bbAxisData.normals[0] );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(float) * ( 6 * _nVertices ),
                     sizeof(float) * bbAxisData.normals2.size(), &bbAxisData.normals2[0] );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(float) * ( 9 * _nVertices ),
                     sizeof(float) * bbAxisData.colors.size(), &bbAxisData.colors[0] );

    glGenBuffers( 1, &_typeBufferId );
    glBindBuffer( GL_ARRAY_BUFFER, _typeBufferId );
    glBufferData( GL_ARRAY_BUFFER, sizeof(GLubyte) * bbAxisData.types.size(),
                  &bbAxisData.types[0], GL_STATIC_DRAW );
}

void BoundingAxis::draw()
{
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, _axisBufferId );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0,
                           (GLvoid*)( sizeof(float) * ( 3 * _nVertices )));

    glEnableVertexAttribArray( 2 );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0,
                           (GLvoid*)( sizeof(float) * ( 6 * _nVertices )));

    glEnableVertexAttribArray( 3 );
    glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, 0,
                           (GLvoid*)( sizeof(float) * ( 9 * _nVertices )));

    glEnableVertexAttribArray( 4 );
    glBindBuffer( GL_ARRAY_BUFFER, _typeBufferId );
    glVertexAttribIPointer( 4, 1, GL_UNSIGNED_BYTE, 0, 0 );

    glDrawArrays( GL_LINES, 0, _nVertices );

    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );
    glDisableVertexAttribArray( 2 );
    glDisableVertexAttribArray( 3 );
}

void BoundingAxis::_createTicks( BoundingAxisData& bbAxisData )
{
    const size_t longestBBAxis = bbAxisData.volInfo.worldSize.find_max_index();

    const Vector2f range = _computeRange( bbAxisData.volInfo, longestBBAxis );

    const uint32_t maxTickNumber = 50;
    const float tickSizes[4] = { 1, 2, 5, 10 };
    bbAxisData.tickSize = 0;

    const float rangeLength = range.y() - range.x();
    const float tickDistanceHint = rangeLength / maxTickNumber;

    // Order of magnitude. This is used to map the tick size into [1 - 10] range.
    bbAxisData.factor = std::pow( 10, std::floor( std::log10( tickDistanceHint )));
    const float normalizedTickDistance = tickDistanceHint / bbAxisData.factor;

    for( uint32_t i = 0; i < 4; ++i )
    {
        if( normalizedTickDistance <= tickSizes[i] )
        {
            bbAxisData.tickSize = tickSizes[i] * bbAxisData.factor;
            break;
        }
    }

    const float x = bbAxisData.volInfo.worldSize.x() * 0.5f;
    const float y = bbAxisData.volInfo.worldSize.y() * 0.5f;
    const float z = bbAxisData.volInfo.worldSize.z() * 0.5f;

//        ____A_____
//       /|        /|
//     B/ |H     D/ |
//     /__|__C___/  |G
//     |  |______|__|
//    E|  /   L  |  /
//     | /I     F| /K
//     |/________|/
//          J

    // edge J
    _createAxisTicks( { -x, -y, z }, { x, -y, z }, { 0, 1, 0 }, { 0, 0, -1 },
                       false, bbAxisData );
    _createAxisTicks( { -x, -y, z }, { x, -y, z }, { 0, 0, -1 }, { 0, 1, 0 },
                       true, bbAxisData );
    // edge F
    _createAxisTicks( { x, -y, z }, { x, y, z }, { -1, 0, 0 }, { 0, 0, -1 },
                       false, bbAxisData );
    _createAxisTicks( { x, -y, z }, { x, y, z }, { 0, 0, -1 }, { -1, 0, 0 },
                       true, bbAxisData );

    // edge C
    _createAxisTicks( { -x, y, z }, { x, y, z }, { 0, 0, -1 }, { 0, -1, 0 },
                       false, bbAxisData );
    _createAxisTicks( { -x, y, z }, { x, y, z }, { 0, -1, 0 }, { 0, 0, -1 },
                       true, bbAxisData );

    // edge E
    _createAxisTicks( { -x, -y, z }, { -x, y, z }, { 0, 0, -1 }, { 1, 0, -0 },
                       false, bbAxisData );
    _createAxisTicks( { -x, -y, z }, { -x, y, z }, { 1, 0, 0 }, { 0, 0, -1 },
                       true, bbAxisData );

    // edge L
    _createAxisTicks( { -x, -y, -z }, { x, -y, -z }, { 0, 1, 0 }, { 0, 0, 1},
                       true, bbAxisData );
    _createAxisTicks( { -x, -y, -z }, { x, -y, -z }, { 0, 0, 1 }, { 0, 1, 0 },
                       false, bbAxisData );

    // edge G
    _createAxisTicks( { x, -y, -z }, { x, y, -z }, { -1, 0, 0 }, { 0, 0, 1 },
                       true, bbAxisData );
    _createAxisTicks( { x, -y, -z }, { x, y, -z }, { 0, 0, 1 }, { -1, 0, 0 },
                       false, bbAxisData );

    // edge A
    _createAxisTicks( { -x, y, -z }, { x, y, -z }, { 0, 0, 1 }, { 0, -1, 0 },
                       true, bbAxisData );
    _createAxisTicks( { -x, y, -z }, { x, y, -z }, { 0, -1, 0 }, { 0, 0, 1 },
                       false, bbAxisData );

    // edge H
    _createAxisTicks( { -x, -y, -z }, { -x, y, -z }, { 0, 0, 1 }, { 1, 0, 0 },
                       true, bbAxisData );
    _createAxisTicks( { -x, -y, -z }, { -x, y, -z }, { 1, 0, 0 }, { 0, 0, 1 },
                       false, bbAxisData );

    // edge B
    _createAxisTicks( { -x, y, -z }, { -x, y, z }, { 0, -1, 0 }, { 1, 0, 0 },
                       true, bbAxisData );
    _createAxisTicks( { -x, y, -z }, { -x, y, z }, { 1, 0, 0 }, { 0, -1, 0 },
                       false, bbAxisData );

    // edge I
    _createAxisTicks( { -x, -y, -z }, { -x, -y, z }, { 0, 1, 0 }, { 1, 0, 0 },
                       false, bbAxisData );
    _createAxisTicks( { -x, -y, -z }, { -x, -y, z }, { 1, 0, 0 }, { 0, 1, 0 },
                       true, bbAxisData );

    // edge K
    _createAxisTicks( { x, -y, -z }, { x, -y, z }, { 0, 1, 0 }, { -1, 0, 0 },
                       true, bbAxisData );
    _createAxisTicks( { x, -y, -z }, { x, -y, z }, { -1, 0, 0 }, { 0, 1, 0 },
                       false, bbAxisData );

    // edge D
    _createAxisTicks( { x, y, -z }, { x, y, z }, { 0, -1, 0 }, { -1, 0, 0 },
                      false, bbAxisData );
    _createAxisTicks( { x, y, -z }, { x, y, z }, { -1, 0, 0 }, { 0, -1, 0 },
                       true, bbAxisData );
}

void BoundingAxis::_createAxisTicks( const Vector3f& start, const Vector3f& end,
                                     const Vector3f& normal, const Vector3f& normal2,
                                     bool flipTick, BoundingAxisData& bbAxisData )
{
    size_t currentAxis;
    if( Vector3f( end - start ).find_max_index() > std::numeric_limits<float>::min( ))
        currentAxis = Vector3f( end - start ).find_max_index();
    else
        currentAxis = Vector3f( end - start ).find_min_index();

    const Vector2f range = _computeRange( bbAxisData.volInfo, currentAxis );
    const float rangeLenght = range.y() - range.x();

    const float worldSpaceLenght = start.distance( end );
    const float dataToWorldScale = worldSpaceLenght / rangeLenght;
    const float startTickValue = bbAxisData.tickSize * std::ceil( range.x() / bbAxisData.tickSize );
    const float worldSpaceOffset = ( startTickValue - range.x( )) * dataToWorldScale;
    const float worldSpaceTickSize = bbAxisData.tickSize * dataToWorldScale;

    const int32_t numberOfTick = ( range.y() - startTickValue ) / bbAxisData.tickSize + 1;
    const Vector3f axisDir = vmml::normalize( end - start );

    Vector3f cross =  vmml::normalize( vmml::cross( axisDir, normal ));
    size_t tickAxis;
    if( cross.find_max() <= std::numeric_limits<float>::min( ))
        tickAxis = cross.find_min_index( );
    else
        tickAxis = cross.find_max_index( );

    for( int32_t i = 0; i < numberOfTick; ++i )
    {
        float tickHeight = 0.014f;
        Vector4f color( 1.0, 0.69, 0.69, 1.0 );
        const int32_t startTickValueInteger = std::round( startTickValue / bbAxisData.factor);
        const int32_t tickSizeInteger = std::round( bbAxisData.tickSize / bbAxisData.factor );

        if((( startTickValueInteger + tickSizeInteger * i ) % ( tickSizeInteger * 5 )) == 0 )
        {
            tickHeight *= 1.5;
            color = Vector4f( 1.0, 1.0, 0.69, 1.0 );
        }

        int sign;
        if( flipTick )
            sign = -(( 0.0f < cross[ tickAxis ]) - ( cross[ tickAxis ] < 0.0f ));
        else
            sign = ( 0.0f < cross[ tickAxis ]) - ( cross[ tickAxis ] < 0.0f );

        Vector3f posOnAxis = start + axisDir * ( worldSpaceOffset + worldSpaceTickSize * i );
        _append3dVector( bbAxisData.vertices, posOnAxis );
        posOnAxis[ tickAxis ] = posOnAxis[ tickAxis ] + sign * tickHeight;
        _append3dVector( bbAxisData.vertices, posOnAxis );

        for( uint32_t j = 0; j < 2; ++j )
        {
            _append3dVector( bbAxisData.normals, normal );
            _append3dVector( bbAxisData.normals2, normal2 );
            _append4dVector( bbAxisData.colors, color );
            bbAxisData.types.push_back( AXIS );
        }

        _nVertices += 2;
    }
}

const Vector2f BoundingAxis::_computeRange( const VolumeInformation& volInfo,
                                             const size_t axis ) const
{
    if( volInfo.resolution.x() == -1 )
        return Vector2f( 0.0f, volInfo.worldSize[ axis ]);
    else
        return Vector2f( 0.0f, volInfo.voxels[ axis ] * volInfo.resolution[ axis ]);
}

void BoundingAxis::_append3dVector( std::vector< float >& container, const Vector3f& vector )
{
    container.push_back( vector.x( ));
    container.push_back( vector.y( ));
    container.push_back( vector.z( ));
}

void BoundingAxis::_append4dVector( std::vector< float >& container, const Vector4f& vector )
{
    container.push_back( vector.x( ));
    container.push_back( vector.y( ));
    container.push_back( vector.z( ));
    container.push_back( vector.w( ));
}

}
