
/* Copyright (c) 2011-2017, Maxim Makhinya <maxmah@gmail.com>
 *                          David Steiner  <steiner@ifi.uzh.ch>
 *                          Ahmet Bilgili  <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#ifndef _RenderSettings_h_
#define _RenderSettings_h_

#include <livre/lib/types.h>

#include <livre/core/render/TransferFunction1D.h>

namespace livre
{
class RenderSettings
{
public:
    /**
     * @brief RenderSettings constructor.
     */
    RenderSettings();

    /**
     * @brief setTransferFunction Sets the transfer function.
     * @param tf Transfer function.
     */
    void setTransferFunction(const TransferFunction1D& tf);

    /**
     * @brief resetTransferFunction Resets the transfer function.
     */
    void resetTransferFunction();

    /**
     * @return Returns the transfer function.
     */
    TransferFunction1D& getTransferFunction()
    {
        _transferFunction.notifyChanged();
        return _transferFunction;
    }
    const TransferFunction1D& getTransferFunction() const
    {
        return _transferFunction;
    }

    /**
     * @brief Sets the clip planes.
     * @param clipPlanes the clip planes
     */
    void setClipPlanes(const ClipPlanesDist& clipPlanes);

    /**
     * @return Returns the clip planes.
     */
    ClipPlanesDist& getClipPlanes()
    {
        _clipPlanes.notifyChanged();
        return _clipPlanes;
    }
    const ClipPlanesDist& getClipPlanes() const { return _clipPlanes; }
private:
    TransferFunction1D _transferFunction;
    ClipPlanesDist _clipPlanes;
};
}

#endif // _RenderInfo_h_
