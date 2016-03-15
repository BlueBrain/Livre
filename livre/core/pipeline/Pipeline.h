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

#ifndef _Pipeline_h_
#define _Pipeline_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/Executable.h>

namespace livre
{

/**
 * Pipeline represents a filter graph. On asynchronous
 * execution through the Executor, the status of the
 * execution can be queried whether the execution is
 * complete or not through the post conditions.
 */
class Pipeline : public Executable::ExecutableImpl
{

public:

    Pipeline();
    ~Pipeline();

    /**
     * Adds a pipeline to be executed.
     * @param pipeline is added to list of executables
     * @param wait If true, on asynchronous execution, pipeline
     * can wait on given pipeline.
     */
    void add( const std::string& name,
              const Pipeline& pipeline,
              bool wait = true )
    {
        _add( name,
              Executable( pipeline ),
              wait );
    }

    /**
     * Creates and adds a pipefilter ( given the FilterT type of filter )
     * @param FilterT is the type of filter to be added to list of executables.
     * @name name the name of the filter instance.
     * @param args for the FilterT construction
     * @param wait If true, on asynchronous execution, pipeline
     * can wait on the added filter.
     * @return returns the generated pipe filter.
     * @throws std::runtime_error if an executable with same name is present
     */

    template< class FilterT, class... Args, bool wait = true >
    PipeFilter add( const std::string& name,
                    Args&&... args )
    {
        PipeFilterT< FilterT > pipeFilter( name, args... );
        _add( name,
              Executable( pipeFilter ),
              wait );
        return pipeFilter;
    }

    /**
     * @return the list of all executables ( pipe filters and
     * pipelines )
     */
    Executables getExecutables() const;

    /**
     * @param name of the executable
     * @return the executable
     * @throws std::runtime_error if a pipe filter or pipeline does not exist
     */
    const Executable& getExecutable( const std::string& name ) const;

    /**
     * @copydoc Executable::execute
     */
    void execute() final;

    /**
     * @copydoc Executable::getPostconditions
     */
    Futures getPostconditions() const final;

    /**
     * @copydoc Executable::getPreconditions
     */
    Futures getPreconditions() const final;

    /**
     * @copydoc Executable::reset
     */
    void reset() final;

private:

    void _add( const std::string& name,
               const Executable& filter,
               bool wait );
private:

    struct Impl;
    std::shared_ptr< Impl > _impl;

};

}

#endif // _Pipeline_h_

