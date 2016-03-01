
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
#include <livre/core/pipeline/PortFutures.h>
#include <livre/core/pipeline/PipeFilterOutput.h.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/PortInfo.h>
#include <livre/core/pipeline/FunctionFilter.h>

namespace livre
{

struct Pipeline::Impl
{
    typedef std::map< std::string, ConstPipelinePtr > PipelineMap;
    typedef std::map< std::string, ConstPipeFilterPtr > PipeFilterMap;

    Impl( Pipeline& pipeline )
        : _pipeline( pipeline )
    {}

    PipeFilterPtr add( const std::string& name,
                       const FilterFunc& filterFunc,
                       const PortInfos& inputPorts,
                       const PortInfos& outputPorts,
                       bool wait )
{
        FilterPtr filter( new FunctionFilter( filterFunc,
                                              inputPorts,
                                              outputPorts ));
        return add( name, filter, wait );
    }

    PipeFilterPtr add( const std::string& name,
                       FilterPtr filter,
                       bool wait )
    {
        if( _filters.count( name ) > 0 || _pipelines.count( name ))
            LBTHROW( std::runtime_error( name + " already exists"));

        PipeFilterPtr pipefilter( new PipeFilter( name, filter ));
        _filters[ name ] = pipefilter;
        if( wait )
        {
            const PortFutures portFutures( pipefilter->getOutputFutures( ));
            _waitFutures.push_back( portFutures.getFutures( pipefilter->getId().getString( )));
        }
    }

    void add( const std::string& name,
              const ConstPipelinePtr& pipeline,
              bool wait )
    {
        if( pipeline.get() == _pipeline )
            return;

        _pipelines[ name ] = pipeline;

        if( wait )
        {
            const ConstFutures& futures = pipeline->getOutputFutures();
            _waitFutures.insert( _waitFutures.end(), futures.begin(), futures.end( ));
        }
    }

    void execute()
    {
        Executables executables = getExecutables();
        Executables::iterator it = executables.begin();
        while( !executables.empty())
        {
            ExecutablePtr executable = *it;
            const PortFutures portFutures( executable->getInputFutures( ));
            if( portFutures.isReady( ))
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

    Executables getExecutables() const
    {
        Executables executables;
        executables.reserve( _filters.size() + _pipelines.size( ));

        for( auto pair: _filters )
            executables.push_back( pair.second );

        for( auto pair: _pipelines )
            executables.push_back( pair.second );

        return executables;
    }

    ConstFutures getInputFutures() const
    {
        return ConstFutures();
    }

    ConstFutures getOutputFutures() const
    {
        return _waitFutures;
    }

    ~Impl()
    {}

    Pipeline& _pipeline;
    PipeFilterMap _filters;
    PipelineMap _pipelines;
    ConstFutures _waitFutures;
};

Pipeline::Pipeline()
    : _impl( new Pipeline::Impl( this ))
{
}

Pipeline::~Pipeline()
{}

void Pipeline::add( const std::string& name,
                    PipelinePtr pipeline,
                    bool wait )
{
    return _impl->addPipeline( name, pipeline, wait );
}

PipeFilterPtr Pipeline::add( const std::string& name,
                             FilterPtr filter,
                             bool wait )
{
    return _impl->add( name, filter, wait );
}

PipeFilterPtr Pipeline::add( const std::string& name,
                             const FilterFunc& filterFunc,
                             const PortInfos& inputPorts,
                             const PortInfos& outputPorts,
                             bool wait )
{
    return _impl->add( name,
                       filterFunc,
                       inputPorts,
                       outputPorts,
                       wait );
}

Executables Pipeline::getExecutables() const
{
    return _impl->getExecutables();
}

void Pipeline::execute()
{
    return _impl->execute();
}

ConstFutures Pipeline::getInputFutures() const
{
    return _impl->getInputFutures();
}

ConstFutures Pipeline::getOutputFutures() const
{
    return _impl->getOutputFutures();
}

}
