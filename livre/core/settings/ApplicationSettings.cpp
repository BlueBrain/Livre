/*
 * Copyright (c) 2016, ahmetbilgili@gmail.com
 *
 * This file is part of Livre <https://github.com/bilgili/Livre>
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

#include <livre/core/settings/ApplicationSettings.h>

namespace livre
{

ApplicationSettings::ApplicationSettings()
    : _resourceFolders( Strings( ))
{}

void ApplicationSettings::addResourceFolder( const std::string& folder )
{
    _resourceFolders.get().push_back( folder );
    std::cout << folder << std::endl;
    _resourceFolders = _resourceFolders.get();
}

Strings ApplicationSettings::getResourceFolders() const
{
    return _resourceFolders.get();
}
}
