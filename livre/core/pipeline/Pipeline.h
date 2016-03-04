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
     * Adds a pipeline to be executed.
     * @param pipeline is added to list of executables
     * @param wait If true, on asynchronous execution, pipeline
     * can wait on given pipeline.
     */
    void add( const std::string& name,
              const PipelinePtr& pipeline,
              bool wait = true );

    /**
     * Creates and adds a pipefilter around a filter to be executed. The
     * notification of completion of execution is connected to the pipeline
     * if user wants to wait on execution of whole pipeline.
     * @param filter is added  to list of executables.
     * @param wait If true, on asynchronous execution, pipeline
     * can wait on the added filter.
     * @return returns the generated pipe filter.
     */
    PipeFilterPtr add( const std::string& name,
                       const FilterPtr& filter,
                       bool wait = true );

    /**
     * Adds a filter function to be executed. The
     * notification of completion of execution is connected to the pipeline
     * if user wants to wait on execution of whole pipeline.
     * @param filterFunc is the filter function object.
     * @param inputPorts are the input ports information.
     * @param outputPorts are the output ports information.
     * @param wait  If true, on asynchronous execution, pipeline
     * can wait on the added filter function.
     * @return returns the generated pipe filter.
     */
    PipeFilterPtr add( const std::string& name,
                       const FilterFunc& filterFunc,
                       const PortInfos& inputPorts,
                       const PortInfos& outputPorts,
                       bool wait = true );

    /**
     * @return the list of all executables ( pipe filters and
     * pipelines )
     */
    Executables getExecutables() const;

    /**
     * Executes each executable element in the graph
     * synchronously.
     */
    void execute() final;

    /**
     * Resets the pipeline. At this point pipeline execution should be complete.
     */
    void reset() final;

private:

    /**
     * @return Returns the writable future
     */
    Futures getPreconditions() const final;

    /**
     * @return Returns the writable future
     */
    Futures getPostconditions() const final;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _Pipeline_h_

