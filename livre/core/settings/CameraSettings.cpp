
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

#include <livre/core/settings/CameraSettings.h>

#include <algorithm>

namespace livre
{

CameraSettings::CameraSettings()
    : _modelview( Matrix4f( ))
{}

void CameraSettings::spinModel( const float x, const float y )
{
    if( x == 0.f && y == 0.f )
        return;

    Matrix4f modelView = _modelview.get();

    float translation[3];
    translation[0] = modelView(0,3);
    translation[1] = modelView(1,3);
    translation[2] = modelView(2,3);

    modelView(0,3) = 0.0;
    modelView(1,3) = 0.0;
    modelView(2,3) = 0.0;

    modelView.pre_rotate_x( x );
    modelView.pre_rotate_y( y );

    modelView(0,3) = translation[0];
    modelView(1,3) = translation[1];
    modelView(2,3) = translation[2];

    _modelview = modelView;
}

void CameraSettings::moveCamera( const float x, const float y, const float z )
{
    auto modelView = _modelview.get();

    modelView(0,3) += x;
    modelView(1,3) += y;
    modelView(2,3) += z;

    _modelview = modelView;
}

void CameraSettings::setCameraPosition( const Vector3f& pos )
{
    auto modelView = _modelview.get();

    modelView(0,3) = pos.x();
    modelView(1,3) = pos.y();
    modelView(2,3) = pos.z();

    _modelview = modelView;
}

void CameraSettings::setCameraLookAt( const Vector3f& lookAt )
{
    const Vector3f eye( _modelview.get()(0,3),
                        _modelview.get()(1,3),
                        _modelview.get()(2,3));
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
}

void CameraSettings::setModelViewMatrix( const Matrix4f& modelview )
{
    _modelview = modelview;
}

Matrix4f CameraSettings::getModelViewMatrix() const
{
    return _modelview.get();
}
}
