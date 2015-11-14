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

#ifndef _RenderingSetGeneratorFilter_h_
#define _RenderingSetGeneratorFilter_h_

#include <livre/lib/types.h>

#include <livre/core/pipeline/Filter.h>
#include <livre/core/render/FrameInfo.h>

namespace livre
{

/**
 * RenderingSetGeneratorFilter class implements generation
 * of rendering set given the visibles.
 */
class RenderingSetGeneratorFilter : public Filter
{

public:

    RenderingSetGeneratorFilter();
    ~RenderingSetGeneratorFilter();

    /**
     * @copydoc Filter::execute( PipeFilter& filter )
     */
    void execute( PipeFilter& filter ) const final;

    /**
     * @copydoc Filter::getInputPorts( PortInfos& inputPorts )
     */
    void getInputPorts( PortInfos& inputPorts ) const final;

    /**
     * @copydoc Filter::getOutputPorts( PortInfos& inputPorts )
     */
    void getOutputPorts( PortInfos& outputPorts ) const final;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
