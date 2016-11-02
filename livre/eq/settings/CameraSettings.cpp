
/* Copyright (c) 2006-2016, Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
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

#include <co/co.h>

#include <algorithm>

namespace livre
{

CameraSettings::CameraSettings()
    : _notifyChangedFunc([&]( const Matrix4f& ){})
{}

void CameraSettings::serialize( co::DataOStream& os, uint64_t )
{
    os << _modelview;
}

void CameraSettings::deserialize( co::DataIStream& is, uint64_t )
{
    is >> _modelview;
}

void CameraSettings::spinModel( const float x, const float y )
{
    if( x == 0.f && y == 0.f )
        return;

    float translation[3];
    translation[0] = _modelview(0,3);
    translation[1] = _modelview(1,3);
    translation[2] = _modelview(2,3);

    _modelview(0,3) = 0.0;
    _modelview(1,3) = 0.0;
    _modelview(2,3) = 0.0;

    _modelview.pre_rotate_x( x );
    _modelview.pre_rotate_y( y );

    _modelview(0,3) = translation[0];
    _modelview(1,3) = translation[1];
    _modelview(2,3) = translation[2];

    setDirty( DIRTY_ALL );
    _notifyChangedFunc( _modelview );
}

void CameraSettings::moveCamera( const float x, const float y, const float z )
{
    _modelview(0,3) += x;
    _modelview(1,3) += y;
    _modelview(2,3) += z;

    setDirty( DIRTY_ALL );
    _notifyChangedFunc( _modelview );
}

void CameraSettings::setCameraPosition( const Vector3f& pos )
{
    _modelview(0,3) = pos.x();
    _modelview(1,3) = pos.y();
    _modelview(2,3) = pos.z();

    setDirty( DIRTY_ALL );
    _notifyChangedFunc( _modelview );
}

void CameraSettings::registerNotifyChanged( const std::function< void( const Matrix4f& )>&
                                            notifyChangedFunc )
{
     _notifyChangedFunc = notifyChangedFunc;
}

void CameraSettings::setCameraLookAt( const Vector3f& lookAt )
{
    const Vector3f eye( _modelview(0,3),
                        _modelview(1,3),
                        _modelview(2,3));
    const Vector3f zAxis = vmml::normalize( eye - lookAt );

    // Avoid Gimbal lock effect when looking upwards/downwards
    Vector3f up( Vector3f::UP );
    const float angle = zAxis.dot( up );
    if( 1.f - std::abs( angle ) < 0.0001f )
    {
        Vector3f right( Vector3f::RIGHT );
        if( angle > 0 ) // Looking downwards
            right = Vector3f::LEFT;
        up = up.rotate( 0.01f, right );
        up.normalize();
    }

    _modelview = Matrix4f( eye, lookAt, up );

    setDirty( DIRTY_ALL );
    _notifyChangedFunc( _modelview );
}

void CameraSettings::setModelViewMatrix( const Matrix4f& modelview )
{
    _modelview = modelview;
     setDirty( DIRTY_ALL );
}

Matrix4f CameraSettings::getModelViewMatrix() const
{
    return _modelview;
}
}
