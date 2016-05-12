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

#ifndef _Filter_h_
#define _Filter_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/FuturePromise.h>
#include <livre/core/pipeline/FutureMap.h>
#include <livre/core/pipeline/PromiseMap.h>

namespace livre
{

/**
 * Filters are similar to functions ( immutable ). Their inputs and
 * outputs are provided with given name and data types. At execution time
 * values can be queried from the map of futures ( name - future pairs )
 * and can be set through the map of promises ( name - promise pairs )
 */
class Filter
{
public:

    /**
     * This function is called by the @PipeFilter while the filter
     * is being executed.
     * @param input The Future that can be read for input parameters
     * @param output The Promise that can be written to for output parameters
     */
    virtual void execute( const FutureMap& input, PromiseMap& output ) const = 0;

    /**
     * @return map for the name and data types for the filter
     * communication. In execution time using these names and types,
     * data can be set.
     */
    virtual DataInfos getInputDataInfos() const  { return DataInfos(); }

    /**
     * @return map for the name and data types for the filter
     * communication. In execution time using these names and types,
     * data can be retrieved.
     */
    virtual DataInfos getOutputDataInfos() const { return DataInfos(); }

    virtual ~Filter() {}
};

}

#endif // _Filter_h_

