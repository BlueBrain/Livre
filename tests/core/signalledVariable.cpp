/* Copyright (c) 2011-2016  Ahmet Bilgili <ahmetbilgili@gmail.com>
 *
 * This file is part of Livre <https://github.com/bilgili/Livre>
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


#include <boost/test/unit_test.hpp>

#include <livre/core/data/SignalledVariable.h>

namespace ut = boost::unit_test;

bool functionIsCalled = false;
bool meaningOfLife = 42;

void slot( size_t variable )
{
    functionIsCalled = true;
    BOOST_CHECK_EQUAL( variable, meaningOfLife );

}

BOOST_AUTO_TEST_CASE( signalVariableTest )
{
    livre::SignalledVariable< size_t > testVar1( meaningOfLife );
    BOOST_CHECK_EQUAL( testVar1.get(), meaningOfLife );
    testVar1 = 51;
    BOOST_CHECK_EQUAL( testVar1.get(), 51 );

    livre::SignalledVariable< size_t > testVar2( 51, &slot );
    BOOST_CHECK_EQUAL( testVar2.get(), 51 );
    testVar2 = meaningOfLife;
    BOOST_CHECK_EQUAL( testVar2.get(), meaningOfLife );
    BOOST_CHECK( functionIsCalled );
}
