/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/HistogramObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/DataSource.h>

namespace livre
{
namespace
{
template <class SRC_TYPE>
void binDataSlow(const SRC_TYPE* rawData, Histogram& histogram,
                 const Vector3ui& blockSize, const Vector3ui& padding,
                 const uint64_t scaleFactor)
{
    std::map<SRC_TYPE, size_t> values;
    const Vector3ui dataBlockSize = blockSize + padding * 2;
    for (size_t i = padding.x(); i < dataBlockSize.x() - padding.x(); ++i)
        for (size_t j = padding.y(); j < dataBlockSize.y() - padding.y(); ++j)
            for (size_t k = padding.z(); k < dataBlockSize.z() - padding.z();
                 ++k)
            {
                const size_t index = i * dataBlockSize.y() * dataBlockSize.z() +
                                     j * dataBlockSize.z() + k;
                const SRC_TYPE data = rawData[index];
                ++values[data];
            }

    const float minVal =
        std::min(float(values.begin()->first), histogram.getMin());
    const float maxVal =
        std::max(float(values.rbegin()->first), histogram.getMax());
    const float range = maxVal - minVal;

    histogram.setMin(minVal);
    histogram.setMax(maxVal);

    if (range == 0.0f)
    {
        histogram.getBins().clear();
        const size_t bins = blockSize.product() * scaleFactor;
        histogram.getBins().push_back(bins);
        return;
    }

    const size_t binCount = histogram.getBins().size();
    uint64_t* dstData = histogram.getBins().data();
    const size_t perBinCount = std::ceil(range / (binCount - 1));
    for (const auto& value : values)
    {
        const size_t binIndex = (value.first - minVal) / perBinCount;
        dstData[binIndex] += (scaleFactor * value.second);
    }
}

template <class SRC_TYPE>
void binData(const SRC_TYPE* rawData, Histogram& histogram,
             const Vector3ui& blockSize, const Vector3ui& padding,
             const uint64_t scaleFactor)
{
    if (padding != Vector3ui())
    {
        binDataSlow(rawData, histogram, blockSize, padding, scaleFactor);
        return;
    }

    SRC_TYPE minVal = histogram.getMin();
    SRC_TYPE maxVal = histogram.getMax();

    std::vector<size_t> values(std::numeric_limits<SRC_TYPE>::max() -
                               std::numeric_limits<SRC_TYPE>::min() + 1);
    const size_t numVoxels = blockSize.x() * blockSize.y() * blockSize.z();
    for (size_t i = 0; i < numVoxels; ++i)
    {
        const SRC_TYPE data = rawData[i];
        ++values[data + std::abs(std::numeric_limits<SRC_TYPE>::min())];
        minVal = std::min(data, minVal);
        maxVal = std::max(data, maxVal);
    }

    histogram.setMin(minVal);
    histogram.setMax(maxVal);
    const float range = maxVal - minVal;

    if (range == 0.0f)
    {
        histogram.getBins().clear();
        const size_t bins = blockSize.product() * scaleFactor;
        histogram.getBins().push_back(bins);
        return;
    }

    const size_t binCount = histogram.getBins().size();
    uint64_t* dstData = histogram.getBins().data();
    const size_t perBinCount = std::ceil(range / (binCount - 1));

    for (size_t i = 0; i < values.size(); ++i)
    {
        const size_t binIndex = std::lround(i / perBinCount);
        dstData[binIndex] += scaleFactor * values[i];
    }
}
}

struct HistogramObject::Impl
{
public:
    Impl(const CacheId& cacheId, const Cache& dataCache,
         const DataSource& dataSource, const Vector2f& dataSourceRange)
        : _size(0)
    {
        if (!load(cacheId, dataCache, dataSource, dataSourceRange))
            LBTHROW(CacheLoadException(
                cacheId, "Unable to construct histogram cache object"));
    }

