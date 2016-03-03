
/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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
#include <livre/core/pipeline/FutureMap.h>
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
    typedef std::map< std::string, ExecutablePtr > ExecutableMap;

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
                       const FilterPtr& filter,
                       bool wait )
    {
        if( _executableMap.count( name ) > 0 )
            LBTHROW( std::runtime_error( name + " already exists"));

        PipeFilterPtr pipefilter( new PipeFilter( name, filter ));
        _executableMap[ name ] = pipefilter;

        if( wait )
        {
            const OutFutureMap portFutures( pipefilter->getPostconditions( ));
            _waitFutures.push_back( portFutures.getFuture( pipefilter->getId().getString( )));
        }

        return pipefilter;
    }

    void add( const std::string& name,
              const PipelinePtr& pipeline,
              bool wait )
    {
        if( pipeline.get() == &_pipeline )
            return;

        if( _executableMap.count( name ) > 0 )
            LBTHROW( std::runtime_error( name + " already exists"));

        _executableMap[ name ] = pipeline;

        if( wait )
        {
            const Futures& futures = pipeline->getPostconditions();
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
            const FutureMap futureMap( executable->getPreconditions( ));
            if( futureMap.isReady( ))
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

        for( auto pair: _executableMap )
            executables.push_back( pair.second );

        return executables;
    }

    Futures getConnectedInFutureMap() const
    {
        return Futures();
    }

    Futures getOutFutures() const
    {
        return _waitFutures;
    }

    ~Impl()
    {}

    Pipeline& _pipeline;
    ExecutableMap _executableMap;
    Futures _waitFutures;
};

Pipeline::Pipeline()
    : _impl( new Pipeline::Impl( *this ))
{
}

Pipeline::~Pipeline()
{}

void Pipeline::add( const std::string& name,
                    const PipelinePtr& pipeline,
                    bool wait )
{
    return _impl->add( name, pipeline, wait );
}

PipeFilterPtr Pipeline::add( const std::string& name,
                             const FilterPtr& filter,
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

Futures Pipeline::getPreconditions() const
{
    return _impl->getConnectedInFutureMap();
}

Futures Pipeline::getPostconditions() const
{
    return _impl->getOutFutures();
}

}
