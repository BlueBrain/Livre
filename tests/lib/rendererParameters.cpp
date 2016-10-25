/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Daniel.Nachbaur@epfl.ch
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

#define BOOST_TEST_MODULE RendererParameters
#include <boost/test/unit_test.hpp>

#include <livre/core/configuration/RendererParameters.h>

BOOST_AUTO_TEST_CASE(defaultValues)
{
    const livre::RendererParameters params;

    BOOST_CHECK_EQUAL( params.getMaxLOD(), (livre::NODEID_LEVEL_BITS << 1) + 1);
    BOOST_CHECK_EQUAL( params.getMinLOD(), 0 );
    BOOST_CHECK( !params.getSynchronousMode( ));
    BOOST_CHECK_EQUAL( params.getSamplesPerRay(), 0 );
    BOOST_CHECK_EQUAL( params.getSamplesPerPixel(), 1 );

#ifdef __i386__
    BOOST_CHECK_EQUAL( params.getSSE(), 8.0f );
    BOOST_CHECK_EQUAL( params.getMaxGPUCacheMemoryMB(), 384u );
    BOOST_CHECK_EQUAL( params.getMaxCPUCacheMemoryMB(), 768u );
#else
    BOOST_CHECK_EQUAL( params.getSSE(), 4.0f );
    BOOST_CHECK_EQUAL( params.getMaxGPUCacheMemoryMB(), 3072u );
    BOOST_CHECK_EQUAL( params.getMaxCPUCacheMemoryMB(), 8192u );
#endif
}

BOOST_AUTO_TEST_CASE(copy)
{
    livre::RendererParameters params;
    params.setMaxLOD( 42 );

    const livre::RendererParameters paramsCopy( params );
    BOOST_CHECK_EQUAL( paramsCopy.getMaxLOD(), params.getMaxLOD( ));
    BOOST_CHECK( paramsCopy == params );

    livre::RendererParameters paramsAssigned;
    paramsAssigned = params;
    BOOST_CHECK_EQUAL( paramsAssigned.getMaxLOD(), paramsAssigned.getMaxLOD( ));
    BOOST_CHECK( paramsAssigned == params );
}

BOOST_AUTO_TEST_CASE(initFromArgv)
{
    const char* app = boost::unit_test::framework::master_test_suite().argv[0];
    const char* argv[] = { app, "--sse", "1.4", "--synchronous",
                           "--gpu-cache-mem", "12345",
                           "--cpu-cache-mem", "54321",
                           "--min-lod", "2", "--max-lod", "6",
                           "--samples-per-ray", "42",
                           "--samples-per-pixel", "4" };
    const int argc = sizeof(argv)/sizeof(char*);

    livre::RendererParameters params;
    params.initialize( argc, argv );

    BOOST_CHECK_EQUAL( params.getMaxLOD(), 6 );
    BOOST_CHECK_EQUAL( params.getMinLOD(), 2 );
    BOOST_CHECK( params.getSynchronousMode( ));
    BOOST_CHECK_EQUAL( params.getSamplesPerRay(), 42 );
    BOOST_CHECK_EQUAL( params.getSamplesPerPixel(), 4 );
    BOOST_CHECK_EQUAL( params.getSSE(), 1.4f );
    BOOST_CHECK_EQUAL( params.getMaxGPUCacheMemoryMB(), 12345u );
    BOOST_CHECK_EQUAL( params.getMaxCPUCacheMemoryMB(), 54321u );
}
