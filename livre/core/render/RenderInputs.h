/* Copyright (c) 2011-2016  Ahmet Bilgili <ahmetbilgili@gmail.com>
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


#ifndef _RenderInputs_h_
#define _RenderInputs_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/mathTypes.h>

#include <livre/core/settings/ApplicationSettings.h>
#include <livre/core/settings/RenderSettings.h>
#include <livre/core/configuration/RendererParameters.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/pipeline/PipeFilter.h>

namespace livre
{

/** The Render input structure refers all the inputs necessary for rendering */
struct RenderInputs
{
    const FrameInfo frameInfo;
    const Range renderDataRange;
    const Vector2f dataSourceRange;
    const PixelViewport pixelViewPort;
    const Viewport viewport;
    const ApplicationSettings appSettings;
    const RenderSettings renderSettings;
    const RendererParameters vrParameters;
    const PipeFilterMap filters;
    DataSource& dataSource;
    Cache& dataCache;
    Cache& histogramCache;
};

}
#endif // _RendererPlugin_h_
