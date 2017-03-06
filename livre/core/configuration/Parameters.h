/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _Parameters_h_
#define _Parameters_h_

#include <livre/core/api.h>
#include <livre/core/configuration/Configuration.h>
#include <livre/core/types.h>

namespace livre
{
/**
 * The Parameters class is used to load/initialize parameters from
 * file/arguments.
 */
class Parameters
{
public:
    LIVRECORE_API virtual ~Parameters();

    /**
     * Initializes parameters from arguments.
     * @param argc Argument count.
     * @param argv Argument list.
     * @return True if arguments can be parsed.
     */
    LIVRECORE_API bool initialize(const int32_t argc, const char** argv);

    /**
     * @return The parameter name.
     */
    LIVRECORE_API const std::string& getParameterName() const;

    /**
     * @return The configuration of parameters.
     */
    LIVRECORE_API const Configuration& getConfiguration() const;

protected:
    /**
     * @param configGroupName Configuration group name.
     */
    LIVRECORE_API Parameters(const std::string& configGroupName);

    /**
     * Initializes member variables from configuration variables.
     */
    virtual void initialize_() = 0;

    /**
     * configuration_ Configuration that keeps variadic data.
     */
    Configuration configuration_;

    /**
     * configGroupName_ Configuration group name.
     */
    std::string configGroupName_;
};
}
#endif // _Parameters_h_
