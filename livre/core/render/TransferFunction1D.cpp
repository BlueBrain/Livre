/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include "TransferFunction1D.h"

#include <fstream>

namespace livre
{
TransferFunction1D::TransferFunction1D(const std::string& file)
    : TransferFunction1D()
{
    if (file.empty())
        return;

    std::ifstream ifs(file);
    if (file.substr(file.find_last_of(".") + 1) != "1dt" || !ifs.is_open())
    {
        LBWARN << "Transfer function file " << file << " could not be opened."
               << std::endl;
        return;
    }

    uint32_t samples;
    ifs >> samples;

    getDiffuse().resize(samples);
    getAlpha().resize(samples);
    for (size_t i = 0; i < samples; ++i)
    {
        Vector4f rgba;
        ifs >> rgba[0] >> rgba[1] >> rgba[2] >> rgba[3];
        getDiffuse()[i] = {rgba[0], rgba[1], rgba[2]};
        getAlpha()[i] = rgba[3];
    }
}

TransferFunction1D::TransferFunction1D()
{
    const Vector3f color1(0.0f, 1.0f, 1.0f);
    const Vector3f color2(1.0f, 0.0f, 1.0f);
    const float alpha1 = 0.39f;
    const float alpha2 = 0.03f;
    const float density1 = 127.f;
    const float density2 = 255.f;
    const Vector3f colorDiff = color2 - color1;
    const float alphaDiff = alpha2 - alpha1;

    getDiffuse().resize(256);
    getAlpha().resize(getDiffuse().size());
    for (size_t i = 0; i < getDiffuse().size(); ++i)
    {
        Vector4f rgba;
        if (i > 0 && i <= density1)
        {
            rgba = color1;
            rgba[3] = alpha1 * (float(i) / density1);
        }
        else if (i > 0)
        {
            rgba = (float(i) - density1 + 1.f) / (density2 - density1 + 1.f) *
                       colorDiff +
                   color1;
            rgba[3] = (float(i) - density1 + 1.f) /
                          (density2 - density1 + 1.f) * alphaDiff +
                      alpha1;
        }

        getDiffuse()[i] = {rgba[0], rgba[1], rgba[2]};
        getAlpha()[i] = rgba[3];
    }
}

std::vector<Vector4ub> TransferFunction1D::getLUT() const
{
    std::vector<Vector4ub> lut;
    lut.reserve(getDiffuse().size());
    for (size_t i = 0; i < getDiffuse().size(); ++i)
    {
        const auto& diffuse = getDiffuse()[i];
        const auto alpha = getAlpha()[i];
        lut.push_back({uint8_t(diffuse.getRed() * 255.f),
                       uint8_t(diffuse.getGreen() * 255.f),
                       uint8_t(diffuse.getBlue() * 255.f),
                       uint8_t(alpha * 255.f)});
    }

    return lut;
}
}
