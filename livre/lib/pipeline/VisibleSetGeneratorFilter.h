/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#ifndef _VisibleSetGeneratorFilter_h_
#define _VisibleSetGeneratorFilter_h_

#include <livre/lib/types.h>

#include <livre/core/pipeline/Filter.h>
#include <livre/core/render/FrameInfo.h>

namespace livre
{

/**
 * Collects all the visibles for given inputs ( Frustums, Frames, Data Ranges,
 * Rendering params and Viewports )
 */
class VisibleSetGeneratorFilter : public Filter
{

public:

    /**
     * Constructor
     * @param dataSource the data source
     */
    explicit VisibleSetGeneratorFilter( const DataSource& dataSource );
    ~VisibleSetGeneratorFilter();

    /** @copydoc Filter::execute */
    void execute( const FutureMap& input, PromiseMap& output ) const final;

    /** @copydoc Filter::getInputDataInfos */
    DataInfos getInputDataInfos() const final
    {
        return
        {
            { "Frustum", getType< Frustum >() },
            { "Frame", getType< uint32_t >() },
            { "DataRange", getType< Range >() },
            { "Params", getType< RendererParameters >() },
            { "Viewport", getType< PixelViewport >() },
            { "ClipPlanes", getType< ClipPlanes >() }
        };
    }

    /** @copydoc Filter::getOutputDataInfos */
    DataInfos getOutputDataInfos() const final
    {
        return
        {
            { "VisibleNodes", getType< NodeIds >() },
            { "Params", getType< RendererParameters >() }
        };
    }

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
