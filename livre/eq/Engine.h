/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                     bbp-open-source@googlegroups.com
 *                     Daniel.Nachbaur@epfl.ch
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

#ifndef _LivreEngine_h_
#define _LivreEngine_h_

#include <livre/eq/api.h>
#include <livre/eq/types.h>

#include <eq/view.h>

namespace livre
{
/**
 * The application class to create a Livre renderer which can run() in an
 * automatic render loop or from an external loop with explicit render() calls.
 */
class Engine
{
public:
    LIVREEQ_API Engine(int argc, char** argv);
    LIVREEQ_API ~Engine();

    /** @return Help string */
    LIVREEQ_API static std::string getHelp();

    /** @return Version string */
    LIVREEQ_API static std::string getVersion();

    /**
     * Run the render loop. Only returns when the application is finished, i.e.
     * maxFrames exceeded, ESC key pressed, etc.
     *
     * @param argc Argument count.
     * @param argv Argument list.
     */
    LIVREEQ_API void run(const int argc, char** argv);

    /**
     * Render a frame and return its result in the provided callback.
     * Do not mix with run().
     *
     * @param func callback function containing the rendered image.
     * @return true if a redraw is required, e.g. data is still dirty/loaded.
     */
    LIVREEQ_API bool render(const eq::View::ScreenshotFunc& func);

    /** @return the per-frame data. */
    LIVREEQ_API FrameData& getFrameData();

    /** @return the information of the currently loaded volume. */
    LIVREEQ_API const VolumeInformation& getVolumeInformation() const;

    /** Resize the active layout aka windows/drawables/buffers */
    LIVREEQ_API void resize(const Vector2ui& size);

    /** @return the current histogram. */
    LIVREEQ_API const Histogram& getHistogram() const;

    /** @return the application parameters */
    LIVREEQ_API ApplicationParameters& getApplicationParameters();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}
#endif
