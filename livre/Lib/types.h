/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

class DataUploadProcessor;
class RenderNodeVisitor;
class TextureCache;
class TextureDataCache;
class TextureDataObject;
class TextureObject;
class TextureUploadProcessor;
struct ApplicationParameters;
struct ClientParameters;
struct EFPrefetchAlgorithmParameters;
struct VolumeRendererParameters;
struct RESTParameters;

/** SmartPtr definitions */
typedef boost::shared_ptr< ClientParameters > ClientParametersPtr;
typedef boost::shared_ptr< const ClientParameters > ConstClientParametersPtr;

typedef boost::shared_ptr< VolumeRendererParameters > VolumeRendererParametersPtr;
typedef boost::shared_ptr< const VolumeRendererParameters > ConstVolumeRendererParametersPtr;

typedef boost::shared_ptr< RESTParameters > RESTParametersPtr;
typedef boost::shared_ptr< const RESTParameters > ConstRESTParametersPtr;

typedef boost::shared_ptr< TextureCache > TextureCachePtr;
typedef boost::shared_ptr< DataUploadProcessor > DataUploadProcessorPtr;
typedef boost::shared_ptr< TextureUploadProcessor > TextureUploadProcessorPtr;
typedef boost::shared_ptr< const DataUploadProcessor > ConstDataUploadProcessorPtr;
typedef boost::shared_ptr< const TextureUploadProcessor > ConstTextureUploadProcessorPtr;
typedef boost::shared_ptr< const DashProcessor > ConstDashProcessorPtr;

typedef boost::intrusive_ptr< const TextureDataObject > ConstTextureDataObjectPtr;
typedef boost::intrusive_ptr< const TextureObject > ConstTextureObjectPtr;
typedef boost::intrusive_ptr< TextureDataObject > TextureDataObjectPtr;
typedef boost::intrusive_ptr< TextureObject > TextureObjectPtr;

typedef boost::scoped_ptr< TextureDataCache > TextureDataCachePtr;

/** Map definitions */
typedef boost::unordered_map< uint32_t, DataUploadProcessorPtr > DataUploadProcessorPtrMap;

const uint32_t CONNECTION_ID( 0u );

}

#endif // _types_h_
