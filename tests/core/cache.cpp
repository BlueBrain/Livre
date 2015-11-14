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
#include "cache/CacheObjectObserver.h"
#include "cache/Cache.h"

namespace ut = boost::unit_test;

BOOST_AUTO_TEST_CASE( testCacheObject )
{
    livre::CacheObjectPtr cacheObject( new test::ValidCacheObject() );

    BOOST_CHECK( cacheObject->isValid() );
    BOOST_CHECK( !cacheObject->isLoaded() );
    BOOST_CHECK( cacheObject->getReferenceCount_() == 1 );

    livre::CacheObjectPtr refCacheObject = cacheObject;
    BOOST_CHECK( cacheObject->getReferenceCount_() == 2 );

    refCacheObject->cacheLoad();
    BOOST_CHECK( refCacheObject->isLoaded() );

    refCacheObject.reset();
    BOOST_CHECK( cacheObject->getReferenceCount_() == 1 );

    cacheObject->cacheUnload();
    BOOST_CHECK( !cacheObject->isLoaded() );

    cacheObject->cacheLoad();
    BOOST_CHECK( cacheObject->isLoaded() );

    test::CacheObjectObserver observer;
    cacheObject->registerObserver( &observer );

    BOOST_CHECK( !observer.isUnloaded() );
    cacheObject.reset();
    BOOST_CHECK( observer.isUnloaded() );
}

BOOST_AUTO_TEST_CASE( testCache )
{
    test::Cache cache;

    livre::CacheObjectPtr validCacheObject( new test::ValidCacheObject() );
    BOOST_CHECK( validCacheObject->getCacheId() == 0 );

    livre::CacheObjectPtr cacheObject = cache.getObjectFromCache( 1 );

    BOOST_CHECK( cacheObject );
    BOOST_CHECK( boost::static_pointer_cast< test::ValidCacheObject >( cacheObject )->getCacheId() == 1 );
    BOOST_CHECK( cacheObject->getReferenceCount_() == 2 );

    cacheObject = cache.getObjectFromCache( 2 );
    BOOST_CHECK( cacheObject );
    BOOST_CHECK( boost::static_pointer_cast< test::ValidCacheObject >( cacheObject )->getCacheId() == 2 );
    BOOST_CHECK( cacheObject->getReferenceCount_() == 2 );

    cacheObject = cache.getObjectFromCache( 1 );
    BOOST_CHECK( cacheObject );
    BOOST_CHECK( boost::static_pointer_cast< test::ValidCacheObject >( cacheObject )->getCacheId() == 1 );
    BOOST_CHECK( cacheObject->getReferenceCount_() == 2 );

}


