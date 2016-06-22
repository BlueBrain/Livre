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

BOOST_AUTO_TEST_CASE( testCache )
{
    const size_t maxMemBytes = 2048u;

    test::Cache cache( maxMemBytes );
    BOOST_CHECK_EQUAL( cache.getCount(), 0 );
    BOOST_CHECK_EQUAL( cache.getStatistics().getMaximumMemory(), maxMemBytes );

    livre::CacheObjectPtr validCacheObject( new test::ValidCacheObject( 0 ) );
    BOOST_CHECK_EQUAL( validCacheObject->getId(), 0 );

    livre::ConstCacheObjectPtr constCacheObject = cache.get( 1 );

    size_t cacheSize = 0;
    BOOST_CHECK( !constCacheObject );

    livre::ConstCacheObjectPtr cacheObject = cache.load( 1 );
    cacheSize = cacheSize + cacheObject->getSize();
    BOOST_CHECK_EQUAL( cache.getStatistics().getUsedMemory(), cacheSize );

    cacheObject = cache.load( 2 );
    BOOST_CHECK( cacheObject );
    BOOST_CHECK_EQUAL( cache.getCount(), 2 );
    BOOST_CHECK_EQUAL( cacheObject->getId(), 2 );
    BOOST_CHECK_EQUAL( cacheObject.use_count(), 2 );
    cacheSize = cacheSize + cacheObject->getSize();
    BOOST_CHECK_EQUAL( cache.getStatistics().getUsedMemory(), cacheSize );

    cacheObject = cache.load( 1 );
    BOOST_CHECK( cacheObject );
    BOOST_CHECK_EQUAL( cache.getCount(), 2 );
    BOOST_CHECK_EQUAL( cacheObject->getId(), 1 );
    BOOST_CHECK_EQUAL( cacheObject.use_count(), 2 );
    BOOST_CHECK_EQUAL( cache.getStatistics().getUsedMemory(), cacheSize );
    cacheObject.reset();

    livre::ConstCacheObjectPtr cacheObjectTriggerClean = cache.load( 3 );
    BOOST_CHECK( cacheObjectTriggerClean );

    BOOST_CHECK_EQUAL( cache.getCount(), 2 );
    BOOST_CHECK_EQUAL( cacheObjectTriggerClean->getId(), 3 );
    BOOST_CHECK_EQUAL( cacheObjectTriggerClean.use_count(), 2 );
    BOOST_CHECK_EQUAL( cache.getStatistics().getUsedMemory(), cacheSize );
}
