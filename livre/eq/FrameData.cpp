/**
 * Copyright (c) BBP/EPFL 2011-2016 Ahmet.Bilgili@epfl.ch
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

#include <eq/eq.h>

#include <livre/eq/FrameData.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/VolumeSettings.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

namespace livre
{

/**
 * magicNumber To initialize the co::Objects with a randomized number.
 */
const uint64_t magicNumber = 18382873472383;

/**
 * The ObjectFactory class is a class to generate objects for the internal \see co::ObjectMap in \see InitData.
 */
class ObjectFactory : public co::ObjectFactory
{
public:

    lunchbox::uint128_t generateID( const uint32_t type ) const
    {
         return lunchbox::uint128_t( magicNumber, type );
    }

    virtual co::Object* createObject( const uint32_t type )
    {
        co::Object *object = 0;
        switch( type )
        {
        case SMI_FRAME_SETTINGS:
            object = new FrameSettings();
            break;
        case SMI_RENDER_SETTINGS:
            object = new RenderSettings();
            break;
        case SMI_CAMERA_SETTINGS:
            object = new CameraSettings();
            break;
        case SMI_VOLUME_SETTINGS:
            object = new VolumeSettings();
            break;
        case SMI_VR_PARAMETERS:
            object = new VolumeRendererParameters();
            break;
        }

        if( object )
        {
            object->setID( generateID( type ) );
        }

        return object;
    }
};

typedef std::shared_ptr< ObjectFactory > ObjectFactoryPtr;

template< class T >
bool mapObject( CoObjectMapPtr objectMapPtr,
                CoObjectFactoryPtr objectFactoryPtr,
                boost::shared_ptr< T > objectPtr,
                const uint32_t type )
{
    ObjectFactoryPtr objectFactory =  std::static_pointer_cast< ObjectFactory >( objectFactoryPtr );
    LBVERB << "Mapping object : " << objectPtr->getID() << std::endl;
    return objectMapPtr->map( objectFactory->generateID( type ), objectPtr.get() );
}

template< class T >
bool registerObject( CoObjectMapPtr objectMapPtr,
                     boost::shared_ptr< T > objectPtr,
                     const uint32_t type )
{
    LBVERB << "Registering object : " << objectPtr->getID() << std::endl;
    return objectMapPtr->register_( objectPtr.get(), type );
}

template< class T >
bool deregisterObject( CoObjectMapPtr objectMapPtr,
                       boost::shared_ptr< T > objectPtr )
{
    LBVERB << "Deregistering object : " << objectPtr->getID() << std::endl;
    return objectMapPtr->deregister( objectPtr.get() );
}

FrameData::FrameData( )
    : _objectFactory( new ObjectFactory )
    , _frameSettings( static_cast< FrameSettings* >
                      ( _objectFactory->createObject( SMI_FRAME_SETTINGS )))
    , _renderSettings( static_cast< RenderSettings* >
                       ( _objectFactory->createObject( SMI_RENDER_SETTINGS )))
    , _cameraSettings( static_cast< CameraSettings* >
                       ( _objectFactory->createObject( SMI_CAMERA_SETTINGS )))
    , _volumeSettings( static_cast< VolumeSettings* >
                       ( _objectFactory->createObject( SMI_VOLUME_SETTINGS )))
    , _vrParameters( static_cast< VolumeRendererParameters* >
                     ( _objectFactory->createObject( SMI_VR_PARAMETERS )))
{
}

void FrameData::registerObjects( )
{
    LBASSERT( _objectMap );
    LBCHECK( registerObject( _objectMap, _frameSettings, SMI_FRAME_SETTINGS ));
    LBCHECK( registerObject( _objectMap, _renderSettings, SMI_RENDER_SETTINGS ));
    LBCHECK( registerObject( _objectMap, _cameraSettings, SMI_CAMERA_SETTINGS ));
    LBCHECK( registerObject( _objectMap, _volumeSettings, SMI_VOLUME_SETTINGS ));
    LBCHECK( registerObject( _objectMap, _vrParameters, SMI_VR_PARAMETERS ));
}

