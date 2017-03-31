
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project
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

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/lib/data/RawDataSource.h>

#include <lunchbox/memoryMap.h>
#include <lunchbox/pluginRegisterer.h>

#include "nrrd/nrrd.hxx"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace livre
{
namespace
{
lunchbox::PluginRegisterer<RawDataSource> registerer;

template <class I, class O>
void _scale(
    const I* in, O* out, const ssize_t nElems,
    typename std::enable_if<(sizeof(I) > sizeof(O)), void>::type* = nullptr)
{
    constexpr size_t shift = (sizeof(I) - sizeof(O)) * 8;
#pragma omp parallel for
    for (ssize_t i = 0; i < nElems; ++i)
        out[i] = in[i] >> shift;
}

template <class I, class O>
MemoryUnitPtr _scale(const uint8_t* ptr, const size_t size)
{
    const ssize_t nElems = size / sizeof(O);
    auto memory = MemoryUnitPtr(new AllocMemoryUnit(size));
    const I* in = reinterpret_cast<const I*>(ptr);
    O* out = memory->getData<O>();

    _scale(in, out, nElems);
    return memory;
}
}

using boost::lexical_cast;

struct RawDataSource::Impl
{
    Impl(const DataSourcePluginData& initData, VolumeInformation& volInfo)
        : _headerSize(0)
        , _inputType(DT_UINT8)
        , _outputType(DT_UINT8)
    {
        const servus::URI& uri = initData.getURI();
        const std::string& path = uri.getPath();
        const bool isExtensionRaw = boost::algorithm::ends_with(path, ".raw") ||
                                    boost::algorithm::ends_with(path, ".img");
        const bool isExtensionNrrd = boost::algorithm::ends_with(path, ".nrrd");

        if (!isExtensionRaw && !isExtensionNrrd)
            LBTHROW(std::runtime_error(
                "Volume extension does not include raw or nrrd"));

        if (isExtensionRaw)
            parseRawData(uri.getPath(), volInfo, uri.getFragment());
        else if (isExtensionNrrd)
            parseNRRDData(uri.getPath(), volInfo);

        volInfo.frameRange = Vector2ui(0u, 1u);
        volInfo.compCount = 1;
        volInfo.worldSpacePerVoxel = 1.0f / float(volInfo.voxels.find_max());
        volInfo.worldSize =
            Vector3f(volInfo.voxels[0], volInfo.voxels[1], volInfo.voxels[2]) *
            volInfo.worldSpacePerVoxel;

        volInfo.overlap = Vector3ui(0u);
        volInfo.rootNode = RootNode(1, Vector3ui(1));
        volInfo.maximumBlockSize = volInfo.voxels;

        _inputType = volInfo.dataType;
        const auto output = uri.findQuery("output");
        if (output == uri.queryEnd())
            _outputType = _inputType;
        else
        {
            _outputType = getDataType(output->second);
            volInfo.dataType = _outputType;
        }
    }

    ~Impl() {}
    MemoryUnitPtr getData(const LODNode& node)
    {
        const size_t size = node.getBlockSize().product();
        const uint8_t* ptr = _mmap.getAddress<uint8_t>() + _headerSize;
        if (_inputType == _outputType)
            return MemoryUnitPtr(new ConstMemoryUnit(ptr, size));

        // only unsigned integer conversions are supported!
        if (_inputType == DT_UINT16 && _outputType == DT_UINT8)
            return _scale<uint16_t, uint8_t>(ptr, size);
        if (_inputType == DT_UINT32 && _outputType == DT_UINT8)
            return _scale<uint32_t, uint8_t>(ptr, size);
        if (_inputType == DT_UINT32 && _outputType == DT_UINT16)
            return _scale<uint32_t, uint16_t>(ptr, size);

        LBTHROW(std::runtime_error("Unsupported data conversion"));
    }

    DataType getDataType(const std::string& dataType)
    {
        if (dataType == "char" || dataType == "int8")
            return DT_INT8;
        if (dataType == "unsigned char" || dataType == "uint8")
            return DT_UINT8;
        if (dataType == "short" || dataType == "int16")
            return DT_INT16;
        if (dataType == "unsigned short" || dataType == "uint16")
            return DT_UINT16;
        if (dataType == "int" || dataType == "int32")
            return DT_INT32;
        if (dataType == "unsigned int" || dataType == "uint32")
            return DT_UINT32;
        if (dataType == "float")
            return DT_FLOAT;
        LBTHROW(std::runtime_error("Unsupported data format " + dataType));
    }

    void parseRawData(const std::string& filename, VolumeInformation& volInfo,
                      const std::string& fragment)
    {
        if (!_mmap.map(filename))
            LBTHROW(std::runtime_error("Cannot mmap file"));

        std::vector<std::string> parameters;
        boost::algorithm::split(parameters, fragment, boost::is_any_of(","));

        if (parameters.size() < 3) // use defaults
        {
            LBTHROW(
                std::runtime_error("Not enough parameters for the raw file"));
        }
        else
        {
            try
            {
                volInfo.voxels[0] = lexical_cast<uint32_t>(parameters[0]);
                volInfo.voxels[1] = lexical_cast<uint32_t>(parameters[1]);
                volInfo.voxels[2] = lexical_cast<uint32_t>(parameters[2]);
                if (parameters.size() > 3)
                    volInfo.dataType = getDataType(parameters[3]);
                else
                    volInfo.dataType = DT_UINT8;
            }
            catch (boost::bad_lexical_cast& except)
            {
                LBTHROW(std::runtime_error(except.what()));
            }
        }
    }

    void parseNRRDData(const std::string& filename, VolumeInformation& volInfo)
    {
        std::map<std::string, std::string> dataInfo;
        _headerSize = ::NRRD::parseHeader(filename, dataInfo);
        if (_headerSize == 0)
            LBTHROW(std::runtime_error("Cannot parse nrrd file"));

        std::string dataFile = filename;
        if (dataInfo.count("datafile") > 0)
        {
            boost::filesystem::path dataFilePath =
                boost::filesystem::path(filename).parent_path();
            dataFilePath /= dataInfo["datafile"];
            dataFile = dataFilePath.string();
        }

        if (!_mmap.map(dataFile))
            LBTHROW(std::runtime_error("Cannot mmap file"));

        volInfo.dataType = getDataType(dataInfo["type"]);

        try
        {
            if (lexical_cast<size_t>(dataInfo["dimension"]) != 3u)
                LBTHROW(std::runtime_error(
                    std::runtime_error("NRRD is not 3D data")))
        }
        catch (boost::bad_lexical_cast& except)
        {
            LBTHROW(std::runtime_error(except.what()));
        }

        const auto& vec = stringToVector<int>(dataInfo["sizes"]);
        volInfo.voxels[0] = vec[0];
        volInfo.voxels[1] = vec[1];
        volInfo.voxels[2] = vec[2];
        volInfo.bigEndian = dataInfo["endian"] == "big";
    }

    lunchbox::MemoryMap _mmap;
    size_t _headerSize;
    DataType _inputType;
    DataType _outputType;
};

RawDataSource::RawDataSource(const DataSourcePluginData& initData)
    : _impl(new RawDataSource::Impl(initData, _volumeInfo))
{
}

RawDataSource::~RawDataSource()
{
}

MemoryUnitPtr RawDataSource::getData(const LODNode& node)
{
    return _impl->getData(node);
}

bool RawDataSource::handles(const DataSourcePluginData& initData)
{
    const servus::URI& uri = initData.getURI();
    if (uri.getScheme() == "raw")
        return true;

    if (!uri.getScheme().empty())
        return false;

    return boost::algorithm::ends_with(uri.getPath(), ".raw") ||
           boost::algorithm::ends_with(uri.getPath(), ".img") ||
           boost::algorithm::ends_with(uri.getPath(), ".nrrd");
}

std::string RawDataSource::getDescription()
{
    return R"(Raw volume: [raw://]/filename.[raw|img|nrrd](?output=format)#1024,1024,1024(,input format)
  with formats being one of: char, int8, unsigned char, uint8, short, int16, unsigned short, uint16, int, int32, unsigned int, uint32, float
  The default input format is uint8, the default output format is the input
  format.)";
}
}
