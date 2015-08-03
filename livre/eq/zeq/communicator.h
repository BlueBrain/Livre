/* Copyright (c) 2006-2015, Daniel.Nachbaur@epfl.ch
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

#include <livre/core/mathTypes.h>
#include <livre/eq/types.h>

#include <memory>

namespace livre
{
namespace zeq
{

/**
 * Communicator using ZeroEQ to setup publishers and subscribers for event
 * exchange with other applications and RESTBridge.
 * @version 0.4
 */
class Communicator
{
public:
    explicit Communicator( Config& config );
    ~Communicator();

    /**
     * Publish EVENT_IMAGEJPEG on the REST schema.
     *
     * @param data pointer to image pixels
     * @param size size of image pixels pointer
     */
    void publishImageJPEG( const uint8_t* data, const uint64_t size );

    /**
     * Publish EVENT_CAMERA on the hbp schema.
     * @param modelView modelView matrix in micron
     */
    void publishModelView( const Matrix4f& modelView );

    /** Publish EVENT_HEARTBEAT on the REST schema. */
    void publishHeartbeat();

    /** Publish EVENT_EXIT on the REST schema. */
    void publishExit();

    /** Non-blocking receive & handle of all pending events. */
    void handleEvents();

private:
    class Impl;
    std::unique_ptr< Impl > _impl;

    Communicator( const Communicator& ) = delete;
    Communicator& operator=( const Communicator& ) = delete;
};

}
}
