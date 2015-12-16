/**
 * Copyright (c) BBP/EPFL 2011-2015 Ahmet.Bilgili@epfl.ch
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
    : objectFactoryPtr_( new ObjectFactory() )
    , frameSettingsPtr_( static_cast< FrameSettings* >
                       ( objectFactoryPtr_->createObject( SMI_FRAME_SETTINGS ) ) )
    , renderSettingsPtr_( static_cast< RenderSettings* >
                        ( objectFactoryPtr_->createObject( SMI_RENDER_SETTINGS ) ) )
    , cameraSettingsPtr_( static_cast< CameraSettings* >
                        ( objectFactoryPtr_->createObject( SMI_CAMERA_SETTINGS ) ) )
    , volumeSettingsPtr_( static_cast< VolumeSettings* >
                        ( objectFactoryPtr_->createObject( SMI_VOLUME_SETTINGS ) ) )
    , vrParametersPtr_( static_cast< VolumeRendererParameters* >
                      ( objectFactoryPtr_->createObject( SMI_VR_PARAMETERS ) ) )
{
}

void FrameData::registerObjects( )
{
    LBASSERT( objectMapPtr_ );
    LBCHECK( registerObject( objectMapPtr_, frameSettingsPtr_, SMI_FRAME_SETTINGS ) );
    LBCHECK( registerObject( objectMapPtr_, renderSettingsPtr_, SMI_RENDER_SETTINGS ) );
    LBCHECK( registerObject( objectMapPtr_, cameraSettingsPtr_, SMI_CAMERA_SETTINGS ) );
    LBCHECK( registerObject( objectMapPtr_, volumeSettingsPtr_, SMI_VOLUME_SETTINGS ) );
    LBCHECK( registerObject( objectMapPtr_, vrParametersPtr_, SMI_VR_PARAMETERS ) );
}

void FrameData::deregisterObjects( )
{
    LBASSERT( objectMapPtr_ );
    LBCHECK( deregisterObject( objectMapPtr_, frameSettingsPtr_ ) );
    LBCHECK( deregisterObject( objectMapPtr_, renderSettingsPtr_ ) );
    LBCHECK( deregisterObject( objectMapPtr_, cameraSettingsPtr_ ) );
    LBCHECK( deregisterObject( objectMapPtr_, volumeSettingsPtr_ ) );
    LBCHECK( deregisterObject( objectMapPtr_, vrParametersPtr_ ) );
}

void FrameData::mapObjects( )
{
    LBASSERT( objectMapPtr_ );
    LBCHECK( mapObject( objectMapPtr_,
                        objectFactoryPtr_,
                        frameSettingsPtr_,
                        SMI_FRAME_SETTINGS ) );

    LBCHECK( mapObject( objectMapPtr_,
                        objectFactoryPtr_,
                        renderSettingsPtr_,
                        SMI_RENDER_SETTINGS ) );

    LBCHECK( mapObject( objectMapPtr_,
                        objectFactoryPtr_,
                        cameraSettingsPtr_,
                        SMI_CAMERA_SETTINGS ) );

    LBCHECK( mapObject( objectMapPtr_,
                        objectFactoryPtr_,
                        volumeSettingsPtr_,
                        SMI_VOLUME_SETTINGS ) );

    LBCHECK( mapObject( objectMapPtr_,
                        objectFactoryPtr_,
                        vrParametersPtr_,
                        SMI_VR_PARAMETERS ) );
}

void FrameData::unmapObjects( )
{
    if( !objectMapPtr_ )
        return;
    LBCHECK( objectMapPtr_->unmap( frameSettingsPtr_.get() ) );
    LBCHECK( objectMapPtr_->unmap( renderSettingsPtr_.get() ) );
    LBCHECK( objectMapPtr_->unmap( cameraSettingsPtr_.get() ) );
    LBCHECK( objectMapPtr_->unmap( volumeSettingsPtr_.get() ) );
    LBCHECK( objectMapPtr_->unmap( vrParametersPtr_.get() ) );
    objectMapPtr_->clear();
}

void FrameData::initialize( eq::Config* eqConfig )
{
    LBASSERT( !objectMapPtr_ );
    objectMapPtr_.reset( new co::ObjectMap( *eqConfig, *objectFactoryPtr_ ) );
}

void FrameData::setup( const VolumeRendererParameters& rendererParams )
{
    *vrParametersPtr_ = rendererParams;
}

FrameSettingsPtr FrameData::getFrameSettings() const
{
    return frameSettingsPtr_;
}

RenderSettingsPtr FrameData::getRenderSettings() const
{
    return renderSettingsPtr_;
}

CameraSettingsPtr FrameData::getCameraSettings() const
{
    return cameraSettingsPtr_;
}

VolumeSettingsPtr FrameData::getVolumeSettings() const
{
    return volumeSettingsPtr_;
}

ConstVolumeRendererParametersPtr FrameData::getVRParameters() const
{
    return vrParametersPtr_;
}

VolumeRendererParametersPtr FrameData::getVRParameters()
{
    return vrParametersPtr_;
}

const eq::uint128_t& FrameData::getID() const
{
    return objectMapPtr_->getID();
}

eq::uint128_t FrameData::commit()
{
    return objectMapPtr_->commit( );
}

eq::uint128_t FrameData::sync( const eq::uint128_t& version )
{
    LBASSERT( objectMapPtr_ );
    return objectMapPtr_->sync( version );
}

bool FrameData::map( eq::Config* config, const eq::uint128_t& uuid )
{
    LBVERB << "Mapping object map to UUID : " << uuid << std::endl;
    return config->mapObject( objectMapPtr_.get(), uuid );
}

void FrameData::unmap( eq::Config* config )
{
    if( objectMapPtr_ )
        config->unmapObject( objectMapPtr_.get() );
}

bool FrameData::registerToConfig_( eq::Config* config )
{
    LBVERB << "Registering object map UUID : " << objectMapPtr_->getID() << std::endl;
    return config->registerObject( objectMapPtr_.get() );
}

bool FrameData::deregisterFromConfig_( eq::Config *config )
{
    LBVERB << "Deregistering object map UUID : " << objectMapPtr_->getID() << std::endl;
    config->deregisterObject( objectMapPtr_.get() );
    return true;
}

FrameData::~FrameData()
{}

}
