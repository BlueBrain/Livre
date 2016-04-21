/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Jafet.VillafrancaDiaz@epfl.ch
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

#define BOOST_TEST_MODULE LibCore

#include <livre/core/render/TransferFunction1D.h>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <lunchbox/file.h>

const std::string& tfDir = lunchbox::getRootPath() +
                               "/share/Livre/examples/";

BOOST_AUTO_TEST_CASE( transferFunction )
{
    const size_t nChannels = livre::TransferFunction1D::getNumChannels();
    const size_t defaultSize = nChannels * 256;
    livre::TransferFunction1D tf_default;
    BOOST_CHECK_EQUAL( tf_default.getLutSize(), defaultSize );
}

std::vector< uint8_t > readFile( const std::string& file )
{
    std::vector< uint8_t > values;
    std::ifstream ifs( file );

    std::string line, val;
    std::getline( ifs, line );

    while( ifs >> val )
        values.push_back( std::stoi( val ));

    return values;
}

BOOST_AUTO_TEST_CASE( loadTransferFunctionFile )
{
    std::vector< uint8_t > values = readFile( tfDir + "tf_f.1dt" );
    livre::TransferFunction1D tfFile( tfDir + "tf_f.1dt" );
    BOOST_CHECK_EQUAL( values.size(), tfFile.getLutSize( ));

    values = readFile( tfDir + "tf_c.1dt" );
    tfFile = livre::TransferFunction1D( tfDir + "tf_c.1dt" );
    BOOST_CHECK_EQUAL( values.size(), tfFile.getLutSize( ));
    BOOST_CHECK_EQUAL_COLLECTIONS( values.begin(), values.end(),
                                   tfFile.getLut(),
                                   tfFile.getLut() + tfFile.getLutSize( ));
}

BOOST_AUTO_TEST_CASE( serialization )
{
    livre::TransferFunction1D tfFile( tfDir + "tf_c.1dt" );
    lunchbox::saveBinary( tfFile, "tf.lbb" );
    lunchbox::saveAscii( tfFile, "tf.lba" );

    livre::TransferFunction1D tfFilea( "tf.lbb" );
    BOOST_CHECK_EQUAL( tfFile, tfFilea );

    livre::TransferFunction1D tfFileb( "tf.lba" );
    BOOST_CHECK_EQUAL( tfFile, tfFileb );
}

BOOST_AUTO_TEST_CASE( loadWrongTransferFunctionFile )
{
    livre::TransferFunction1D defaultTf;
    livre::TransferFunction1D tfFile( tfDir + "inexistent_file.1dt" );
    BOOST_CHECK_EQUAL_COLLECTIONS( defaultTf.getLut(),
                                   defaultTf.getLut() + defaultTf.getLutSize(),
                                   tfFile.getLut(),
                                   tfFile.getLut() + tfFile.getLutSize( ));
}
