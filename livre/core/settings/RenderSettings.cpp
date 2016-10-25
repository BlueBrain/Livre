
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
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

#include <livre/core/settings/RenderSettings.h>
#include <lexis/render/ColorMap.h>

namespace livre
{
RenderSettings::RenderSettings()
    : _colorMap( lexis::render::ColorMap::getDefaultColorMap( 0, 256 ))
    , _clipPlanes( ClipPlanes( ))
    , _depth( 0 )
{
}

void RenderSettings::resetColorMap( )
{
    setColorMap( lexis::render::ColorMap::getDefaultColorMap( 0.0f, 256.0f ));
}

void RenderSettings::setColorMap( const lexis::render::ColorMap& cm )
{
    _colorMap = cm;
}

void RenderSettings::setClipPlanes( const ClipPlanes& clipPlanes )
{
    _clipPlanes = clipPlanes;
}

void RenderSettings::setMaxTreeDepth( const uint8_t depth )
{
    _depth = depth;
}

uint8_t RenderSettings::getMaxTreeDepth() const
{
    return _depth.get();
}

}
