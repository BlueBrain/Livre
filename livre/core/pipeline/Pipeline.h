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

#ifndef _Pipeline_h_
#define _Pipeline_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/Executable.h>

namespace livre
{

/**
 * Pipeline represents a filter graph. On asynchronous
 * execution through the Executor, the status of the
 * execution can be queried whether the execution is
 * complete or not.
 */
class Pipeline : public Executable
{

public:

    Pipeline();
    ~Pipeline();

    /**
     * Executes each executable element in the graph
     * synchronously.
     */
    void execute() final;

    /**
     * Adds a pipeline to be executed.
     * @param pipeline is added to list of executables
     * @param wait If true, on asynchronous execution, pipeline
     * can wait on given pipeline.
     */
    void add( PipelinePtr pipeline, bool wait = true );

    /**
     * Adds a filter to be executed
     * @param filter is added  to list of executables.
     * @param wait If true, on asynchronous execution, pipeline
     * can wait on given filter.
     * @return returns the generated pipe filter.
     */
    PipeFilterPtr add( FilterPtr filter, bool wait = true );

    /**
     * Adds a filter function to be executed
     * @param filterFunc is the filter function object.
     * @param inputPorts are the input ports information.
     * @param outputPorts are the output ports information.
     * @param wait  If true, on asynchronous execution, pipeline
     * can wait on given filter function.
     * @return returns the generated pipe filter.
     */
    PipeFilterPtr add( const FilterFunc& filterFunc,
                       const PortInfos& inputPorts,
                       const PortInfos& outputPorts,
                       bool wait = true );

    /**
     * @return the list of added pipe filters
     */
    PipeFilterPtrs& getFilters() const;

    /**
     * @return the list of added pipelines
     */
    PipelinePtrs& getPipelines() const;

    /**
     * @return the list of all executables ( pipe filters and
     * pipelines )
     */
    ExecutablePtrs getExecutables() const;

    /**
     * @return waits for any filter to finish
     */
    bool waitForAny();

    /**
     * @return waits for all filters to finish
     */
    void waitForAll();

    /**
     * @return true if all inputs are ready
     */
    bool isInputReady() const final;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _Pipeline_h_

