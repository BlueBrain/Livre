
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

#include <livre/core/settings/VolumeSettings.h>

namespace livre
{

VolumeSettings::VolumeSettings()
    : _uri( "" )
    , _dataSourceRange( Vector2f( std::numeric_limits< float >::infinity(),
                                 -std::numeric_limits< float >::infinity( )))
{}


void VolumeSettings::setURI( const std::string& uri )
{
    _uri = uri;
}

std::string VolumeSettings::getURI() const
{
    return _uri.get();
}

void VolumeSettings::setDataSourceRange( const Vector2f& range )
{
    _dataSourceRange = range;
}

Vector2f VolumeSettings::getDataSourceRange() const
{
    return _dataSourceRange.get();
}
}
