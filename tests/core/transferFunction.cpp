/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Jafet.VillafrancaDiaz@epfl.ch
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

#define BOOST_TEST_MODULE TransferFunction1D

#include <livre/core/render/TransferFunction1D.h>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <lunchbox/file.h>

const std::string& tfDir = lunchbox::getRootPath() + "/share/Livre/examples/";

std::vector<livre::Vector4ub> readFile(const std::string& file)
{
    std::vector<livre::Vector4ub> values;
    std::ifstream ifs(file);

    size_t numValues;
    ifs >> numValues;

    values.resize(numValues);
    for (size_t i = 0; i < numValues; ++i)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            float value;
            ifs >> value;
            values[i][j] = value * 255.f;
        }
    }

    return values;
}

BOOST_AUTO_TEST_CASE(loadTransferFunctionFile)
{
    const auto& values = readFile(tfDir + "tf_f.1dt");
    livre::TransferFunction1D tfFile(tfDir + "tf_f.1dt");
    const auto& lut = tfFile.getLUT();
    BOOST_CHECK_EQUAL(values.size(), lut.size());
    BOOST_CHECK_EQUAL_COLLECTIONS(values.begin(), values.end(), lut.begin(),
                                  lut.end());
}

BOOST_AUTO_TEST_CASE(loadWrongTransferFunctionFile)
{
    livre::TransferFunction1D defaultTf;
    livre::TransferFunction1D tfFile(tfDir + "inexistent_file.1dt");

    const auto& defaultLUT = defaultTf.getLUT();
    const auto& tfLUT = tfFile.getLUT();
    BOOST_CHECK_EQUAL_COLLECTIONS(defaultLUT.begin(), defaultLUT.end(),
                                  tfLUT.begin(), tfLUT.end());
}
