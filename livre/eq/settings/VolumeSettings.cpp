
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

#include <eq/eq.h>

#include <livre/eq/types.h>
#include <livre/eq/settings/VolumeSettings.h>

namespace livre
{

VolumeSettings::VolumeSettings()
{
    reset();
}

void VolumeSettings::reset()
{
    _uri = "";
    _dataSourceRange = Vector2f( std::numeric_limits< float >::infinity(),
                                 -std::numeric_limits< float >::infinity( ));
    setDirty( DIRTY_ALL );
}


void VolumeSettings::setURI( const std::string& uri )
{
    _uri = uri;
    setDirty( DIRTY_ALL );
}

const std::string& VolumeSettings::getURI() const
{
    return _uri;
}

void VolumeSettings::setDataSourceRange( const Vector2f& range )
{
    _dataSourceRange = range;
    setDirty( DIRTY_ALL );
}

const vmml::Vector2f& VolumeSettings::getDataSourceRange() const
{
    return _dataSourceRange;
}

void VolumeSettings::serialize( co::DataOStream& os, uint64_t )
{
    os  << _uri << _dataSourceRange;
}

void VolumeSettings::deserialize( co::DataIStream& is, uint64_t )
{
    is >> _uri >> _dataSourceRange;
}

}
