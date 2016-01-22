
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
 *                          Daniel.Nachbaur@epfl.ch
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

#include <livre/eq/settings/CameraSettings.h>

#include <livre/core/maths/maths.h>
#include <co/co.h>

namespace livre
{

CameraSettings::CameraSettings()
{}

void CameraSettings::spinModel( const float x, const float y )
{
    if( x == 0.f && y == 0.f )
        return;

    ::zerobuf::render::Vector3f& pos = getOrigin();
    ::zerobuf::render::Vector3f& lookat = getLookAt();

    Vector3f look( pos.getX() - lookat.getX(), pos.getY() - lookat.getY(),
                   pos.getZ() - lookat.getZ( ));
    const float length = look.length();

    const Vector3f up( getUp().getX(),  getUp().getY(),  getUp().getZ( ));
    const Vector3f left = vmml::cross( look, up );
    std::cout << *this;
    look.rotate( -x, up );
    look.rotate( -y, left );
    look *= length / look.length();

    pos.setX( look.x() + lookat.getX( ));
    pos.setY( look.y() + lookat.getY( ));
    pos.setZ( look.z() + lookat.getZ( ));
    std::cout << " -> " << *this << std::endl;
}

void CameraSettings::moveCamera( const float x, const float y, const float z )
{
    ::zerobuf::render::Vector3f& pos = getOrigin();
    pos.setX( pos.getX() + x );
    pos.setY( pos.getY() + y );
    pos.setZ( pos.getZ() + z );

    ::zerobuf::render::Vector3f& lookat = getLookAt();
    lookat.setX( lookat.getX() + x );
    lookat.setY( lookat.getY() + y );
    lookat.setZ( lookat.getZ() + z );
    std::cout << pos << std::endl;
}

void CameraSettings::setCameraPosition( const Vector3f& pos )
{
    setOrigin( ::zerobuf::render::Vector3f( pos.x(), pos.y(), pos.z( )));
}

void CameraSettings::setCameraLookAt( const Vector3f& lookAt )
{
    setLookAt( ::zerobuf::render::Vector3f( lookAt.x(), lookAt.y(),
                                            lookAt.z( )));
}

Matrix4f CameraSettings::computeMatrix() const
{
    // see 'man gluLookAt'
    Vector3f f( getLookAt().getX() - getOrigin().getX(),
                getLookAt().getY() - getOrigin().getY(),
                getLookAt().getZ() - getOrigin().getZ( ));
    f.normalize();

    Vector3f up( getUp().getX(), getUp().getY(), getUp().getZ( ));
    up.normalize();

    const Vector3f s = vmml::cross( f, up );
    const Vector3f u = vmml::cross( s, f );
    const float matrix[16] = { s.x(), u.x(), -f.x(), 0.f,
                               s.y(), u.y(), -f.y(), 0.f,
                               s.z(), u.z(), -f.z(), 0.f,
                               0.f, 0.f, 0.f, 1.f };
    const Matrix4f rotation( matrix, matrix + 16 );
    Matrix4f translation;
    translation.set_translation( Vector3f( -getOrigin().getX(),
                                           -getOrigin().getY(),
                                           -getOrigin().getZ( )));

    const Matrix4f m = rotation * translation;
    std::cout << *this << " = " << m << std::endl
              << std::atan( m.at( 3, 2 ) / m.at( 3, 3 )) << std::endl
              << -std::asin( m.at( 3, 1 )) << std::endl
              << std::atan( m.at( 2, 1 ) / m.at( 1, 1 )) << std::endl;
    return m;
}
}
