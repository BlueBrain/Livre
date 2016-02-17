
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#ifndef _CameraSettings_h_
#define _CameraSettings_h_

#include <livre/core/mathTypes.h>
#include <co/serializable.h>
#include <eq/fabric/vmmlib.h>

namespace livre
{
/**
 * The CameraSettings class is the information sent from app node to the clients for camera operations
 */
class CameraSettings : public co::Serializable
{
public:
    /**
     * CameraInfo constructor.
     */
    CameraSettings();

    /**
     * Spins the camera around (x,y,z) axis.
     * @param x x component.
     * @param y y component.
     * @param z z component.
     */
    void spinModel( float x, float y, float z );

    /**
     * Moves the camera to (x,y,z) amount.
     * @param x x amount.
     * @param y y amount.
     * @param z z amount.
     */
    void moveCamera( float x, float y, float z );

    /**
     * Sets the camera position.
     * @param position
     */
    void setCameraPosition( const Vector3f& position );

    /**
     * Sets the camera orientation, defined by a lookAt vector.
     * @param lookAt the reference point position to orient the camera to.
     */
    void setCameraLookAt( const Vector3f& lookAt );

    /**
     * Sets the modelviewmatrix.
     * @param modelviewMatrix
     */
    void setModelViewMatrix( const Matrix4f& modelViewMatrix );

    /**
     * @return Returns model rotation matrix.
     */
    const Matrix4f& getModelRotation() const { return modelRotation_; }

    /**
     * @return Returns the camera position in x,y and z.
     */
    const Vector3f& getCameraPosition() const { return cameraPosition_; }

    /**
     * @return the modelview matrix is computed using the model and camera matrix.
     */
    Matrix4f getModelViewMatrix() const;

private:
    void serialize( co::DataOStream& os, const uint64_t dirtyBits ) final;
    void deserialize( co::DataIStream& is, const uint64_t dirtyBits ) final;

    Matrix4f modelRotation_;
    Vector3f cameraPosition_;
};

}


#endif // _CameraSettings_h_
