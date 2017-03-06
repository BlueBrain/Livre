/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include <livre/core/cache/Cache.h>
#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/HistogramObject.h>

#include <livre/core/cache/CacheStatistics.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/NodeId.h>
#include <livre/core/data/VolumeInformation.h>

#define BOOST_TEST_MODULE Cache
#include <boost/test/unit_test.hpp>

const uint32_t BLOCK_SIZE = 32;
const uint32_t VOXEL_SIZE_X = 1024;
const uint32_t VOXEL_SIZE_Y = 1024;
const uint32_t VOXEL_SIZE_Z = 512;

BOOST_AUTO_TEST_CASE(testCache)
{
    std::stringstream volumeName;
    volumeName << "mem://#" << VOXEL_SIZE_X << "," << VOXEL_SIZE_Y << ","
               << VOXEL_SIZE_Z << "," << BLOCK_SIZE;

    const lunchbox::URI uri(volumeName.str());
    livre::DataSource source(uri);
    const livre::VolumeInformation& info = source.getVolumeInfo();

    const uint32_t level = 0;
    const livre::Vector3f position(0, 0, 0);
    const uint32_t frame = 0;
    const livre::NodeId parentNodeId(level, position, frame);
    const livre::NodeId firstChildNodeId = parentNodeId.getChildren().front();

    const livre::LODNode& lodNode = source.getNode(firstChildNodeId);

    const livre::Vector3ui blockSize =
        lodNode.getBlockSize() + livre::Vector3ui(info.overlap) * 2;

    // Read data manually
    livre::MemoryUnitPtr memUnit = source.getData(firstChildNodeId);
    const size_t allocSize =
        blockSize.product() * info.compCount * info.getBytesPerVoxel();

    // Read same data with the data cache
    const size_t maxMemory = 2048;
    livre::CacheT<livre::DataObject> dataCache("DataCache", maxMemory);

    livre::ConstCacheObjectPtr constData =
        dataCache.get(firstChildNodeId.getId());
    BOOST_CHECK(dataCache.getCount() == 0);
    BOOST_CHECK(!constData);

    livre::ConstCacheObjectPtr dataCacheObject =
        dataCache.load<livre::DataObject>(firstChildNodeId.getId(), source);
    BOOST_CHECK(dataCacheObject);

    constData = dataCache.get(livre::INVALID_CACHE_ID);
    BOOST_CHECK(dataCache.getCount() == 1);
    BOOST_CHECK(constData.get() == 0);

    // get() on cache returns already loaded data
    constData = dataCache.get(firstChildNodeId.getId());
    BOOST_CHECK(constData);
    BOOST_CHECK(dataCache.getCount() == 1);

    livre::ConstDataObjectPtr dataObject =
        dataCache.get<livre::DataObject>(firstChildNodeId.getId());

    BOOST_CHECK(dataObject);
    BOOST_CHECK(dataObject->getSize() == allocSize);
    BOOST_CHECK(dataCache.getStatistics().getUsedMemory() == allocSize);

    BOOST_CHECK_THROW(dataCache.get<livre::HistogramObject>(
                          firstChildNodeId.getId()),
                      std::runtime_error);

    const uint8_t* manual = memUnit->getData<const uint8_t>();
    const uint8_t* cached =
        static_cast<const uint8_t*>(dataObject->getDataPtr());

    BOOST_CHECK_EQUAL_COLLECTIONS(manual, manual + allocSize, cached,
                                  cached + allocSize);

    livre::CacheT<livre::HistogramObject> histogramCache("HistogramCache",
                                                         1024);
    livre::ConstCacheObjectPtr histCacheObject =
        histogramCache.load<livre::HistogramObject>(firstChildNodeId.getId(),
                                                    dataCache, source,
                                                    livre::Vector2f(0.0f,
                                                                    255.f));
    BOOST_CHECK(histCacheObject);

    livre::ConstHistogramObjectPtr histObject =
        histogramCache.get<livre::HistogramObject>(firstChildNodeId.getId());

    const livre::Histogram& histogram = histObject->getHistogram();
    const uint64_t* bins = histogram.getBins().data();

    BOOST_CHECK_EQUAL(histogram.getMinIndex(), 0);
    BOOST_CHECK_EQUAL(histogram.getMaxIndex(), 17);
    BOOST_CHECK_EQUAL(bins[size_t(histogram.getMaxIndex())], 1u << 24);
    BOOST_CHECK_EQUAL(histogram.getSum(), 1u << 24);
    BOOST_CHECK(!histogram.isEmpty());

    livre::Histogram hist;
    BOOST_CHECK(hist.isEmpty());
    BOOST_CHECK_EQUAL(hist.getSum(), 0);
    hist = histogram;
    hist += histogram;
    const uint64_t* binAcc = hist.getBins().data();
    BOOST_CHECK_EQUAL(binAcc[size_t(hist.getMaxIndex())], 1u << 25);
    BOOST_CHECK_EQUAL(hist.getSum(), 1u << 25);
}
