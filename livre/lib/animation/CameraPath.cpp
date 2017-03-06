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

#include <fstream>
#include <iostream>

#include <livre/lib/animation/CameraPath.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace livre
{
Step::Step()
    : frame(0)
    , position(Vector3f(.0f, .0f, -1.0f))
    , rotation(Vector3f(.0f, .0f, .0f))
{
}

Step::Step(const int32_t fr, const Vector3f& pos, const Vector3f& rot)
    : frame(fr)
    , position(pos)
    , rotation(rot)
{
}

CameraPath::CameraPath()
    : curStep_(0)
    , curFrame_(0)
    , totalFrameNumber_(0)
{
}

uint32_t CameraPath::getNumberOfFrames() const
{
    return totalFrameNumber_;
}

bool CameraPath::isValid() const
{
    return !steps_.empty();
}

uint32_t CameraPath::getCurrentFrame() const
{
    return curFrame_;
}

const Vector3f& CameraPath::modelRotation() const
{
    return modelRotation_;
}

Step CameraPath::getNextStep()
{
    LBASSERT(!steps_.empty());
    LBASSERT(curStep_ < steps_.size());

    if (steps_.empty())
        return Step();

    if (steps_.size() == 1)
        return steps_[curStep_];

    LBASSERT(curStep_ < steps_.size() - 1);

    ++curFrame_;
    if (curFrame_ > steps_[curStep_ + 1].frame)
    {
        if (curStep_ == steps_.size() - 2)
        {
            curFrame_ = 1;
            curStep_ = 0;
        }
        else
            ++curStep_;
    }
    // else
    const Step& curStep = steps_[curStep_];
    const Step& nextStep = steps_[curStep_ + 1];

    if (curFrame_ < curStep.frame)
        curFrame_ = curStep.frame + 1;

    const float interval = float(nextStep.frame - curStep.frame);
    const float u = (curFrame_ - curStep.frame) / interval;

    Step result(curFrame_, curStep.position * (1.0 - u) + nextStep.position * u,
                curStep.rotation * (1.0 - u) + nextStep.rotation * u);

    return result;
}

bool CameraPath::loadAnimation(const std::string& fileName)
{
    steps_.clear();

    if (fileName.empty())
        return false;

    std::ifstream file;
    file.open(fileName.c_str());
    if (!file)
    {
        LBERROR << "Path file could not be opened" << std::endl;
        return false;
    }

    // read model pre-rotation
    file >> modelRotation_.x();
    file >> modelRotation_.y();
    file >> modelRotation_.z();

    const float m = static_cast<float>(M_PI_2) / 90.f;
    modelRotation_ *= m;

    uint32_t count = 0;
    float v[7];
    totalFrameNumber_ = 0;
    while (!file.eof())
    {
        file >> v[count++];
        if (count == 7)
        {
            count = 0;
            totalFrameNumber_ += LB_MAX(static_cast<int>(v[0]), 1);

            steps_.push_back(Step(totalFrameNumber_, Vector3f(v[1], v[2], v[3]),
                                  Vector3f(-v[5] * m, v[4] * m, v[6] * m)));
        }
    }
    file.close();

    return true;
}
}
