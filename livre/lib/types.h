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
#include <livre/lib/configuration/VolumeRendererParameters.h>

namespace livre
{

class DataUploadProcessor;
class RenderNodeVisitor;
class TextureCache;
class TextureDataCache;
class TextureDataObject;
class TextureObject;
class TextureUploadProcessor;
struct ApplicationParameters;

typedef std::shared_ptr< const TextureDataObject > ConstTextureDataObjectPtr;
typedef std::shared_ptr< const TextureObject > ConstTextureObjectPtr;
typedef std::shared_ptr< TextureDataObject > TextureDataObjectPtr;
typedef std::shared_ptr< TextureObject > TextureObjectPtr;

static const uint32_t CONNECTION_ID( 0u );

typedef std::vector< DashRenderNode > DashRenderNodes;

}

#endif // _types_h_
