/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef _livreTypes_h_
#define _livreTypes_h_

#include <livre/core/types.h>

namespace livre
{

class DataObject;
class HistogramObject;
class GLRaycasterPipeline;
class RenderSettings;
class TextureObject;

struct ApplicationParameters;

typedef std::shared_ptr< const DataObject > ConstDataObjectPtr;
typedef std::shared_ptr< const TextureObject > ConstTextureObjectPtr;
typedef std::shared_ptr< const HistogramObject > ConstHistogramObjectPtr;

}

#endif // _types_h_
