
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

#ifndef _EqCameraSettings_h_
#define _EqCameraSettings_h_

#include <livre/core/mathTypes.h>
#include <livre/core/settings/CameraSettings.h>

#include <co/serializable.h>

namespace livre
{
/**
 * The CameraSettings class is the information sent from app node
 *  to the clients for camera operations
 */
class EqCameraSettings : public co::Serializable, public livre::CameraSettings
{
public:
    /** CameraInfo constructor. */
    EqCameraSettings();

    /**
     * Register a callback for camera changes.
     * @param notifyChangedFunc the callback function.
     */
    void registerNotifyChanged( const std::function< void( const Matrix4f& )>& notifyChangedFunc );

private:

    virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    std::function< void( const Matrix4f& )> _notifyChangedFunc;
};
}
#endif // _EqCameraSettings_h_
