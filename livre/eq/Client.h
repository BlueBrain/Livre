/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
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

#ifndef _Client_h_
#define _Client_h_

#include <livre/eq/api.h>
#include <livre/eq/types.h>

#include <eq/client.h> // base class
#include <eq/view.h>   // nested type

namespace livre
{
typedef std::function<void()> IdleFunc;

/**
 * The client application class.
 */
class Client : public eq::Client
{
public:
    LIVREEQ_API Client(int argc, char* argv[]);
    LIVREEQ_API ~Client();

    /**
     * @param idleFunc function which is called every frame, e.g. for updating
     *                 information like frame ranges in data sources
     */
    LIVREEQ_API void setIdleFunction(const IdleFunc& idleFunc);

    /** @return the currently set idle function. */
    LIVREEQ_API const IdleFunc& getIdleFunction() const;

    /** Exit this client. Can't be done in dtor, it would never be reached due
     * to Eq ref-counting. */
    LIVREEQ_API void exit();

    /**
     * Run the render loop. Only returns when the application is finished, i.e.
     * maxFrames exceeded, ESC key pressed, etc.
     */
    LIVREEQ_API void run();

    /**
     * Render a frame and return its result in the provided callback.
     * Do not mix with run().
     *
     * @param func callback function containing the rendered image.
     * @return true if a redraw is required, e.g. data is still dirty/loaded.
     */
    LIVREEQ_API bool render(const eq::View::ScreenshotFunc& func);

    /** Resize the active layout aka windows/drawables/buffers */
    LIVREEQ_API void resize(const Vector2ui& size);

    /** @return the current histogram. */
    const Histogram& getHistogram() const;

    /** @return The per-frame data. */
    const FrameData& getFrameData() const;
    FrameData& getFrameData();

    /** @return the current volume information. */
    const VolumeInformation& getVolumeInformation() const;

    /** @return the application parameters */
    ApplicationParameters& getApplicationParameters();

protected:
    /** @override eq::Client::initLocal() */
    bool initLocal(const int argc, char* argv[]) final;

    /** Infinite loop on remote render client. */
    void clientLoop() final;

    /** @return the first active layout of this client. */
    eq::Layout* getActiveLayout();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}
#endif // _Client_h_
