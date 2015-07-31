
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *               2013, Ahmet Bilgili        <ahmet.bilgili@epfl.ch>
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

#include <livre/lib/types.h>
#include <livre/core/mathTypes.h>

#include <co/serializable.h>
#include <lunchbox/thread.h>

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
     * Resets the camera state.
     */
    void reset();

    /**
     * Sets the initial camera position, used also when reset() is called.
     * @param position camera position in world space.
     */
    void setDefaultCameraPosition( const Vector3f& position );

    /**
     * Sets the initial camera lookAt, used also when reset() is called.
     * @param lookAt camera lookAt vector in world space.
     */
    void setDefaultCameraLookAt( const Vector3f& lookAt );

    /**
     * Spins the camera around ( x, y )
     * @param x x position.
     * @param y y position.
     */
    void spinCamera( const float x, const float y );

    /**
     * Spins the camera around (x,y,z) axis.
     * @param x x component.
     * @param y y component.
     * @param z z component.
     */
    void spinModel( const float x, const float y, const float z );

    /**
     * Sets the rotation of the model.
     * @param rotation Rotation in x, y an z angles.
     */
    void setModelRotation( const Vector3f& rotation );

    /**
     * Moves the camera to (x,y,z) amount.
     * @param x x amount.
     * @param y y amount.
     * @param z z amount.
     */
    void moveCamera( const float x, const float y, const float z );

    /**
     * Resets the spin of camera.
     */
    void resetCameraSpin();

    /**
     * Sets the camera rotation.
     * @param rotation Rotation in x, y an z angles.
     */
    void setCameraRotation( const Vector3f& rotation );

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
     * @return true if pilot mode is on.
     */
    bool getPilotMode() const;

    /**
     * Sets the pilot mode.
     * @param pilotMode Pilot mode toggle.
     */
    void setPilotMode( bool pilotMode );

    /**
     * Inverts the pilot mode.
     */
    void togglePilotMode( );

    /**
     * @return camera rotation matrix.
     */
    const Matrix4f& getCameraRotation() const { return cameraRotation_; }

    /**
     * @return Returns model rotation matrix.
     */
    const Matrix4f& getModelRotation() const { return modelRotation_; }

    /**
     * @return Returns the camera position in x,y and z.
     */
    const Vector3f& getCameraPosition() const { return cameraPosition_; }

    /**
     * @return the amount of spin.
     */
    float getCameraSpin() const { return cameraSpin_; }

    /**
     * @return the translation length for the last position.
     */
    float getCameraTranslation() const { return cameraTranslation_; }

    /**
     * @return the modelview matrix is computed using the model and camera matrix.
     */
    Matrix4f getModelViewMatrix() const;

private:

    virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    Vector3f defaultCameraPosition_;
    Vector3f defaultCameraLookAt_;

    Matrix4f cameraRotation_;
    Matrix4f modelRotation_;
    Vector3f cameraPosition_;

    float cameraSpin_;
    float cameraTranslation_;

    bool pilotMode_;
};

}


#endif // _CameraSettings_h_