    bool load(const CacheId& cacheId, const Cache& dataCache,
              const DataSource& dataSource, const Vector2f& dataSourceRange)
    {
        const VolumeInformation& volumeInfo = dataSource.getVolumeInfo();
        if (volumeInfo.compCount > 1)
            LBTHROW(std::runtime_error("Multiple channels are not supported "));

        ConstDataObjectPtr data = dataCache.get<DataObject>(cacheId);

        if (!data)
            return false;

        const void* rawData = data->getDataPtr();
        const LODNode& lodNode = dataSource.getNode(NodeId(cacheId));
        const Vector3ui& voxelBox = lodNode.getVoxelBox().getSize();
        const Vector3ui& padding = volumeInfo.overlap;

        const uint64_t scaleFactor1d =
            1 << (volumeInfo.rootNode.getDepth() - lodNode.getRefLevel() - 1);

        const uint64_t scaleFactor =
            scaleFactor1d * scaleFactor1d * scaleFactor1d;

        const DataType dataType = volumeInfo.dataType;
        switch (dataType)
        {
        case DT_UINT8:
            _histogram.setMin(std::numeric_limits<uint8_t>::min());
            _histogram.setMax(std::numeric_limits<uint8_t>::max());
            _histogram.resize(256);
            binData(static_cast<const uint8_t*>(rawData), _histogram, voxelBox,
                    padding, scaleFactor);
            break;
        case DT_UINT16:
            _histogram.setMin(std::numeric_limits<uint16_t>::max());
            _histogram.setMax(std::numeric_limits<uint16_t>::min());
            _histogram.resize(1024);
            binData(static_cast<const uint16_t*>(rawData), _histogram, voxelBox,
                    padding, scaleFactor);
            break;
        case DT_UINT32:
            _histogram.setMin(std::numeric_limits<uint32_t>::max());
            _histogram.setMax(std::numeric_limits<uint32_t>::min());
            _histogram.resize(4096);
            binDataSlow(static_cast<const uint32_t*>(rawData), _histogram,
                        voxelBox, padding, scaleFactor);
            break;
        case DT_INT8:
            _histogram.setMin(std::numeric_limits<int8_t>::min());
            _histogram.setMax(std::numeric_limits<int8_t>::max());
            _histogram.resize(256);
            binData(static_cast<const int8_t*>(rawData), _histogram, voxelBox,
                    padding, scaleFactor);
            break;
        case DT_INT16:
            _histogram.setMin(std::numeric_limits<int16_t>::max());
            _histogram.setMax(std::numeric_limits<int16_t>::min());
            _histogram.resize(1024);
            binData(static_cast<const int16_t*>(rawData), _histogram, voxelBox,
                    padding, scaleFactor);
            break;
        case DT_INT32:
            _histogram.setMin(std::numeric_limits<int32_t>::max());
            _histogram.setMax(std::numeric_limits<int32_t>::min());
            _histogram.resize(4096);
            binDataSlow(static_cast<const int32_t*>(rawData), _histogram,
                        voxelBox, padding, scaleFactor);
            break;
        case DT_FLOAT:
            _histogram.setMin(dataSourceRange[0]);
            _histogram.setMax(dataSourceRange[1]);
            _histogram.resize(256);
            binDataSlow(static_cast<const float*>(rawData), _histogram,
                        voxelBox, padding, scaleFactor);
            break;
        case DT_UNDEFINED:
        default:
        {
            LBTHROW(std::runtime_error("Unimplemented data type."));
        }
        }

        _size = sizeof(uint64_t) * _histogram.getBins().size();
        return true;
    }

    Histogram _histogram;
    size_t _size;
};

HistogramObject::HistogramObject(const CacheId& cacheId, const Cache& dataCache,
                                 const DataSource& dataSource,
                                 const Vector2f& dataSourceRange)
    : CacheObject(cacheId)
    , _impl(new Impl(cacheId, dataCache, dataSource, dataSourceRange))
{
}

HistogramObject::~HistogramObject()
{
}

size_t HistogramObject::getSize() const
{
    return _impl->_size;
}

const Histogram& HistogramObject::getHistogram() const
{
    return _impl->_histogram;
}
}