void FrameData::deregisterObjects( )
{
    LBASSERT( _objectMap );
    LBCHECK( deregisterObject( _objectMap, _frameSettings ));
    LBCHECK( deregisterObject( _objectMap, _renderSettings ));
    LBCHECK( deregisterObject( _objectMap, _cameraSettings ));
    LBCHECK( deregisterObject( _objectMap, _volumeSettings ));
    LBCHECK( deregisterObject( _objectMap, _vrParameters ));
}

void FrameData::mapObjects( )
{
    LBASSERT( _objectMap );
    LBCHECK( mapObject( _objectMap, _objectFactory, _frameSettings,
                        SMI_FRAME_SETTINGS ));
    LBCHECK( mapObject( _objectMap, _objectFactory, _renderSettings,
                        SMI_RENDER_SETTINGS ));
    LBCHECK( mapObject( _objectMap, _objectFactory, _cameraSettings,
                        SMI_CAMERA_SETTINGS ));
    LBCHECK( mapObject( _objectMap, _objectFactory, _volumeSettings,
                        SMI_VOLUME_SETTINGS ));
    LBCHECK( mapObject( _objectMap, _objectFactory, _vrParameters,
                        SMI_VR_PARAMETERS ));
}

void FrameData::unmapObjects( )
{
    if( !_objectMap )
        return;
    LBCHECK( _objectMap->unmap( _frameSettings.get( )));
    LBCHECK( _objectMap->unmap( _renderSettings.get( )));
    LBCHECK( _objectMap->unmap( _cameraSettings.get( )));
    LBCHECK( _objectMap->unmap( _volumeSettings.get( )));
    LBCHECK( _objectMap->unmap( _vrParameters.get( )));
    _objectMap->clear();
}

void FrameData::initialize( eq::Config* eqConfig )
{
    LBASSERT( !_objectMap );
    _objectMap.reset( new co::ObjectMap( *eqConfig, *_objectFactory ) );
}

void FrameData::setup( const VolumeRendererParameters& rendererParams )
{
    *_vrParameters = rendererParams;
}

FrameSettingsPtr FrameData::getFrameSettings()
{
    return _frameSettings;
}

ConstFrameSettingsPtr FrameData::getFrameSettings() const
{
    return _frameSettings;
}

RenderSettingsPtr FrameData::getRenderSettings()
{
    return _renderSettings;
}

ConstRenderSettingsPtr FrameData::getRenderSettings() const
{
    return _renderSettings;
}

ConstCameraSettingsPtr FrameData::getCameraSettings() const
{
    return _cameraSettings;
}

CameraSettingsPtr FrameData::getCameraSettings()
{
    return _cameraSettings;
}

VolumeSettingsPtr FrameData::getVolumeSettings()
{
    return _volumeSettings;
}

ConstVolumeRendererParametersPtr FrameData::getVRParameters() const
{
    return _vrParameters;
}

VolumeRendererParametersPtr FrameData::getVRParameters()
{
    return _vrParameters;
}

const eq::uint128_t& FrameData::getID() const
{
    return _objectMap->getID();
}

eq::uint128_t FrameData::commit()
{
    return _objectMap->commit( );
}

eq::uint128_t FrameData::sync( const eq::uint128_t& version )
{
    LBASSERT( _objectMap );
    return _objectMap->sync( version );
}

bool FrameData::map( eq::Config* config, const eq::uint128_t& uuid )
{
    LBVERB << "Mapping object map to UUID : " << uuid << std::endl;
    return config->mapObject( _objectMap.get(), uuid );
}

void FrameData::unmap( eq::Config* config )
{
    if( _objectMap )
        config->unmapObject( _objectMap.get() );
}

bool FrameData::registerToConfig_( eq::Config* config )
{
    LBVERB << "Registering object map UUID : " << _objectMap->getID() << std::endl;
    return config->registerObject( _objectMap.get() );
}

bool FrameData::deregisterFromConfig_( eq::Config *config )
{
    LBVERB << "Deregistering object map UUID : " << _objectMap->getID() << std::endl;
    config->deregisterObject( _objectMap.get() );
    return true;
}

FrameData::~FrameData()
{}

}
