/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Daniel.Nachbaur@epfl.ch
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

#define BOOST_TEST_MODULE VolumeRendererParameters
#include <boost/test/unit_test.hpp>

#include <livre/lib/configuration/VolumeRendererParameters.h>

BOOST_AUTO_TEST_CASE(defaultValues)
{
    const livre::VolumeRendererParameters params;

    BOOST_CHECK_EQUAL(params.getMaxLod(), (livre::NODEID_LEVEL_BITS << 1) + 1);
    BOOST_CHECK_EQUAL(params.getMinLod(), 0);
    BOOST_CHECK(!params.getSynchronousMode());
    BOOST_CHECK_EQUAL(params.getSamplesPerRay(), 0);
    BOOST_CHECK(!params.getShowAxes());

#ifdef __i386__
    BOOST_CHECK_EQUAL(params.getScreenSpaceError(), 8.0f);
    BOOST_CHECK_EQUAL(params.getMaxGpuCacheMemory(), 384u);
    BOOST_CHECK_EQUAL(params.getMaxCpuCacheMemory(), 768u);
#else
    BOOST_CHECK_EQUAL(params.getScreenSpaceError(), 4.0f);
    BOOST_CHECK_EQUAL(params.getMaxGpuCacheMemory(), 3072u);
    BOOST_CHECK_EQUAL(params.getMaxCpuCacheMemory(), 8192u);
#endif
}

BOOST_AUTO_TEST_CASE(copy)
{
    livre::VolumeRendererParameters params;
    params.setMaxLod(42);

    const livre::VolumeRendererParameters paramsCopy(params);
    BOOST_CHECK_EQUAL(paramsCopy.getMaxLod(), params.getMaxLod());
    BOOST_CHECK(paramsCopy == params);

    livre::VolumeRendererParameters paramsAssigned;
    paramsAssigned = params;
    BOOST_CHECK_EQUAL(paramsAssigned.getMaxLod(), paramsAssigned.getMaxLod());
    BOOST_CHECK(paramsAssigned == params);
}

BOOST_AUTO_TEST_CASE(initFromArgv)
{
    const char* app = boost::unit_test::framework::master_test_suite().argv[0];
    const char* argv[] = {app,
                          "--sse",
                          "1.4",
                          "--synchronous",
                          "--gpu-cache-mem",
                          "12345",
                          "--cpu-cache-mem",
                          "54321",
                          "--min-lod",
                          "2",
                          "--max-lod",
                          "6",
                          "--samples-per-ray",
                          "42"};
    const int argc = sizeof(argv) / sizeof(char*);

    livre::VolumeRendererParameters params(argc, argv);

    BOOST_CHECK_EQUAL(params.getMaxLod(), 6);
    BOOST_CHECK_EQUAL(params.getMinLod(), 2);
    BOOST_CHECK(params.getSynchronousMode());
    BOOST_CHECK_EQUAL(params.getSamplesPerRay(), 42);
    BOOST_CHECK_EQUAL(params.getScreenSpaceError(), 1.4f);
    BOOST_CHECK_EQUAL(params.getMaxGpuCacheMemory(), 12345u);
    BOOST_CHECK_EQUAL(params.getMaxCpuCacheMemory(), 54321u);
}
