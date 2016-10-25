/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _GLRenderUploadFilter_h_
#define _GLRenderUploadFilter_h_

#include <livre/lib/types.h>
#include <livre/core/render/RenderInputs.h>
#include <livre/core/pipeline/Filter.h>

namespace livre
{


/**
 * GLRenderUploaderFilter class implements the parallel data loading for raw volume data and
 * textures. A group of uploaders is executed in rendering pipeline and each uploader
 * has an id in the group.
 */
class GLRenderUploadFilter : public Filter
{
public:

    /**
     * Constructor
     * @param dataCache data cache
     * @param dataSource data source
     */
    GLRenderUploadFilter( Cache& textureCache,
                          TexturePool& texturePool,
                          size_t nUploadThreads,
                          Executor& executor );
    ~GLRenderUploadFilter();

    /**
     * @copydoc Filter::execute
     */
    void execute( const FutureMap& input, PromiseMap& output ) const final;

    /** @copydoc Filter::getInputDataInfos */
    DataInfos getInputDataInfos() const final
    {
        return
        {
            { "RenderInputs", getType< RenderInputs >() },
            { "NodeIds", getType< NodeIds >() },
        };
    }

    /** @copydoc Filter::getOutputDataInfos */
    DataInfos getOutputDataInfos() const final
    {
        return
        {
            { "TextureCacheObjects", getType< ConstCacheObjects >() },
        };
    }

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}

#endif // _GLRenderUploadFilter_h_
