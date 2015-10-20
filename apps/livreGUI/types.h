/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _GuiTypes_h_
#define _GuiTypes_h_

#include <QVariant>

#include <livre/eq/types.h>
#include <zeq/types.h>

namespace livre
{
/**
 * Class definitions
 */
class Controller;
class MainWindow;

/**
 * SmartPtr definitions
 */
typedef std::vector< std::string > Strings;
typedef std::pair< std::string, QVariant > Property;
typedef std::vector< Property > PropertyList;

}

#endif // _GuiTypes_h_
