/**
 * Copyright (c) BBP/EPFL 2011-2017 Ahmet.Bilgili@epfl.ch
 *                                  Stefan.Eilemann@epfl.ch
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

#include <livre/eq/FrameData.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/eq/settings/VolumeSettings.h>
#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <eq/eq.h>

namespace livre
{
class FrameData::Impl : public co::ObjectMap
{
public:
    Impl(co::ObjectHandler& handler, co::ObjectFactory& factory)
        : co::ObjectMap(handler, factory)
    {
        frameSettings.setID(servus::make_uint128("livre::FrameSettings"));
        cameraSettings.setID(servus::make_uint128("livre::CameraSettings"));
        volumeSettings.setID(servus::make_uint128("livre::VolumeSettings"));
        vrParameters.setID(
            servus::make_uint128("livre::VolumeRendererParameters"));
        renderSettings.getClipPlanes().setID(
            servus::make_uint128("livre::ClipPlanes"));
        renderSettings.getTransferFunction().setID(
            servus::make_uint128("livre::TransferFunction"));
    }

    FrameSettings frameSettings;
    RenderSettings renderSettings;
    CameraSettings cameraSettings;
    VolumeSettings volumeSettings;
    VolumeRendererParameters vrParameters;
    ApplicationParameters applicationParameters;
};

FrameData::FrameData()
{
}

void FrameData::registerObjects()
{
    LBCHECK(_impl->register_(&_impl->frameSettings, co::OBJECTTYPE_CUSTOM));
    LBCHECK(_impl->register_(&_impl->cameraSettings, co::OBJECTTYPE_CUSTOM));
    LBCHECK(_impl->register_(&_impl->volumeSettings, co::OBJECTTYPE_CUSTOM));
    LBCHECK(_impl->register_(&_impl->vrParameters, co::OBJECTTYPE_CUSTOM));
    LBCHECK(_impl->register_(&_impl->renderSettings.getClipPlanes(),
                             co::OBJECTTYPE_CUSTOM));
    LBCHECK(_impl->register_(&_impl->renderSettings.getTransferFunction(),
                             co::OBJECTTYPE_CUSTOM));
}

void FrameData::deregisterObjects()
{
    LBCHECK(_impl->deregister(&_impl->frameSettings));
    LBCHECK(_impl->deregister(&_impl->cameraSettings));
    LBCHECK(_impl->deregister(&_impl->volumeSettings));
    LBCHECK(_impl->deregister(&_impl->vrParameters));
    LBCHECK(_impl->deregister(&_impl->renderSettings.getClipPlanes()));
    LBCHECK(_impl->deregister(&_impl->renderSettings.getTransferFunction()));
}

void FrameData::mapObjects()
{
    LBCHECK(_impl->map(_impl->frameSettings.getID(), &_impl->frameSettings));
    LBCHECK(_impl->map(_impl->cameraSettings.getID(), &_impl->cameraSettings));
    LBCHECK(_impl->map(_impl->volumeSettings.getID(), &_impl->volumeSettings));
    LBCHECK(_impl->map(_impl->vrParameters.getID(), &_impl->vrParameters));
    LBCHECK(_impl->map(_impl->renderSettings.getClipPlanes().getID(),
                       &_impl->renderSettings.getClipPlanes()));
    LBCHECK(_impl->map(_impl->renderSettings.getTransferFunction().getID(),
                       &_impl->renderSettings.getTransferFunction()));
}

void FrameData::unmapObjects()
{
    if (!_impl)
        return;

    LBCHECK(_impl->unmap(&_impl->frameSettings));
    LBCHECK(_impl->unmap(&_impl->cameraSettings));
    LBCHECK(_impl->unmap(&_impl->volumeSettings));
    LBCHECK(_impl->unmap(&_impl->vrParameters));
    LBCHECK(_impl->unmap(&_impl->renderSettings.getClipPlanes()));
    LBCHECK(_impl->unmap(&_impl->renderSettings.getTransferFunction()));
    _impl->clear();
}

void FrameData::initialize(eq::Config* eqConfig)
{
    LBASSERT(!_impl);
    _impl.reset(new Impl(*eqConfig, _factory));
}

void FrameData::setup(const ApplicationParameters& appParams,
                      const VolumeRendererParameters& rendererParams)
{
    _impl->applicationParameters = appParams;
    _impl->vrParameters = rendererParams;
    getVolumeSettings().setURI(_impl->applicationParameters.dataFileName);
}

FrameSettings& FrameData::getFrameSettings()
{
    return _impl->frameSettings;
}

const FrameSettings& FrameData::getFrameSettings() const
{
    return _impl->frameSettings;
}

RenderSettings& FrameData::getRenderSettings()
{
    return _impl->renderSettings;
}

const RenderSettings& FrameData::getRenderSettings() const
{
    return _impl->renderSettings;
}

const CameraSettings& FrameData::getCameraSettings() const
{
    return _impl->cameraSettings;
}

CameraSettings& FrameData::getCameraSettings()
{
    return _impl->cameraSettings;
}

VolumeSettings& FrameData::getVolumeSettings()
{
    return _impl->volumeSettings;
}

const VolumeSettings& FrameData::getVolumeSettings() const
{
    return _impl->volumeSettings;
}

const VolumeRendererParameters& FrameData::getVRParameters() const
{
    return _impl->vrParameters;
}

VolumeRendererParameters& FrameData::getVRParameters()
{
    return _impl->vrParameters;
}

const ApplicationParameters& FrameData::getApplicationParameters() const
{
    return _impl->applicationParameters;
}

ApplicationParameters& FrameData::getApplicationParameters()
{
    return _impl->applicationParameters;
}

const eq::uint128_t& FrameData::getID() const
{
    return _impl->getID();
}

eq::uint128_t FrameData::commit()
{
    return _impl->commit();
}

eq::uint128_t FrameData::sync(const eq::uint128_t& version)
{
    return _impl->sync(version);
}

bool FrameData::map(eq::Config* config, const eq::uint128_t& uuid)
{
    return config->mapObject(_impl.get(), uuid);
}

void FrameData::unmap(eq::Config* config)
{
    if (_impl)
        config->unmapObject(_impl.get());
}

bool FrameData::registerToConfig(eq::Config* config)
{
    return config->registerObject(_impl.get());
}

bool FrameData::deregisterFromConfig(eq::Config* config)
{
    config->deregisterObject(_impl.get());
    return true;
}

FrameData::~FrameData()
{
}
}
