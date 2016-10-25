/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _CameraPath_h_
#define _CameraPath_h_

#include <livre/core/types.h>
#include <livre/core/mathTypes.h>

namespace livre
{
/**
 * The Step struct keeps the information of a step of an animation
 */
struct Step
{
    Step();

    Step( const int32_t fr,
          const Vector3f& pos,
          const Vector3f& rot  );

    int32_t frame;
    Vector3f position;
    Vector3f rotation;
};

/**
 * The CameraPath class is used to iterate over the steps of an animation. It keeps the current state of
 * animation. The code is mostly taken from the eqPly example of Equalizer package.
 */
class CameraPath
{
public:

    CameraPath();

    /**
     * Loads the animation text file form the filename.
     * @param fileName Animation filename to load.
     * @return False if the animation cannot be loaded.
     */
    bool loadAnimation( const std::string& fileName );

    /**
     * @return The number of frames loaded.
     */
    uint32_t getNumberOfFrames() const;

    /**
     * @return True if animation is loaded.
     */
    bool isValid() const;

    /**
     * @return The next step of the animation.
     */
    Step getNextStep( );

    /**
     * @return The current step of animation.
     */
    uint32_t getCurrentFrame() const;

    /**
     * @return The rotation angles in degrees in x,y and z.
     */
    const Vector3f& modelRotation() const;

private:

    Vector3f modelRotation_;
    std::vector< Step > steps_;
    uint32_t curStep_;
    int32_t curFrame_;
    int32_t totalFrameNumber_;
};

}
#endif //_CameraPath_h_
