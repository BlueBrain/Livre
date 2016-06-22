
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

#include <livre/core/pipeline/FutureMap.h>
#include <livre/core/pipeline/Pipeline.h>

namespace livre
{

struct Pipeline::Impl
{
    typedef std::map< std::string, const Pipeline > PipelineMap;
    typedef std::map< std::string, const PipeFilter > PipeFilterMap;
    typedef std::map< std::string, std::unique_ptr< Executable >> ExecutableMap;

    Impl( Pipeline& pipeline )
        : _pipeline( pipeline )
    {}

    void add( const std::string& name,
              Pipeline::UniqueExecutablePtr executable,
              bool wait )
    {
        if( _executableMap.count( name ) > 0 )
            LBTHROW( std::runtime_error( name + " already exists"));

        if( wait )
        {
            const Futures& futures = executable->getPostconditions();
            _outFutures.insert( _outFutures.end(), futures.begin(), futures.end( ));
        }

        _executableMap.emplace( std::piecewise_construct,
                                std::forward_as_tuple( name ),
                                std::forward_as_tuple( std::move( executable )));
    }

    void execute()
    {
        Executables executables = getExecutables();
        Executables::iterator it = executables.begin();
        while( !executables.empty( ))
        {
            Executable* executable = *it;
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

        for( auto& nameExec: _executableMap )
            executables.push_back( nameExec.second.get( ));

        return executables;
    }

    Executable& getExecutable( const std::string& name )
    {
        if( _executableMap.count( name ) == 0 )
            LBTHROW( std::runtime_error( name + " executable does not exist"));

        return *_executableMap.find( name )->second;
    }

    Futures getPreconditions() const
    {
        Futures inFutures;
        for( auto& nameExec: _executableMap )
        {
            const Futures& futures = nameExec.second->getPreconditions();
            inFutures.insert( inFutures.end(), futures.begin(), futures.end( ));
        }
        return inFutures;
    }

    Futures getPostconditions() const
    {
        return _outFutures;
    }

    void schedule( Executor& executor )
    {
        for( auto& nameExec: _executableMap )
            executor.schedule( nameExec.second->clone( ));
    }

    void reset()
    {
        for( auto& nameExec: _executableMap )
            nameExec.second->reset();
    }

    Pipeline& _pipeline;
    ExecutableMap _executableMap;
    Futures _outFutures;
};

Pipeline::Pipeline()
    : _impl( new Impl( *this ))
{
}

Pipeline::~Pipeline()
{}

void Pipeline::_add( const std::string& name,
                     UniqueExecutablePtr exec,
                     bool wait )
{
    _impl->add( name, std::move( exec ), wait );
}

Executable& Pipeline::getExecutable( const std::string& name )
{
    return _impl->getExecutable( name );
}

void Pipeline::execute()
{
    _impl->execute();
}

Futures Pipeline::getPostconditions() const
{
    return _impl->getPostconditions();
}

Futures Pipeline::getPreconditions() const
{
    return _impl->getPreconditions();
}

void Pipeline::reset()
{
    _impl->reset();
}

void Pipeline::_schedule( Executor& executor )
{
    _impl->schedule( executor );
}

ExecutablePtr Pipeline::clone() const
{
    return ExecutablePtr( new Pipeline( *this ));
}

}
