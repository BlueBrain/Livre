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
#include <livre/core/Render/TransferFunction1D.h>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>

const std::string& tfDir = lunchbox::getRootPath() +
                               "/share/Livre/examples/";

BOOST_AUTO_TEST_CASE( testTransferFunction )
{
    const size_t defaultSize = TF_NCHANNELS * 256;
    livre::TransferFunction1Dc tf_default;
    BOOST_CHECK_EQUAL( tf_default.getData().size(), defaultSize );

    livre::TransferFunction1Dc tf_size( 5 );
    BOOST_CHECK_EQUAL( tf_size.getData().size(), TF_NCHANNELS * 5 );

    const std::vector< float > floatVec( 20, 1.0f );
    livre::TransferFunction1Df tf_float( floatVec );
    BOOST_CHECK_EQUAL( tf_float.getData().size(), floatVec.size( ));
    BOOST_CHECK( std::equal( floatVec.begin(), floatVec.end(),
                             tf_float.getData().begin( )));

    tf_default = tf_size;
    BOOST_CHECK_EQUAL( tf_default.getData().size(), tf_size.getData().size( ));
    tf_default.reset();
    BOOST_CHECK_EQUAL( tf_default.getData().size(), defaultSize );
}

std::vector< uint8_t > readFileToVec( const std::string& file )
{
    std::vector< uint8_t > values;
    std::ifstream ifs( file );

    std::string line, val;
    std::getline( ifs, line );

    while( ifs >> val )
        values.push_back( atoi( val.c_str( )));

    return values;
}

BOOST_AUTO_TEST_CASE( testLoadTransferFunctionFile )
{
    std::vector< uint8_t > values = readFileToVec( tfDir + "tf_f.1dt" );
    livre::TransferFunction1Dc tfFile( tfDir + "tf_f.1dt" );
    BOOST_CHECK_EQUAL( values.size(), tfFile.getData().size( ));

    values = readFileToVec( tfDir + "tf_c.1dt" );
    tfFile = livre::TransferFunction1Dc( tfDir + "tf_c.1dt" );
    BOOST_CHECK_EQUAL( values.size(), tfFile.getData().size( ));
    BOOST_CHECK( std::equal( values.begin(), values.end(),
                             tfFile.getData().begin( )));
}

BOOST_AUTO_TEST_CASE( testLoadWrongTransferFunctionFile )
{
    livre::TransferFunction1Dc defaultTf;
    livre::TransferFunction1Dc tfFile( tfDir + "wrong_file_format.txt" );
    BOOST_CHECK_EQUAL( defaultTf.getData().size(), tfFile.getData().size( ));
    BOOST_CHECK( std::equal( defaultTf.getData().begin(),
                             defaultTf.getData().end(),
                             tfFile.getData().begin( )));

    tfFile = livre::TransferFunction1Dc( tfDir + "inexistent_file.1dt" );
    BOOST_CHECK_EQUAL( defaultTf.getData().size(), tfFile.getData().size( ));
    BOOST_CHECK( std::equal( defaultTf.getData().begin(),
                             defaultTf.getData().end(),
                             tfFile.getData().begin( )));
}

