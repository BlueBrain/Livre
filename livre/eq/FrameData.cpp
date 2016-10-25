/**
 * Copyright (c) BBP/EPFL 2011-2016 Ahmet.Bilgili@epfl.ch
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
#include <livre/eq/settings/EqFrameSettings.h>
#include <livre/eq/settings/EqRenderSettings.h>
#include <livre/eq/settings/EqCameraSettings.h>
#include <livre/eq/settings/EqVolumeSettings.h>
#include <livre/eq/settings/EqApplicationSettings.h>
#include <livre/core/configuration/RendererParameters.h>

#include <eq/eq.h>
#include <seq/objectType.h>


namespace livre
{
class FrameData::Impl : public co::ObjectMap
{
public:
    Impl( co::ObjectHandler& handler, co::ObjectFactory& factory )
        : co::ObjectMap( handler, factory )
    {
        frameSettings.setID( servus::make_uint128( "eq::FrameSettings" ));
        renderSettings.setID( servus::make_uint128( "eq::RenderSettings" ));
        cameraSettings.setID( servus::make_uint128( "eq::CameraSettings" ));
        volumeSettings.setID( servus::make_uint128( "eq::VolumeSettings" ));
        applicationSettings.setID( servus::make_uint128( "eq::ApplicationSettings" ));
        vrParameters.setID(
            servus::make_uint128( "eq::RendererParameters" ));
    }

    EqFrameSettings frameSettings;
    EqRenderSettings renderSettings;
    EqCameraSettings cameraSettings;
    EqVolumeSettings volumeSettings;
    EqApplicationSettings applicationSettings;
    RendererParameters vrParameters;
};

FrameData::FrameData()
{}

void FrameData::registerObjects()
{
    LBCHECK( _impl->register_( &_impl->frameSettings, seq::OBJECTTYPE_CUSTOM ));
    LBCHECK( _impl->register_( &_impl->renderSettings, seq::OBJECTTYPE_CUSTOM));
    LBCHECK( _impl->register_( &_impl->cameraSettings, seq::OBJECTTYPE_CUSTOM));
    LBCHECK( _impl->register_( &_impl->volumeSettings, seq::OBJECTTYPE_CUSTOM));
    LBCHECK( _impl->register_( &_impl->applicationSettings, seq::OBJECTTYPE_CUSTOM));
    LBCHECK( _impl->register_( &_impl->vrParameters, seq::OBJECTTYPE_CUSTOM ));
}

void FrameData::deregisterObjects()
{
    LBCHECK( _impl->deregister( &_impl->frameSettings ));
    LBCHECK( _impl->deregister( &_impl->renderSettings ));
    LBCHECK( _impl->deregister( &_impl->cameraSettings ));
    LBCHECK( _impl->deregister( &_impl->volumeSettings ));
    LBCHECK( _impl->deregister( &_impl->applicationSettings ));
    LBCHECK( _impl->deregister( &_impl->vrParameters ));
}

void FrameData::mapObjects()
{
    LBCHECK( _impl->map( _impl->frameSettings.getID(), &_impl->frameSettings ));
    LBCHECK( _impl->map( _impl->renderSettings.getID(), &_impl->renderSettings ));
    LBCHECK( _impl->map( _impl->cameraSettings.getID(), &_impl->cameraSettings ));
    LBCHECK( _impl->map( _impl->volumeSettings.getID(), &_impl->volumeSettings ));
    LBCHECK( _impl->map( _impl->applicationSettings.getID(), &_impl->applicationSettings ));
    LBCHECK( _impl->map( _impl->vrParameters.getID(), &_impl->vrParameters ));
}

void FrameData::unmapObjects()
{
    if( !_impl )
        return;

    LBCHECK( _impl->unmap( &_impl->frameSettings ));
    LBCHECK( _impl->unmap( &_impl->renderSettings ));
    LBCHECK( _impl->unmap( &_impl->cameraSettings ));
    LBCHECK( _impl->unmap( &_impl->volumeSettings ));
    LBCHECK( _impl->unmap( &_impl->applicationSettings ));
    LBCHECK( _impl->unmap( &_impl->vrParameters ));
    _impl->clear();
}

void FrameData::initialize( eq::Config* eqConfig )
{
    LBASSERT( !_impl );
    _impl.reset( new Impl( *eqConfig, _factory ));
}

void FrameData::setup( const RendererParameters& rendererParams )
{
    _impl->vrParameters = rendererParams;
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

ApplicationSettings& FrameData::getApplicationSettings()
{
    return _impl->applicationSettings;
}

const ApplicationSettings& FrameData::getApplicationSettings() const
{
    return _impl->applicationSettings;
}

const RendererParameters& FrameData::getVRParameters() const
{
    return _impl->vrParameters;
}

RendererParameters& FrameData::getVRParameters()
{
    return _impl->vrParameters;
}

const eq::uint128_t& FrameData::getID() const
{
    return _impl->getID();
}

eq::uint128_t FrameData::commit()
{
    return _impl->commit();
}

eq::uint128_t FrameData::sync( const eq::uint128_t& version )
{
    return _impl->sync( version );
}

bool FrameData::map( eq::Config* config, const eq::uint128_t& uuid )
{
    return config->mapObject( _impl.get(), uuid );
}

void FrameData::unmap( eq::Config* config )
{
    if( _impl )
        config->unmapObject( _impl.get() );
}

bool FrameData::registerToConfig( eq::Config* config )
{
    return config->registerObject( _impl.get() );
}

bool FrameData::deregisterFromConfig( eq::Config *config )
{
    config->deregisterObject( _impl.get() );
    return true;
}

FrameData::~FrameData()
{}

}
