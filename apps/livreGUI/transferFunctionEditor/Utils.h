/* Copyright (c) 2017, EPFL/Blue Brain Project
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

#pragma once

#include <QList>
#include <QVariant>
#include <QVector2D>

#include <lexis/render/Histogram.h>
#include <vmmlib/vector.hpp>
#include <vmmlib/types.hpp>

namespace livre
{

inline QVariant sampleHistogram( const lexis::render::Histogram& histogram,
                                 const bool logScale,
                                 const vmml::Vector2f& range = {0, 1} )
{
    QList< QVariant > points;
    const auto& samples = histogram.sampleCurve( logScale, range );
    for( const auto& sample : samples )
        points.push_back( QVector2D{ sample.x(), sample.y()});
    return points;
}

}
