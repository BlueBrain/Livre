/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#define BOOST_TEST_MODULE LibCore

#include <boost/test/unit_test.hpp>

#include "cache/ValidCacheObject.h"
#include "cache/Cache.h"

#include <livre/core/cache/CacheStatistics.h>

namespace ut = boost::unit_test;

BOOST_AUTO_TEST_CASE( testCache )
{
    test::Cache cache( 2048 );
    BOOST_CHECK( cache.getCount() == 0 );

    livre::CacheObjectPtr validCacheObject( new test::ValidCacheObject( 0 ) );
    BOOST_CHECK( validCacheObject->getId() == 0 );

    livre::CacheObjectPtr cacheObject = cache.get( 1 );

    size_t cacheSize = 0;
    BOOST_CHECK( cacheObject );
    BOOST_CHECK( cache.getCount() == 1 );
    BOOST_CHECK( boost::static_pointer_cast< test::ValidCacheObject >( cacheObject )->getId() == 1 );
    BOOST_CHECK( cacheObject->getRefCount() == 1 );

    cacheObject = cache.load( 1 );
    cacheSize = cacheSize + cacheObject->getSize();
    BOOST_CHECK( cache.getStatistics().getUsedMemory() == cacheSize );

    cacheObject = cache.load( 2 );
    BOOST_CHECK( cacheObject );
    BOOST_CHECK( cache.getCount() == 2 );
    BOOST_CHECK( boost::static_pointer_cast< test::ValidCacheObject >( cacheObject )->getId() == 2 );
    BOOST_CHECK( cacheObject->getRefCount() == 1 );
    cacheSize = cacheSize + cacheObject->getSize();
    BOOST_CHECK( cache.getStatistics().getUsedMemory() == cacheSize );

    cacheObject = cache.load( 1 );
    BOOST_CHECK( cacheObject );
    BOOST_CHECK( cache.getCount() == 2 );
    BOOST_CHECK( boost::static_pointer_cast< test::ValidCacheObject >( cacheObject )->getId() == 1 );
    BOOST_CHECK( cacheObject->getRefCount() == 1 );
    BOOST_CHECK( cache.getStatistics().getUsedMemory() == cacheSize );

    livre::CacheObjectPtr cacheObjectTriggerClean = cache.load( 3 );
    BOOST_CHECK( cacheObjectTriggerClean );

    BOOST_CHECK( cache.getCount() == 3 );
    BOOST_CHECK( boost::static_pointer_cast< test::ValidCacheObject >( cacheObjectTriggerClean )->getId() == 3 );
    BOOST_CHECK( cacheObjectTriggerClean->getRefCount() == 1 );
    BOOST_CHECK( cache.getStatistics().getUsedMemory() == cacheSize );
}


