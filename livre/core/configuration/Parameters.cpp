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

#include "Parameters.h"

namespace livre
{
bool Parameters::initialize(const int32_t argc, const char **argv)
{
    try
    {
        configuration_.parseCommandLine(argc, argv);
        initialize_();
        return true;
    }
    catch (const boost::program_options::error &exception)
    {
        LBINFO << "Error parsing : " << exception.what() << std::endl;
    }

    return false;
}
const std::string &Parameters::getParameterName() const
{
    return configGroupName_;
}

const Configuration &Parameters::getConfiguration() const
{
    return configuration_;
}

Parameters::~Parameters()
{
}

Parameters::Parameters(const std::string &configGroupName)
    : configGroupName_(configGroupName)
{
}
}
