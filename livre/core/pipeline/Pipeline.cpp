
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

#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/OutputPort.h>
#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/FilterOutput.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/PortInfo.h>
#include <livre/core/pipeline/FunctionFilter.h>

namespace livre
{

struct Pipeline::Impl
{
    typedef std::vector< PipeFilterPtr > PipeFilterPtrs;

    Impl( Pipeline *pipeline )
        : _pipeline( pipeline )
    {}

    PipeFilterPtr getPipeFilterFunc( const FilterFunc& filterFunc,
                                     const PortInfos& inputPorts,
                                     const PortInfos& outputPorts,
                                     bool wait )
    {
        FilterPtr filter( new FunctionFilter( filterFunc,
                                              inputPorts,
                                              outputPorts ));
        return getPipeFilter( filter, wait );
    }

    PipeFilterPtr getPipeFilter( FilterPtr filter, bool wait )
    {
        PipeFilterPtr pipefilter( new PipeFilter( filter ));
        _filters.push_back( pipefilter );

        const ConstOutputPortPtrs& outputPorts =
                pipefilter->_getOutput().getPorts();

        if( wait )
        {
            _outputs.insert( _outputs.end(),
                             outputPorts.begin(),
                             outputPorts.end( ));
        }
        return pipefilter;
    }

    void addPipeline( PipelinePtr pipeline, bool wait )
    {
        if( pipeline.get() == _pipeline )
            return;

        _pipelines.push_back( pipeline );

        if( wait )
        {
            _outputs.insert( _outputs.end(),
                             pipeline->_impl->_outputs.begin(),
                             pipeline->_impl->_outputs.end());
        }
    }

    void waitForAll()
    {
        livre::waitForAll( _outputs );
    }

    bool waitForAny()
    {
        return livre::waitForAny( _outputs );
    }

    void execute()
    {
        ExecutablePtrs executables = getExecutables();
        ExecutablePtrs::iterator it = executables.begin();
        while( !executables.empty())
        {
            ExecutablePtr executable = *it;
            if( executable->isInputReady( ))
            {
                executable->execute();
                executables.erase( it );
                it = executables.begin();
            }
            else
            {
                if( it == executables.end( ))
                    break;
                ++it;
            }
        }
    }

    bool isInputReady() const
    {
        const ExecutablePtrs& executables = getExecutables();
        BOOST_FOREACH( const ExecutablePtr& executable, executables )
        {
            if( !executable->isInputReady( ))
                return false;
        }
        return true;
    }

    ExecutablePtrs getExecutables() const
    {
        ExecutablePtrs executables;
        executables.insert( executables.end(),
                            _filters.begin(),
                            _filters.end( ));

        executables.insert( executables.end(),
                            _pipelines.begin(),
                            _pipelines.end( ));

        return executables;
    }

    ~Impl()
    {}

    Pipeline *_pipeline;
    ConstOutputPortPtrs _outputs;
    PipeFilterPtrs _filters;
    PipelinePtrs _pipelines;
};

Pipeline::Pipeline()
    : _impl( new Pipeline::Impl( this ))
{
}

Pipeline::~Pipeline()
{}

void Pipeline::add( PipelinePtr pipeline, bool wait )
{
    return _impl->addPipeline( pipeline, wait );
}

PipeFilterPtr Pipeline::add( FilterPtr filter, bool wait )
{
    return _impl->getPipeFilter( filter, wait );
}

PipeFilterPtr Pipeline::add( const FilterFunc& filterFunc,
                             const PortInfos& inputPorts,
                             const PortInfos& outputPorts,
                             bool wait )
{
    return _impl->getPipeFilterFunc( filterFunc,
                                     inputPorts,
                                     outputPorts,
                                     wait );
}

PipeFilterPtrs& Pipeline::getFilters() const
{
    return _impl->_filters;
}

PipelinePtrs& Pipeline::getPipelines() const
{
    return _impl->_pipelines;
}

ExecutablePtrs Pipeline::getExecutables() const
{
    return _impl->getExecutables();
}

bool Pipeline::waitForAny()
{
    return _impl->waitForAny();
}

void Pipeline::waitForAll()
{
    _impl->waitForAll();
}

bool Pipeline::isInputReady() const
{
    return _impl->isInputReady();
}

void Pipeline::execute()
{
    _impl->execute();
}

}
