/* Copyright (c) 2006-2016, Daniel.Nachbaur@epfl.ch
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

#include <livre/eq/types.h>

#include <memory>

namespace livre
{
namespace zeroeq
{

/**
 * Communicator using ZeroEQ to setup publishers and subscribers for event
 * exchange with other applications.
 * @version 0.4
 */
class Communicator
{
public:
    Communicator( Config& config, int argc, char** argv );
    ~Communicator();

    /**
     * Publish EVENT_IMAGEJPEG on the REST schema.
     *
     * @param data pointer to image pixels
     * @param size size of image pixels pointer
     */
    void publishImageJPEG( const uint8_t* data, const uint64_t size );

    /** Publish frame (start, current, end, delta) parameters */
    void publishFrame();

    /**
     * Publish Camera
     * @param modelview the livre space modelview matrix
     */
    void publishCamera( const Matrix4f& modelview );

    /** Publish a serializable object */
    bool publish( const servus::Serializable& serializable );

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
